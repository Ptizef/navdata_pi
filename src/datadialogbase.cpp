/************************************************************************************************
 *
 * Project:  OpenCPN - plugin navdata_pi
 * Purpose:  navdata_pi dialog
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
 ***************************************************************************/
#include "datadialogbase.h"

///////////////////////////////////////////////////////////////////////////

DataTableBase::DataTableBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxSize( -1,-1 ) );

    wxBoxSizer* Sizer01;
    Sizer01 = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer* fgSizer01;
    fgSizer01 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer01->SetFlexibleDirection( wxBOTH );
    fgSizer01->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );

    fgSizer01->AddSpacer(DIALOG_CAPTION_HEIGHT/* - SINGLE_BORDER_THICKNESS*/);

    m_pTripSizer01 = new wxFlexGridSizer( 0, 6, 0, 0 );
    m_pTripSizer01->SetFlexibleDirection( wxBOTH );
    m_pTripSizer01->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );

    m_pStartDText = new wxTextCtrl( this, wxID_ANY, _("Started"), wxDefaultPosition, wxSize( -1, -1 ), wxTE_READONLY);
    m_pTripSizer01->Add( m_pStartDText, 0, wxLEFT|wxBOTTOM|wxEXPAND, 3 );

    m_pStartDate = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, -1 ), wxTE_READONLY);
    m_pTripSizer01->Add( m_pStartDate, 0, wxLEFT|wxBOTTOM|wxEXPAND, 3 );

    m_pStartDTextat = new wxTextCtrl( this, wxID_ANY, _("at"), wxDefaultPosition, wxSize( -1, -1 ), wxALIGN_CENTRE_HORIZONTAL|wxTE_READONLY);
    m_pTripSizer01->Add( m_pStartDTextat, 0, wxALIGN_CENTRE_HORIZONTAL|wxLEFT|wxBOTTOM|wxEXPAND, 3 );

    m_pStartTime = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, -1 ), wxTE_READONLY);
    m_pTripSizer01->Add( m_pStartTime, 0, wxLEFT|wxBOTTOM|wxEXPAND, 2 );

    m_pTimetText = new wxTextCtrl( this, wxID_ANY, _("TimeSpent"), wxDefaultPosition, wxSize( -1, -1 ), wxTE_READONLY );
    m_pTripSizer01->Add( m_pTimetText, 0, wxLEFT|wxBOTTOM|wxEXPAND, 3 );

    m_pTimeValue = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, -1 ), wxTE_READONLY);
    m_pTripSizer01->Add( m_pTimeValue, 0, wxLEFT|wxBOTTOM|wxEXPAND, 2 );

    m_pDistText = new wxTextCtrl( this, wxID_ANY, _("Distance"), wxDefaultPosition, wxSize( -1, -1 ), wxTE_READONLY );
    m_pTripSizer01->Add( m_pDistText, 0, wxLEFT|wxBOTTOM|wxEXPAND, 3 );

    m_pDistValue = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, -1 ), wxTE_READONLY);
    m_pTripSizer01->Add( m_pDistValue, 0, wxLEFT|wxBOTTOM|wxEXPAND, 2 );

    m_pSpeedText = new wxTextCtrl( this, wxID_ANY, _("Avg.speed"), wxDefaultPosition, wxSize( -1, -1 ), wxTE_READONLY );
    m_pTripSizer01->Add( m_pSpeedText, 0, wxLEFT|wxBOTTOM|wxEXPAND, 3 );

    m_pSpeedValue = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, -1 ), wxTE_READONLY);
    m_pTripSizer01->Add( m_pSpeedValue, 0, wxLEFT|wxBOTTOM|wxEXPAND, 2 );

    m_pEndDText = new wxTextCtrl( this, wxID_ANY, _("Ended"), wxDefaultPosition, wxSize( -1, -1 ), wxTE_READONLY);
    m_pTripSizer01->Add( m_pEndDText, 0, wxLEFT|wxBOTTOM|wxEXPAND, 3 );

    m_pEndDate = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, -1 ), wxTE_READONLY);
    m_pTripSizer01->Add( m_pEndDate, 0, wxLEFT|wxBOTTOM|wxEXPAND, 2 );

    fgSizer01->Add( m_pTripSizer01, 1, wxEXPAND, 0 );

    Sizer01->Add( fgSizer01, 1, wxEXPAND, 0 );

    this->SetSizer( Sizer01 );
	this->Layout();
    Sizer01->Fit( this );

	this->Centre( wxBOTH );
}

DataTableBase::~DataTableBase()
{}

SettingsBase::SettingsBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
    this->SetSizeHints( wxSize( -1,-1 ), wxSize( -1,-1 ) );

    wxBoxSizer* Sizer01;
    Sizer01 = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer* fgSizer01;
    fgSizer01 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer01->AddGrowableCol( 0 );
    fgSizer01->SetFlexibleDirection( wxBOTH );
    fgSizer01->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );

    wxString s[] = { _("Verticaly (2 columns)"), _("Intermediate (4 columns)"), _("Horizontaly (6columns)") };
    m_tripDialogStyle = new wxRadioBox(this, wxID_ANY, wxString(_("The Trip Data Dialog will be displayed") + _T("   ")), wxDefaultPosition, wxDefaultSize,
                                   2, s, 0, wxRA_SPECIFY_ROWS );
    fgSizer01->Add( m_tripDialogStyle, 0, wxALL|wxEXPAND, 5 );


    wxStdDialogButtonSizer *m_pSettings = new wxStdDialogButtonSizer();
    m_pSettingsOK = new wxButton( this, wxID_OK, _("OK") );
    m_pSettings->AddButton( m_pSettingsOK );
    m_pSettings->Realize();

    fgSizer01->Add( m_pSettings, 1, wxALL|wxEXPAND, 5 );

    Sizer01->Add( fgSizer01, 1, wxEXPAND, 5 );this->SetSizer( Sizer01 );
    this->Layout();
    Sizer01->Fit( this );

    this->Centre( wxBOTH );
}





