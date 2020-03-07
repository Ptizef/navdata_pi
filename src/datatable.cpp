/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB table
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 */

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <wx/progdlg.h>
#include <wx/grid.h>
#include "navdata_pi.h"
#include <wx/tokenzr.h>

extern wxString       g_activeRouteGuid;
extern wxString       g_selectedPointGuid;
extern wxString       g_shareLocn;
extern int            g_selectedPointCol;
extern bool           g_showTripData;
extern bool           g_withSog;
extern int            g_scrollPos;
//----------------------------------------------------------------------------------------------------------
//          Data Table Implementation
//----------------------------------------------------------------------------------------------------------

DataTable::DataTable(wxWindow *parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ,navdata_pi *ppi )
    : DataTableBase(parent, id, title, pos, size, style )
{
    pPlugin = ppi;
}

DataTable::~DataTable(void)
{ 
	this->Unbind(wxEVT_SIZE, &DataTable::OnSize, this);
	m_SizeTimer.Unbind(wxEVT_TIMER, &DataTable::OnSizeTimer, this);
	delete m_pDataTable;
}

void DataTable::InitDataTable()
{
    m_pDataTable->m_pParent = this;
    g_scrollPos = 0;

    //connect events
    //dialog level
    Bind( wxEVT_SIZE, &DataTable::OnSize, this );
    Bind( wxEVT_LEFT_DOWN, &DataTable::OnMouseEvent,this );
    Bind( wxEVT_RIGHT_DOWN, &DataTable::OnMouseEvent,this );
    //wxStaticTexts level
    wxWindowListNode *node =  this->GetChildren().GetFirst();
    while( node ) {
        wxWindow *win = node->GetData();
        if( win && win->IsKindOf(CLASSINFO(wxStaticText)) ){
            win->Bind( wxEVT_LEFT_DOWN, &DataTable::OnMouseEvent,this );
            win->Bind( wxEVT_RIGHT_DOWN, &DataTable::OnMouseEvent,this );
        }
        node = node->GetNext();
    }
    //connect timers
	m_SizeTimer.Bind(wxEVT_TIMER, &DataTable::OnSizeTimer, this);

    ////init some variables
    wxFileConfig *pConf = GetOCPNConfigObject();
    if (pConf) {
        pConf->SetPath(_T("/Settings/NAVDATA"));
        pConf->Read(_T("DataTablePosition_x"), &m_dialPosition.x, 0);
        pConf->Read(_T("DataTablePosition_y"), &m_dialPosition.y, 0);
        pConf->Read(_T("NumberColVisibles"), &m_numVisCols, -1);
        pConf->Read(_T("ShowTripData"), &g_showTripData,1);
        pConf->Read(_T("ShowTTCETAatVMG"), &g_withSog,0);
    }
    //set text controls sizing trip data
    wxFont font = GetOCPNGUIScaledFont_PlugIn(_("Dialog") );
    int w;
    GetTextExtent( wxDateTime::Now().Format(_T("%b %d %Y")), &w, NULL, 0, 0, &font); // Time width text control size
    m_pStartDate->SetMinSize( wxSize(w, -1) );
    GetTextExtent( wxDateTime::Now().Format(_T("at %H:%M:%S")), &w, NULL, 0, 0, &font); // Time width text control size
    m_pStartTime->SetMinSize( wxSize(w, -1) );
    GetTextExtent( _T("0000.0NMI"), &w, NULL, 0, 0, &font); // Lenght width text control size
    m_pDistValue->SetMinSize( wxSize(w, -1) );
    GetTextExtent( _T("00h 00m"), &w, NULL, 0, 0, &font); // duration width text control size
    m_pTimeValue->SetMinSize( wxSize(w, -1) );
	//set the best size for all columns
    int h;
	if(pPlugin->GetShowMag() == 2)
		GetTextExtent(wxString(_T("300°300°(M)")), &w, &h, 0, 0, &font);
	else
		GetTextExtent( wxDateTime::Now().Format(_T("%b %d %H:%M")), &w, &h, 0, 0, &font);
    m_pDataTable->SetDefaultColSize( w, true);
    m_pDataTable->SetDefaultRowSize( h + 4, true);
    //Set scroll step X
    m_pDataTable->SetScrollLineX(w);
    //init cell attr
    m_pDataCol = new wxGridCellAttr();
    m_pDataCol->SetFont( font );
    wxColour colour;
    GetGlobalColor(_T("DILG2"), &colour);
    m_pDataCol->SetBackgroundColour( colour );
    GetGlobalColor(_T("DILG3"), &colour);
    m_pDataCol->SetTextColour( colour );
    m_pDataCol->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
    //create, populate and size rows labels
    wxString sl;
    sl.Append(_("RNG")).Append(_T("(")).Append(getUsrDistanceUnit_Plugin( pPlugin->GetDistFormat())).Append(_T(")"));
    const wxString s[] = { sl, _("BRG"), _("Total RNG") ,_("TTG"), _("ETA"), _T("END") };
    wxString v = wxEmptyString;
    for( int i = 0;; i++ ){
        m_pDataTable->AppendRows();
        if( i == 3 || i == 4 ){
            wxString s = g_withSog? _("SOG"): _("VMG");
            v = _T(" @ ") + s;
        }
        m_pDataTable->SetRowLabelValue(i, s[i] + v );
        if( s[i + 1] == _T("END") ) break;
    }
    m_pDataTable->SetLabelFont( font );
    m_pDataTable->SetRowLabelSize(wxGRID_AUTOSIZE);
    //put cursor outside the grid
    m_pDataTable->SetGridCursor(m_pDataTable->GetNumberRows() +1, 0);
    //set scroll step Y
    m_pDataTable->SetScrollLineY( m_pDataTable->GetRowSize(0) );
}

