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

#include "GL/gl.h"

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
wxWindow       *g_pParentWin;
wxString       g_shareLocn;
wxString       g_activeRouteGuid;
wxString       g_selectedPointGuid;
int            g_blinkTrigger;
int            g_selectedPointCol;
bool           g_showTripData;
bool           g_withSog;

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
 }

int navdata_pi::Init(void)
{
    m_lenghtTimer.Connect(wxEVT_TIMER, wxTimerEventHandler(navdata_pi::OnTripLenghtTimer), NULL, this );
    m_rotateTimer.Connect(wxEVT_TIMER, wxTimerEventHandler(navdata_pi::OnRotateTimer), NULL, this );
    m_selectTimer.Connect(wxEVT_TIMER, wxTimerEventHandler(navdata_pi::OnSelectGuidTimer), NULL, this );
    //m_selectTimer.Bind(wxEVT_TIMER, &navdata_pi::OnSelectGuidTimer, this );

    AddLocaleCatalog( _T("opencpn-navdata_pi") );

    // If multicanvas are active, render the overlay on the right canvas only
    if(GetCanvasCount() > 1)            // multi?
        g_pParentWin = GetCanvasByIndex(1);
    else
        g_pParentWin = GetOCPNCanvasWindow();

    m_pTable = NULL;

    g_selectedPointGuid = wxEmptyString;
    m_gTrkGuid = wxEmptyString;
    g_activeRouteGuid = wxEmptyString;
    m_gMustRotate = false;
    m_gHasRotated = false;
    m_gRotateLenght = 0;
    g_blinkTrigger = 0;

    //find and store share path
    g_shareLocn =*GetpSharedDataLocation() +
                    _T("plugins") + wxFileName::GetPathSeparator() +
                    _T("navdata_pi") + wxFileName::GetPathSeparator()
                    +_T("data") + wxFileName::GetPathSeparator();

    //    This PlugIn needs a toolbar icon, so request its insertion
    wxString inactive = g_shareLocn + _T("inactive.svg");
    if( wxFile::Exists( inactive) )
        m_leftclick_tool_id  = InsertPlugInToolSVG(_T(""), inactive, inactive, inactive,
                    wxITEM_CHECK, _("Nav data"), _T(""), NULL, CALCULATOR_TOOL_POSITION, 0, this);
    else
        m_leftclick_tool_id  = InsertPlugInTool(_T(""), _img_inactive, _img_inactive,
                                    wxITEM_CHECK, _("Nav data"), _T(""), NULL, CALCULATOR_TOOL_POSITION, 0, this);

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
                if( NodeNr > TRACKPOINT_FIRST ){
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
                if( TotalNodes == TRACKPOINT_FIRST ){
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
            g_activeRouteGuid = root[_T("GUID")].AsString();
            wxString active = g_shareLocn + _T("active.svg");
            wxString toggled = g_shareLocn + _T("toggled.svg");
            if( wxFile::Exists( active) && wxFile::Exists( toggled) )
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
            m_activePointGuid = p1root[_T("GUID")].AsString();
            if( m_pTable )
                m_pTable->UpdateRouteData( m_activePointGuid, m_gLat, m_gLon, m_gCog, m_gSog );
        }
    }

    else if(message_id == _T("OCPN_WPT_ARRIVED"))
    {
        wxJSONValue  p2root;
        wxJSONReader p2reader;
        int p2numErrors = p2reader.Parse( message_body, &p2root );
        if ( p2numErrors == 0 ) {
            if( p2root.HasMember(_T("Next_WP"))){
                m_activePointGuid = p2root[_T("GUID")].AsString();
                if( m_pTable )
                    m_pTable->UpdateRouteData( m_activePointGuid, m_gLat, m_gLon, m_gCog, m_gSog );
            }
        }

    }

    else if(message_id == _T("OCPN_RTE_DEACTIVATED") || message_id == _T("OCPN_RTE_ENDED") )
    {
        m_activePointGuid = wxEmptyString;
        g_activeRouteGuid = wxEmptyString;
        //SetToolbarItemState( m_leftclick_tool_id, false );
        wxString inactive = g_shareLocn + _T("inactive.svg");
        if( wxFile::Exists( inactive) )
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
    g_blinkTrigger++;
    if( m_pTable ){
        m_pTable->UpdateRouteData( m_activePointGuid, m_gLat, m_gLon, m_gCog, m_gSog );
    }
    RequestRefresh( g_pParentWin );
}

bool navdata_pi::MouseEventHook( wxMouseEvent &event )
{
    if( !event.RightDown() )
        return false;

    if( m_pTable )
       m_selectTimer.Start(10, wxTIMER_ONE_SHOT);

    return false;
}

bool navdata_pi::RenderOverlayMultiCanvas( wxDC &dc, PlugIn_ViewPort *vp, int canvasIndex)
{
    // If multicanvas are active, render the overlay on the right canvas only
    if(GetCanvasCount() > 1 && canvasIndex != 1)           // multi?
        return false;

    if( !m_pTable ) return false;
    if( g_selectedPointCol == wxNOT_FOUND ) return false;

    if( g_blinkTrigger & 1 ) {
        wxDC *pdc;
        pdc = &dc;
        float scale =  GetOCPNChartScaleFactor_Plugin();
        int  imgw = _img_target->GetWidth();
        int  imgh = _img_target->GetHeight();
        wxImage image;
        wxString file = g_shareLocn + _T("target.svg");
        if( wxFile::Exists( file ) ){
            wxBitmap bmp = GetBitmapFromSVGFile( file, imgw, imgh);
            image = bmp.ConvertToImage();
        } else
            image = _img_target->ConvertToImage();
        unsigned char *d = image.GetData();
        if (d == 0)
            return false;

        wxPoint2DDouble pp = m_pTable->GetSelPointPos();
        wxPoint r;
        GetCanvasPixLL( vp, &r, pp.m_x, pp.m_y );
        //draw
        imgw *= scale;
        imgh *= scale;
        int x = r.x - (imgw/2);
        int y = r.y - (imgh/2);
        wxBitmap scaled_Bitmap;
        scaled_Bitmap =  wxBitmap(image.Scale(imgw, imgh) );
        pdc->DrawBitmap( scaled_Bitmap, x, y );
    }
    return true;
}

bool navdata_pi::RenderGLOverlayMultiCanvas( wxGLContext *pcontext, PlugIn_ViewPort *vp, int canvasIndex)
{
    // If multicanvas are active, render the overlay on the right canvas only
    if(GetCanvasCount() > 1 && canvasIndex != 1)           // multi?
        return false;

    if( !m_pTable ) return false;
    if( g_selectedPointCol == wxNOT_FOUND ) return false;

    if( g_blinkTrigger & 1 ) {
        float scale =  GetOCPNChartScaleFactor_Plugin();
        wxImage image;
        int  imgw = _img_target->GetWidth();
        int  imgh = _img_target->GetHeight();
        wxString file = g_shareLocn + _T("target.svg");
        if( wxFile::Exists( file ) ){
            wxBitmap bmp = GetBitmapFromSVGFile( file, imgw, imgh);
            image = bmp.ConvertToImage();
        } else
            image = _img_target->ConvertToImage();
        unsigned char *d = image.GetData();
        if (d == 0)
            return false;
        wxPoint2DDouble pp = m_pTable->GetSelPointPos();
        wxPoint r;
        GetCanvasPixLL( vp, &r, pp.m_x, pp.m_y );
        //create texture
        int w = image.GetWidth(), h = image.GetHeight();
        wxRect r1 = wxRect( r.x - w/2, r.y - h/2, w, h );
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

        float ws = r1.width * scale;
        float hs = r1.height * scale;
        float xs = r.x - (ws/2.);
        float ys = r.y - hs/2.;
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
    return true;
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

void navdata_pi::OnSelectGuidTimer( wxTimerEvent & event)
{
    wxString guid = GetSelectedWaypointGUID_Plugin(  );
    if( guid.IsEmpty() )
        return;
    if( guid != g_selectedPointGuid ){
        g_selectedPointGuid = guid;
        if( m_pTable ){
            m_pTable->SetTargetFlag( true );
            m_pTable->UpdateRouteData( m_activePointGuid, m_gLat, m_gLon, m_gCog, m_gSog );
        }
    }
    RequestRefresh( g_pParentWin );
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

    if( g_activeRouteGuid == wxEmptyString ) {
        OCPNMessageBox_PlugIn( g_pParentWin, _("There is no Active Route!\nYou must active one before using this fonctionality"), _("Warning!"), wxICON_WARNING|wxOK, 100, 50 );
        SetToolbarItemState( m_leftclick_tool_id, false );
        return;
     }

    if( m_pTable ){
        CloseDataTable();
    } else {
    SetToolbarItemState( m_leftclick_tool_id, true );

    LoadocpnConfig();

    long style = wxCAPTION|wxRESIZE_BORDER;
    m_pTable = new DataTable( g_pParentWin, wxID_ANY, wxEmptyString, wxDefaultPosition,
                           wxDefaultSize, style, this );
    wxFont font = GetOCPNGUIScaledFont_PlugIn(_T("Dialog"));
    SetDialogFont( m_pTable, &font );//Apply global font
    DimeWindow( m_pTable ); //apply colour sheme
    m_pTable->InitDataTable();
    m_pTable->UpdateRouteData( m_activePointGuid, m_gLat, m_gLon, m_gCog, m_gSog );
    m_pTable->UpdateTripData();
    m_pTable->SetTableSizePosition( false );
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
