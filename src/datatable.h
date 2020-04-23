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
 */
#ifndef __DATATABLE_H__
#define __DATATABLE_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include <wx/grid.h>

#include "datadialogbase.h"
#include <wx/dcgraph.h>

#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif

class navdata_pi;
class TripData;

//----------------------------------------------------------------------------------------------------------
//    Data table dialog Specification
//----------------------------------------------------------------------------------------------------------
class DataTable: public DataTableBase
{
public:

    navdata_pi *pPlugin;

    DataTable(wxWindow *parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ,navdata_pi *ppi );
    
	~DataTable();

    void InitDataTable();
    void UpdateRouteData(wxString pointGuid,
                    double shiplon, double shiplat, double shipcog, double shipsog );
    void UpdateRouteData();
    void UpdateTripData(TripData *ptripData);
    void UpdateTripData();
    void SetTableSizePosition();
    void CloseDialog();
    void SetColorScheme();
    void SetTripDialogFont();
private:
    int  GetDialogHeight(int nVisCols);
    int  GetDataGridWidth(int nVisCols);
    void OnPaintEvent(wxPaintEvent &event);
    void OnMouseEvent(wxMouseEvent &event);

    int            m_numDialCols;
    wxPoint        m_dialPosition;
};

#endif //__DATATABLE_H__