void DataTable::UpdateRouteData( wxString pointGuid,
                double shiplat, double shiplon, double shipcog, double shipsog )
{
    g_selectedPointCol = wxNOT_FOUND;

    //find active route and wpts
    std::unique_ptr<PlugIn_Route> r;
    r = GetRoute_Plugin( g_activeRouteGuid );
    //set label route name in title
    SetTitle( r.get()->m_NameString );
    //populate cols
    m_pDataTable->BeginBatch();
    double latprev, lonprev, trng = 0.;
    float tttg_sec =0.;
    int ncols = wxNOT_FOUND;
    bool first = true;
    wxPlugin_WaypointListNode *node = r.get()->pWaypointList->GetFirst();
    while( node ){
        PlugIn_Waypoint *wpt = node->GetData();
        if(wpt) {
            wxString srng, sbrg;
            if( first ){
                latprev = wpt->m_lat;
                lonprev = wpt->m_lon;
                first = false;
            }
            if( wpt->m_GUID == pointGuid ) //find active wpt
                ncols = ACTIVE_POINT_IDX;
            if( ncols > wxNOT_FOUND ){
                double brg, rng, speed, nrng;
                speed = 0.; nrng = 0.;
                AddDataCol( ncols );
                if( wpt->m_GUID == g_selectedPointGuid ){
                    if( ncols > ACTIVE_POINT_IDX ){
                        g_selectedPointCol = ncols;
                        m_SelPointPos.m_x = wpt->m_lat;
                        m_SelPointPos.m_y = wpt->m_lon;
                    }
                }
                m_pDataTable->SetColLabelValue( ncols, wpt->m_MarkName );
                if( ncols == ACTIVE_POINT_IDX ){ //active leg from ownship to active point
					//rng, brg, nrng
                    rng = DistGreatCircle_Plugin( shiplat, shiplon, wpt->m_lat, wpt->m_lon );
                    BrgRngMercatorToActiveNormalArrival( wpt->m_lat, wpt->m_lon, latprev, lonprev,
                                            shiplat, shiplon, &brg, &nrng);
					//vmg
					if (!std::isnan(shipcog) && !std::isnan(shipsog)) {
						double vmg = shipsog * cos((brg - shipcog) * PI / 180.);
						if (g_withSog)
							speed = shipsog;
						else
							speed = vmg;
					}
					//sbrg
					if (brg > 359.5) brg = 0;
					if (pPlugin->GetShowMag() == 0 || pPlugin->GetShowMag() == 2)
						sbrg << wxString::Format(wxString("%3.0f°", wxConvUTF8), brg);
					if (pPlugin->GetShowMag() == 1 || pPlugin->GetShowMag() == 2)
						sbrg << wxString::Format(wxString("%3.0f°(M)", wxConvUTF8), pPlugin->GetMag(brg));
					//srng with eventually nrng
                    double deltarng = fabs( rng - nrng );
                    bool delta = false;
                    if( ( deltarng > .01 ) && ( ( deltarng / rng ) > .10 ) && ( rng < 10.0 ) )
                        delta = true;
                    srng = FormatDistance( rng, nrng, delta );
					//total rng
                    trng = rng;
                } else {// following legs
                    //brg, rng
                    DistanceBearingMercator_Plugin(wpt->m_lat, wpt->m_lon, latprev, lonprev,
                                &brg, &rng);
                    srng = FormatDistance( rng );
                    sbrg << wxString::Format( wxString("%3d°", wxConvUTF8 ), (int)brg );
                    speed = shipsog;
					//total rng
                    trng += rng;
                }
                // print brg, ttg, eta
                wxString tttg_s, teta_s;
                if( speed > 0. ) {
                    float ttg_sec = ( rng / speed ) * 3600.;
                    tttg_sec += ttg_sec;
                    wxTimeSpan tttg_span = wxTimeSpan::Seconds( (long) tttg_sec );
                    //Show also #days if TTG > 24 h
                    tttg_s = tttg_sec > SECONDS_PER_DAY ?
                            tttg_span.Format(_("%Dd %H:%M")) : tttg_span.Format("%H:%M:%S");
                    wxDateTime dtnow, eta;
                    dtnow.SetToCurrent();
                    eta = dtnow.Add( tttg_span );
                    teta_s = eta.GetDateOnly() > wxDateTime::Today() ?
                            eta.Format(_T("%b%d %H:%M")) : eta.Format(_T("%H:%M"));
                }
                else
                {
                    tttg_s = _T("---");
                    teta_s = _T("---");
                }
                //populate or update cells
                for (int j = 0; j < m_pDataTable->GetNumberRows(); j++ ){
                m_pDataCol->IncRef();
                    switch(j){
                    case 0:
                        m_pDataTable->SetCellValue( j, ncols, srng );
                        break;
                    case 1:
                        m_pDataTable->SetCellValue( j, ncols, sbrg );
                        break;
                    case 2:
                        if( ncols == 0 )
                            m_pDataTable->SetCellValue( j, ncols, _T("---") );
                        else
                            m_pDataTable->SetCellValue( j, ncols, FormatDistance( trng ) );
                        break;
                    case 3:
                        m_pDataTable->SetCellValue( j, ncols, tttg_s );
                        break;
                    case 4:
                        m_pDataTable->SetCellValue( j, ncols, teta_s );
                    }
                }
                ncols++;
            }
        }
        latprev = wpt->m_lat;
        lonprev = wpt->m_lon;
        node = node->GetNext();
    }
    //Clear all useless lines if exist
    int ex = m_pDataTable->GetNumberCols() - ncols;
    if( ex > 0 )
        m_pDataTable->DeleteCols( ncols, ex );
    //ensure the nb vis cols variable is valid
	if (m_numVisCols == -1 || m_numVisCols > ncols)
		m_numVisCols = ncols;
    if( g_selectedPointCol == wxNOT_FOUND ){
        if ( !g_selectedPointGuid.IsEmpty() )
            MakeVisibleCol( 0 );
        g_selectedPointGuid = wxEmptyString;
	}
	else {
		if (m_targetFlag)
			MakeVisibleCol(g_selectedPointCol);
	}
    //close counters
    m_pDataTable->EndBatch();
    m_pDataCol->DecRef();              // Give up pointer contrl to Grid
}

