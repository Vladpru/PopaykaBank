#pragma once
#include "BankService.h"
#include "MyForm.h"
#include "ModernUI.h"
#include "ThemeManager.h"

namespace lab9bank {
    using namespace System;
    using namespace System::Windows::Forms;
    using namespace System::Drawing;
    using namespace ModernUI;

    public ref class Login : public System::Windows::Forms::Form
    {
    public:
        Login(void) { InitializeComponent(); }

    protected:
        ~Login() { if (components) delete components; }

    private:
        ModernTextBox^ txtUser;
        ModernTextBox^ txtPass;
        ModernButton^ btnLogin;
        Label^ lblTitle;
        Label^ lblDesc;
        System::ComponentModel::Container^ components;

        void InitializeComponent(void) {
            this->lblTitle = gcnew Label();
            this->lblDesc = gcnew Label();
            this->txtUser = gcnew ModernTextBox();
            this->txtPass = gcnew ModernTextBox();
            this->btnLogin = gcnew ModernButton();

            // Налаштування форми (Світла тема)
            this->Size = System::Drawing::Size(400, 550);
            this->StartPosition = FormStartPosition::CenterScreen;
            this->Text = "Login";
            this->BackColor = ThemeManager::SecondaryBg; // Світло-сірий фон

            // Заголовок
            lblTitle->Text = "Welcome Back";
            lblTitle->Font = gcnew System::Drawing::Font("Segoe UI", 22, FontStyle::Bold);
            lblTitle->ForeColor = ThemeManager::PrimaryDark; // Темно-синій текст
            lblTitle->AutoSize = true;
            lblTitle->Location = Point((this->Width - 220) / 2, 80);

            lblDesc->Text = "Please enter your details";
            lblDesc->Font = gcnew System::Drawing::Font("Segoe UI", 10);
            lblDesc->ForeColor = Color::Gray;
            lblDesc->AutoSize = true;
            lblDesc->Location = Point((this->Width - 160) / 2, 125);

            // Поля вводу
            txtUser->Location = Point(50, 180);
            txtUser->Size = System::Drawing::Size(280, 45);
            // Плейсхолдери можна імітувати, але поки просто пусті

            txtPass->Location = Point(50, 240);
            txtPass->Size = System::Drawing::Size(280, 45);
            txtPass->UseSystemPasswordChar = true;

            // Кнопка (Синя суцільна)
            btnLogin->Text = "Log In";
            btnLogin->PrimaryColor = ThemeManager::PrimaryLight; // Яскраво-синій
            btnLogin->Location = Point(50, 320);
            btnLogin->Size = System::Drawing::Size(280, 50);
            btnLogin->Click += gcnew EventHandler(this, &Login::button1_Click);

            // Лінк на реєстрацію
            Label^ regLink = gcnew Label();
            regLink->Text = "Don't have an account? Sign up";
            regLink->ForeColor = ThemeManager::PrimaryLight;
            regLink->AutoSize = true;
            regLink->Cursor = Cursors::Hand;
            regLink->Font = gcnew System::Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            regLink->Location = Point(85, 400);
            regLink->Click += gcnew EventHandler(this, &Login::onRegClick);

            this->Controls->Add(lblTitle);
            this->Controls->Add(lblDesc);
            this->Controls->Add(txtUser);
            this->Controls->Add(txtPass);
            this->Controls->Add(btnLogin);
            this->Controls->Add(regLink);
        }

        void button1_Click(Object^ sender, EventArgs^ e) {
            if (txtUser == nullptr || txtPass == nullptr) return;
            String^ res = BankService::Login(txtUser->Text, txtPass->Text);
            if (res->StartsWith("LOGIN_SUCCESS")) {
                this->Hide();
                MyForm^ form = gcnew MyForm(txtUser->Text);
                form->ShowDialog();
                this->Close();
            }
            else {
                MessageBox::Show("Login Failed: " + res);
            }
        }

        void onRegClick(Object^ sender, EventArgs^ e) {
            // Тут можна відкрити форму реєстрації, але за логікою програми
            // реєстрація відкривається з main.cpp, тому просто закриємо логін
            // або покажемо повідомлення
            MessageBox::Show("Please restart app to register (Architecture limitation)");
        }
    };
}