/******************************************************************************
 *
 * Project:  OpenCPN - plugin navdata_pi
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

#include "GL/gl.h"

#include "jsonreader.h"
#include "jsonwriter.h"

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
wxString       g_shareLocn;
wxString       g_activeRouteGuid;
wxString       g_activePointGuid;
wxString       g_selectedPointGuid;
double         g_Lat;
double         g_Lon;
double         g_Cog;
double         g_Sog;
int            g_ocpnDistFormat;
int            g_ocpnSpeedFormat;
wxColour       g_labelColour;
wxColour       g_valueColour;
wxFont         g_labelFont;
wxFont         g_valueFont;


int NextPow2(int size)
{
    int n = size-1;          // compute dimensions needed as next larger power of 2
    int shift = 1;
    while ((n+1) & n){
        n |= n >> shift;
        shift <<= 1;
    }
    return n + 1;
}

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
    delete _img_inactive;
    delete _img_toggled;
    delete _img_target;
    delete _img_activewpt;
    delete _img_targetwpt;
    delete _img_setting;
    delete m_vp[0];
    delete m_vp[1];
 }

int navdata_pi::Init(void){
    //connect timers
    m_lenghtTimer.Connect(wxEVT_TIMER, wxTimerEventHandler(navdata_pi::OnTripLenghtTimer), NULL, this );
    m_rotateTimer.Connect(wxEVT_TIMER, wxTimerEventHandler(navdata_pi::OnRotateTimer), NULL, this );

    AddLocaleCatalog( _T("opencpn-navdata_pi") );

    m_pTable = NULL;
    m_ptripData = NULL;
    m_console = NULL;
    m_selectablePoint = false;
    g_selectedPointGuid = wxEmptyString;
    m_activeTrkGuid = wxEmptyString;
    g_activeRouteGuid = wxEmptyString;
    m_nearRotate = false;
    m_hasRotated = false;
    m_blinkTrigger = 0;
    //allow multi-canvas
    m_vp[0] = new PlugIn_ViewPort;
    if(GetCanvasCount() > 1)
        m_vp[1] = new PlugIn_ViewPort;
    else
        m_vp[1] = NULL;

    //to do: get it from style
    m_defLabelColor.Set( 50, 240, 50);

    //find and store share path
    g_shareLocn = *GetpSharedDataLocation() +
                    _T("plugins") + wxFileName::GetPathSeparator() +
                    _T("navdata_pi") + wxFileName::GetPathSeparator()
                    +_T("data") + wxFileName::GetPathSeparator();

    //    This PlugIn needs a toolbar icon, so request its insertion
    wxString active = g_shareLocn + _T("active.svg");
    wxString toggled = g_shareLocn + _T("toggled.svg");
    if( wxFile::Exists( active) && wxFile::Exists( toggled) )
        m_leftclick_tool_id  = InsertPlugInToolSVG(_T(""), active, active, toggled,
                    wxITEM_CHECK, _("Navigation data"), _T(""), NULL, CALCULATOR_TOOL_POSITION, 0, this);
    else
    m_leftclick_tool_id  = InsertPlugInTool(_T(""), _img_active, _img_toggled,
                    wxITEM_CHECK, _("Navigation data"), _T(""), NULL, CALCULATOR_TOOL_POSITION, 0, this);

    return (WANTS_OVERLAY_CALLBACK          |
            WANTS_ONPAINT_VIEWPORT          |
            WANTS_OPENGL_OVERLAY_CALLBACK   |
            WANTS_TOOLBAR_CALLBACK          |
            INSTALLS_TOOLBAR_TOOL           |
            WANTS_PREFERENCES               |
            WANTS_PLUGIN_MESSAGING          |
            WANTS_NMEA_EVENTS               |
            WANTS_CONFIG                    |
            WANTS_MOUSE_EVENTS
           );
}

bool navdata_pi::DeInit(void)
{
    //stop and disconnect timers
    m_rotateTimer.Stop();
    m_lenghtTimer.Stop();
    m_lenghtTimer.Disconnect(wxEVT_TIMER, wxTimerEventHandler(navdata_pi::OnTripLenghtTimer), NULL, this );
    m_rotateTimer.Disconnect(wxEVT_TIMER, wxTimerEventHandler(navdata_pi::OnRotateTimer), NULL, this );

    //save RoutePointconsole position
    wxFileConfig *pConf = GetOCPNConfigObject();
    if(pConf) {
        pConf->SetPath ( _T ( "/Settings/NAVDATA" ) );
        pConf->Write(_T("NavDataConsolePosition_x"), m_consPosition.x);
        pConf->Write(_T("NavDataConsolePosition_Y"), m_consPosition.y);
    }

    delete m_console;
    m_console = NULL;

    if(m_ptripData){
        if(!m_ptripData->m_isEnded){
            m_ptripData->m_totalDist += GetDistFromLastTrkPoint(g_Lat, g_Lon);
            m_ptripData->m_isEnded = true;
            m_ptripData->m_endTime = wxDateTime::Now();
            //wxString s = wxString::Format(wxString(_T("Do you want to save this trip\nlenght = %f")), m_ptripData->m_totalDist);
            //OCPNMessageBox_PlugIn(GetNAVCanvas(),s, _T("answer"));
        }
        delete m_ptripData;
        m_ptripData = NULL;
    }

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
      return _T("NAVDATA");
}

wxString navdata_pi::GetShortDescription()
{
      return _("Navigation data plugin for OpenCPN");
}

wxString navdata_pi::GetLongDescription()
{
      return _("Navigation data plugin for OpenCPN\nShows RNG (range),TTG (time to go) and ETA (estimated time of arrival)\nto a selected route point of the active route.\nAlso shows current trip summary:\nstart time, time spent, distance, mean speed since departure...");
}

int navdata_pi::GetToolbarToolCount(void)
{
      return 1;
}

void navdata_pi::SetColorScheme(PI_ColorScheme cs)
{
    if(m_pTable)
        m_pTable->SetColorScheme();
    if(m_console)
        m_console->SetColorScheme();
}

void navdata_pi::SetPluginMessage(wxString &message_id, wxString &message_body)
{
    if(message_id == _T("OCPN_TRACKPOINTS_COORDS"))
    {
		//compute the lenght of the track if requested by "m_lenghtTimer"
        wxJSONValue  root;
        wxJSONReader reader;
        int rnumErrors = reader.Parse( message_body, &root );
        if ( rnumErrors > 0 ) return;
        if( root[_T("error")].AsBool() ) return;

        if( root[_T("Track_ID")].AsString() != m_activeTrkGuid )
            return;

        int NodeNr = root[_T("NodeNr")].AsInt();
        if( NodeNr > m_gNodeNbr ) {//skip track point already treated
            double lat = root[_T("lat")].AsDouble();
            double lon = root[_T("lon")].AsDouble();
            if( NodeNr > TRACKPOINT_FIRST ) {//more than one track point
                m_ptripData->m_tempDist = 0;
                m_ptripData->m_totalDist += GetDistFromLastTrkPoint(lat, lon);
            }
            m_oldtpLat = lat;
            m_oldtpLon = lon;
            m_gNodeNbr = NodeNr;
        }// NodeNr >= m_gNodeNbr

        int TotalNodes = root[_T("TotalNodes")].AsInt();
        if( NodeNr != TotalNodes )
            return;

        if( m_ptripData->m_isEnded ){ //end of Trip
            m_activeTrkGuid = wxEmptyString;
            return;
        }

        //the last track point
        /* 1)if the boat has moved compute and add distance from the last created point
        * 2) if we are still on first track point and no movement has been made,
        *  change continuously the starting time except if we are in rotation situation
        * 3)then re-start the lenght timer for the next lap;*/
        double dist = 0.;
        if(m_oldtpLat != g_Lat || m_oldtpLon != g_Lon){ //the boat has moved
            dist = GetDistFromLastTrkPoint(g_Lat, g_Lon);
            if( dist < .001 ) //no significant movement
                dist = 0.;
        } //
        if( TotalNodes == TRACKPOINT_FIRST ){ //only one point created
            if (!m_hasRotated) {
                if (dist == 0.) //no movement
                    m_ptripData->m_startDate = wxDateTime::Now();
                else
                    m_ptripData->m_isStarted = true;
            } else
                m_hasRotated = false;
        }

        m_ptripData->m_tempDist = dist;

        //try to keep performance by reducing calc frequency
        double timerInterval = TotalNodes < 500? INTERVAL_1SECOND: wxMin((TotalNodes / 0.5), 6000);
        m_lenghtTimer.Start(timerInterval, wxTIMER_ONE_SHOT);
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
            g_activeRouteGuid = root[_T("GUID")].AsString();
        }
    }

    else if(message_id == _T("OCPN_TRK_ACTIVATED"))
    {
        /*when closing the track properties dialog, ocpn send an "OCPN_TRK_ACTIVATED"
         * message. In this case we must ignore it. Nevertheless I think it's a logic error
         *  and must be corrected*/
        wxString guid = m_activeTrkGuid;
        if( m_ptripData && !m_ptripData->m_isEnded && !m_nearRotate )
            return;
        wxJSONValue  root;
        wxJSONReader reader;
        int rnumErrors = reader.Parse( message_body, &root );
        if ( rnumErrors == 0 )  {
            m_activeTrkGuid = root[_T("GUID")].AsString();

            bool newtrip = false;
            if(m_isDailyTrack){
                if(m_nearRotate){ //this is not a "real" trackOn but a rotation
                    if( guid != m_activeTrkGuid ) //it should never happens but...
                        m_gNodeNbr = 0;
                    m_hasRotated = true;
                    m_nearRotate = false;
                } else { //this is a "real" trackOn
                    newtrip = true;
                     m_rotateTimer.Start( INTERVAL_10SECOND, wxTIMER_ONE_SHOT);
                }
            } else //this is a "real" trackOn
                newtrip = true;

            if(newtrip) {
                m_gNodeNbr = 0;
                m_nearRotate = false;
                if( m_ptripData ) {
                    delete m_ptripData;
                    m_ptripData = NULL;
                }
                m_ptripData = new TripData();
                if( m_pTable )
                    m_pTable->UpdateTripData();
            }
            m_lenghtTimer.Start( INTERVAL_10MSECOND, wxTIMER_ONE_SHOT );
        }
    }

    else if(message_id == _T("OCPN_WPT_ACTIVATED"))
    {
        wxJSONValue  p1root;
        wxJSONReader p1reader;
        int pnumErrors = p1reader.Parse( message_body, &p1root );
        if ( pnumErrors == 0 ){
            g_activePointGuid = p1root[_T("GUID")].AsString();
            if( m_console && m_console->IsShown() )
                CheckRoutePointSelectable();
        }
    }

    else if(message_id == _T("OCPN_WPT_ARRIVED"))
    {
        wxJSONValue  p2root;
        wxJSONReader p2reader;
        int p2numErrors = p2reader.Parse( message_body, &p2root );
        if ( p2numErrors == 0 ) {
            if( p2root.HasMember(_T("Next_WP"))){
                g_activePointGuid = p2root[_T("GUID")].AsString();
                if( m_console && m_console->IsShown() )
                    CheckRoutePointSelectable();
            }
        }
    }

    else if(message_id == _T("OCPN_RTE_DEACTIVATED") || message_id == _T("OCPN_RTE_ENDED") )
    {
        m_selectablePoint = false;
        g_selectedPointGuid = wxEmptyString;
        g_activePointGuid = wxEmptyString;
        g_activeRouteGuid = wxEmptyString;
        delete m_console;
        m_console = NULL;
    }

    else if(message_id == _T("OCPN_TRK_DEACTIVATED"))
    {
        m_lenghtTimer.Stop();

        if(m_isDailyTrack){
            /*when "DailyTrack" is set it is impossible to get the last track point as
            * the GUI of the extended track is unknown so to complete the trip lenght
            * we have to get the distance from the last created point to the current boat position*/
            m_ptripData->m_tempDist = 0;
            m_ptripData->m_totalDist += GetDistFromLastTrkPoint( g_Lat, g_Lon );
            if(m_nearRotate)//this is not a "real" trackOff but a rotation
                return;
            else { //this is real trackOff so close the current Trip
                m_activeTrkGuid = wxEmptyString;
                m_rotateTimer.Stop();
            }
        }
        m_ptripData->m_isEnded = true;
        m_ptripData->m_endTime = wxDateTime::Now();
        //get the last created track point to complete trip lenght
        if(!m_isDailyTrack)//
            m_lenghtTimer.Start(INTERVAL_10MSECOND, wxTIMER_ONE_SHOT);
    }

    else if(message_id == _T("OpenCPN Config"))
    {
        LoadocpnConfig();
    }
}

