#pragma once
#include "BankService.h"
#include "ThemeManager.h"

namespace lab9bank {
    using namespace System;
    using namespace System::Windows::Forms;
    using namespace System::Drawing;

    public ref class ExchangeRatesForm : public System::Windows::Forms::Form
    {
    public:
        ExchangeRatesForm(void) { InitializeComponent(); Load(); }
    private:
        Label^ lblInfo;
        Panel^ head; Label^ headL;

        void InitializeComponent(void) {
            this->Size = System::Drawing::Size(400, 300);
            this->StartPosition = FormStartPosition::CenterParent;
            this->Text = "Rates";
            this->BackColor = Color::White;

            head = gcnew Panel(); head->Dock = DockStyle::Top; head->Height = 50; head->BackColor = ThemeManager::PrimaryDark;
            headL = gcnew Label(); headL->Text = "Exchange Rates"; headL->ForeColor = Color::White;
            headL->Font = gcnew System::Drawing::Font("Segoe UI", 12, FontStyle::Bold);
            headL->Location = Point(15, 12); headL->AutoSize = true;
            head->Controls->Add(headL); this->Controls->Add(head);

            lblInfo = gcnew Label();
            lblInfo->Dock = DockStyle::Fill;
            lblInfo->TextAlign = ContentAlignment::MiddleCenter;
            lblInfo->Font = gcnew System::Drawing::Font("Consolas", 11);
            lblInfo->Text = "Loading...";
            this->Controls->Add(lblInfo);
            head->SendToBack();
        }

        void Load() {
            lblInfo->Text = BankService::GetExchangeRates();
        }
    };
}