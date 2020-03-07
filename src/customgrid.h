/******************************************************************************
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
 */
#ifndef __CUSTOMGRID_H__
#define __CUSTOMGRID_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include <wx/grid.h>
#include "ocpn_plugin.h"

//class CustomRenderer;
class DataTable;

//------------------------------------------------------------------------------------
//    A derived class from wxGrid
//----------------------------------------------------------------------------------------------------------
class CustomGrid: public wxGrid
{
public:

    CustomGrid(wxWindow *parent, wxWindowID id, const wxPoint &pos,
                const wxSize &size, long style,
                const wxString &name );

    ~CustomGrid();

    void GetFirstVisibleCell(int& frow, int& fcol);
    void CorrectUnwantedScroll();

    DataTable *m_pParent;
    wxTimer   m_stopLoopTimer;
    int       m_colLongname;

private:
    void DrawColLabel(wxDC& dc, int col);
    void DrawCornerLabel(wxDC & dc);
    void DrawLongWptName();
    void GetLastVisibleCell(int& lrow, int& lcol);
    void OnScroll(wxScrollEvent& event);
    void OnResize(wxSizeEvent& event);
    void OnResizeTimer(wxTimerEvent& event) {ForceRefresh();}
    void OnMouseRollOverColLabel(wxMouseEvent& event);
    void OnMouseEvent(wxMouseEvent& event);
    void OnStopLoopTimer(wxTimerEvent& event);
    void OnNameLoopTimer(wxTimerEvent& event);
    void OnLabelClik(wxGridEvent& event);

    wxTimer  m_resizeTimer;
    wxTimer  m_nameLoopTimer;
    int      m_nameFlag;
    wxString m_LongName;

#ifdef __WXOSX__
    bool     m_bLeftDown;
#endif
};
#endif //__CUSTOMGRID_H__
