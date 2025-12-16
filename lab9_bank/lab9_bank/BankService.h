#pragma once
#include "NetworkClient.h"

using namespace System;
using namespace System::Collections::Generic;

public ref class BankService {
private:
    static String^ DefaultIP = "172.20.10.3";
    static int DefaultPort = 8080;

public:
    static String^ Login(String^ username, String^ password) {
        if (!NetworkClient::Connect(DefaultIP, DefaultPort)) return "CONNECTION_FAILED";
        return NetworkClient::SendRequest("LOGIN " + username + " " + password);
    }

    static String^ Register(String^ username, String^ password) {
        if (!NetworkClient::Connect(DefaultIP, DefaultPort)) return "CONNECTION_FAILED";
        return NetworkClient::SendRequest("REG " + username + " " + password);
    }

    static String^ GetBalance() {
        String^ resp = NetworkClient::SendRequest("BALANCE");
        if (resp != nullptr && resp->StartsWith("BALANCE")) {
            return resp->Substring(8);
        }
        return "---";
    }

    static List<String^>^ GetHistory() {
        List<String^>^ list = gcnew List<String^>();
        String^ resp = NetworkClient::SendRequest("HISTORY");
        if (resp != nullptr && resp->Contains("HISTORY_START")) {
            array<String^>^ lines = resp->Split('\n');
            for each (String ^ line in lines) {
                String^ clean = line->Trim();
                if (clean != "HISTORY_START" && clean != "HISTORY_END" && clean->Length > 0) {
                    list->Add(clean);
                }
            }
        }
        return list;
    }

    static String^ Transfer(String^ receiver, String^ amount) {
        return NetworkClient::SendRequest("TRANS " + receiver + " " + amount);
    }

    // НОВЕ: Отримання курсів
    static String^ GetExchangeRates() {
        String^ resp = NetworkClient::SendRequest("RATES");
        if (resp == nullptr || resp->StartsWith("ERROR")) {
            return "RATES_API_ERROR";
        }
        // Чистимо відповідь від тегів
        return resp->Replace("RATES_START", "")->Replace("RATES_END", "")->Trim();
    }
};