void navdata_pi::LoadocpnConfig()
{
    wxFileConfig *pConf = GetOCPNConfigObject();
    if(pConf)
    {
        float radiusPixel;
        pConf->SetPath(_T("/Settings"));
        pConf->Read(_T("DistanceFormat"), &g_ocpnDistFormat, 0);
        pConf->Read(_T("SpeedFormat"), &g_ocpnSpeedFormat, 0);
        pConf->Read(_T("AutomaticDailyTracks"), &m_isDailyTrack, 0);
        pConf->Read( _T ( "TrackRotateAt" ), &m_trackRotateAt, 0 );
        pConf->Read( _T ( "TrackRotateTimeType" ), &m_trackRotateTimeType, 1);
        if(IsTouchInterface_PlugIn()){
            pConf->Read( _T ("SelectionRadiusTouchMM"), &radiusPixel);
            m_ocpnSelRadiusMM = wxMax(radiusPixel, 1.0);
        } else {
            pConf->Read( _T ("SelectionRadiusMM"), &radiusPixel);
            m_ocpnSelRadiusMM = wxMax(radiusPixel, 0.5);
        }
    }
    //if the "daily track" parameter has changed it is necessary to start or stop timer
    if(m_isDailyTrack && !m_activeTrkGuid.IsEmpty()){
        if(!m_rotateTimer.IsRunning())
            m_rotateTimer.Start( INTERVAL_10SECOND, wxTIMER_ONE_SHOT);
    } else
        m_rotateTimer.Stop();

    bool changeFont = false, changeColor = false;
    wxFont  lfont = *OCPNGetFont( _("Console Legend"), 0);
    if( g_labelFont != lfont ) {
        changeFont = true;
        g_labelFont = lfont;
    }
    wxFont  vfont = *OCPNGetFont(_("Console Value"), 0);
    if( g_valueFont != vfont ){
        changeFont = true;
        g_valueFont = vfont;
    }
    wxColour back_color;
    GetGlobalColor(_T("UBLCK"), &back_color);
    wxColour lcol = GetFontColour_PlugIn( _("Console Legend") );
    if( g_labelColour != lcol ) {
        wxColor ncol = lcol;
        if( (abs(ncol.Red() - back_color.Red()) < 5) &&
                    (abs(ncol.Green() - back_color.Blue()) < 5) &&
                    (abs(ncol.Blue() - back_color.Blue()) < 5)) {
            if(g_labelColour != m_defLabelColor) {
                g_labelColour = m_defLabelColor;
                changeColor = true;
            }
        } else {
            g_labelColour = lcol;
            changeColor = true;
        }

    }
    wxColour vcol = GetFontColour_PlugIn( _("Console Value") );
    if( g_valueColour != vcol ) {
        wxColor mcol = vcol;
        if( (abs(mcol.Red() - back_color.Red()) < 5) &&
                (abs(mcol.Green() - back_color.Blue()) < 5) &&
                (abs(mcol.Blue() - back_color.Blue()) < 5)) {
            if(g_valueColour != m_defLabelColor) {
                g_valueColour = m_defLabelColor;
                changeColor = true;
            }
        } else {
            g_valueColour = vcol;
            changeColor = true;
        }
    }
    if(changeColor)
        if( m_pTable ) m_pTable->SetColorScheme();

    if(changeFont){
        if( m_pTable ){
            m_pTable->SetTripDialogFont();
            m_pTable->SetTableSizePosition();
        }
        if(m_console) m_console->UpdateFonts();
    }

    return;
}

