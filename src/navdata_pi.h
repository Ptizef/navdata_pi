/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  ROUTE Plugin
 * Author:   Walbert Schulpen (SaltyPaws)
 *
 ***************************************************************************
 *   Copyright (C) 2012-2016 by Walbert Schulpen                           *
 *   $EMAIL$                                                               *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#ifndef _NAVDATA_PI_H_
#define _NAVDATA_PI_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
  #include <wx/glcanvas.h>
#endif //precompiled headers

//#include "wx/jsonreader.h"
//#include "wx/jsonwriter.h"

#include <wx/fileconf.h>

#include "ocpn/ocpn_plugin.h"

#include "version.h"
#include <vector>

#include "datatable.h"

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------

#define     MY_API_VERSION_MAJOR    1

#define     MY_API_VERSION_MINOR    16

#define CALCULATOR_TOOL_POSITION    -1          // Request default positioning of toolbar tool

#define     TIME_TYPE_UTC 1
#define     TIME_TYPE_LMT 2
#define     TIME_TYPE_COMPUTER 3

#define     TRACKPOINT_ONE            1

#define     TIMER_INTERVAL_HOUR   3600000  //3600 s 1 hour
#define     TIMER_INTERVAL_10SECOND 10000  //10 s
#define     TIMER_INTERVAL_SECOND    1000  //1 s
#define     TIMER_INTERVAL_MSECOND      1  //1 ms

#define     OFFSET_LAT     1e-6

#if !wxUSE_GRAPHICS_CONTEXT
#define wxGCDC wxDC
#endif

class navdata_pi : public opencpn_plugin_116, wxTimer
{
public:
      navdata_pi(void *ppimgr);
      ~navdata_pi(void);
//    The required PlugIn Methods
      int Init(void);
      bool DeInit(void);

      int GetAPIVersionMajor();
      int GetAPIVersionMinor();
      int GetPlugInVersionMajor();
      int GetPlugInVersionMinor();
      wxBitmap *GetPlugInBitmap();
      wxString GetCommonName();
      wxString GetShortDescription();
      wxString GetLongDescription();

      double GetMag(double a);
      bool GetShowMag() {return m_ocpnShowMag;}
      int GetDistFormat() {return m_ocpnDistFormat;}
      int GetSpeedFormat() {return m_ocpnSpeedFormat;}
      void CloseDataTable();
      wxString GetActiveRouteGUID(){ return m_ActiveRouteGuid;}
//    The override PlugIn Methods
      int GetToolbarToolCount(void);

private:  
      void SetPluginMessage(wxString &message_id, wxString &message_body);
      void OnToolbarToolCallback(int id);
      void SetPositionFix(PlugIn_Position_Fix &pfix);
      void OnTripLenghtTimer(wxTimerEvent & event);
      void OnRotateTimer( wxTimerEvent & event);
      bool GetOcpnDailyTrack(int *roTime, int *rotimeType);
      void LoadocpnConfig();
      void SetDialogFont( wxWindow *dialog, wxFont *font);

      int m_leftclick_tool_id;

      //data table variables
      DataTable   *m_pTable;
      wxWindow    *m_pParentWin;
      double      m_gLat;
      double      m_gLon;
      double      m_gCog;
      double      m_gSog;
      double      m_gWmmVar;

      //Route & wpoint variables
      wxString    m_ActiveRouteGuid;
      wxString    m_ActivePointGuid;
      wxString    m_SelectedPointGuid;

      //ocpn options variables
      int         m_ocpnDistFormat;
      int         m_ocpnSpeedFormat;
      double      m_ocpnUserVar;
      bool        m_ocpnShowMag;

      //Track variables
      wxString    m_gTrkGuid;
      wxDateTime  m_gTrkStart;
      wxDateTime  m_gTrkRotate;
      int         m_gNodeNbr;
      bool        m_gHasRotated;
      bool        m_gMustRotate;
      double      m_gTrkLenght;
      double      m_gRotateLenght;
      wxTimeSpan  m_gTrkRunning;    
      double      m_oldtpLat;
      double      m_oldtpLon;

      wxTimer     m_lenghtTimer;
      wxTimer     m_rotateTimer;

      wxString m_shareLocn;
};
#endif //_NAVDATA_PI_H_
