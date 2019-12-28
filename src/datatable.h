/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Plugin Friends - gribtable
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
//#include <wx/list.h>

#include "datadialogbase.h"
//#include "navdata_pi.h"
#include "ocpn/ocpn_plugin.h"
#include "ocpn/vector2D.h"
#include "customgrid.h"
#include <wx/dcgraph.h>

#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif
#define SECONDS_PER_DAY     86400
#define ACTIVE_POINT_IDX    0
/*
#if !wxUSE_GRAPHICS_CONTEXT
#define wxGCDC wxDC
#endif
*/
class navdata_pi;

//----------------------------------------------------------------------------------------------------------
//    Data table dialog Specification
//----------------------------------------------------------------------------------------------------------
class DataTable: public DataTableBase
{
public:

    navdata_pi *pPlugin;

    DataTable(wxWindow *parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ,navdata_pi *ppi );
    
     ~DataTable(){ delete m_pDataTable; }

    void InitDataTable();
    void UpdateRouteData(wxString pointGuid,
                    double shiplon, double shiplat, double shipcog, double shipsog );
    void UpdateTripData(wxDateTime starttime, double tdist, wxTimeSpan times);
    void UpdateTripData();
    void DrawWptName(int index, wxSize size, wxPoint pos);
    void SetTableSizePosition();
    void SetTargetFlag( bool flag ) { m_targetFlag = flag; }
    void CloseDialog();
    wxPoint2DDouble GetSelPointPos() {return m_SelPointPos;}

    wxTimer    m_NameTimer;  //timer to hide long wpt name
    int        m_selectCol;

private:
    void AddDataCol(int num_cols);
    wxString FormatDistance(double val1 , double val2 = 0., bool delta = false);
    void BrgRngMercatorToActiveNormalArrival(double wptlat, double wptlon,
                                  double wptprevlat, double wptprevlon,
                                  double glat, double glon,
                                  double *brg, double *nrng);
    void MakeVisibleCol( int col );
    void OnSize(wxSizeEvent &event );
    void OnNameTimer( wxTimerEvent & event );
    void OnClose( wxCloseEvent& event );
    void OnOKButton( wxCommandEvent& event );

    wxGridCellAttr *m_pDataCol;
    bool           m_targetFlag;
    int            m_oldIndex;
    wxPoint2DDouble  m_SelPointPos;

};
#endif //__DATATABLE_H__