void DataTable::UpdateTripData( wxDateTime starttime, double tdist, wxTimeSpan times )
{
    if( !g_showTripData ) return;
        m_pStartDate->SetLabel( starttime.Format(_T("%b %d %Y")) );
        m_pStartTime->SetLabel( starttime.Format(_T("%H:%M:%S")) );
        tdist = toUsrDistance_Plugin( tdist, pPlugin->GetDistFormat());
        wxString sd = tdist > 999.99 ? wxString(_T("%1.0f"),wxConvUTF8 ):
                                       tdist > 99.99 ? wxString(_T("%1.1f"),wxConvUTF8 ):
                                       wxString(_T("%1.2f"),wxConvUTF8 );
        m_pDistValue->SetLabel( wxString::Format( sd, tdist )
                                + getUsrDistanceUnit_Plugin( pPlugin->GetDistFormat() ) );
        m_pTimeValue->SetLabel( times.Format(_T("%Hh %Mm")) );
        double th = times.GetSeconds().ToDouble() / 3600;
        double sp = tdist / ( th );
        if( std::isnan(sp) )
            m_pSpeedValue->SetLabel( _T("----") );
        else
            m_pSpeedValue->SetLabel( wxString::Format( wxString("%2.2f", wxConvUTF8 ),
                        toUsrSpeed_Plugin( sp, pPlugin->GetSpeedFormat() ) )
                               + getUsrSpeedUnit_Plugin( pPlugin->GetSpeedFormat() ) );
}

