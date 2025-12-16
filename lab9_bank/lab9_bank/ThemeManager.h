#pragma once

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

public ref class ThemeManager {
public:
    static Color PrimaryDark = Color::FromArgb(7, 33, 70);
    static Color PrimaryLight = Color::FromArgb(20, 120, 200);
    static Color SecondaryBg = Color::FromArgb(244, 248, 251);
    static Color CardBg = Color::White;
    static Color TextDark = Color::FromArgb(18, 18, 18);
    static Color TextLight = Color::White;

    static void Apply(Form^ form) {
        if (form == nullptr) return;
        form->BackColor = SecondaryBg;
        form->ForeColor = TextDark;
        if (form->Font->Name != "Segoe UI") {
            form->Font = gcnew System::Drawing::Font("Segoe UI", 10, FontStyle::Regular);
        }
        ApplyToControls(form->Controls);
    }

private:
    static void ApplyToControls(System::Windows::Forms::Control::ControlCollection^ controls) {
        if (controls == nullptr) return;

        for each (Control ^ c in controls) {
            if (c == nullptr) continue; // Перевірка на null

            if (dynamic_cast<Panel^>(c)) {
                Panel^ p = (Panel^)c;
                if (p->Dock == DockStyle::Top || p->BackColor == Color::MidnightBlue) {
                    p->BackColor = PrimaryDark;
                    ApplyTextColor(p->Controls, TextLight);
                }
                ApplyToControls(p->Controls);
            }
            else if (dynamic_cast<Button^>(c)) {
                Button^ b = (Button^)c;
                if (b->GetType()->Name == "Button") {
                    b->FlatStyle = FlatStyle::Flat;
                    b->FlatAppearance->BorderSize = 0;
                    b->BackColor = PrimaryLight;
                    b->ForeColor = TextLight;
                }
            }
            else if (dynamic_cast<TextBox^>(c)) {
                c->BackColor = CardBg;
                c->ForeColor = TextDark;
            }
            else if (dynamic_cast<Label^>(c)) {
                // Перевірка Parent != nullptr
                if (c->Parent != nullptr && c->Parent->BackColor != PrimaryDark) {
                    c->ForeColor = TextDark;
                }
            }
        }
    }

    static void ApplyTextColor(System::Windows::Forms::Control::ControlCollection^ controls, Color color) {
        if (controls == nullptr) return;
        for each (Control ^ c in controls) {
            if (c != nullptr) c->ForeColor = color;
        }
    }
};