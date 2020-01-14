///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun  5 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "datadialogbase.h"

///////////////////////////////////////////////////////////////////////////

DataTableBase::DataTableBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxSize( -1,-1 ) );

    wxBoxSizer* Sizer01;
    Sizer01 = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer* fgSizer01;
    fgSizer01 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer01->AddGrowableRow( 1 );
    fgSizer01->SetFlexibleDirection( wxBOTH );
    fgSizer01->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );

    m_pTripSizer = new wxFlexGridSizer( 0, 1, 0, 0 );
    m_pTripSizer->SetFlexibleDirection( wxBOTH );
    m_pTripSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );

    m_pfgSizer03 = new wxFlexGridSizer( 0, 4, 0, 0 );
    m_pfgSizer03->SetFlexibleDirection( wxBOTH );
    m_pfgSizer03->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );

    m_pStartDText = new wxStaticText( this, wxID_ANY, _("Trip Started"), wxDefaultPosition, wxSize( -1, -1 ), 0);
    m_pStartDText->Wrap( -1 );
    m_pfgSizer03->Add( m_pStartDText, 0, wxLEFT|wxTOP|wxEXPAND, 5 );

    m_pStartDate = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, -1 ), 0 );
    m_pStartDate->Wrap( -1 );
    m_pfgSizer03->Add( m_pStartDate, 0, wxALL|wxEXPAND, 5 );

    m_pStartDTextat = new wxStaticText( this, wxID_ANY, _("at"), wxDefaultPosition, wxSize( -1, -1 ), 0);
    m_pStartDTextat->Wrap( -1 );
    m_pfgSizer03->Add( m_pStartDTextat, 0, wxALIGN_RIGHT|wxLEFT|wxTOP|wxEXPAND, 5 );

    m_pStartTime = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, -1 ), 0 );
    m_pStartTime->Wrap( -1 );
    m_pfgSizer03->Add( m_pStartTime, 0, wxALL|wxEXPAND, 5 );

    m_pTripSizer->Add( m_pfgSizer03, 1, wxEXPAND, 0 );

    m_pfgSizer04 = new wxFlexGridSizer( 0, 6, 0, 0 );
    m_pfgSizer04->SetFlexibleDirection( wxBOTH );
    m_pfgSizer04->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );

    m_pTimetText = new wxStaticText( this, wxID_ANY, _("Time"), wxDefaultPosition, wxSize( -1, -1 ), 0 );
    m_pTimetText->Wrap( -1 );
    m_pfgSizer04->Add( m_pTimetText, 0, wxLEFT|wxTOP|wxEXPAND, 5 );

    m_pTimeValue = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, -1 ), 0 );
    m_pTimeValue->Wrap( -1 );
    m_pfgSizer04->Add( m_pTimeValue, 0, wxALL|wxEXPAND, 5 );

    m_pDistText = new wxStaticText( this, wxID_ANY, _("Dist"), wxDefaultPosition, wxSize( -1, -1 ), 0 );
    m_pDistText->Wrap( -1 );
    m_pfgSizer04->Add( m_pDistText, 0, wxLEFT|wxTOP|wxEXPAND, 5 );

    m_pDistValue = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, -1 ), 0 );
    m_pDistValue->Wrap( -1 );
    m_pfgSizer04->Add( m_pDistValue, 0, wxALL|wxEXPAND, 5 );

    m_pSpeedText = new wxStaticText( this, wxID_ANY, _("Avg.speed:"), wxDefaultPosition, wxSize( -1, -1 ), 0 );
    m_pSpeedText->Wrap( -1 );
    m_pfgSizer04->Add( m_pSpeedText, 0, wxLEFT|wxTOP|wxEXPAND, 5 );

    m_pSpeedValue = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, -1 ), 0 );
    m_pSpeedValue->Wrap( -1 );
    m_pfgSizer04->Add( m_pSpeedValue, 0, wxALL|wxEXPAND, 5 );

    m_pTripSizer->Add( m_pfgSizer04, 1, wxEXPAND, 0 );

    fgSizer01->Add( m_pTripSizer, 1, wxEXPAND, 0 );

    wxFlexGridSizer* fgSizer05;
    fgSizer05 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer05->AddGrowableRow( 0 );
    fgSizer05->SetFlexibleDirection( wxBOTH );
    fgSizer05->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );

    //create and add grid table
    m_pDataTable = new CustomGrid( this, wxID_ANY, wxDefaultPosition, wxSize( -1,50 ), 0, _T(" ") );
    m_pDataTable->SetDefaultCellAlignment( wxALIGN_CENTRE, wxALIGN_TOP );
    fgSizer05->Add( m_pDataTable, 0, wxALL|wxEXPAND, 5 );

    fgSizer01->Add( fgSizer05, 1, wxEXPAND, 5 );

    Sizer01->Add( fgSizer01, 1, wxEXPAND, 5 );

    this->SetSizer( Sizer01 );
	this->Layout();
    Sizer01->Fit( this );

	this->Centre( wxBOTH );

    DimeWindow( this );                                     //aplly global colours scheme

    // Connect Events
    //Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DataTableBase::OnClose ) );
}

DataTableBase::~DataTableBase()
{
	// Disconnect Events
    //Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DataTableBase::OnClose ) );
}

SettingsBase::SettingsBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
    this->SetSizeHints( wxSize( -1,-1 ), wxSize( -1,-1 ) );

    wxBoxSizer* Sizer01;
    Sizer01 = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer* fgSizer01;
    fgSizer01 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer01->AddGrowableRow( 1 );
    fgSizer01->SetFlexibleDirection( wxBOTH );
    fgSizer01->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );

    m_pShowTripData = new wxCheckBox( this, wxID_ANY, _("Show Trip Data"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer01->Add( m_pShowTripData, 0, wxALL, 5 );

    wxString s[] = { _("at VMG"), _("at SOG") };
    m_pShowspeed = new wxRadioBox( this, wxID_ANY, _("Compute and Show TTG & ETA"), wxDefaultPosition, wxDefaultSize,
                                   2, s, 0, wxRA_SPECIFY_ROWS );
    fgSizer01->Add( m_pShowspeed, 0, wxALL, 5 );

    wxStdDialogButtonSizer *m_pSettings = new wxStdDialogButtonSizer();
    m_pSettingsOK = new wxButton( this, wxID_OK, _("Close") );
    m_pSettingsOK->SetFont( GetOCPNGUIScaledFont_PlugIn(_T("Dialog")) );
    m_pSettings->AddButton( m_pSettingsOK );
    m_pSettings->Realize();

    fgSizer01->Add( m_pSettings, 1, wxEXPAND, 5 );

    Sizer01->Add( fgSizer01, 1, wxEXPAND, 5 );this->SetSizer( Sizer01 );
    this->Layout();
    Sizer01->Fit( this );

    this->Centre( wxBOTH );

    DimeWindow( this );                                     //aplly global colours scheme

}