void DataTable::UpdateTripData()
{
    if( !g_showTripData ) return;
    m_pStartDate->SetLabel( _T("----") );
    m_pStartTime->SetLabel( _T("----") );
    m_pDistValue->SetLabel( _T("----") );
    m_pTimeValue->SetLabel( _T("----") );
    m_pSpeedValue->SetLabel( _T("----") );
}

void DataTable::MakeVisibleCol( int col )
{
    int row, fcol;
    m_pDataTable->GetFirstVisibleCell( row, fcol );
    if( m_pDataTable->IsVisible(row, col, true) ) return;
    m_pDataTable->MakeCellVisible( row, m_pDataTable->GetNumberCols() - 1 );
    m_pDataTable->MakeCellVisible( row, col );
}

void DataTable::OnMouseEvent(wxMouseEvent& event)
{
    //suppress all unwanted scroll
    if(g_scrollPos > 0)
        m_pDataTable->CorrectUnwantedScroll();
    //eventually stop long wpt name display
    m_pDataTable->m_stopLoopTimer.Start( TIMER_INTERVAL_MSECOND, wxTIMER_ONE_SHOT );
}

void DataTable::SetTableSizePosition(bool initrun )
{
    m_InvalidateSizeEvent = true;

    //1)adjust visibles columns number
    int scw = GetCanvasByIndex(0)->GetSize().GetWidth();
	int w = GetDataGridWidth(m_numVisCols);
    if(m_dialPosition.x + w > scw - 1 || m_dialPosition.x < 1 ){
        m_dialPosition.x = scw * 0.1;
		scw *= 0.8;
        if(w > scw ){
			for( int i = m_numVisCols; i > 0; i-- ){
				if(GetDataGridWidth(i) <= scw ) {
					m_numVisCols = i;
                    break;
                }
            }
			//show at least one column
			if (m_numVisCols < 1) m_numVisCols = 1;
			w = GetDataGridWidth(m_numVisCols);
        }
    }
	
	int mw = GetDataGridWidth(1);

	int h = GetBestDialogHeight(w);

    int wc = (m_numVisRows < m_pDataTable->GetNumberRows()) ? SCROLL_BAR_THICKNESS : 0;
    int hc = ((m_numVisCols < m_pDataTable->GetNumberCols()) && initrun)? SCROLL_BAR_THICKNESS: 0;
	this->SetMinClientSize(wxSize(mw + wc, h + hc));
	this->SetClientSize(wxSize(w + wc, h + hc));

	m_pTripSizer->Show(g_showTripData);
	this->Layout();
	this->Refresh();

	if( initrun )
		this->Move(m_dialPosition);

	m_targetFlag = true;

	m_InvalidateSizeEvent = false;
}

