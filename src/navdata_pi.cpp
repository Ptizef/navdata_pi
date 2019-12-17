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

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "navdata_pi.h"
#include "icons.h"
#include <wx/list.h>

#include "wx/jsonreader.h"
#include "wx/jsonwriter.h"

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
    return new navdata_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
    delete p;
}
// static variables
wxString       m_SelectedPointGuid;

//-------------------------------------------------------
//          PlugIn initialization and de-init
//-------------------------------------------------------

navdata_pi::navdata_pi(void *ppimgr)
      :opencpn_plugin_116(ppimgr), wxTimer(this)
{
      // Create the PlugIn icons
      initialize_images();
}

navdata_pi::~navdata_pi(void)
{
    delete _img_active;
    delete _img_toggled;
    delete _img_inactive;
 }

int navdata_pi::Init(void)
{
      m_lenghtTimer.Connect(wxEVT_TIMER, wxTimerEventHandler(navdata_pi::OnTripLenghtTimer), NULL, this );
      m_rotateTimer.Connect(wxEVT_TIMER, wxTimerEventHandler(navdata_pi::OnRotateTimer), NULL, this );

      AddLocaleCatalog( _T("opencpn-navdata_pi") );
      m_pParentWin = GetOCPNCanvasWindow();

      m_pTable = NULL;

      //LoadConfig();

      m_ToolIconType = 0xffff;

      m_SelectedPointGuid = wxEmptyString;
      m_gTrkGuid = wxEmptyString;
      m_ActiveRouteGuid = wxEmptyString;
      m_gMustRotate = false;
      m_gHasRotated = false;
      m_gRotateLenght = 0;

      m_leftclick_tool_id = -1;

      //    This PlugIn needs a toolbar icon, so request its insertion
      wxString active, toggled, inactive;
      if( GetSVGFileIcons( active, toggled, inactive ))
          m_leftclick_tool_id  = InsertPlugInToolSVG(_T(""), inactive, inactive, inactive,
                    wxITEM_CHECK, _("Nav data"), _T(""), NULL, CALCULATOR_TOOL_POSITION, 0, this);
      else
          m_leftclick_tool_id  = InsertPlugInTool(_T(""), _img_inactive, _img_inactive,
                                    wxITEM_CHECK, _("Nav data"), _T(""), NULL, CALCULATOR_TOOL_POSITION, 0, this);

      return (WANTS_TOOLBAR_CALLBACK   |
              INSTALLS_TOOLBAR_TOOL    |
              WANTS_PREFERENCES        |
              WANTS_PLUGIN_MESSAGING   |
              WANTS_NMEA_EVENTS        |
              WANTS_CONFIG
           );
}

bool navdata_pi::DeInit(void)
{
      //SaveConfig();

      if( m_pTable ){
          CloseDataTable();
      }

      return true;
}

int navdata_pi::GetAPIVersionMajor()
{
      return MY_API_VERSION_MAJOR;
}

int navdata_pi::GetAPIVersionMinor()
{
      return MY_API_VERSION_MINOR;
}

int navdata_pi::GetPlugInVersionMajor()
{
      return PLUGIN_VERSION_MAJOR;
}

int navdata_pi::GetPlugInVersionMinor()
{
      return PLUGIN_VERSION_MINOR;
}

wxBitmap *navdata_pi::GetPlugInBitmap()
{
      return _img_active;;
}

wxString navdata_pi::GetCommonName()
{
      return _("NAVDATA");
}

wxString navdata_pi::GetShortDescription()
{
      return _("Navigation progress data plugin for OpenCPN");
}

wxString navdata_pi::GetLongDescription()
{
      return _("Navigation progress data\n plugin for OpenCPN\n\
shows range, TTG and ETA for all route points\n of an active route.");
}

