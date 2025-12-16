#pragma once
#include "BankService.h"
#include "Transaction.h"
#include "History.h"
#include "ExchangeRatesForm.h"
#include "ModernUI.h"
#include "ThemeManager.h"

namespace lab9bank {
    using namespace System;
    using namespace System::Windows::Forms;
    using namespace System::Drawing;
    using namespace ModernUI;

    public ref class MyForm : public System::Windows::Forms::Form
    {
    private:
        String^ _currentUser;
        Timer^ timer1;
    public:
        MyForm(String^ user) {
            InitializeComponent();
            _currentUser = user;
            lblUser->Text = "Welcome, " + _currentUser;
            UpdateBalance();
            timer1 = gcnew Timer();
            timer1->Interval = 5000;
            timer1->Tick += gcnew EventHandler(this, &MyForm::OnTimerTick);
            timer1->Start();
        }
    protected:
        ~MyForm() { if (components) delete components; }

    private:
        Panel^ topBar;
        Label^ lblLogo;
        Label^ lblUser;
        Button^ btnLogout;

        RoundedPanel^ balanceCard;
        Label^ lblBalTitle;
        Label^ lblBalValue;

        // Dashboard Buttons
        ModernButton^ btnTrans;
        ModernButton^ btnHist;
        ModernButton^ btnRates;

        System::ComponentModel::Container^ components;

        void InitializeComponent(void) {
            this->Size = System::Drawing::Size(800, 500);
            this->Text = "PopaykaBank Dashboard";
            this->StartPosition = FormStartPosition::CenterScreen;
            this->BackColor = ThemeManager::SecondaryBg;

            // --- TOP BAR ---
            topBar = gcnew Panel();
            topBar->Dock = DockStyle::Top;
            topBar->Height = 60;
            topBar->BackColor = ThemeManager::PrimaryDark;
            this->Controls->Add(topBar);

            lblLogo = gcnew Label();
            lblLogo->Text = "PopaykaBank";
            lblLogo->ForeColor = Color::White;
            lblLogo->Font = gcnew System::Drawing::Font("Segoe UI", 14, FontStyle::Bold);
            lblLogo->Location = Point(20, 15);
            lblLogo->AutoSize = true;
            topBar->Controls->Add(lblLogo);

            lblUser = gcnew Label();
            lblUser->ForeColor = Color::LightGray;
            lblUser->Font = gcnew System::Drawing::Font("Segoe UI", 10);
            lblUser->Location = Point(600, 20);
            lblUser->AutoSize = true;
            topBar->Controls->Add(lblUser);

            btnLogout = gcnew Button();
            btnLogout->Text = "Logout";
            btnLogout->FlatStyle = FlatStyle::Flat;
            btnLogout->FlatAppearance->BorderSize = 0;
            btnLogout->ForeColor = Color::White;
            btnLogout->Location = Point(700, 15);
            btnLogout->Click += gcnew EventHandler(this, &MyForm::OnLogout);
            topBar->Controls->Add(btnLogout);

            // --- BALANCE CARD ---
            balanceCard = gcnew RoundedPanel();
            balanceCard->Location = Point(40, 90);
            balanceCard->Size = System::Drawing::Size(700, 100);
            this->Controls->Add(balanceCard);

            lblBalTitle = gcnew Label();
            lblBalTitle->Text = "Total Balance";
            lblBalTitle->ForeColor = Color::Gray;
            lblBalTitle->Location = Point(20, 20);
            balanceCard->Controls->Add(lblBalTitle);

            lblBalValue = gcnew Label();
            lblBalValue->Text = "$ 0.00";
            lblBalValue->Font = gcnew System::Drawing::Font("Segoe UI", 24, FontStyle::Bold);
            lblBalValue->ForeColor = ThemeManager::PrimaryDark;
            lblBalValue->Location = Point(20, 45);
            lblBalValue->AutoSize = true;
            balanceCard->Controls->Add(lblBalValue);

            // --- ACTION CARDS (Всі сині) ---
            int yPos = 230;
            int btnW = 210;
            int gap = 35;

            // Transfer
            btnTrans = gcnew ModernButton();
            btnTrans->Text = "Transfer Money";
            btnTrans->PrimaryColor = ThemeManager::PrimaryLight; // Blue
            btnTrans->Location = Point(40, yPos);
            btnTrans->Size = System::Drawing::Size(btnW, 120);
            btnTrans->Click += gcnew EventHandler(this, &MyForm::btnTrans_Click);
            this->Controls->Add(btnTrans);

            // History
            btnHist = gcnew ModernButton();
            btnHist->Text = "History";
            btnHist->PrimaryColor = ThemeManager::PrimaryLight; // Blue
            btnHist->Location = Point(40 + btnW + gap, yPos);
            btnHist->Size = System::Drawing::Size(btnW, 120);
            btnHist->Click += gcnew EventHandler(this, &MyForm::btnHist_Click);
            this->Controls->Add(btnHist);

            // Rates
            btnRates = gcnew ModernButton();
            btnRates->Text = "Exchange Rates";
            btnRates->PrimaryColor = ThemeManager::PrimaryLight; // Blue
            btnRates->Location = Point(40 + 2 * (btnW + gap), yPos);
            btnRates->Size = System::Drawing::Size(btnW, 120);
            btnRates->Click += gcnew EventHandler(this, &MyForm::btnRates_Click);
            this->Controls->Add(btnRates);
        }

        void OnTimerTick(Object^ s, EventArgs^ e) { UpdateBalance(); }
        void UpdateBalance() {
            String^ bal = BankService::GetBalance();
            if (bal && bal != "---") {
                try {
                    double b = Convert::ToDouble(bal->Trim(), System::Globalization::CultureInfo::InvariantCulture);
                    lblBalValue->Text = "$ " + b.ToString("N2");
                }
                catch (...) { lblBalValue->Text = bal; }
            }
        }
        void OnLogout(Object^ s, EventArgs^ e) {
            NetworkClient::Disconnect(); Application::Restart(); Environment::Exit(0);
        }
        void btnTrans_Click(Object^ s, EventArgs^ e) { (gcnew Transaction())->ShowDialog(); UpdateBalance(); }
        void btnHist_Click(Object^ s, EventArgs^ e) { (gcnew History())->ShowDialog(); }
        void btnRates_Click(Object^ s, EventArgs^ e) { (gcnew ExchangeRatesForm())->ShowDialog(); }
    };
}