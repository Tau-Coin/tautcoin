/***********************************************************************
 MainForm.cpp - Defines the dialog box behavior for the MySQL++ C++/CLI
	Windows Forms example.

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

#include <mysql++.h>

namespace wforms {

	using namespace Microsoft::Win32;
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class MainForm : public System::Windows::Forms::Form
	{
	public:
		MainForm()
		{
			InitializeComponent();
			LoadDefaults();
		}

	protected:
		~MainForm()
		{
			if (components) {
				delete components;
			}
		}

	private:
		// Insert a text string into the output list control
		Void AddMessage(String^ msg)
		{
			resultsList_->Items->Add(msg);
		}

		// Handle Close button click by shutting down application
		Void CloseButton_Click(Object^ sender, EventArgs^ e)
		{
			Application::Exit();
		}
		
		// Handle Connect button click.  The body of this function is
		// essentially the same as the simple2 command line example, with
		// some GUI overhead.
		Void ConnectButton_Click(Object^ sender, EventArgs^ e)
		{
			// Clear out the results list, in case this isn't the first time
			// we've come in here.
			resultsList_->Items->Clear();

			// Translate the Unicode text we get from the UI into the UTF-8
			// form that MySQL wants.
			const int kInputBufSize = 100;
			char acServerAddress[kInputBufSize];
			char acUserName[kInputBufSize];
			char acPassword[kInputBufSize];
			ToUTF8(acServerAddress, kInputBufSize, serverAddress_->Text);
			ToUTF8(acUserName, kInputBufSize, userName_->Text);
			ToUTF8(acPassword, kInputBufSize, password_->Text);

			// Connect to the sample database.
			mysqlpp::Connection con(false);
			if (!con.connect("mysql_cpp_data", acServerAddress, acUserName,
					acPassword)) {
				AddMessage("Failed to connect to server:");
				AddMessage(gcnew String(con.error()));
				return;
			}

			// Retrieve a subset of the sample stock table set up by resetdb
			mysqlpp::Query query = con.query();
			query << "select item from stock";
			mysqlpp::StoreQueryResult res = query.store();

			if (res) {
				// Display the result set
				for (size_t i = 0; i < res.num_rows(); ++i) {
					AddMessage(ToUCS2(res[i][0]));
				}

				// Retreive was successful, so save user inputs now
				SaveInputs();
			}
			else {
				// Retreive failed
				AddMessage("Failed to get item list:");
				AddMessage(ToUCS2(query.error()));
			}
		}

		// Load the default input field values, if there are any
		Void LoadDefaults()
		{
			RegistryKey^ settings = OpenSettingsRegistryKey();
			if (settings) {
				userName_->Text = LoadSetting(settings, L"user");
				serverAddress_->Text = LoadSetting(settings, L"server");
			}
			
			if (String::IsNullOrEmpty(userName_->Text)) {
				userName_->Text = Environment::UserName;
			}
			if (String::IsNullOrEmpty(serverAddress_->Text)) {
				serverAddress_->Text = L"localhost";
			}
		}

		// Returns a setting from underneath the given registry key.
		// Assumes that it's a string value under the MySQL++ examples' 
		// settings area.
		String^ LoadSetting(RegistryKey^ key, String^ name)
		{
			return (String^)key->GetValue(name);
		}

		// Returns a reference to the MySQL++ examples' settings area in the
		// registry.
		RegistryKey^ OpenSettingsRegistryKey()
		{
			RegistryKey^ key = Registry::CurrentUser->OpenSubKey(L"Software",
					true);
			return key ? key->CreateSubKey(L"MySQL++ Examples") : nullptr;
		}

		// Saves the input fields' values to the registry, except for the
		// password field.
		Void SaveInputs()
		{
			RegistryKey^ settings = OpenSettingsRegistryKey();
			if (settings) {
				SaveSetting(settings, "user", userName_->Text);
				SaveSetting(settings, "server", serverAddress_->Text);
			}			
		}

		// Saves the given value as a named entry under the given registry
		// key.
		Void SaveSetting(RegistryKey^ key, String^ name, String^ value)
		{
			key->SetValue(name, value);
		}

		// Takes a string in the .NET platform's native Unicode format and
		// copies it to the given C string buffer in UTF-8 encoding.
		Void ToUTF8(char* pcOut, int nOutLen, String^ sIn)
		{
			array<Byte>^ bytes = System::Text::Encoding::UTF8->GetBytes(sIn);
			nOutLen = Math::Min(nOutLen - 1, bytes->Length);
			System::Runtime::InteropServices::Marshal::Copy(bytes, 0, 
				IntPtr(pcOut), nOutLen);
			pcOut[nOutLen] = '\0';
		}

		// Takes the given C string encoded in UTF-8 and converts it to a
		// Unicode string in the .NET platform's native Unicode encoding.
		String^ ToUCS2(const char* utf8)
		{
			return gcnew String(utf8, 0, strlen(utf8), 
					System::Text::Encoding::UTF8);
		}

	private: System::Windows::Forms::TextBox^ serverAddress_;
	private: System::Windows::Forms::TextBox^ password_;
	private: System::Windows::Forms::TextBox^ userName_;
	private: System::Windows::Forms::ListBox^ resultsList_;
	private: System::Windows::Forms::Button^ connectButton_;
	private: System::Windows::Forms::Button^ closeButton_;
	private: System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::Windows::Forms::Label^  label1;
			System::Windows::Forms::Label^  label2;
			System::Windows::Forms::Label^  label3;
			System::Windows::Forms::Label^  label4;
			this->serverAddress_ = (gcnew System::Windows::Forms::TextBox());
			this->password_ = (gcnew System::Windows::Forms::TextBox());
			this->userName_ = (gcnew System::Windows::Forms::TextBox());
			this->resultsList_ = (gcnew System::Windows::Forms::ListBox());
			this->connectButton_ = (gcnew System::Windows::Forms::Button());
			this->closeButton_ = (gcnew System::Windows::Forms::Button());
			label1 = (gcnew System::Windows::Forms::Label());
			label2 = (gcnew System::Windows::Forms::Label());
			label3 = (gcnew System::Windows::Forms::Label());
			label4 = (gcnew System::Windows::Forms::Label());
			this->SuspendLayout();
			// 
			// label1
			// 
			label1->AutoSize = true;
			label1->Location = System::Drawing::Point(29, 13);
			label1->Name = L"label1";
			label1->Size = System::Drawing::Size(41, 13);
			label1->TabIndex = 6;
			label1->Text = L"Server:";
			label1->TextAlign = System::Drawing::ContentAlignment::TopRight;
			// 
			// label2
			// 
			label2->AutoSize = true;
			label2->Location = System::Drawing::Point(9, 39);
			label2->Name = L"label2";
			label2->Size = System::Drawing::Size(61, 13);
			label2->TabIndex = 7;
			label2->Text = L"User name:";
			label2->TextAlign = System::Drawing::ContentAlignment::TopRight;
			// 
			// label3
			// 
			label3->AutoSize = true;
			label3->Location = System::Drawing::Point(14, 65);
			label3->Name = L"label3";
			label3->Size = System::Drawing::Size(56, 13);
			label3->TabIndex = 8;
			label3->Text = L"Password:";
			label3->TextAlign = System::Drawing::ContentAlignment::TopRight;
			// 
			// label4
			// 
			label4->AutoSize = true;
			label4->Location = System::Drawing::Point(25, 92);
			label4->Name = L"label4";
			label4->Size = System::Drawing::Size(45, 13);
			label4->TabIndex = 9;
			label4->Text = L"Results:";
			label4->TextAlign = System::Drawing::ContentAlignment::TopRight;
			// 
			// serverAddress_
			// 
			this->serverAddress_->Location = System::Drawing::Point(70, 9);
			this->serverAddress_->Name = L"serverAddress_";
			this->serverAddress_->Size = System::Drawing::Size(139, 20);
			this->serverAddress_->TabIndex = 0;
			// 
			// password_
			// 
			this->password_->Location = System::Drawing::Point(70, 61);
			this->password_->Name = L"password_";
			this->password_->Size = System::Drawing::Size(139, 20);
			this->password_->TabIndex = 2;
			this->password_->UseSystemPasswordChar = true;
			// 
			// userName_
			// 
			this->userName_->Location = System::Drawing::Point(70, 35);
			this->userName_->Name = L"userName_";
			this->userName_->Size = System::Drawing::Size(139, 20);
			this->userName_->TabIndex = 1;
			// 
			// resultsList_
			// 
			this->resultsList_->Enabled = false;
			this->resultsList_->FormattingEnabled = true;
			this->resultsList_->Location = System::Drawing::Point(70, 88);
			this->resultsList_->Name = L"resultsList_";
			this->resultsList_->Size = System::Drawing::Size(228, 95);
			this->resultsList_->TabIndex = 3;
			this->resultsList_->TabStop = false;
			// 
			// connectButton_
			// 
			this->connectButton_->Location = System::Drawing::Point(224, 9);
			this->connectButton_->Name = L"connectButton_";
			this->connectButton_->Size = System::Drawing::Size(75, 23);
			this->connectButton_->TabIndex = 3;
			this->connectButton_->Text = L"Connect!";
			this->connectButton_->UseVisualStyleBackColor = true;
			this->connectButton_->Click += gcnew System::EventHandler(this, &MainForm::ConnectButton_Click);
			// 
			// closeButton_
			// 
			this->closeButton_->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->closeButton_->Location = System::Drawing::Point(224, 38);
			this->closeButton_->Name = L"closeButton_";
			this->closeButton_->Size = System::Drawing::Size(75, 23);
			this->closeButton_->TabIndex = 4;
			this->closeButton_->Text = L"Close";
			this->closeButton_->UseVisualStyleBackColor = true;
			this->closeButton_->Click += gcnew System::EventHandler(this, &MainForm::CloseButton_Click);
			// 
			// MainForm
			// 
			this->AcceptButton = this->connectButton_;
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->CancelButton = this->closeButton_;
			this->ClientSize = System::Drawing::Size(310, 192);
			this->ControlBox = false;
			this->Controls->Add(label4);
			this->Controls->Add(label3);
			this->Controls->Add(label2);
			this->Controls->Add(label1);
			this->Controls->Add(this->closeButton_);
			this->Controls->Add(this->connectButton_);
			this->Controls->Add(this->resultsList_);
			this->Controls->Add(this->userName_);
			this->Controls->Add(this->password_);
			this->Controls->Add(this->serverAddress_);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"MainForm";
			this->ShowIcon = false;
			this->Text = L"MySQL++ Windows Forms Examples";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
};
}