void DataTable::OnSize( wxSizeEvent& event )
{
	if(!m_InvalidateSizeEvent){

        int tempDialWidth = event.GetSize().GetWidth();
        m_numVisCols = (tempDialWidth - (DOUBLE_BORDER_THICKNESS + m_pDataTable->GetRowLabelSize())) / m_pDataTable->GetDefaultColSize();
        m_SizeTimer.Start(TIMER_INTERVAL_10MSECOND, wxTIMER_ONE_SHOT);
    }
    event.Skip();
}

void DataTable::OnSizeTimer(wxTimerEvent & event)
{
	m_InvalidateSizeEvent = true;

	int w = GetDataGridWidth(m_numVisCols);
	int mw = GetDataGridWidth(1);
	int h = GetBestDialogHeight(w);
    int wc = (m_numVisRows < m_pDataTable->GetNumberRows()) ? SCROLL_BAR_THICKNESS : 0;
	this->SetMinClientSize(wxSize(mw + wc, h));
	this->SetClientSize(wxSize(w + wc, h));

	this->Layout();
	this->Refresh();

	m_targetFlag = true;

	m_InvalidateSizeEvent = false;
}

int DataTable::GetBestDialogHeight( int dialogWidth )
{
    int h = DIALOG_CAPTION_HEIGHT;
	if (g_showTripData) {
		//Compute Trip Data
		int width = 0;
		int nbw = 0.;
		int col = 6;
		m_pfgSizer04->SetCols( 6 );
		m_pfgSizer03->SetCols(4);
		wxwxSizerItemListNode *node =  m_pfgSizer04->GetChildren().GetFirst();
		while( node ) {
			wxSizerItem *item = node->GetData();
			if( item ){
                width += item->GetSize().x + SINGLE_BORDER_THICKNESS;
				nbw++;
				if( nbw%2 == 0 ) {
					if( width > dialogWidth ){
						col = wxMax( 2, (nbw - 2) );
						m_pfgSizer04->SetCols( col );
						m_pfgSizer03->SetCols( col );
						break;
					}
				}
			}
			node = node->GetNext();
		}
		switch (col) {
		case 2:
            h += (m_pStartDText->GetSize().GetHeight() + SINGLE_BORDER_THICKNESS) * 5;
			break;
		case 4:
            h += (m_pStartDText->GetSize().GetHeight() + SINGLE_BORDER_THICKNESS) * 3;
			break;
		case 6:
            h += (m_pStartDText->GetSize().GetHeight() + SINGLE_BORDER_THICKNESS) * 2;
		}
	}
	//then compute best dialog height
	m_numVisRows = 5;
	//get display zone heigh
    int sch = GetCanvasByIndex(0)->GetSize().GetHeight() - 1;
	int ht = h + GetDataGridHeight(m_numVisRows);
	if (m_dialPosition.y + ht > sch || m_dialPosition.y < 1) {
        m_dialPosition.y = sch * 0.1;
		sch -= m_dialPosition.y;
		if (ht > sch) {
			for (int j = m_numVisRows; j > 0; j--) {
				if (h + GetDataGridHeight(j) <= sch) {
					m_numVisRows = j;
					break;
				}
			}
			//show at least one row
			if (m_numVisRows < 1) m_numVisRows = 1;
			ht = h + GetDataGridHeight(m_numVisRows);
		}
	}//
    return ht;
}

int DataTable::GetDataGridWidth(int visColsnumb)
{
    return  DOUBLE_BORDER_THICKNESS + m_pDataTable->GetRowLabelSize() + (m_pDataTable->GetDefaultColSize() * visColsnumb);
}

int DataTable::GetDataGridHeight(int visRowsnumb)
{
	return m_pDataTable->GetColLabelSize() + (m_pDataTable->GetDefaultRowSize() * visRowsnumb);
}

