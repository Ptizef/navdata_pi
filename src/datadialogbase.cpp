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

    wxFlexGridSizer* fgSizer02;
    fgSizer02 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer02->SetFlexibleDirection( wxBOTH );
    fgSizer02->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );

    m_pfgSizer03 = new wxFlexGridSizer( 0, 4, 0, 0 );
    m_pfgSizer03->SetFlexibleDirection( wxBOTH );
    m_pfgSizer03->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );

    m_pStartDText = new wxStaticText( this, wxID_ANY, _("Trip Started"), wxDefaultPosition, wxSize( -1, -1 ), 0);
    m_pStartDText->Wrap( -1 );
    m_pfgSizer03->Add( m_pStartDText, 0, wxTOP|wxEXPAND, 5 );

    m_pStartDate = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, -1 ), 0 );
    m_pStartDate->Wrap( -1 );
    m_pfgSizer03->Add( m_pStartDate, 0, wxALL|wxEXPAND, 5 );

    m_pStartDTextat = new wxStaticText( this, wxID_ANY, _("at"), wxDefaultPosition, wxSize( -1, -1 ), 0);
    m_pStartDTextat->Wrap( -1 );
    m_pfgSizer03->Add( m_pStartDTextat, 0, wxALIGN_RIGHT|wxTOP|wxEXPAND, 5 );

    m_pStartTime = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, -1 ), 0 );
    m_pStartTime->Wrap( -1 );
    m_pfgSizer03->Add( m_pStartTime, 0, wxALL|wxEXPAND, 5 );

    fgSizer02->Add( m_pfgSizer03, 1, wxEXPAND, 0 );

    m_pfgSizer04 = new wxFlexGridSizer( 0, 6, 0, 0 );
    m_pfgSizer04->SetFlexibleDirection( wxBOTH );
    m_pfgSizer04->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );

    m_pTimetText = new wxStaticText( this, wxID_ANY, _("Time"), wxDefaultPosition, wxSize( -1, -1 ), 0 );
    m_pTimetText->Wrap( -1 );
    m_pfgSizer04->Add( m_pTimetText, 0, wxTop|wxEXPAND, 5 );

    m_pTimeValue = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, -1 ), 0 );
    m_pTimeValue->Wrap( -1 );
    m_pfgSizer04->Add( m_pTimeValue, 0, wxALL|wxEXPAND, 5 );

    m_pDistText = new wxStaticText( this, wxID_ANY, _("Dist"), wxDefaultPosition, wxSize( -1, -1 ), 0 );
    m_pDistText->Wrap( -1 );
    m_pfgSizer04->Add( m_pDistText, 0, wxTOP|wxEXPAND, 5 );

    m_pDistValue = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, -1 ), 0 );
    m_pDistValue->Wrap( -1 );
    m_pfgSizer04->Add( m_pDistValue, 0, wxALL|wxEXPAND, 5 );

    m_pSpeedText = new wxStaticText( this, wxID_ANY, _("Avg.speed:"), wxDefaultPosition, wxSize( -1, -1 ), 0 );
    m_pSpeedText->Wrap( -1 );
    m_pfgSizer04->Add( m_pSpeedText, 0, wxTOP|wxEXPAND, 5 );

    m_pSpeedValue = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, -1 ), 0 );
    m_pSpeedValue->Wrap( -1 );
    m_pfgSizer04->Add( m_pSpeedValue, 0, wxALL|wxEXPAND, 5 );

    fgSizer02->Add( m_pfgSizer04, 1, wxEXPAND, 0 );

    fgSizer01->Add( fgSizer02, 1, wxEXPAND, 0 );

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

	wxStdDialogButtonSizer *m_pButtonTable = new wxStdDialogButtonSizer();
	m_pButtonTableOK = new wxButton( this, wxID_OK, _("Close") );
	m_pButtonTableOK->SetFont( GetOCPNGUIScaledFont_PlugIn(_T("Dialog")) );
	m_pButtonTable->AddButton( m_pButtonTableOK );
	m_pButtonTable->Realize();

    fgSizer01->Add( m_pButtonTable, 1, wxEXPAND, 5 );


    Sizer01->Add( fgSizer01, 1, wxEXPAND, 5 );

    this->SetSizer( Sizer01 );
	this->Layout();
    Sizer01->Fit( this );

	this->Centre( wxBOTH );

    DimeWindow( this );                                     //aplly global colours scheme

    // Connect Events
    Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DataTableBase::OnClose ) );
    m_pButtonTableOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DataTableBase::OnOKButton ), NULL, this );
}

DataTableBase::~DataTableBase()
{
	// Disconnect Events
    Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DataTableBase::OnClose ) );
    m_pButtonTableOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DataTableBase::OnOKButton ), NULL, this );

}