void navdata_pi::CheckRoutePointSelectable()
{
    //check if the selected point is still after the new active point
    if( !m_selectablePoint ) return;
    bool past = false, selectable;
    std::unique_ptr<PlugIn_Route> r;
    r = GetRoute_Plugin( g_activeRouteGuid );
    wxPlugin_WaypointListNode *node = r.get()->pWaypointList->GetFirst();
    while( node ){
        PlugIn_Waypoint *wpt = node->GetData();
        if(wpt) {
            selectable = past;
            if( wpt->m_GUID == g_activePointGuid )
                past = true;
            if( wpt->m_GUID == g_selectedPointGuid ){
                if( !selectable ){
                    m_console->Show(false);
                    g_selectedPointGuid = wxEmptyString;
                    m_selectablePoint = false;
                }
                break;
            }
        }
        node = node->GetNext();
    }

}

double navdata_pi::GetDistFromLastTrkPoint(double lat, double lon)
{
    double dist = 0.;
        const double deltaLat = m_oldtpLat - lat;
        if ( fabs( deltaLat ) > OFFSET_LAT )
            dist = DistGreatCircle_Plugin( m_oldtpLat, m_oldtpLon, lat, lon );
        else
            dist = DistGreatCircle_Plugin( m_oldtpLat + copysign( OFFSET_LAT, deltaLat ), m_oldtpLon,  lat, lon );

        return dist;
}

