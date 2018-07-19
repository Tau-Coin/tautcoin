/***********************************************************************
 mfc_dlg.h - Declares the dialog class for the MySQL++ MFC example.

 Copyright (c) 2007 by Educational Technology Resources, Inc.  Others 
 may also hold copyrights on code in this file.  See the CREDITS.txt
 file in the top directory of the distribution for details.

 This file is part of MySQL++.

 MySQL++ is free software; you can redistribute it and/or modify it
 under the terms of the GNU Lesser General Public License as published
 by the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.

 MySQL++ is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with MySQL++; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 USA
***********************************************************************/

#pragma once
#include "afxwin.h"
#include "resource.h"

class CExampleDlg : public CDialog
{
public:
	//// Public interface
	CExampleDlg(CWnd* pParent = 0);

	//// Public data
	CString sServerAddress;
	CString sUserName;
	CString sPassword;
	CListBox ResultsList;

protected:
	//// Internal support functions
	void AddMessage(LPCTSTR pcMessage);
	void DoDataExchange(CDataExchange* pDX);
	void LoadDefaults();
	bool LoadSetting(HKEY key, LPCTSTR pcName, LPTSTR pcValue,
			DWORD nValueSize);
	HKEY OpenSettingsRegistryKey();
	bool SaveInputs();
	bool SaveSetting(HKEY key, LPCTSTR pcName, LPCTSTR pcValue);
	bool ToUCS2(LPTSTR pcOut, int nOutLen, const char* kpcIn);
	bool ToUTF8(char* pcOut, int nOutLen, LPCWSTR kpcIn);

	//// Message map
	afx_msg void OnBnClickedConnectButton();
	DECLARE_MESSAGE_MAP()
};
