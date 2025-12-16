#pragma once
using namespace System;
using namespace System::Net::Sockets;
using namespace System::Text;
using namespace System::Windows::Forms;

public ref class NetworkClient {
private:
    static TcpClient^ _client;
    static NetworkStream^ _stream;

public:
    static bool Connect(String^ ip, int port) {
        try {
            Disconnect();
            _client = gcnew TcpClient();
            _client->ReceiveTimeout = 5000;
            _client->SendTimeout = 5000;
            _client->Connect(ip, port);
            _stream = _client->GetStream();
            return true;
        }
        catch (Exception^ ex) {
            MessageBox::Show("Server Connection Error: " + ex->Message, "Network Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
            return false;
        }
    }

    static String^ SendRequest(String^ message) {
        if (_client == nullptr || !_client->Connected) return "ERROR_NO_CONNECTION";

        try {
            array<Byte>^ data = Encoding::UTF8->GetBytes(message + "\n");
            _stream->Write(data, 0, data->Length);

            array<Byte>^ buffer = gcnew array<Byte>(4096);
            int bytesRead = _stream->Read(buffer, 0, buffer->Length);

            if (bytesRead == 0) {
                Disconnect();
                return "ERROR_SERVER_CLOSED";
            }

            String^ response = Encoding::UTF8->GetString(buffer, 0, bytesRead);
            return response->Trim();
        }
        catch (Exception^) {
            Disconnect();
            return "ERROR_CONNECTION_LOST";
        }
    }

    static void Disconnect() {
        if (_stream != nullptr) _stream->Close();
        if (_client != nullptr) _client->Close();
        _client = nullptr;
        _stream = nullptr;
    }
};