void navdata_pi::SetPositionFix(PlugIn_Position_Fix &pfix)
{
    static int new_canvas_nbr = GetCanvasCount();
    g_Lat = pfix.Lat;
    g_Lon = pfix.Lon;
    g_Cog = pfix.Cog;
    g_Sog = pfix.Sog;
    m_blinkTrigger++;
    //CheckFontColourChange();
    //when the canvas number has changed, do nothing, this could create a crash
    if(GetCanvasCount() == new_canvas_nbr ){
        if( (m_console && m_console->IsShown()))
            m_console->UpdateRouteData();
        if ( m_pTable )
            m_pTable->UpdateTripData(m_ptripData);
    } else {
        new_canvas_nbr = GetCanvasCount();
        delete m_vp[1];
        if(new_canvas_nbr > 1){ //initialise view port for right canvas
            m_vp[1] = new PlugIn_ViewPort;
            GetCanvasByIndex(1)->Refresh();
        } else                  //close vp for right canvas
            m_vp[1] = NULL;
    }
}

bool navdata_pi::MouseEventHook( wxMouseEvent &event )
{
    if( g_activeRouteGuid.IsEmpty() )
        return false;
    if( event.Dragging() ){
        m_blinkTrigger = 0;
        return false;
    }

    if(IsTouchInterface_PlugIn()){
        if( !event.LeftUp() )
            return false;
    } else {
        if( !event.LeftDown() )
            return false;
    }
    if( !m_vp[GetCanvasIndexUnderMouse()] )
        return false;
    wxPoint p = event.GetPosition();
    double plat, plon;
    GetCanvasLLPix( m_vp[GetCanvasIndexUnderMouse()], p, &plat, &plon);
    float selectRadius = GetSelectRadius( m_vp[GetCanvasIndexUnderMouse()] );
    double dist_from_cursor = IDLE_STATE_NUMBER;
    /*walk along the route to find the nearest point guid from the cursor
     * way point visibility parameter unuseful here is use to
     * store if the selected is before or after the active point*/
    wxString SelGuid = wxEmptyString;
    wxString pointLabel;
    bool past = false;
    std::unique_ptr<PlugIn_Route> r;
    r = GetRoute_Plugin( g_activeRouteGuid );
    wxPlugin_WaypointListNode *node = r.get()->pWaypointList->GetFirst();
    while( node ){
        PlugIn_Waypoint *wpt = node->GetData();
        if(wpt) {
            wpt->m_IsVisible = past;
            if( wpt->m_GUID == g_activePointGuid )
                past = true;
            double of_lat = fabs(plat - wpt->m_lat);
            double of_lon = fabs(plon - wpt->m_lon);
            if( (of_lat < selectRadius) &&  (of_lon < selectRadius) ){
                double dis = sqrt( pow(of_lat,2) + pow(of_lon,2) ) ;
                if( dis < dist_from_cursor ) {
                    SelGuid = wpt->m_GUID;
                    pointLabel = wpt->m_MarkName;
                    m_selectablePoint = wpt->m_IsVisible;
                    dist_from_cursor = dis;
                }
            }
        }
        node = node->GetNext();
    }
    if(SelGuid.IsEmpty())
        return false;

    if(g_selectedPointGuid != SelGuid){
        g_selectedPointGuid = SelGuid;
        if( m_selectablePoint ){
            m_blinkTrigger = 1;
            for( int i = 0; i < GetCanvasCount(); i++ ){
                GetCanvasByIndex(i)->Refresh();
            }
            if( !m_console )
                m_console = new RouteCanvas( GetOCPNCanvasWindow(), this);
            m_console->m_pointName = _T("-->") + pointLabel;
            m_console->ShowWithFreshFonts();
            m_console->SetColorScheme();

        } else {
            if( m_console )
                m_console->Show(false);
        }
        return m_selectablePoint;
    }
    return false;
}

