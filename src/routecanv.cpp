/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Console Canvas
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 *
 *
 */


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "wx/datetime.h"

#include "ocpn_plugin.h"
#include "navdata_pi.h"
#include "vector2D.h"

extern wxString       g_activeRouteGuid;
extern wxString       g_activePointGuid;
extern wxString       g_selectedPointGuid;
extern double         g_Lat;
extern double         g_Lon;
extern double         g_Cog;
extern double         g_Sog;
extern int            g_ocpnDistFormat;
extern wxColour       g_labelColour;
extern wxColour       g_valueColour;
extern wxFont         g_labelFont;
extern wxFont         g_valueFont;
//------------------------------------------------------------------------------
//    RouteCanvas Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(RouteCanvas, wxWindow)
EVT_MOUSE_EVENTS ( RouteCanvas::OnMouseEvent )
END_EVENT_TABLE()
// Define a constructor for my canvas
RouteCanvas::RouteCanvas(wxWindow *parent, navdata_pi *ppi)
{
    m_speedUsed = SPEED_VMG;
    m_bNeedClear = false;

    pPlugin = ppi;

    //get console position
    wxFileConfig *pConf = GetOCPNConfigObject();
    if(pConf) {
        pConf->SetPath ( _T ( "/Settings/NAVDATA" ) );
        int val;
        pConf->Read(_T("NavDataConsolePosition_x"), &val, 60);
        pPlugin->m_consPosition.x = val;
        pConf->Read(_T("NavDataConsolePosition_Y"), &val, 250);
        pPlugin->m_consPosition.y = val;
    }

    long style = wxSIMPLE_BORDER | wxCLIP_CHILDREN | wxFRAME_FLOAT_ON_PARENT;

    wxFrame::Create( parent, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, style );

    m_pitemBoxSizerLeg = new wxBoxSizer( wxVERTICAL );

    pThisLegText = new wxStaticText( this, -1, _T("This Point"), wxDefaultPosition, wxDefaultSize,wxALIGN_LEFT );
    pThisLegText->Fit();
    m_pitemBoxSizerLeg->Add( pThisLegText, 0, wxALIGN_CENTER, 2 );


    wxFont *qFont = OCPNGetFont(_("Dialog"), 0);
    
    wxFont *pThisLegFont = FindOrCreateFont_PlugIn( 10, wxFONTFAMILY_DEFAULT,
                                                          qFont->GetStyle(), wxFONTWEIGHT_BOLD, false,
                                                          qFont->GetFaceName() );
    pThisLegText->SetFont( *pThisLegFont );

    pRNG = new AnnunText( this, -1 );
    pRNG->SetALabel( _("RNG") );
    m_pitemBoxSizerLeg->Add( pRNG, 1, wxALIGN_LEFT | wxALL, 2 );

    pTTG = new AnnunText( this, -1 );
    pTTG->SetALabel( _("TTG  @VMG") );
    m_pitemBoxSizerLeg->Add( pTTG, 1, wxALIGN_LEFT | wxALL, 2 );

    pETA = new AnnunText( this, -1 );
    pETA->SetALabel( _("ETA  @VMG") );
    m_pitemBoxSizerLeg->Add( pETA, 1, wxALIGN_LEFT | wxALL, 2 );


    SetSizer( m_pitemBoxSizerLeg );      // use the sizer for layout
    m_pitemBoxSizerLeg->SetSizeHints( this );
    Layout();
    Fit();
    
    Hide();

     //connect timers at console label level
     pThisLegText->Bind(wxEVT_LEFT_UP, &RouteCanvas::OnTextMouseEvent, this);
     pThisLegText->Bind(wxEVT_LEFT_DOWN, &RouteCanvas::OnTextMouseEvent, this);
     pThisLegText->Bind(wxEVT_MOTION, &RouteCanvas::OnTextMouseEvent, this);
}

RouteCanvas::~RouteCanvas()
{}

void RouteCanvas::OnTextMouseEvent( wxMouseEvent &event )
{
    wxMouseEvent evt = event;
    evt.SetPosition( GetParent()->ClientToScreen(event.GetPosition()) );
    OnMouseEvent(event);
}

void RouteCanvas::OnMouseEvent( wxMouseEvent &event )
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

        wxPoint cons_pos = GetPosition();

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

void RouteCanvas::SetColorScheme()
{
    wxColour colour;
    GetGlobalColor( _T("DILG1"/*UIBDR*/), &colour );

    SetBackgroundColour( colour );
    
    //  Also apply color scheme to all known children

    pThisLegText->SetBackgroundColour( colour );

    pRNG->SetColorScheme();
    pTTG->SetColorScheme();
    pETA->SetColorScheme();

    Refresh();
}

void RouteCanvas::ToggleVmgSogDisplay()
{
    if( m_speedUsed == SPEED_VMG ) {
        m_speedUsed = SPEED_SOG;
        pTTG->SetALabel( wxString( _("TTG  @SOG") ) );
        pETA->SetALabel( wxString( _("ETA  @SOG") ) );
    } else {
        m_speedUsed = SPEED_VMG;
        pTTG->SetALabel( wxString( _("TTG  @VMG") ) );
        pETA->SetALabel( wxString( _("ETA  @VMG") ) );
    }
    pTTG->Refresh();
    pETA->Refresh();
}

