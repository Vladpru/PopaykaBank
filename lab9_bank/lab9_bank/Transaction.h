#pragma once
#include "BankService.h"
#include "ModernUI.h"
#include "ThemeManager.h"

namespace lab9bank {
    using namespace System;
    using namespace System::Windows::Forms;
    using namespace System::Globalization;
    using namespace ModernUI;

    public ref class Transaction : public System::Windows::Forms::Form
    {
    public:
        Transaction(void) { InitializeComponent(); }
    private:
        ModernTextBox^ txtRec;
        ModernTextBox^ txtAmt;
        ModernButton^ btnSend;
        Label^ lblTitle;
        Label^ lbl1; Label^ lbl2;

        void InitializeComponent(void) {
            this->Size = System::Drawing::Size(400, 350);
            this->Text = "New Transaction";
            this->StartPosition = FormStartPosition::CenterParent;
            this->BackColor = Color::White;

            lblTitle = gcnew Label();
            lblTitle->Text = "Send Money";
            lblTitle->Font = gcnew System::Drawing::Font("Segoe UI", 16, FontStyle::Bold);
            lblTitle->Location = Point(30, 20);
            lblTitle->AutoSize = true;

            lbl1 = gcnew Label(); lbl1->Text = "Receiver:"; lbl1->Location = Point(35, 70);
            txtRec = gcnew ModernTextBox(); txtRec->Location = Point(35, 95); txtRec->Size = System::Drawing::Size(310, 40);

            lbl2 = gcnew Label(); lbl2->Text = "Amount:"; lbl2->Location = Point(35, 150);
            txtAmt = gcnew ModernTextBox(); txtAmt->Location = Point(35, 175); txtAmt->Size = System::Drawing::Size(310, 40);

            btnSend = gcnew ModernButton();
            btnSend->Text = "Send Now";
            btnSend->PrimaryColor = ThemeManager::PrimaryLight;
            btnSend->Location = Point(35, 240);
            btnSend->Size = System::Drawing::Size(310, 45);
            btnSend->Click += gcnew EventHandler(this, &Transaction::OnSend);

            this->Controls->Add(lblTitle); this->Controls->Add(lbl1); this->Controls->Add(lbl2);
            this->Controls->Add(txtRec); this->Controls->Add(txtAmt); this->Controls->Add(btnSend);
        }

        void OnSend(Object^ s, EventArgs^ e) {
            String^ cleanAmt = txtAmt->Text->Replace("$", "")->Replace(" ", "")->Replace(",", ".")->Trim();
            String^ res = BankService::Transfer(txtRec->Text, cleanAmt);
            if (res->StartsWith("SUCCESS")) { MessageBox::Show("Sent!"); this->Close(); }
            else MessageBox::Show(res);
        }
    };
}