/***********************************************************************
 mfc_dlg.cpp - Defines the dialog box behavior for the MySQL++ MFC
	example.

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

#include "stdafx.h"
#include "mfc_dlg.h"

#include <mysql++.h>

BEGIN_MESSAGE_MAP(CExampleDlg, CDialog)
	ON_BN_CLICKED(IDC_CONNECT_BUTTON, &CExampleDlg::OnBnClickedConnectButton)
END_MESSAGE_MAP()


//// ctor //////////////////////////////////////////////////////////////

CExampleDlg::CExampleDlg(CWnd* pParent) :
CDialog(IDD_MFC_DIALOG, pParent)
{
	LoadDefaults();
}


//// AddMessage ////////////////////////////////////////////////////////
// Inserts the given string at the end of the list box we're using for
// output to the user.

void 
CExampleDlg::AddMessage(LPCTSTR pcMessage)
{
	ResultsList.InsertString(-1, pcMessage);
}


//// DoDataExchange ////////////////////////////////////////////////////
// Transfer data from the controls into our member variables

void
CExampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SERVER_EDIT, sServerAddress);
	DDX_Text(pDX, IDC_USER_EDIT, sUserName);
	DDX_Text(pDX, IDC_PASSWORD_EDIT, sPassword);
	DDX_Control(pDX, IDC_RESULTS_LIST, ResultsList);
}


//// LoadDefaults //////////////////////////////////////////////////////
// Load default input values from registry, if they exist.

void
CExampleDlg::LoadDefaults()
{
	HKEY key = OpenSettingsRegistryKey();
	if (key) {
		TCHAR acSetting[100];
		if (LoadSetting(key, _T("user"), acSetting, sizeof(acSetting))) {
			sUserName = acSetting;
		}
		if (LoadSetting(key, _T("server"), acSetting, sizeof(acSetting))) {
			sServerAddress = acSetting;
		}
		RegCloseKey(key);
	}

	if (sUserName.IsEmpty()) {
		TCHAR acUserName[100];
		DWORD nBufferSize = sizeof(acUserName);
		if (GetUserName(acUserName, &nBufferSize)) {
			sUserName = acUserName;
		}
	}
	if (sServerAddress.IsEmpty()) {
		sServerAddress = _T("localhost");
	}
}


//// LoadSetting ///////////////////////////////////////////////////////
// Loads up the value of the named registry value underneath the given
// key and returns it in pcValue.

bool
CExampleDlg::LoadSetting(HKEY key, LPCTSTR pcName, LPTSTR pcValue,
		DWORD nValueSize)
{
	return RegQueryValueEx(key, pcName, 0, 0, LPBYTE(pcValue),
			&nValueSize) == ERROR_SUCCESS;
}


//// OnBnClickedConnectButton //////////////////////////////////////////
// This is essentially the same thing as examples/simple1.cpp

void
CExampleDlg::OnBnClickedConnectButton()
{
	WCHAR awcTempBuf[100];
	const int kTempBufSize = sizeof(awcTempBuf) / sizeof(awcTempBuf[0]);

	// Pull user input into our member variables
	UpdateData(TRUE);

	// Clear out the results list, in case this isn't the first time
	// we've come in here.
	ResultsList.ResetContent();

	// Translate the Unicode text we get from the UI into the UTF-8 form
	// that MySQL wants.
	const int kInputBufSize = 100;
	char acServerAddress[kInputBufSize];
	char acUserName[kInputBufSize];
	char acPassword[kInputBufSize];
	ToUTF8(acServerAddress, kInputBufSize, sServerAddress);
	ToUTF8(acUserName, kInputBufSize, sUserName);
	ToUTF8(acPassword, kInputBufSize, sPassword);

	// Connect to the sample database.
	mysqlpp::Connection con(false);
	if (!con.connect("mysql_cpp_data", acServerAddress, acUserName,
			acPassword)) {
		AddMessage(_T("Failed to connect to server:"));
		if (ToUCS2(awcTempBuf, kTempBufSize, con.error())) {
			AddMessage(awcTempBuf);
		}
		return;
	}

	// Retrieve a subset of the sample stock table set up by resetdb
	mysqlpp::Query query = con.query();
	query << "select item from stock";
	mysqlpp::StoreQueryResult res = query.store();

	if (res) {
		// Display the result set
		for (size_t i = 0; i < res.num_rows(); ++i) {
			if (ToUCS2(awcTempBuf, kTempBufSize, res[i][0])) {
				AddMessage(awcTempBuf);
			}
		}

		// Retreive was successful, so save user inputs now
		SaveInputs();
	}
	else {
		// Retreive failed
		AddMessage(_T("Failed to get item list:"));
		if (ToUCS2(awcTempBuf, kTempBufSize, query.error())) {
			AddMessage(awcTempBuf);
		}
	}
}


//// OpenSettingsRegistryKey ///////////////////////////////////////////

HKEY
CExampleDlg::OpenSettingsRegistryKey()
{
	HKEY key1, key2;
	if ((RegOpenKey(HKEY_CURRENT_USER, _T("Software"), &key1) ==
			ERROR_SUCCESS) && (RegCreateKey(key1,
			_T("MySQL++ Examples"), &key2) == ERROR_SUCCESS)) {
		RegCloseKey(key1);
		return key2;
	}
	else {
		return 0;
	}
}


//// SaveInputs ////////////////////////////////////////////////////////
// Saves the input fields' values to the registry, except for the
// password field.

bool
CExampleDlg::SaveInputs()
{
	HKEY key = OpenSettingsRegistryKey();
	if (key) {
		SaveSetting(key, _T("user"), sUserName);
		SaveSetting(key, _T("server"), sServerAddress);
		RegCloseKey(key);
		return true;
	}
	else {
		return false;
	}
}


//// SaveSetting ///////////////////////////////////////////////////////
// Saves the given value as a named entry under the given registry key.

bool
CExampleDlg::SaveSetting(HKEY key, LPCTSTR pcName, LPCTSTR pcValue)
{
	DWORD nBytes = DWORD(sizeof(TCHAR) * (_tcslen(pcValue) + 1));
	return RegSetValueEx(key, pcName, 0, REG_SZ, LPBYTE(pcValue),
			nBytes) == ERROR_SUCCESS;
}


//// ToUCS2 ////////////////////////////////////////////////////////////
// Convert a C string in UTF-8 format to UCS-2 format.

bool
CExampleDlg::ToUCS2(LPTSTR pcOut, int nOutLen, const char* kpcIn)
{
	if (strlen(kpcIn) > 0) {
		// Do the conversion normally
		return MultiByteToWideChar(CP_UTF8, 0, kpcIn, -1, pcOut,
				nOutLen) > 0;
	}
	else if (nOutLen > 1) {
		// Can't distinguish no bytes copied from an error, so handle
		// an empty input string as a special case.
		_tccpy(pcOut, _T(""));
		return true;
	}
	else {
		// Not enough room to do anything!
		return false;
	}
}


//// ToUTF8 ////////////////////////////////////////////////////////////
// Convert a UCS-2 multibyte string to the UTF-8 format required by
// MySQL, and thus MySQL++.

bool
CExampleDlg::ToUTF8(char* pcOut, int nOutLen, LPCWSTR kpcIn)
{
	if (_tcslen(kpcIn) > 0) {
		// Do the conversion normally
		return WideCharToMultiByte(CP_UTF8, 0, kpcIn, -1, pcOut,
				nOutLen, 0, 0) > 0;
	}
	else if (nOutLen > 0) {
		// Can't distinguish no bytes copied from an error, so handle
		// an empty input string as a special case.
		*pcOut = '\0';
		return true;
	}
	else {
		// Not enough room to do anything!
		return false;
	}
}