int navdata_pi::GetToolbarToolCount(void)
{
      return 1;
}
/*
bool navdata_pi::LoadConfig(void)
{
    wxFileConfig *pConf = GetOCPNConfigObject();
    if(pConf)
    {
            return true;
    } else

        return false;
}
*/
void navdata_pi::LoadocpnConfig()
{
      wxFileConfig *pConf = GetOCPNConfigObject();
      if(pConf)
      {
          pConf->SetPath(_T("/Settings"));
          pConf->Read(_T("ShowMag"), &m_ocpnShowMag, 0);
          pConf->Read(_T("UserMagVariation"), &m_ocpnUserVar, 0);
          pConf->Read(_T("DistanceFormat"), &m_ocpnDistFormat, 0);
          pConf->Read(_T("SpeedFormat"), &m_ocpnSpeedFormat, 0);
      }
}

/*bool navdata_pi::SaveConfig(void)
{
    wxFileConfig *pConf = GetOCPNConfigObject();
    if(pConf)
    {
        return true;
    }
    else
        return false;
}*/

void navdata_pi::SetPluginMessage(wxString &message_id, wxString &message_body)
{
    if(message_id == _T("OCPN_TRACKPOINTS_COORDS"))
    {
        wxJSONValue  root;
        wxJSONReader reader;
        int rnumErrors = reader.Parse( message_body, &root );
        if ( rnumErrors > 0 ) return;
        if( root[_T("error")].AsBool() ) return;
        if( root[_T("Track_ID")].AsString() == m_gTrkGuid ) {
            int NodeNr = root[_T("NodeNr")].AsInt();
            int TotalNodes = root[_T("TotalNodes")].AsInt();
            if( NodeNr >= m_gNodeNbr ){ //skip track point already treated
                double lat = root[_T("lat")].AsDouble();
                double lon = root[_T("lon")].AsDouble();
                if( NodeNr > TRACKPOINT_ONE ){
                    /*if more than one track point, compute total lenght*/
                    const double deltaLat = m_oldtpLat - lat;
                    if ( fabs( deltaLat ) > OFFSET_LAT )
                        m_gTrkLenght += DistGreatCircle_Plugin( m_oldtpLat, m_oldtpLon, lat, lon );
                    else
                        m_gTrkLenght += DistGreatCircle_Plugin( m_oldtpLat + copysign( OFFSET_LAT,
                                    deltaLat ), m_oldtpLon, lat, lon );
                }
                m_oldtpLat = lat;
                m_oldtpLon = lon;
            } // NodeNr >= m_gNodeNbr
            if( NodeNr == TotalNodes ) {
                /*if the last created track point have been reached
                 * 1)if it not the first time and no new track point has been created,
                 * compute distance from the last point
                 * 2) if we are on first track point and either it's the first time
                 * or no movement have been made, change continuously the starting time
                 * except if we are in rotation situation
                 * 3) update the trip data and eventually re-start the lenght timer for
                 * the next lap;*/

                double dist = 0.;
                if( m_gNodeNbr > TotalNodes ){
                    const double deltaLat = m_oldtpLat - m_gLat;
                    if ( fabs( deltaLat ) > OFFSET_LAT )
                        dist = DistGreatCircle_Plugin( m_oldtpLat, m_oldtpLon, m_gLat, m_gLon );
                    else
                        dist = DistGreatCircle_Plugin( m_oldtpLat + copysign( OFFSET_LAT, deltaLat ), m_oldtpLon,  m_gLat, m_gLon );
                    if( dist < .001 ) dist = 0.; //no movement
                }// m_gNodeNbr > TotalNodes
                if( TotalNodes == TRACKPOINT_ONE ){
                    if( !m_gHasRotated )
                        if( dist == 0. ) //no movement
                            m_gTrkStart = wxDateTime::Now();
                } else
                    m_gHasRotated = false;
                m_gTrkRunning = wxDateTime::Now() - m_gTrkStart;
                m_gNodeNbr = NodeNr + 1;
                if( m_pTable )
                    m_pTable->UpdateTripData( m_gTrkStart, (dist + m_gTrkLenght + m_gRotateLenght), m_gTrkRunning );
                if( m_pTable ||  m_gHasRotated )
                    m_lenghtTimer.Start( TIMER_INTERVAL_SECOND, wxTIMER_ONE_SHOT );
            }//NodeNr == TotalNodes
        }
    }
    else if(message_id == _T("OCPN_RTE_ACTIVATED"))
    {
        // construct the JSON root object
        wxJSONValue  root;
        // construct a JSON parser
        wxJSONReader reader;
        // now read the JSON text and store it in the 'root' structure
        // check for errors before retreiving values...
        int rnumErrors = reader.Parse( message_body, &root );
        if ( rnumErrors == 0 )  {
            // get route GUID values from the JSON message
            m_ActiveRouteGuid = root[_T("GUID")].AsString();
            wxString active, toggled, inactive;
            if( GetSVGFileIcons( active, toggled, inactive ))
                SetToolbarToolBitmapsSVG(m_leftclick_tool_id, active,
                                     active, toggled );
            else
                SetToolbarToolBitmaps(m_leftclick_tool_id, _img_active,
                                     _img_toggled );
        }
    }

    else if(message_id == _T("OCPN_TRK_ACTIVATED"))
    {
        wxJSONValue  root;
        wxJSONReader reader;
        int rnumErrors = reader.Parse( message_body, &root );
        if ( rnumErrors == 0 )  {
            m_gTrkGuid = root[_T("GUID")].AsString();
            m_gNodeNbr = 0;
            if( m_pTable )
                m_pTable->UpdateTripData();
            if( m_gMustRotate ){
                m_gRotateLenght += m_gTrkLenght;
                m_gHasRotated = true;
            } else {
                m_gHasRotated = false;
                m_gRotateLenght = 0.;
            }
            m_gTrkLenght = 0.;
            m_gMustRotate = false;
            m_lenghtTimer.Start( TIMER_INTERVAL_MSECOND, wxTIMER_ONE_SHOT );
            m_rotateTimer.Start( TIMER_INTERVAL_10SECOND, wxTIMER_ONE_SHOT);
        }
    }

    else if(message_id == _T("OCPN_WPT_ACTIVATED"))
    {
        wxJSONValue  p1root;
        wxJSONReader p1reader;
        int pnumErrors = p1reader.Parse( message_body, &p1root );
        if ( pnumErrors == 0 ){
            m_ActivePointGuid = p1root[_T("GUID")].AsString();
            if( m_pTable )
                m_pTable->UpdateRouteData( m_ActiveRouteGuid, m_ActivePointGuid, m_gLat, m_gLon, m_gCog, m_gSog );
        }
    }

    else if(message_id == _T("OCPN_WPT_ARRIVED"))
    {
        wxJSONValue  p2root;
        wxJSONReader p2reader;
        int p2numErrors = p2reader.Parse( message_body, &p2root );
        if ( p2numErrors == 0 ) {
            if( p2root.HasMember(_T("Next_WP"))){
                m_ActivePointGuid = p2root[_T("GUID")].AsString();
                if( m_pTable )
                    m_pTable->UpdateRouteData( m_ActiveRouteGuid, m_ActivePointGuid, m_gLat, m_gLon, m_gCog, m_gSog );
            }
        }

    }

    else if(message_id == _T("OCPN_RTE_DEACTIVATED") || message_id == _T("OCPN_RTE_ENDED") )
    {
        m_ActivePointGuid = wxEmptyString;
        m_ActiveRouteGuid = wxEmptyString;
        //SetToolbarItemState( m_leftclick_tool_id, false );
        wxString active, toggled, inactive;
        if( GetSVGFileIcons( active, toggled, inactive ))
            SetToolbarToolBitmapsSVG(m_leftclick_tool_id, inactive,
                                 inactive, inactive );
        else
            SetToolbarToolBitmaps(m_leftclick_tool_id, _img_inactive,
                                 _img_inactive );
        if(m_pTable)
            CloseDataTable();
    }

    else if(message_id == _T("OCPN_TRK_DEACTIVATED"))
    {
        m_gTrkGuid = wxEmptyString;
        m_rotateTimer.Stop();
        if( m_pTable )
                m_pTable->UpdateTripData();
    }

    else if(message_id == _T("WMM_VARIATION_BOAT"))
    {
        wxJSONValue  root;
        wxJSONReader reader;
        int numErrors = reader.Parse( message_body, &root );
        if ( numErrors == 0 )
            m_gWmmVar = root[_T("Decl")].AsDouble();
    }
}