float navdata_pi::GetSelectRadius(PlugIn_ViewPort *vp)
{
    int w, h;
    ::wxDisplaySize(&w, &h);
    unsigned int radiusPixel = (w / PlugInGetDisplaySizeMM()) * m_ocpnSelRadiusMM;
    double  canvasScaleFactor = wxMax( w, h) / (PlugInGetDisplaySizeMM() / 1000.);
    double trueScalePPM = canvasScaleFactor / vp->chart_scale;
    float selectRadius =  radiusPixel / (trueScalePPM * 1852 * 60);
    return selectRadius;
}

bool navdata_pi::RenderOverlayMultiCanvas( wxDC &dc, PlugIn_ViewPort *vp, int canvasIndex)
{
    if(m_vp[canvasIndex])
        *m_vp[canvasIndex] = *vp;

    if(!m_selectablePoint)
        return false;

    wxDC *pdc = (&dc);      //inform render of non GL mode

    return RenderTargetPoint( pdc , vp);

}

bool navdata_pi::RenderGLOverlayMultiCanvas( wxGLContext *pcontext, PlugIn_ViewPort *vp, int canvasIndex)
{
    if(m_vp[canvasIndex])
        *m_vp[canvasIndex] = *vp;

    if(!m_selectablePoint)
        return false;

    return RenderTargetPoint( NULL , vp );		 //NULL inform renderer of GL mode
}