void DataTable::CloseDialog()
{
    m_dialPosition = this->GetPosition();

    this->Show(false);

   // m_numVisCols = m_pDataTable->GetNumVisibleCols();

    wxFileConfig *pConf = GetOCPNConfigObject();
    if(pConf) {
        pConf->SetPath ( _T ( "/Settings/NAVDATA" ) );
        pConf->Write(_T("DataTablePosition_x"), m_dialPosition.x);
        pConf->Write(_T("DataTablePosition_y"), m_dialPosition.y);
        pConf->Write(_T("NumberColVisibles"), m_numVisCols);
        pConf->Write(_T("ShowTripData"), g_showTripData);
        pConf->Write(_T("ShowTTCETAatVMG"), g_withSog);
    }
}

void DataTable::AddDataCol( int num_cols )
{
    if(m_pDataTable->GetNumberCols() == num_cols) {
        m_pDataTable->AppendCols(1);
        m_pDataCol->IncRef();
        m_pDataTable->SetColAttr(num_cols, m_pDataCol);
    }
}

wxString DataTable::FormatDistance( double val1, double val2, bool delta ){
    wxString srng;
    if( delta ){
        if( val2 < 10.0 )
            srng.Printf( _T("%5.2f/%5.2f"), toUsrDistance_Plugin( val1, pPlugin->GetDistFormat() ), toUsrDistance_Plugin( val2, pPlugin->GetDistFormat() ) );
        else
            srng.Printf( _T("%5.1f/%5.1f"), toUsrDistance_Plugin( val1, pPlugin->GetDistFormat() ), toUsrDistance_Plugin( val2, pPlugin->GetDistFormat() ) );
    } else {
        if( val1 < 10.0 )
            srng.Printf( _T("%6.2f"), toUsrDistance_Plugin( val1, pPlugin->GetDistFormat() ) );
        else
            srng.Printf( _T("%6.1f"), toUsrDistance_Plugin( val1, pPlugin->GetDistFormat() ) );
    }
    return srng;
}

void DataTable::BrgRngMercatorToActiveNormalArrival(double wptlat, double wptlon,
                                          double wptprevlat, double wptprevlon,
                                          double glat, double glon,
                                          double *brg, double *nrng)
{
            vector2D va, vb, vn;

            double brg1, dist1, brg2, dist2;
            DistanceBearingMercator_Plugin( wptlat, wptlon,
                    wptprevlat, wptprevlon, &brg1,
                    &dist1 );
            vb.x = dist1 * sin( brg1 * PI / 180. );
            vb.y = dist1 * cos( brg1 * PI / 180. );

            DistanceBearingMercator_Plugin( wptlat, wptlon, glat, glon, &brg2,
                    &dist2 );
            va.x = dist2 * sin( brg2 * PI / 180. );
            va.y = dist2 * cos( brg2 * PI / 180. );

            *brg = brg2;

            vGetLengthOfNormal( &va, &vb, &vn );             // NM
    //    Calculate the distance to the arrival line, which is perpendicular to the current route segment
    //    Taking advantage of the calculated normal from current position to route segment vn
            vector2D vToArriveNormal;
            vSubtractVectors( &va, &vn, &vToArriveNormal );

            *nrng = vVectorMagnitude( &vToArriveNormal );
}


//----------------------------------------------------------------------------------------------------------
//          Settings Dialog Implementation
//----------------------------------------------------------------------------------------------------------

Settings::Settings(wxWindow *parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style )
    : SettingsBase(parent, id, title, pos, size, style )
{
    m_pShowTripData->SetValue( g_showTripData );
    m_pShowspeed->SetSelection(g_withSog ? 1 : 0);

    m_pSettingsOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Settings::OnOKButton ), NULL, this );
}

void Settings::OnOKButton( wxCommandEvent& event )
{
        g_showTripData = m_pShowTripData->GetValue();
        g_withSog = (m_pShowspeed->GetSelection() == 1);
        this->EndModal(wxID_OK);
}