void navdata_pi::SetPositionFix(PlugIn_Position_Fix &pfix)
{
    //update route data
    m_gLat = pfix.Lat;
    m_gLon = pfix.Lon;
    m_gCog = pfix.Cog;
    m_gSog = pfix.Sog;
    if( m_pTable ){
        m_pTable->UpdateRouteData( m_ActiveRouteGuid,
                  m_ActivePointGuid, m_gLat, m_gLon, m_gCog, m_gSog );
    }
}

void navdata_pi::OnTripLenghtTimer( wxTimerEvent & event)
{
    if( m_gTrkGuid == wxEmptyString ) return;
    wxJSONValue v;
    v[_T("Track_ID")] = m_gTrkGuid;
    wxJSONWriter w;
    wxString out;
    w.Write(v, out);
    SendPluginMessage( _T("OCPN_TRACK_REQUEST"), out );
}

void navdata_pi::OnRotateTimer( wxTimerEvent & event)
{
    int rotateTime, rotateTimeType;
    if( !GetOcpnDailyTrack( &rotateTime, &rotateTimeType ) ) return;

    /* find when the track rotation will happen by first starting timer for an hour, then
     * for a second until 10 s before rotation time then eventually start lenght
     * timer to complete computation before the current track was canceled by the rotation
     * process. this will allow to keep the whole trip data even if the rotation happened*/

    size_t nexRotate;
    time_t now = wxDateTime::Now().GetTicks();
    time_t today = wxDateTime::Today().GetTicks();
    int rotate_at = 0;
    switch( rotateTimeType )
    {
        case TIME_TYPE_LMT:
            rotate_at = rotateTime + wxRound(m_gLon * 3600. / 15.);
            break;
        case TIME_TYPE_COMPUTER:
            rotate_at = rotateTime;
            break;
        case TIME_TYPE_UTC:
            int utc_offset = wxDateTime::Now().GetTicks() - wxDateTime::Now().ToUTC().GetTicks();
            rotate_at = rotateTime + utc_offset;
            break;
    }
    if( rotate_at > 86400 )
        rotate_at -= 86400;
    else if (rotate_at < 0 )
        rotate_at += 86400;

    if( now - today > rotate_at)
        nexRotate = today + rotate_at + 86400;
    else
        nexRotate = today + rotate_at;

    time_t to_rotate = (nexRotate - now) * 1000;

    if( to_rotate >  TIMER_INTERVAL_HOUR + TIMER_INTERVAL_10SECOND )
        m_rotateTimer.Start( TIMER_INTERVAL_HOUR, wxTIMER_ONE_SHOT );
    else if( to_rotate > TIMER_INTERVAL_10SECOND )
        m_rotateTimer.Start( TIMER_INTERVAL_SECOND, wxTIMER_ONE_SHOT );
    else {
        m_gMustRotate = true;
        if( !m_pTable )
            m_lenghtTimer.Start( TIMER_INTERVAL_MSECOND, wxTIMER_ONE_SHOT );
    }
}