bool navdata_pi::RenderTargetPoint( wxDC *pdc, PlugIn_ViewPort *vp )
{
    if( !vp ) return false;
    if( m_blinkTrigger & 1 || pdc ) {
        //way point position
        std::unique_ptr<PlugIn_Waypoint> rp;
        rp = GetWaypoint_Plugin(g_selectedPointGuid);
        if( !rp ) return false;
        wxPoint p;
        GetCanvasPixLL( vp, &p, rp.get()->m_lat, rp.get()->m_lon );
        //icon image
        float scale =  GetOCPNChartScaleFactor_Plugin();
        wxImage image;
        wxBitmap bmp;
        int  w = _img_target->GetWidth() * scale;
        int  h = _img_target->GetHeight() * scale;
        int px = p.x - w/2;
        int py = p.y - h/2;
        wxString file = g_shareLocn + _T("target.svg");
        if( wxFile::Exists( file ) ){
            bmp = GetBitmapFromSVGFile( file, w, h);
        } else {
            wxImage im = _img_target->ConvertToImage();
            wxBitmap bmp = wxBitmap(im.Scale( w, h) );
        }
        image = bmp.ConvertToImage();
        //control image
        unsigned char *d = image.GetData();
        if (d == 0)
            return false;
        //draw
        if( pdc ){                // no GL
            pdc->DrawBitmap( image, px, py );		//Don't work properly in this mode! (no blinking)
        }
#ifdef ocpnUSE_SVG
        else {                    // GL
            //create texture
            unsigned int IconTexture;
            glGenTextures(1, &IconTexture);
            glBindTexture(GL_TEXTURE_2D, IconTexture);

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

            unsigned char *a = image.GetAlpha();
            unsigned char mr, mg, mb;
            if(!a)
                image.GetOrFindMaskColour( &mr, &mg, &mb );
            unsigned char *e = new unsigned char[4 * w * h];
            for( int y = 0; y < h; y++ ) {
                for( int x = 0; x < w; x++ ) {
                    unsigned char r, g, b;
                    int off = ( y * w + x );
                    r = d[off * 3 + 0];
                    g = d[off * 3 + 1];
                    b = d[off * 3 + 2];
                    e[off * 4 + 0] = r;
                    e[off * 4 + 1] = g;
                    e[off * 4 + 2] = b;
                    e[off * 4 + 3] =  a ? a[off] : ( ( r == mr ) && ( g == mg ) && ( b == mb ) ? 0 : 255 );
                }
            }
            unsigned int glw = NextPow2(w);
            unsigned int glh = NextPow2(h);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glw, glh,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h,
                    GL_RGBA, GL_UNSIGNED_BYTE, e);
            delete []e;
            //draw
            glBindTexture(GL_TEXTURE_2D, IconTexture);

            glEnable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glColor3f(1, 1, 1);

            float ws = w;
            float hs = h;
            float xs = px;
            float ys = py;
            float u = (float)w/glw, v = (float)h/glh;

            glBegin(GL_QUADS);
            glTexCoord2f(0, 0); glVertex2f(xs, ys);
            glTexCoord2f(u, 0); glVertex2f(xs+ws, ys);
            glTexCoord2f(u, v); glVertex2f(xs+ws, ys+hs);
            glTexCoord2f(0, v); glVertex2f(xs, ys+hs);
            glEnd();

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
        }
#endif
    }
    return true;
}

