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

#ifndef __DATADIALOGBASE_H__
#define __DATADIALOGBASE_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#define DIALOG_CAPTION_HEIGHT       25
#define SINGLE_BORDER_THICKNESS     3
#define DOUBLE_BORDER_THICKNESS     6

///////////////////////////////////////////////////////////////////////////////
/// Class DataTableBase
///////////////////////////////////////////////////////////////////////////////
class DataTableBase : public wxDialog
{
	private:

	protected:
        wxTextCtrl* m_pTitleText;
        wxTextCtrl* m_pStartDText;
        wxTextCtrl*   m_pStartDate;
        wxTextCtrl* m_pStartDTextat;
        wxTextCtrl*   m_pStartTime;
        wxTextCtrl* m_pDistText;
        wxTextCtrl*   m_pDistValue;
        wxTextCtrl* m_pTimetText;
        wxTextCtrl*   m_pTimeValue;
        wxTextCtrl* m_pSpeedText;
        wxTextCtrl*   m_pSpeedValue;
        wxTextCtrl* m_pEndDText;
        wxTextCtrl*   m_pEndDate;
        wxFlexGridSizer* m_pTripSizer01;

		// Virtual event handlers, overide them in your derived class
        //virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }

    public:
        DataTableBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _T("Trip Data"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCAPTION|wxRESIZE_BORDER );
        ~DataTableBase();

};

///////////////////////////////////////////////////////////////////////////////
/// Class SettingsBase
///////////////////////////////////////////////////////////////////////////////
class SettingsBase : public wxDialog
{
    private:

    protected:
        wxRadioBox*     m_tripDialogStyle;
        wxButton*       m_pSettingsOK;

        SettingsBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Preferences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 )
                , long style = wxCAPTION );

        ~SettingsBase(){}

};


#endif //__DATADIALOGBASE_H__
