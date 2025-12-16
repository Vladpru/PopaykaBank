#pragma once
#include "BankService.h"
#include "ThemeManager.h"

namespace lab9bank {
    using namespace System;
    using namespace System::Windows::Forms;
    using namespace System::Drawing;

    public ref class History : public System::Windows::Forms::Form
    {
    public:
        History(void) { InitializeComponent(); LoadData(); }
    private:
        DataGridView^ grid;
        Panel^ header;
        Label^ title;

        void InitializeComponent(void) {
            this->Size = System::Drawing::Size(600, 500);
            this->StartPosition = FormStartPosition::CenterParent;
            this->Text = "Transaction History";

            // Header
            header = gcnew Panel();
            header->Dock = DockStyle::Top;
            header->Height = 60;
            header->BackColor = ThemeManager::PrimaryDark;

            title = gcnew Label();
            title->Text = "Activity Log";
            title->ForeColor = Color::White;
            title->Font = gcnew System::Drawing::Font("Segoe UI", 14, FontStyle::Bold);
            title->Location = Point(20, 15);
            title->AutoSize = true;
            header->Controls->Add(title);
            this->Controls->Add(header);

            // Grid
            grid = gcnew DataGridView();
            grid->Dock = DockStyle::Fill;
            grid->BackgroundColor = Color::White;
            grid->BorderStyle = BorderStyle::None;
            grid->AllowUserToAddRows = false;
            grid->RowHeadersVisible = false;
            grid->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;

            // Styling Grid
            grid->EnableHeadersVisualStyles = false;
            grid->ColumnHeadersDefaultCellStyle->BackColor = Color::FromArgb(240, 240, 240);
            grid->ColumnHeadersDefaultCellStyle->ForeColor = Color::Black;
            grid->ColumnHeadersDefaultCellStyle->Font = gcnew System::Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            grid->ColumnHeadersHeight = 40;

            grid->DefaultCellStyle->Font = gcnew System::Drawing::Font("Segoe UI", 10);
            grid->DefaultCellStyle->SelectionBackColor = ThemeManager::PrimaryLight;
            grid->DefaultCellStyle->Padding = System::Windows::Forms::Padding(5);
            grid->RowTemplate->Height = 35;

            // Columns
            grid->Columns->Add("Desc", "Description");
            grid->Columns->Add("Date", "Date");

            this->Controls->Add(grid);
            header->SendToBack(); // ўоб гр≥д не перекрив
        }

        void LoadData() {
            for each (String ^ s in BankService::GetHistory()) {
                grid->Rows->Add(s, DateTime::Now.ToShortDateString());
            }
        }
    };
}