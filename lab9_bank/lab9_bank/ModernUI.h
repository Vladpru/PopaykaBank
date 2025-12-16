#pragma once

using namespace System;
using namespace System::Windows::Forms;
using namespace System::Drawing;
using namespace System::Drawing::Drawing2D;

namespace ModernUI {

    // --- КРУГЛА ПАНЕЛЬ ---
    public ref class RoundedPanel : public Panel {
    public:
        Color BorderColor = Color::Transparent;
        int Radius = 15;

        RoundedPanel() {
            this->DoubleBuffered = true;
            this->BackColor = Color::White;
            this->Padding = System::Windows::Forms::Padding(20);
        }

    protected:
        void OnPaint(PaintEventArgs^ e) override {
            try {
                e->Graphics->SmoothingMode = SmoothingMode::AntiAlias;
                // Малюємо фон батьківського елемента, щоб кути були чистими
                if (this->Parent != nullptr) {
                    e->Graphics->Clear(this->Parent->BackColor);
                }

                GraphicsPath^ path = gcnew GraphicsPath();
                Rectangle rect = this->ClientRectangle;
                rect.Width--; rect.Height--;

                int d = Radius * 2;
                path->AddArc(rect.X, rect.Y, d, d, 180, 90);
                path->AddArc(rect.Right - d, rect.Y, d, d, 270, 90);
                path->AddArc(rect.Right - d, rect.Bottom - d, d, d, 0, 90);
                path->AddArc(rect.X, rect.Bottom - d, d, d, 90, 90);
                path->CloseFigure();

                e->Graphics->FillPath(gcnew SolidBrush(this->BackColor), path);
                if (BorderColor != Color::Transparent) {
                    e->Graphics->DrawPath(gcnew Pen(BorderColor, 1.0f), path);
                }
            }
            catch (...) {}
        }
    };

    // --- СУЧАСНА КНОПКА (Виправлена) ---
    public ref class ModernButton : public Button {
    public:
        int Radius = 20;
        Color PrimaryColor = Color::FromArgb(0, 122, 204);
        bool IsOutlined = false;

        ModernButton() {
            this->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->FlatAppearance->BorderSize = 0;
            this->ForeColor = Color::White;
            this->Font = gcnew System::Drawing::Font("Segoe UI", 10, FontStyle::Bold); // Жирний шрифт
            this->Cursor = Cursors::Hand;
            this->Size = System::Drawing::Size(150, 40);
            this->BackColor = Color::Transparent; // Важливо
        }

    protected:
        void OnPaint(PaintEventArgs^ pevent) override {
            try {
                pevent->Graphics->SmoothingMode = SmoothingMode::AntiAlias;

                // Очищаємо фон під кнопкою кольором батька
                if (this->Parent != nullptr) {
                    pevent->Graphics->Clear(this->Parent->BackColor);
                }

                Rectangle rect = this->ClientRectangle;
                rect.Width--; rect.Height--;

                GraphicsPath^ path = gcnew GraphicsPath();
                int d = Radius; // Радіус закруглення
                path->AddArc(rect.X, rect.Y, d, d, 180, 90);
                path->AddArc(rect.Right - d, rect.Y, d, d, 270, 90);
                path->AddArc(rect.Right - d, rect.Bottom - d, d, d, 0, 90);
                path->AddArc(rect.X, rect.Bottom - d, d, d, 90, 90);
                path->CloseFigure();

                // Встановлюємо регіон кліку (обрізаємо кути фізично)
                this->Region = gcnew System::Drawing::Region(path);

                if (IsOutlined) {
                    // Тільки рамка
                    pevent->Graphics->FillPath(gcnew SolidBrush(Color::White), path); // Білий фон всередині
                    pevent->Graphics->DrawPath(gcnew Pen(PrimaryColor, 2.0f), path);
                    pevent->Graphics->DrawString(this->Text, this->Font, gcnew SolidBrush(PrimaryColor), GetTextRect(pevent->Graphics));
                }
                else {
                    // Суцільна заливка
                    pevent->Graphics->FillPath(gcnew SolidBrush(PrimaryColor), path);
                    pevent->Graphics->DrawString(this->Text, this->Font, gcnew SolidBrush(this->ForeColor), GetTextRect(pevent->Graphics));
                }
            }
            catch (...) {}
        }

    private:
        PointF GetTextRect(Graphics^ g) {
            SizeF stringSize = g->MeasureString(this->Text, this->Font);
            return PointF(
                (float)(this->Width - stringSize.Width) / 2,
                (float)(this->Height - stringSize.Height) / 2
            );
        }
    };

    // --- ТЕКСТОВЕ ПОЛЕ ---
    public ref class ModernTextBox : public UserControl {
    private:
        TextBox^ txt;
    public:
        Color BorderColor = Color::Gray;

        property String^ Text {
            String^ get() override { return txt != nullptr ? txt->Text : ""; }
            void set(String^ value) override { if (txt != nullptr) txt->Text = value; }
        }
        property bool UseSystemPasswordChar {
            bool get() { return txt != nullptr ? txt->UseSystemPasswordChar : false; }
            void set(bool value) { if (txt != nullptr) txt->UseSystemPasswordChar = value; }
        }

        ModernTextBox() {
            txt = gcnew TextBox();
            this->Size = System::Drawing::Size(300, 45); // Трохи вище
            this->BackColor = Color::White;
            this->Padding = System::Windows::Forms::Padding(2);

            txt->BorderStyle = System::Windows::Forms::BorderStyle::None;
            txt->BackColor = Color::White;
            txt->Font = gcnew System::Drawing::Font("Segoe UI", 12); // Більший шрифт
            txt->Location = Point(10, 10);
            txt->Width = this->Width - 20;

            this->Controls->Add(txt);
        }

    protected:
        void OnPaint(PaintEventArgs^ e) override {
            try {
                e->Graphics->SmoothingMode = SmoothingMode::AntiAlias;
                if (this->Parent != nullptr) e->Graphics->Clear(this->Parent->BackColor);

                Pen^ pen = gcnew Pen(BorderColor, 1);
                Rectangle rect = this->ClientRectangle;
                rect.Width--; rect.Height--;

                int r = 10;
                GraphicsPath^ path = gcnew GraphicsPath();
                path->AddArc(rect.X, rect.Y, r, r, 180, 90);
                path->AddArc(rect.Right - r, rect.Y, r, r, 270, 90);
                path->AddArc(rect.Right - r, rect.Bottom - r, r, r, 0, 90);
                path->AddArc(rect.X, rect.Bottom - r, r, r, 90, 90);
                path->CloseFigure();

                e->Graphics->FillPath(gcnew SolidBrush(Color::White), path);
                e->Graphics->DrawPath(pen, path);
            }
            catch (...) {}
        }

        void OnResize(EventArgs^ e) override {
            if (txt != nullptr) txt->Width = this->Width - 20;
            this->Invalidate();
        }
    };
}