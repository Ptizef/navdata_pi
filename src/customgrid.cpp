/************************************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  personalized GRID
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

#include "customgrid.h"

#include <wx/graphics.h>

#define SCROLL_SENSIBILITY    20

#include "navdata_pi.h"
#include "icons.h"

extern wxString m_shareLocn;
extern wxString m_ActiveRouteGuid;
extern int      m_Blink;



//------------------------------------------------------------------------------
//          custom grid implementation
//------------------------------------------------------------------------------
CustomGrid::CustomGrid( wxWindow *parent, wxWindowID id, const wxPoint &pos,
					   const wxSize &size, long style,
                       const wxString &name )
  : wxGrid( parent, id, pos, size, style, name )
{
    //create grid
    SetTable( new wxGridStringTable(0, 0), true, wxGridSelectRows );
    //some general settings
    EnableEditing( false );
    EnableGridLines( true );
    EnableDragGridSize( false );
    SetMargins( 0, 0 );
    EnableDragColMove( false );
    EnableDragColSize( false );
    EnableDragRowSize( false );
    //init labels attr
    GetGlobalColor(_T("DILG0"), &m_labelBackgroundColour);
    GetGlobalColor(_T("DILG3"), &m_labelTextColour );
    GetGlobalColor(_T("YELO1"), &m_yellowColour);
    GetGlobalColor(_T("URED"), &m_redColour);
    //init useful variable
    m_withSog = false;

#ifdef __WXOSX__
    m_bLeftDown = false;
#endif

    //connect events at dialog level
    Connect(wxEVT_SCROLLWIN_THUMBTRACK, wxScrollEventHandler( CustomGrid::OnScroll ), NULL, this );
    Connect(wxEVT_SIZE, wxSizeEventHandler( CustomGrid::OnResize ), NULL, this );
    Connect(wxEVT_GRID_LABEL_LEFT_CLICK, wxGridEventHandler( CustomGrid::OnLabelClik ), NULL, this );
    //connect events at grid windows level
    GetGridWindow()->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler( CustomGrid::OnMouseEvent ), NULL, this );
    GetGridWindow()->Connect(wxEVT_LEFT_UP, wxMouseEventHandler( CustomGrid::OnMouseEvent ), NULL, this );
    GetGridWindow()->Connect(wxEVT_MOTION, wxMouseEventHandler( CustomGrid::OnMouseEvent ), NULL, this );
    if( IsTouchInterface_PlugIn() )
        GetGridColLabelWindow()->Bind( wxEVT_LEFT_UP, &CustomGrid::OnMouseRollOverColLabel,this);
    else {
        GetGridColLabelWindow()->Bind( wxEVT_MOTION, &CustomGrid::OnMouseRollOverColLabel,this);
        GetGridColLabelWindow()->Bind( wxEVT_LEAVE_WINDOW, &CustomGrid::OnMouseRollOverColLabel,this);
    }
    //connect timer event
    m_refreshTimer.Connect(wxEVT_TIMER, wxTimerEventHandler( CustomGrid::OnRefreshTimer ), NULL, this);
}

 CustomGrid::~CustomGrid()
 {}


void CustomGrid::DrawColLabel( wxDC& dc, int col )
{
    //draw label only inside width available
    int w;
    wxString label = GetColLabelValue(col);
    GetTextExtent( label, &w, NULL, 0, 0, &m_labelFont);
    if( w > (GetColWidth(col) - 2)){
        int len =  label.Len() * ((double)(GetColWidth(col) - 2) / (double)w);
        label = GetColLabelValue(col).Mid(0, len);
    }
    //draw rectangle
    dc.SetFont( m_labelFont );
    dc.SetPen(GetDefaultGridLinePen());
    dc.SetBrush(wxBrush(m_labelBackgroundColour, wxBRUSHSTYLE_SOLID));
    dc.DrawRectangle(wxRect(GetColLeft(col), 0, GetColWidth(col),  m_colLabelHeight));
    //draw selected or active mark
    if( (col == 0 && m_gParent->m_selectCol == wxNOT_FOUND )
            || col == m_gParent->m_selectCol ){
         if( m_Blink & 1 ) {
            wxImage image;
            if( col == 0  )
                image = _img_activewpt->ConvertToImage();
            else
                image = _img_targetwpt->ConvertToImage();

            unsigned char *i = image.GetData();
            if (i == 0)
                return;
            int w = image.GetWidth();
            int h = image.GetHeight();
            double scale = ((((double)m_colLabelHeight/2)/h)*4)/4;
            w *= scale;
            h *= scale;
            wxBitmap scaled;
            scaled =  wxBitmap(image.Scale( w, h) );
            dc.DrawBitmap( scaled, GetColLeft(col), 0 );
         }
    }
    //draw label
    dc.SetPen( wxPen(m_labelTextColour, 1, wxPENSTYLE_SOLID));
    dc.DrawLabel(label, wxRect( GetColLeft(col) + 1, 1, GetColWidth(col) - 2,
                            m_colLabelHeight - 2), wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
}

void CustomGrid::DrawCornerLabel( wxDC& dc )
{
    //draw corner rectangle
    dc.SetPen(GetDefaultGridLinePen());
    dc.SetBrush( wxBrush( m_labelBackgroundColour, wxBRUSHSTYLE_SOLID ) );
    dc.DrawRectangle(0, 0, m_rowLabelWidth, m_colLabelHeight );
    //draw button
    wxImage image;
    if( m_withSog )
        image = _img_SOG->ConvertToImage();
    else
        image = _img_VMG->ConvertToImage();

    unsigned char *i = image.GetData();
    if (i == 0)
        return;
    int w = image.GetWidth();
    int h = image.GetHeight();
    double scale = ((((double)m_colLabelHeight - 10)/h)*4)/4;
    w *= scale;
    h *= scale;
    int x = m_rowLabelWidth - (w+5);
    wxBitmap scaled;
    scaled =  wxBitmap(image.Scale( w, h) );
    dc.DrawBitmap( scaled, x, 5);

}

void CustomGrid::OnScroll( wxScrollEvent& event )
{
    m_refreshTimer.Start( 10, wxTIMER_ONE_SHOT );
    event.Skip();
}

void CustomGrid::OnLabelClik( wxGridEvent& event)
{
    ClearSelection();
    if( event.GetCol() == wxNOT_FOUND && event.GetRow() == wxNOT_FOUND )
        m_withSog = !m_withSog;
}

void CustomGrid::OnResize( wxSizeEvent& event )
{
    m_refreshTimer.Start( 10, wxTIMER_ONE_SHOT );
    event.Skip();
}

void CustomGrid::OnMouseRollOverColLabel( wxMouseEvent& event)
{
    bool refresh = event.Leaving();
    if( event.Moving() || event.LeftUp() ){
        int c = event.GetPosition().x;
        int col = XToCol( c );
        if( col != wxNOT_FOUND ) {           //column label
            int index = GetColIndex( col );
            wxFont font = GetOCPNGUIScaledFont_PlugIn(_("Dialog") );
            int w, h;
            GetTextExtent( GetColLabelValue( index ), &w, &h, 0, 0, &font);
            h-=2;
            if( w > GetColSize(index) ){
                //compute best position
                wxPoint p = GetPosition();
                wxSize  s = m_gParent->GetSize();
                int x = p.x + GetRowLabelSize() + (GetDefaultColSize() * col) + (GetDefaultColSize() / 2 );
                x -= (w/2);
                int y = p.y - h;
                if( (x + w)  > s.x ){
                    x = s.x - w;
                    if( x < 0 )
                        x = 0;
                }
                m_gParent->DrawWptName( index, wxSize(w, y), wxPoint(x, y) );
                return;
            } else
               refresh = true;
        } else
            refresh = true;
    }
    if( refresh )
        m_gParent->m_NameTimer.Start( 1, wxTIMER_ONE_SHOT );
}

int CustomGrid::GetColIndex( int col )
{
    int rw,c;
    GetFirstVisibleCell( rw, c );
    std::unique_ptr<PlugIn_Route> r;
    r = GetRoute_Plugin( m_ActiveRouteGuid );
    wxPlugin_WaypointListNode *node = r.get()->pWaypointList->GetFirst();
    int i = 0;
    while( node ){
        PlugIn_Waypoint *wpt = node->GetData();
        if(wpt) {
            if( c == i )
                return col + c;
            i++;
        }
        node = node->GetNext();
    }
    return c;
}

void CustomGrid::OnMouseEvent( wxMouseEvent& event )
{
    static wxPoint s_pevt;
    wxPoint pevt = event.GetPosition();
#ifdef __WXOSX__
    if (!m_bLeftDown && event.LeftIsDown()){
        m_bLeftDown = true;
        s_pevt = pevt;
    }
    else if (m_bLeftDown && !event.LeftIsDown()){
        m_bLeftDown = false;
        if (HasCapture()) ReleaseMouse();
    }
#else
    if(event.LeftDown())
        s_pevt = pevt;
    if(event.LeftUp()){
        if(HasCapture()) ReleaseMouse();
    }
#endif
    if(event.Dragging()){
        m_gParent->SetTargetFlag( false );
        int frow, fcol, lrow, lcol;
        GetFirstVisibleCell(frow, fcol);
        GetLastVisibleCell(lrow, lcol);
        if( pevt != s_pevt ) {
            bool rfh = false;
            int diff = pevt.x - s_pevt.x;
            //scrolling right
            if( diff > SCROLL_SENSIBILITY ){
                s_pevt.x = pevt.x;
                if( fcol > 0 ){
                    MakeCellVisible(frow, fcol - 1 );
                    rfh = true;
                }
            }
            //scrolling left
            else  if ( -diff > SCROLL_SENSIBILITY ){
                s_pevt.x = pevt.x;
                if( lcol < m_numCols - 1 ){
                    MakeCellVisible(frow, lcol + 1);
                    rfh = true;
                }
            }
            //scrolling down
            diff = pevt.y - s_pevt.y;
            if( diff > SCROLL_SENSIBILITY ){
                s_pevt.y = pevt.y;
                if( frow > 0 ){
                    MakeCellVisible(frow - 1, fcol);
                    rfh = true;
                }
            }
            //scrolling up
            else if( -diff > SCROLL_SENSIBILITY ) {
                s_pevt.y = pevt.y;
                if( lrow < m_numRows - 1 ){
                    MakeCellVisible(lrow + 1, fcol);
                    MakeCellVisible(frow + 1, fcol);      //workaroud for what seems curious moving 2 rows instead of 1 in previous line
                }
            }
            if(rfh)
                m_refreshTimer.Start( 10, wxTIMER_ONE_SHOT );
        }
    }
}

//find the first top/left visible cell coords
void CustomGrid::GetFirstVisibleCell(int& frow, int& fcol)
{
    frow = 0;
	for(fcol = 0; fcol < m_numCols; fcol++){
		for(frow = 0; frow < m_numRows; frow++) {
            if(IsVisible(frow, fcol)) //find the first row/col
                return;
		}
	}
}

//find the visible cell coords
void CustomGrid::GetLastVisibleCell(int& lrow, int& lcol)
{
    lrow = wxMax(m_numRows - 1, 0);
	for(lcol = wxMax(m_numCols - 1, 0); lcol > -1; lcol--){
		for(lrow = m_numRows - 1; lrow > -1; lrow--) {
            if(IsVisible(lrow, lcol)){
                return;
            }
		}
	}
}
/*

//------------------------------------------------------------------------------
//          custom renderer
//------------------------------------------------------------------------------
void CustomRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected)
{
    dc.SetPen(wxPen(attr.GetBackgroundColour(), 1));
    dc.SetBrush(wxBrush( attr.GetBackgroundColour(), wxBRUSHSTYLE_SOLID ));
    dc.DrawRectangle( rect );

    if( m_IsDigit || m_dDir == GRIB_NOTDEF ) {        //digital format
        wxString text(wxEmptyString);
        if( m_dDir != GRIB_NOTDEF )
            text.Printf(_T("%03d\u00B0"), (int)m_dDir);
        dc.DrawLabel(text, rect, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
    } else {                                        //graphical format
        double si = sin( (m_dDir - 90) * M_PI / 180. );
        double co = cos( (m_dDir - 90) * M_PI / 180. );

        int i = rect.GetTopLeft().x + (rect.GetWidth() / 2);
        int j = rect.GetTopLeft().y + (rect.GetHeight() / 2);

        int arrowSize = rect.GetHeight() - 3;
        int dec = -arrowSize / 2;

#if wxUSE_GRAPHICS_CONTEXT
        wxGraphicsContext *gdc;
        wxClientDC *cdc = new wxClientDC(wxDynamicCast( &grid, wxWindow));
        cdc = wxDynamicCast(&dc, wxClientDC);
        if( cdc ) {
            gdc = wxGraphicsContext::Create(*cdc);
#ifdef __WXGTK__
            // platforms don't manage the same way the gdc origin
            for linux, we have to re-compute the good one.
            To DO : verify it works on all other plateforms (done for MSW
            bool vis = false;
            int r = 0;
            for( int c = 0; c < grid.GetNumberCols(); c++){
                for(r = 0; r < grid.GetNumberRows(); r++) {
                    if(grid.IsVisible(r, c)){  //find the first row/col
                        vis = true;
                        i -= (c * grid.GetColSize(0));
                        j -= (r * grid.GetRowHeight(0));
                        break;
                    }
                }
                if(vis) break;
            }
#endif
            gdc->SetPen(wxPen(attr.GetTextColour(), 3));
            gdc->SetBrush(wxBrush( attr.GetBackgroundColour(), wxBRUSHSTYLE_SOLID ));

            double ii, jj, kk, ll;
            GetArrowsPoints( si, co, i, j, dec, 0, dec + arrowSize, 0, ii, jj, kk, ll );
            gdc->StrokeLine( ii, jj, kk, ll );
            GetArrowsPoints( si, co, i, j, dec - 3, 0, dec + 5, 3, ii, jj, kk, ll );
            gdc->StrokeLine( ii, jj, kk, ll );
            GetArrowsPoints( si, co, i, j, dec - 3, 0, dec + 5, -3, ii, jj, kk, ll );
            gdc->StrokeLine( ii, jj, kk, ll );
            delete gdc;
        } else
#endif
        {
            dc.SetPen(wxPen(attr.GetTextColour(), 3));
            double ii, jj, kk, ll;
            GetArrowsPoints( si, co, i, j, dec, 0, dec + arrowSize, 0, ii, jj, kk, ll );
            dc.DrawLine( (int)ii, (int)jj, (int)kk, (int)ll );
            GetArrowsPoints( si, co, i, j, dec - 3, 0, dec + 5, 3, ii, jj, kk, ll );
            dc.DrawLine( (int)ii, (int)jj, (int)kk, (int)ll );
            GetArrowsPoints( si, co, i, j, dec - 3, 0, dec + 5, -3, ii, jj, kk, ll );
            dc.DrawLine( (int)ii, (int)jj, (int)kk, (int)ll );
        }
    }
}
*/