void navdata_pi::OnTripLenghtTimer( wxTimerEvent & event)
{
    if( m_activeTrkGuid == wxEmptyString ) return;
    wxJSONValue v;
    v[_T("Track_ID")] = m_activeTrkGuid;
    wxJSONWriter w;
    wxString out;
    w.Write(v, out);
    SendPluginMessage( _T("OCPN_TRACK_REQUEST"), out );
}

void navdata_pi::PositionConsole()
{
    m_console->Move( m_consPosition );
}

void navdata_pi::OnRotateTimer( wxTimerEvent & event)
{
    if( !m_isDailyTrack ) return;
    if( m_activeTrkGuid.IsEmpty() ) return;

    /* find when the track rotation will happen by first starting timer until 10 s before
     * rotation time set the rotation flag. This will allow to get the last track points
     * created before the current track desapear cancelles by the ocpn ratation process*/

    size_t nexRotate;
    time_t now = wxDateTime::Now().GetTicks();
    time_t today = wxDateTime::Today().GetTicks();
    int rotate_at = 0;
    switch( m_trackRotateTimeType )
    {
        case TIME_TYPE_LMT:
            rotate_at = m_trackRotateAt + wxRound(g_Lon * 3600. / 15.);
            break;
        case TIME_TYPE_COMPUTER:
            rotate_at = m_trackRotateAt;
            break;
        case TIME_TYPE_UTC:
            int utc_offset = wxDateTime::Now().GetTicks() - wxDateTime::Now().ToUTC().GetTicks();
            rotate_at = m_trackRotateAt + utc_offset;
            break;
    }
    if(rotate_at > SECONDS_PER_DAY)
        rotate_at -= SECONDS_PER_DAY;
    else if (rotate_at < 0)
        rotate_at += SECONDS_PER_DAY;

    bool nextDay = false;
    if(now - today > rotate_at){
        nexRotate = (today + rotate_at + SECONDS_PER_DAY) - now;;
        nextDay = true;
    }
    else
        nexRotate = (today + rotate_at) - now;

    nexRotate *= 1000; // convert in ms
    nexRotate -= INTERVAL_20SECOND;//20s before rotate time( in  ms )

    if(nexRotate > INTERVAL_90MN || nextDay )
        m_rotateTimer.Start( INTERVAL_HOUR, wxTIMER_ONE_SHOT );
    else if(nexRotate > 0)
        m_rotateTimer.Start( nexRotate, wxTIMER_ONE_SHOT );
    else {
        m_nearRotate = true;
        //start timer to get the last track point created before rotation
        m_lenghtTimer.Start( INTERVAL_10MSECOND, wxTIMER_ONE_SHOT );
        //start timer to get the next day ratation date
        m_rotateTimer.Start( INTERVAL_HOUR, wxTIMER_ONE_SHOT );
    }
}

void navdata_pi::OnToolbarToolCallback(int id)
{
	if (m_pTable) {
		CloseDataTable();
	}
	else {
		SetToolbarItemState(m_leftclick_tool_id, true);

        long style = wxSIMPLE_BORDER | wxCLIP_CHILDREN ;
        m_pTable = new DataTable(GetOCPNCanvasWindow(), wxID_ANY, wxEmptyString, wxDefaultPosition,
			wxDefaultSize, style, this);
        m_pTable->SetColorScheme();
        m_pTable->UpdateTripData(m_ptripData);
        m_pTable->SetTripDialogFont();
        m_pTable->SetTableSizePosition();
		m_pTable->Show();
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

//-------------------------------------------------------------------------------------------------
//                  Trip Data Implementation
//-------------------------------------------------------------------------------------------------
/*
#include <wx/listimpl.cpp>
WX_DEFINE_LIST ( TripData );
*/
TripData::TripData()
{
    m_startDate = wxDateTime::Now();
    m_endTime = wxInvalidDateTime;
    m_totalDist = 0.;
    m_tempDist = 0.;
    m_isStarted = false;
    m_isEnded = false;
}

TripData::~TripData()
{}
