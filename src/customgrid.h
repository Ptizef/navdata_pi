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
#include "ocpn/ocpn_plugin.h"
//#include <wx/graphics.h>
//#include <vector>

class CustomRenderer;
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

    void GetFirstVisibleCell( int& frow, int& fcol );
    bool GetSpeedAsSog() { return m_withSog; }

    DataTable* m_gParent;

private:
    void DrawColLabel( wxDC& dc, int col );
    void DrawCornerLabel(wxDC & dc);
    //void GetFirstVisibleCell( int& frow, int& fcol );
    void GetLastVisibleCell( int& lrow, int& lcol );
    int  GetColIndex( int col );
    void OnScroll( wxScrollEvent& event );
    void OnResize( wxSizeEvent& event );
    void OnMouseRollOverColLabel( wxMouseEvent& event);
    void OnMouseEvent( wxMouseEvent& event );
    void OnRefreshTimer( wxTimerEvent& event ) {ForceRefresh();}
    void OnLabelClik( wxGridEvent& event);

    wxTimer  m_refreshTimer;
    wxColour m_yellowColour;
    wxColour m_redColour;
    bool     m_withSog;

#ifdef __WXOSX__
    bool     m_bLeftDown;
#endif

};
/*
//----------------------------------------------------------------------------------------------------------
//    A derived class from wxGridCellRenderer
//----------------------------------------------------------------------------------------------------------
class CustomRenderer : public wxGridCellRenderer
{
public:
	CustomRenderer( double dir, bool isdigit)
        : wxGridCellRenderer(), m_dDir(dir), m_IsDigit(isdigit) {}

	virtual void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected);

	wxSize GetBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, int row, int col) {
		return wxSize(-1, -1);
	}
	wxGridCellRenderer *Clone() const { return new CustomRenderer(m_dDir, m_IsDigit);}

private:

	double m_dDir;
    bool m_IsDigit;
};
*/
#endif //__CUSTOMGRID_H__