void RouteCanvas::UpdateRouteData()
{
    if( this->IsShown() )
    {
        wxString str_buf;
        //find active route and wpts
        std::unique_ptr<PlugIn_Route> r;
        r = GetRoute_Plugin( g_activeRouteGuid );
        if( !r ) return;
        double rng, speed = 0., nrng = 0.;
        double latprev, lonprev, trng = 0.;
        float tttg_sec =0.;
        int nwpts = wxNOT_FOUND;
        bool first = true;
        bool delta = false;
        wxPlugin_WaypointListNode *node = r.get()->pWaypointList->GetFirst();
        while( node ){
            str_buf = wxEmptyString;
            PlugIn_Waypoint *wpt = node->GetData();
            if(wpt) {
                if( first ){
                    latprev = wpt->m_lat;
                    lonprev = wpt->m_lon;
                    first = false;
                }
                if( wpt->m_GUID == g_activePointGuid ) //find active wpt
                    nwpts = ACTIVE_POINT_IDX;
                if( nwpts > wxNOT_FOUND ){
                    if( nwpts == ACTIVE_POINT_IDX ){ //active leg from ownship to active point
                        //rng, brg, nrng
                        rng = DistGreatCircle_Plugin( g_Lat, g_Lon, wpt->m_lat, wpt->m_lon );
                        double brg;
                        BrgRngMercatorToActiveNormalArrival( wpt->m_lat, wpt->m_lon, latprev, lonprev,
                                                g_Lat, g_Lon, &brg, &nrng);
                        //vmg
                        if (!std::isnan(g_Cog) && !std::isnan(g_Sog)) {
                            double vmg = g_Sog * cos((brg - g_Cog) * PI / 180.);
                            if( m_speedUsed == SPEED_VMG )
                                speed = vmg;
                            else
                                speed = g_Sog;
                        }
                        //rng with eventually nrng
                        double deltarng = fabs( rng - nrng );
                        if( ( deltarng > .01 ) && ( ( deltarng / rng ) > .10 ) && ( rng < 10.0 ) )
                            delta = true;
                        //trng/tttg_sec
                        trng = rng;
                        tttg_sec = ( rng / speed ) * 3600.;
                    } else {// following legs
                        //rng
                        DistanceBearingMercator_Plugin(wpt->m_lat, wpt->m_lon, latprev, lonprev,
                                    NULL, &rng);

                        //trng/ tttg_sec
                        trng += rng;
                        tttg_sec += ( rng / g_Sog ) * 3600.;
                    }
                    if( wpt->m_GUID == g_selectedPointGuid ){
                        int c = trng < 10.0 ? 2: 1;
                        str_buf = wxString::Format( _T("%5.*f"), c, toUsrDistance_Plugin( trng, g_ocpnDistFormat ) );
                        if( nwpts == ACTIVE_POINT_IDX && delta )
                                str_buf << wxString::Format( _T("/%5.*f"), c, toUsrDistance_Plugin( nrng, g_ocpnDistFormat ) );
                        pRNG->SetAValue( str_buf );
                        // ttg, eta
                        if( speed > 0. ) {
                            wxTimeSpan tttg_span = wxTimeSpan::Seconds( (long) tttg_sec );
                            //Show also #days if TTG > 24 h
                            str_buf = tttg_sec > SECONDS_PER_DAY ?
                                            tttg_span.Format(_T("%Dd %H:%M")) : tttg_span.Format("%H:%M:%S");
                            pTTG->SetAValue(str_buf);
                            wxDateTime dtnow, eta;
                            dtnow.SetToCurrent();
                            eta = dtnow.Add( tttg_span );
                            if( tttg_sec > SECONDS_PER_DAY ){
                                str_buf = eta.Format(_T("%x")).BeforeLast('/');
                                str_buf << eta.Format(_T(" %H:%M"));
                            } else
                                str_buf = eta.Format(_T("%H:%M"));
                            pETA->SetAValue(str_buf);
                        } else {
                            pTTG->SetAValue(_T("---"));
                            pETA->SetAValue(_T("---"));
                        }
                        pRNG->Refresh();
                        pTTG->Refresh();
                        pETA->Refresh();
                        break;
                    }
                    nwpts++;
                }
                latprev = wpt->m_lat;
                lonprev = wpt->m_lon;
            }
            node = node->GetNext();
        }
    }
}

