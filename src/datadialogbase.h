///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun  5 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DATADIALOGBASE_H__
#define __DATADIALOGBASE_H__
/*
#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/slider.h>
#include <wx/sizer.h>
#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/scrolwin.h>
#include <wx/radiobut.h>
#include <wx/statbmp.h>
#include <wx/notebook.h>
#include <wx/radiobox.h>
#include <wx/statline.h>*/
//#include <wx/grid.h>
#include "customgrid.h"

///////////////////////////////////////////////////////////////////////////////
/// Class DataTableBase
///////////////////////////////////////////////////////////////////////////////
class DataTableBase : public wxDialog
{
	private:

	protected:
        wxStaticText* m_pStartDText;
        wxStaticText* m_pStartDate;
        wxStaticText* m_pStartDTextat;
        wxStaticText* m_pStartTime;
        wxStaticText* m_pDistText;
        wxStaticText* m_pDistValue;
        wxStaticText* m_pTimetText;
        wxStaticText* m_pTimeValue;
        wxStaticText* m_pSpeedText;
        wxStaticText* m_pSpeedValue;
        wxFlexGridSizer* m_pTripSizer00;
        wxFlexGridSizer* m_pTripSizer01;

		// Virtual event handlers, overide them in your derived class
        //virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }

	public:
        CustomGrid* m_pDataTable;
        DataTableBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Navigation Data"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCAPTION|wxRESIZE_BORDER );
        ~DataTableBase();
};

#endif //__DATADIALOGBASE_H__

///////////////////////////////////////////////////////////////////////////////
/// Class SettingsBase
///////////////////////////////////////////////////////////////////////////////
class SettingsBase : public wxDialog
{
    private:

    protected:
        wxCheckBox*     m_pShowTripData;
        wxRadioBox*     m_pShowspeed;
        wxButton*       m_pSettingsOK;

        SettingsBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Settings"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 )
                , long style = wxCAPTION );

        ~SettingsBase(){}

};