bool navdata_pi::GetSVGFileIcons( wxString& active, wxString& toggled, wxString& inactive )
{
    //find share path
    wxString shareLocn =*GetpSharedDataLocation() +
                _T("plugins") + wxFileName::GetPathSeparator() +
                _T("navdata_pi") + wxFileName::GetPathSeparator()
                +_T("data") + wxFileName::GetPathSeparator();
    //find svg files
    active = shareLocn + _T("active.svg");
    toggled = shareLocn + _T("toggled.svg");
    inactive = shareLocn + _T("inactive.svg");
    if(wxFile::Exists( active) && wxFile::Exists( toggled )
            && wxFile::Exists( inactive ) )
        return true;

    return false;
}

double navdata_pi::GetMag(double a)
{
    if(!std::isnan(m_gWmmVar)) {
        if((a - m_gWmmVar) >360.)
            return (a - m_gWmmVar - 360.);
        else
            return ((a - m_gWmmVar) >= 0.) ? (a - m_gWmmVar) : (a - m_gWmmVar + 360.);
    }
    else {
        if((a - m_ocpnUserVar) >360.)
            return (a - m_ocpnUserVar - 360.);
        else
            return ((a - m_ocpnUserVar) >= 0.) ? (a - m_ocpnUserVar) : (a - m_ocpnUserVar + 360.);
    }
}