void RouteCanvas::BrgRngMercatorToActiveNormalArrival(double wptlat, double wptlon,
                                          double wptprevlat, double wptprevlon,
                                          double glat, double glon,
                                          double *brg, double *nrng)
{
            vector2D va, vb, vn;

            double brg1, dist1, brg2, dist2;
            DistanceBearingMercator_Plugin( wptlat, wptlon,
                    wptprevlat, wptprevlon, &brg1,
                    &dist1 );
            vb.x = dist1 * sin( brg1 * PI / 180. );
            vb.y = dist1 * cos( brg1 * PI / 180. );

            DistanceBearingMercator_Plugin( wptlat, wptlon, glat, glon, &brg2,
                    &dist2 );
            va.x = dist2 * sin( brg2 * PI / 180. );
            va.y = dist2 * cos( brg2 * PI / 180. );

            *brg = brg2;

            vGetLengthOfNormal( &va, &vb, &vn );             // NM
    //    Calculate the distance to the arrival line, which is perpendicular to the current route segment
    //    Taking advantage of the calculated normal from current position to route segment vn
            vector2D vToArriveNormal;
            vSubtractVectors( &va, &vn, &vToArriveNormal );

            *nrng = vVectorMagnitude( &vToArriveNormal );
}

void RouteCanvas::ShowWithFreshFonts( void )
{
    Hide();
    Move( 0, 0 );

    UpdateFonts();
    UpdateRouteData();
    pPlugin->PositionConsole();
    Show();

}

void RouteCanvas::UpdateFonts( void )
{
    pTTG->CalculateMinSize();
    pRNG->CalculateMinSize();
    pETA->CalculateMinSize();

    //correct route point name lenght regarding the allowed space
    int w;
    wxFont font = pThisLegText->GetFont();
    GetTextExtent( m_pointName, &w, NULL, NULL, NULL, &font );
    int len = m_pointName.Len() * pTTG->GetMinSize().GetX() / w;
    wxString s = m_pointName.Mid(0, wxMin(len, m_pointName.Len()));
    pThisLegText->SetLabel(s);

    m_pitemBoxSizerLeg->SetSizeHints( this );
    Layout();
    Fit();

    Refresh();
}

//------------------------------------------------------------------------------
//    AnnunText Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(AnnunText, wxWindow)
EVT_PAINT(AnnunText::OnPaint)
EVT_MOUSE_EVENTS ( AnnunText::MouseEvent )
END_EVENT_TABLE()

AnnunText::AnnunText( wxWindow *parent, wxWindowID id ) :
        wxWindow( parent, id, wxDefaultPosition, wxDefaultSize, wxNO_BORDER )
{
    m_label = _T("Label");
    m_value = _T("-----");

    CalculateMinSize();
}

AnnunText::~AnnunText()
{
}
void AnnunText::MouseEvent( wxMouseEvent& event )
{
    if( event.LeftDown() ) {
        RouteCanvas *ccp = dynamic_cast<RouteCanvas*>(GetParent());
        if(ccp){
            ccp->ToggleVmgSogDisplay();
        }
    }
    
}

void AnnunText::CalculateMinSize( void )
{
    //    Calculate the minimum required size of the window based on text size

    int wl = 50;            // reasonable defaults?
    int hl = 20;
    int wv = 50;
    int hv = 20;

    if( g_labelFont.IsOk() ) GetTextExtent( _T("1234"), &wl, &hl, NULL, NULL, &g_labelFont );

    if( g_valueFont.IsOk() ) GetTextExtent( _T("123.456"), &wv, &hv, NULL, NULL, &g_valueFont );

    wxSize min;
    min.x = wl + wv;
    
    // Space is tight on Android....
#ifdef __OCPN__ANDROID__
    min.x = wv * 1.2; 
#endif    
    
    min.y = (int) ( ( hl + hv ) * 1.05 );

    SetMinSize( min );
}

void AnnunText::SetColorScheme()
{
    wxColour colour;
    GetGlobalColor(_T("UBLCK"), &colour);
    m_backBrush = wxBrush( colour, wxBRUSHSTYLE_SOLID );

    Refresh();

   // CalculateMinSize();
}

void AnnunText::SetALabel( const wxString &l )
{
    m_label = l;
}

void AnnunText::SetAValue( const wxString &v )
{
    m_value = v;
}

void AnnunText::OnPaint( wxPaintEvent& event )
{
    int sx, sy;
    GetClientSize( &sx, &sy );
    //    Do the drawing on an off-screen memory DC, and blit into place
    //    to avoid objectionable flashing
    wxMemoryDC mdc;

    wxBitmap m_bitmap( sx, sy, -1 );
    mdc.SelectObject( m_bitmap );
    mdc.SetBackground( m_backBrush );
    mdc.Clear();

    if( g_labelFont.IsOk() ) {
        mdc.SetFont( g_labelFont );
        mdc.SetTextForeground( g_labelColour );
        mdc.DrawText( m_label, 5, 2 );
    }

    if( g_valueFont.IsOk() ) {
        mdc.SetFont( g_valueFont );
        mdc.SetTextForeground( g_valueColour );

        int w, h;
        mdc.GetTextExtent( m_value, &w, &h );
        int cw, ch;
        mdc.GetSize( &cw, &ch );

        mdc.DrawText( m_value, cw - w - 2, ch - h - 2 );
    }

    wxPaintDC dc( this );
    dc.Blit( 0, 0, sx, sy, &mdc, 0, 0 );   
}


