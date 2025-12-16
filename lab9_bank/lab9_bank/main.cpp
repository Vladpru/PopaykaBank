#include "Registration.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThread]
int main(array<String^>^ args) {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    lab9bank::Registration^ form = gcnew lab9bank::Registration();
    Application::Run(form);

    return 0;
}