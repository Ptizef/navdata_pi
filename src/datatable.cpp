/******************************************************************************
 *
 * Project:  OpenCPN - plugin navdata_pi
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

extern int            g_ocpnDistFormat;
extern int            g_ocpnSpeedFormat;
extern wxColour       g_labelColour;
extern wxColour       g_valueColour;
extern wxFont         g_labelFont;
extern wxFont         g_valueFont;

//----------------------------------------------------------------------------------------------------------
//          Data Table Implementation
//----------------------------------------------------------------------------------------------------------
DataTable::DataTable(wxWindow *parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ,navdata_pi *ppi )
    : DataTableBase(parent, id, title, pos, size, style )
{
    pPlugin = ppi;
    InitDataTable();
}

DataTable::~DataTable(void)
{}

void DataTable::InitDataTable()
{
    //get navdata_pi options
    wxFileConfig *pConf = GetOCPNConfigObject();
    int numCols;
    if (pConf) {
        pConf->SetPath(_T("/Settings/NAVDATA"));
        pConf->Read(_T("DataTablePosition_x"), &m_dialPosition.x, 60);
        pConf->Read(_T("DataTablePosition_y"), &m_dialPosition.y, 30);
        pConf->Read(_T("NumberColVisibles"), &numCols, 1);
    }
    m_numDialCols = numCols > 3? 3: numCols < 1? 1: numCols;

    //connect timers at data tablelevel
    Bind(wxEVT_PAINT, &DataTable::OnPaintEvent, this);
    Bind(wxEVT_LEFT_UP, &DataTable::OnMouseEvent, this);
    Bind(wxEVT_LEFT_DOWN, &DataTable::OnMouseEvent, this);
    Bind(wxEVT_MOTION, &DataTable::OnMouseEvent, this);
}

void DataTable::SetColorScheme()
{
    //Dialog background colour
    wxColour back_color;
    GetGlobalColor(_T("DILG1"), &back_color);
    SetBackgroundColour(back_color);

    //data and label colours
    GetGlobalColor(_T("UBLCK"), &back_color);
    wxWindowListNode *node =  GetChildren().GetFirst();
    int i = 0;
    while( node ) {
        wxWindow *win = node->GetData();
        if( win ){
            if( win->IsKindOf(CLASSINFO(wxTextCtrl)) ){
                win->SetBackgroundColour(back_color);
                if(i%2 == 0)
                    win->SetForegroundColour(g_labelColour);
                else
                    win->SetForegroundColour(g_valueColour);
                i++;
            }
        }
        node = node->GetNext();
    }
}

void DataTable::SetTripDialogFont()
{
    int w, wt, h;
    GetTextExtent(wxString(_T("Abcdefghijkln")), &wt, NULL, 0, 0, &g_labelFont);
    GetTextExtent(wxString(_T("ABCDEFG000i")), &w, &h, 0, 0, &g_valueFont);
    wxWindowListNode *node =  GetChildren().GetFirst();
    int i = 0;
    while( node ) {
        wxWindow *win = node->GetData();
        if( win ){
            if(win->IsKindOf(CLASSINFO(wxTextCtrl))){
                if(i%2 == 0){
                    win->SetFont(g_labelFont);
                    win->SetMinSize(wxSize(wt, h+4));
                    win->SetMaxSize(wxSize(wt, h+4));
                } else {
                    win->SetFont(g_valueFont);
                    win->SetMinSize(wxSize(w, h+4));
                    win->SetMaxSize(wxSize(w, h+4));
                }
                i++;
            }
        }
            node = node->GetNext();
    }
}

void DataTable::UpdateTripData(TripData *ptripData)
{
    if(!ptripData){
        UpdateTripData();
        return;
    }
        //start time
        m_pStartDate->SetValue( ptripData->m_startDate.Format(_T("%x")) );
        m_pStartTime->SetValue( ptripData->m_startDate.Format(_T("%H:%M:%S")) );
        // total time
        wxTimeSpan span =  ptripData->m_endTime - ptripData->m_startDate;
        double th = span.GetSeconds().ToDouble() / 3600; //total time in hours
        if( th < 23 )
            m_pTimeValue->SetValue( span.Format(_T("%H:%M:%S")) );
        else
            m_pTimeValue->SetValue( span.Format(_T("%Dd %H:%M")) );

        //end time
        if(ptripData->m_isEnded){
            wxString send = ptripData->m_endTime.Format(_T("%x")).BeforeLast('/');
            send << ptripData->m_startDate.Format(_T(" %H:%M"));
            m_pEndDate->SetValue(send);
        } else {
            m_pEndDate->SetValue( _T("---") );
        }
        //dist
        double tdist = ptripData->m_totalDist + ptripData->m_tempDist;
        if( tdist > 0.1 ){
            tdist = toUsrDistance_Plugin( tdist, g_ocpnDistFormat);
            int c = tdist > 999.99 ? 0: tdist > 9.99 ? 1: 2;
            m_pDistValue->SetValue( wxString::Format( wxString(_T("%1.*f")), c, tdist )
                                + getUsrDistanceUnit_Plugin( g_ocpnDistFormat ) );
        } else
            m_pDistValue->SetValue( _T("---") );
        //speed
        double sp = tdist / ( th );
        if( std::isnan(sp) ){
            m_pSpeedValue->SetValue( _T("---") );
            return;
        }
        if( tdist > 0.1 ){
            m_pSpeedValue->SetValue( wxString::Format( wxString("%2.2f", wxConvUTF8 ),
                        toUsrSpeed_Plugin( sp, g_ocpnSpeedFormat ) )
                               + getUsrSpeedUnit_Plugin( g_ocpnSpeedFormat ) );
        } else
            m_pSpeedValue->SetValue( _T("---") );
}

void DataTable::UpdateTripData()
{
    m_pStartDate->SetValue( _T("---") );
    m_pStartTime->SetValue( _T("---") );
    m_pDistValue->SetValue( _T("---") );
    m_pTimeValue->SetValue( _T("---") );
    m_pSpeedValue->SetValue( _T("---") );
    m_pEndDate->SetValue( _T("---") );
}

void DataTable::SetTableSizePosition()
{
    m_numDialCols = 1;

    Fit();

    int hd = GetDialogHeight(m_numDialCols);
    SetClientSize(wxSize(GetDataGridWidth(m_numDialCols), hd));

    Layout();
    Refresh();

    Move(m_dialPosition);
}

int DataTable::GetDialogHeight( int nVisCols )
{
    m_pTripSizer01->SetCols(nVisCols * 2);
    int h = (m_pStartDate->GetSize().GetHeight() + SINGLE_BORDER_THICKNESS) * (6 / nVisCols);
    return h + DIALOG_CAPTION_HEIGHT;
}

int DataTable::GetDataGridWidth(int nVisCols)
{
    int w =  ( DOUBLE_BORDER_THICKNESS + m_pTimetText->GetSize().GetX() + m_pStartTime->GetSize().GetX()) * nVisCols;
    return w + SINGLE_BORDER_THICKNESS;
}

void DataTable::OnPaintEvent( wxPaintEvent &event )
{
    //caption size
    const int sx = GetSize().x;
    //caption font
    wxFont font = GetOCPNGUIScaledFont_PlugIn(_("Dialog"));
    wxFont lfont = *FindOrCreateFont_PlugIn( 10, wxFONTFAMILY_DEFAULT,
                font.GetStyle(), wxFONTWEIGHT_BOLD, false, font.GetFaceName() );
    //draw in a memory dc then blit
    wxMemoryDC mdc;
    wxBitmap bmp(GetSize());
    mdc.SelectObject(bmp);
    //draw caption background
    wxColour bcolour, fcolour;
    GetGlobalColor(_T("DILG0"), &bcolour);
    mdc.SetPen(*wxTRANSPARENT_PEN);
    mdc.SetBrush(wxBrush(bcolour, wxBRUSHSTYLE_SOLID));
    mdc.DrawRectangle(wxRect(0 , 0, sx, DIALOG_CAPTION_HEIGHT));
    //draw caption label
    mdc.SetFont(lfont);
    mdc.SetPen(wxPen(fcolour,wxPENSTYLE_SOLID));
    mdc.SetBrush(*wxTRANSPARENT_BRUSH);
    mdc.DrawLabel( _("Trip Data"), wxRect(0 , 0, sx, DIALOG_CAPTION_HEIGHT),wxALIGN_CENTER);

    wxPaintDC pdc(this);
    pdc.Blit( 0, 0, sx, DIALOG_CAPTION_HEIGHT, &mdc, 0, 0 );

    event.Skip();
}

void DataTable::OnMouseEvent( wxMouseEvent &event )
{
    static wxPoint s_gspt;
    int x, y;

    event.GetPosition( &x, &y );
    wxPoint spt = wxPoint( x, y );
    spt = ClientToScreen( spt );

#ifdef __WXOSX__
    wxPoint par_pos = par_pos_old;
    if (!m_bLeftDown && event.LeftIsDown())
    {
        m_bLeftDown = true;
        s_gspt = spt;
        if (!HasCapture()) CaptureMouse();
    }
    else if (m_bLeftDown && !event.LeftIsDown())
    {
        m_bLeftDown = false;
        if (HasCapture()) ReleaseMouse();
    }
#else

    if( event.LeftDown() ) {
        s_gspt = spt;
        if (!HasCapture()) CaptureMouse();
    }

    if( event.LeftUp() ) {
        if( HasCapture() ) ReleaseMouse();
    }
#endif

    if( event.Dragging() ) {

        wxPoint cons_pos = GetScreenPosition();//GetPosition();

        wxPoint cons_pos_old = cons_pos;
        cons_pos.x += spt.x - s_gspt.x;
        cons_pos.y += spt.y - s_gspt.y;

        if( cons_pos != cons_pos_old ) {
            Move( cons_pos );
            pPlugin->m_consPosition = cons_pos;
        }
        s_gspt = spt;
    }
}

void DataTable::CloseDialog()
{
    m_dialPosition = GetPosition();

    Show(false);

    wxFileConfig *pConf = GetOCPNConfigObject();
    if(pConf) {
        pConf->SetPath ( _T ( "/Settings/NAVDATA" ) );
        pConf->Write(_T("DataTablePosition_x"), m_dialPosition.x);
        pConf->Write(_T("DataTablePosition_y"), m_dialPosition.y);
        pConf->Write(_T("NumberColVisibles"), m_numDialCols);
    }
}
