#pragma once
#include "BankService.h"
#include "Login.h"
#include "ModernUI.h"
#include "ThemeManager.h"

namespace lab9bank {
    using namespace System;
    using namespace System::Windows::Forms;
    using namespace System::Drawing;
    using namespace ModernUI;

    public ref class Registration : public System::Windows::Forms::Form
    {
    public:
        Registration(void) { InitializeComponent(); }
    protected:
        ~Registration() { if (components) delete components; }

    private:
        ModernTextBox^ txtUser;
        ModernTextBox^ txtPass;
        ModernButton^ btnReg;
        ModernButton^ btnLoginLink;
        Label^ lblTitle;
        System::ComponentModel::Container^ components;

        void InitializeComponent(void) {
            this->txtUser = gcnew ModernTextBox();
            this->txtPass = gcnew ModernTextBox();
            this->btnReg = gcnew ModernButton();
            this->btnLoginLink = gcnew ModernButton();
            this->lblTitle = gcnew Label();

            this->Size = System::Drawing::Size(500, 480);
            this->StartPosition = FormStartPosition::CenterScreen;
            this->Text = "Create Account";
            this->BackColor = ThemeManager::SecondaryBg; // Світлий фон

            // Title
            lblTitle->Text = "Join PopaykaBank";
            lblTitle->Font = gcnew System::Drawing::Font("Segoe UI", 20, FontStyle::Bold);
            lblTitle->ForeColor = ThemeManager::PrimaryDark;
            lblTitle->AutoSize = true;
            lblTitle->Location = Point(130, 40);

            // Inputs
            txtUser->Location = Point(100, 120);
            txtUser->Size = System::Drawing::Size(300, 40);

            txtPass->Location = Point(100, 180);
            txtPass->Size = System::Drawing::Size(300, 40);
            txtPass->UseSystemPasswordChar = true;

            // Register Button
            btnReg->Text = "Register";
            btnReg->PrimaryColor = ThemeManager::PrimaryDark; // Темно-синя
            btnReg->Location = Point(100, 260);
            btnReg->Size = System::Drawing::Size(300, 45);
            btnReg->Click += gcnew EventHandler(this, &Registration::button1_Click);

            // Login Button (Outlined - Чітка)
            btnLoginLink->Text = "Log In";
            btnLoginLink->IsOutlined = true; // Буде біла з синьою рамкою
            btnLoginLink->PrimaryColor = ThemeManager::PrimaryDark; // Колір тексту і рамки
            btnLoginLink->Location = Point(100, 320);
            btnLoginLink->Size = System::Drawing::Size(300, 45);
            btnLoginLink->Click += gcnew EventHandler(this, &Registration::button2_Click);

            this->Controls->Add(lblTitle);
            this->Controls->Add(txtUser);
            this->Controls->Add(txtPass);
            this->Controls->Add(btnReg);
            this->Controls->Add(btnLoginLink);
        }

        void button1_Click(Object^ s, EventArgs^ e) {
            String^ res = BankService::Register(txtUser->Text, txtPass->Text);
            if (res == "REG_SUCCESS") {
                MessageBox::Show("Success! Please login.");
                this->Hide(); (gcnew Login())->ShowDialog(); this->Close();
            }
            else MessageBox::Show(res);
        }
        void button2_Click(Object^ s, EventArgs^ e) {
            this->Hide(); (gcnew Login())->ShowDialog(); this->Close();
        }
    };
}