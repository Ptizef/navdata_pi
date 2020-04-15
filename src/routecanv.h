/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Console Canvas
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 */

#ifndef __concanv_H__
#define __concanv_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

#define SPEED_VMG 0
#define SPEED_SOG 1

#define ID_LEGROUTE 1000
#define SECONDS_PER_DAY 86400

// Class declarations
class navdata_pi;
//----------------------------------------------------------------------------
// AnnunText
//----------------------------------------------------------------------------
class AnnunText : public wxWindow
{
public:
      AnnunText(wxWindow *parent, wxWindowID id, const wxString& LegendElement, const wxString& ValueElement);

      ~AnnunText();

      void SetALabel(const wxString &l);
      void SetAValue(const wxString &v);
      void OnPaint(wxPaintEvent& event);
      void RefreshFonts(void);
      void SetLegendElement(const wxString &element);
      void SetValueElement(const wxString &element);
      void SetColorScheme(PI_ColorScheme cs);
      void MouseEvent( wxMouseEvent& event );
      
private:
      void CalculateMinSize(void);

      wxBrush     m_backBrush;
      wxColour    m_default_text_color;

      wxString    m_label;
      wxString    m_value;
      wxFont      m_plabelFont;
      wxFont      m_pvalueFont;

      wxString    m_LegendTextElement;
      wxString    m_ValueTextElement;
      wxColour    m_legend_color;
      wxColour    m_val_color;

DECLARE_EVENT_TABLE()

};




//----------------------------------------------------------------------------
// RouteCanvas
//----------------------------------------------------------------------------
class RouteCanvas: public wxFrame
{
public:
      RouteCanvas(wxWindow *parent, navdata_pi *ppi);
      ~RouteCanvas();
      void UpdateRouteData();
      void ShowWithFreshFonts(void);
      void UpdateFonts(void);
      void SetColorScheme(PI_ColorScheme cs);
      void LegRoute();
      void OnContextMenu( wxContextMenuEvent& event );
      void OnContextMenuSelection( wxCommandEvent& event );
      void RefreshConsoleData(void);
      void ToggleVmgSogDisplay();

      navdata_pi *pPlugin;

      wxStaticText      *pThisLegText;
      wxBoxSizer        *m_pitemBoxSizerLeg;

      AnnunText         *pRNG;
      AnnunText         *pTTG;
      AnnunText         *pETA;

      bool              m_bNeedClear;

private:
      void OnTextMouseEvent( wxMouseEvent &event );
      void OnMouseEvent( wxMouseEvent &event );
      void BrgRngMercatorToActiveNormalArrival(double wptlat, double wptlon,
                                    double wptprevlat, double wptprevlon,
                                    double glat, double glon,
                                    double *brg, double *nrng);
      char m_speedUsed;

DECLARE_EVENT_TABLE()
};

#endif