void navdata_pi::OnToolbarToolCallback(int id)
{

    if( m_ActiveRouteGuid == wxEmptyString ) {
        OCPNMessageBox_PlugIn(m_pParentWin, _("There is no Active Route!\nYou must active one before using this fonctionality"), _("Warning!"), wxICON_WARNING|wxOK, 100, 50 );
        SetToolbarItemState( m_leftclick_tool_id, false );
        return;
     }

    if( m_pTable ){
        CloseDataTable();
    } else {
    SetToolbarItemState( m_leftclick_tool_id, true );

    LoadocpnConfig();

    long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER;
    m_pTable = new DataTable(m_pParentWin, wxID_ANY, wxEmptyString, wxDefaultPosition,
                           wxDefaultSize, style, this );
    wxFont font = GetOCPNGUIScaledFont_PlugIn(_T("Dialog"));
    SetDialogFont( m_pTable, &font );//Apply global font
    DimeWindow( m_pTable ); //apply colour sheme
    m_pTable->InitDataTable();
    m_pTable->UpdateRouteData( m_ActiveRouteGuid, m_ActivePointGuid, m_gLat, m_gLon, m_gCog, m_gSog );
    m_pTable->UpdateTripData();
    m_pTable->SetTableSizePosition();
    m_pTable->Show();
    if( !m_gTrkGuid.IsEmpty() )
        m_lenghtTimer.Start( TIMER_INTERVAL_MSECOND, wxTIMER_ONE_SHOT );
    }
}

bool navdata_pi::GetOcpnDailyTrack( int *roTime, int *rotimeType)
{
    bool isdailytrack;
    int rtime, rtimetype;
    wxFileConfig *pConf = GetOCPNConfigObject();
    if(pConf)
    {
        pConf->SetPath(_T("/Settings"));
        pConf->Read(_T("AutomaticDailyTracks"), &isdailytrack, 0);
        if( !isdailytrack ) return false;
        pConf->Read( _T ( "TrackRotateAt" ), &rtime, 0 );
        pConf->Read( _T ( "TrackRotateTimeType" ), &rtimetype, 1);
        *roTime = rtime;
        *rotimeType = rtimetype;

        return true;
    }
    return false;
}

void navdata_pi::SetDialogFont( wxWindow *dialog, wxFont *font)
{
    dialog->SetFont( *font );
    wxWindowList list = dialog->GetChildren();
    wxWindowListNode *node = list.GetFirst();
    for( size_t i = 0; i < list.GetCount(); i++ ) {
        wxWindow *win = node->GetData();
        win->SetFont( *font );
        node = node->GetNext();
    }
}

void navdata_pi::CloseDataTable()
{
    SetToolbarItemState( m_leftclick_tool_id, false );
    if( m_pTable ) {
        m_pTable->CloseDialog();
        delete m_pTable;
        m_pTable = NULL;
    }
}
