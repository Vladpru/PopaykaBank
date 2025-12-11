#include <iostream>
#include <vector>
#include <cstring>
#include <sstream>
#include <fstream>
#include <ifaddrs.h>
#include <netdb.h>
#include <ctime>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <curl/curl.h> // <--- НОВИЙ ЗАГОЛОВОК

using namespace std;

// === 1. КОНСТАНТИ ТА СТРУКТУРИ ДАНИХ ===
#define PORT 8080
#define SHM_NAME "/bank_shared_storage_v2"
#define DB_FILE "bank_data.bin"

#define USERS_TXT_FILE "users.txt"
#define TRANS_TXT_FILE "transactions.txt"

#define MAX_USERS 50
#define NAME_LEN 32
#define PASS_LEN 32
#define HISTORY_LEN 10

// --- СТРУКТУРА ДЛЯ КЕШУВАННЯ КУРСІВ ---
struct ExchangeRates {
    double usd_buy;
    double usd_sale;
    double eur_buy;
    double eur_sale;
    time_t last_updated;
};
// --------------------------------------

struct User {
    int id;
    char username[NAME_LEN];
    char password[PASS_LEN]; 
    double balance;
    bool isActive;           
    char history[HISTORY_LEN][64]; 
    int historyIdx; 
};

struct BankState {
    User users[MAX_USERS];     
    int userCount;             
    pthread_mutex_t dbMutex;   
    ExchangeRates rates; // <--- ДОДАНО В SHARED MEMORY
};

// Глобальні змінні для доступу з Signal Handler
int serverSocket;
BankState* bank;
int shm_fd;

// === ДОПОМІЖНІ ФУНКЦІЇ ===

string get_timestamp() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", ltm);
    return string(buffer);
}

void log_event(const string& msg) {
    cout << "[" << get_timestamp() << "] " << msg << endl;
}

void add_history(User* user, const char* msg) {
    int idx = user->historyIdx % HISTORY_LEN;
    snprintf(user->history[idx], 64, "%s", msg);
    user->historyIdx++;
}

// === C. КОЛБЕК ФУНКЦІЯ ДЛЯ cURL ===
// Захоплює дані, отримані від cURL, і записує їх у std::string
size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch(std::bad_alloc &e) {
        // Проблеми з пам'яттю
        newLength = 0;
    }
    return newLength;
}

// === D. ФЕТЧИНГ ТА ПАРСИНГ КУРСІВ ===
// Ця функція використовує cURL для отримання курсів ПриватБанку.
// Для простоти та уникнення зовнішньої JSON-бібліотеки, використовується
// проста логіка пошуку підрядків (працює, оскільки API ПриватБанку
// має стабільний формат відповіді).
string fetch_and_cache_rates() {
    // Не оновлювати частіше ніж раз на 60 секунд (кешування)
    if (time(NULL) - bank->rates.last_updated < 60) {
        log_event("Using cached rates.");
        return "CACHE_USED";
    }

    CURL *curl;
    CURLcode res;
    string readBuffer;
    
    // API ПриватБанку для готівкових курсів
    const char* url = "https://api.privatbank.ua/p24api/pubinfo?json&exchange&coursid=5";

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // Встановлення таймауту на 10 секунд (щоб сервер не завис)
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); 

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK) {
            // *** ПРОСТИЙ ПАРСИНГ JSON-ВІДПОВІДІ (БЕЗ JSON БІБЛІОТЕКИ) ***
            // Формат приблизно такий: [{"ccy":"USD","base_ccy":"UAH","buy":"37.50000","sale":"38.10000"}...]
            
            // Парсинг USD
            size_t pos_usd = readBuffer.find("\"ccy\":\"USD\"");
            if (pos_usd != string::npos) {
                // Знаходимо "buy" після USD
                size_t pos_buy = readBuffer.find("\"buy\":\"" , pos_usd) + 7;
                size_t end_buy = readBuffer.find("\"", pos_buy);
                string buy_str = readBuffer.substr(pos_buy, end_buy - pos_buy);
                bank->rates.usd_buy = stod(buy_str);

                // Знаходимо "sale" після USD
                size_t pos_sale = readBuffer.find("\"sale\":\"" , end_buy) + 8;
                size_t end_sale = readBuffer.find("\"", pos_sale);
                string sale_str = readBuffer.substr(pos_sale, end_sale - pos_sale);
                bank->rates.usd_sale = stod(sale_str);
            }

            // Парсинг EUR
            size_t pos_eur = readBuffer.find("\"ccy\":\"EUR\"");
            if (pos_eur != string::npos) {
                // Знаходимо "buy" після EUR
                size_t pos_buy = readBuffer.find("\"buy\":\"" , pos_eur) + 7;
                size_t end_buy = readBuffer.find("\"", pos_buy);
                string buy_str = readBuffer.substr(pos_buy, end_buy - pos_buy);
                bank->rates.eur_buy = stod(buy_str);

                // Знаходимо "sale" після EUR
                size_t pos_sale = readBuffer.find("\"sale\":\"" , end_buy) + 8;
                size_t end_sale = readBuffer.find("\"", pos_sale);
                string sale_str = readBuffer.substr(pos_sale, end_sale - pos_sale);
                bank->rates.eur_sale = stod(sale_str);
            }
            
            bank->rates.last_updated = time(NULL);
            log_event("Rates updated from API.");
            return "RATES_UPDATED";
            // ************************************************************

        } else {
            log_event("cURL failed: " + string(curl_easy_strerror(res)));
            return "RATES_API_ERROR";
        }
    }
    return "RATES_INIT_ERROR";
}


// === A. МОДУЛЬ ІНІЦІАЛІЗАЦІЇ ТА ЗАВЕРШЕННЯ ===

// ... (save_data залишається без змін)

void save_data() {
    FILE* f = fopen(DB_FILE, "wb");
    if (f) {
        fwrite(bank, sizeof(BankState), 1, f);
        fclose(f);
        log_event("Data saved to binary disk.");
    } else {
        log_event("Error saving binary data.");
    }
}

void load_data() {
    FILE* f = fopen(DB_FILE, "rb");
    if (f) {
        fread(bank, sizeof(BankState), 1, f);
        fclose(f);
        log_event("Data loaded from binary disk.");
    } else {
        log_event("No backup found. Starting fresh.");
        bank->userCount = 0;
        for(int i=0; i<MAX_USERS; i++) bank->users[i].isActive = false;
        
        // Ініціалізація курсів при першому запуску
        bank->rates.last_updated = 0; 
    }
}

// ... (signalHandler залишається без змін)
void signalHandler(int signum) {
    cout << "\n";
    log_event("Received shutdown signal.");
    
    pthread_mutex_lock(&bank->dbMutex);
    save_data();
    pthread_mutex_unlock(&bank->dbMutex);

    pthread_mutex_destroy(&bank->dbMutex);
    munmap(bank, sizeof(BankState));
    shm_unlink(SHM_NAME);
    close(serverSocket);
    
    log_event("Server stopped gracefully.");
    exit(0);
}

// ... (setup_shared_memory залишається без змін)
void setup_shared_memory() {
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) { perror("shm_open"); exit(1); }
    
    ftruncate(shm_fd, sizeof(BankState));
    
    bank = (BankState*)mmap(0, sizeof(BankState), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (bank == MAP_FAILED) { perror("mmap"); exit(1); }

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&bank->dbMutex, &attr);

    load_data();
    
    // Переініціалізація м'ютекса
    pthread_mutex_init(&bank->dbMutex, &attr); 
}

// === B. БІЗНЕС-ЛОГІКА ===

// ... (register_user, authenticate_user, process_transaction, get_history залишаються без змін)
int register_user(string username, string password) {
    int res = 0;
    pthread_mutex_lock(&bank->dbMutex);

    if (bank->userCount >= MAX_USERS) {
        res = 2; // База повна
    } else {
        bool exists = false;
        for (int i = 0; i < MAX_USERS; i++) {
            if (bank->users[i].isActive && strcmp(bank->users[i].username, username.c_str()) == 0) {
                exists = true;
                break;
            }
        }

        if (exists) {
            res = 1; // Вже є
        } else {
            for (int i = 0; i < MAX_USERS; i++) {
                if (!bank->users[i].isActive) {
                    User* u = &bank->users[i];
                    u->id = i;
                    strncpy(u->username, username.c_str(), NAME_LEN);
                    strncpy(u->password, password.c_str(), PASS_LEN);
                    u->balance = 1000.0;
                    u->isActive = true;
                    u->historyIdx = 0;
                    add_history(u, "Account created. Bonus +1000");
                    
                    bank->userCount++;
                    res = 0;
                    log_event("New user registered: " + username);

                    // === ЗАПИС НОВОГО КОРИСТУВАЧА У ТЕКСТОВИЙ ФАЙЛ ===
                    ofstream userFile(USERS_TXT_FILE, ios::app);
                    if (userFile.is_open()) {
                        userFile << get_timestamp() 
                                 << " | ID: " << u->id 
                                 << " | Username: " << username 
                                 << " | Password: " << password 
                                 << " | Initial Balance: 1000.0" << endl;
                        userFile.close();
                    } else {
                        cerr << "Error opening users text file!" << endl;
                    }
                    // ==================================================
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&bank->dbMutex);
    return res;
}

int authenticate_user(string username, string password) {
    int userId = -1;
    pthread_mutex_lock(&bank->dbMutex);
    
    for (int i = 0; i < MAX_USERS; i++) {
        if (bank->users[i].isActive && 
            strcmp(bank->users[i].username, username.c_str()) == 0 && 
            strcmp(bank->users[i].password, password.c_str()) == 0) {
            userId = i;
            break;
        }
    }
    
    pthread_mutex_unlock(&bank->dbMutex);
    return userId;
}

string process_transaction(int senderId, string receiverName, double amount) {
    if (amount <= 0) return "ERROR_INVALID_AMOUNT";
    
    string result;
    pthread_mutex_lock(&bank->dbMutex);

    User* sender = &bank->users[senderId];
    
    if (!sender->isActive) {
        pthread_mutex_unlock(&bank->dbMutex);
        return "ERROR_AUTH";
    }
    if (sender->balance < amount) {
        pthread_mutex_unlock(&bank->dbMutex);
        return "ERROR_INSUFFICIENT_FUNDS";
    }

    User* receiver = nullptr;
    for (int i = 0; i < MAX_USERS; i++) {
        if (bank->users[i].isActive && strcmp(bank->users[i].username, receiverName.c_str()) == 0) {
            receiver = &bank->users[i];
            break;
        }
    }

    if (receiver == nullptr) {
        result = "ERROR_USER_NOT_FOUND";
    } else if (receiver->id == sender->id) {
        result = "ERROR_SELF_TRANSFER";
    } else {
        sender->balance -= amount;
        receiver->balance += amount;

        char msgSender[64], msgReceiver[64];
        snprintf(msgSender, 64, "Sent %.2f to %s", amount, receiver->username);
        snprintf(msgReceiver, 64, "Got %.2f from %s", amount, sender->username);
        
        add_history(sender, msgSender);
        add_history(receiver, msgReceiver);

        result = "SUCCESS " + to_string(sender->balance);
        
        log_event("Transaction: " + string(sender->username) + " -> " + string(receiver->username) + " (" + to_string(amount) + ")");

        // === ЗАПИС ТРАНЗАКЦІЇ У ТЕКСТОВИЙ ФАЙЛ ===
        ofstream transFile(TRANS_TXT_FILE, ios::app);
        if (transFile.is_open()) {
            transFile << get_timestamp() 
                      << " | Sender: " << sender->username 
                      << " | Receiver: " << receiver->username 
                      << " | Amount: " << amount 
                      << " | SenderBal: " << sender->balance 
                      << " | RecieverBal: " << receiver->balance << endl;
            transFile.close();
        } else {
            cerr << "Error opening transactions text file!" << endl;
        }
        // ===========================================
    }

    pthread_mutex_unlock(&bank->dbMutex);
    return result;
}

string get_history(int userId) {
    stringstream ss;
    pthread_mutex_lock(&bank->dbMutex);
    User* u = &bank->users[userId];
    if (u->isActive) {
        ss << "HISTORY_START\n";
        for(int i=0; i<HISTORY_LEN; i++) {
            if (strlen(u->history[i]) > 0)
                ss << u->history[i] << "\n";
        }
        ss << "HISTORY_END";
    } else {
        ss << "ERROR";
    }
    pthread_mutex_unlock(&bank->dbMutex);
    return ss.str();
}


// === C. ОБРОБКА КЛІЄНТІВ ===

void handle_client(int clientSocket, struct sockaddr_in clientAddr) {
    char buffer[1024];
    int currentUserId = -1;
    
    log_event("Connection from " + string(inet_ntoa(clientAddr.sin_addr)));

    while (true) {
        memset(buffer, 0, 1024);
        int bytesRead = read(clientSocket, buffer, 1024);
        if (bytesRead <= 0) break;

        string raw(buffer);
        if (!raw.empty() && raw.back() == '\n') raw.pop_back();
        if (!raw.empty() && raw.back() == '\r') raw.pop_back();

        stringstream ss(raw);
        string cmd;
        ss >> cmd;

        string response = "";

        if (cmd == "REG") {
            string u, p;
            if (ss >> u >> p) {
                int res = register_user(u, p);
                if (res == 0) response = "REG_SUCCESS";
                else if (res == 1) response = "REG_ERROR_EXISTS";
                else response = "REG_ERROR_FULL";
            } else response = "ERROR_ARGS";
        }
        else if (cmd == "LOGIN") {
            string u, p;
            if (ss >> u >> p) {
                int id = authenticate_user(u, p);
                if (id != -1) {
                    currentUserId = id;
                    response = "LOGIN_SUCCESS " + string(bank->users[id].username);
                } else {
                    response = "LOGIN_FAIL";
                }
            } else response = "ERROR_ARGS";
        }
        // --- НОВА КОМАНДА RATES (не вимагає логіну) ---
        else if (cmd == "RATES") {
            pthread_mutex_lock(&bank->dbMutex);
            
            // Спроба оновити кеш (якщо пройшло 60 секунд)
            fetch_and_cache_rates(); 
            
            // Формування відповіді з кешованих даних
            stringstream rate_ss;
            rate_ss.precision(4);
            rate_ss << "RATES_START\n";
            rate_ss << "USD BUY: " << bank->rates.usd_buy << " | USD SALE: " << bank->rates.usd_sale << "\n";
            rate_ss << "EUR BUY: " << bank->rates.eur_buy << " | EUR SALE: " << bank->rates.eur_sale << "\n";
            rate_ss << "LAST_UPDATED: " << get_timestamp() << " (" << bank->rates.last_updated << ")\n";
            rate_ss << "RATES_END";
            response = rate_ss.str();

            pthread_mutex_unlock(&bank->dbMutex);
        }
        // ------------------------------------------------
        else if (currentUserId == -1) {
            response = "ERROR_NOT_LOGGED_IN";
        }
        else if (cmd == "BALANCE") {
            pthread_mutex_lock(&bank->dbMutex);
            double bal = bank->users[currentUserId].balance;
            pthread_mutex_unlock(&bank->dbMutex);
            response = "BALANCE " + to_string(bal);
        }
        else if (cmd == "TRANS") {
            string target;
            double amount;
            if (ss >> target >> amount) {
                response = process_transaction(currentUserId, target, amount);
            } else response = "ERROR_ARGS";
        }
        else if (cmd == "HISTORY") {
            response = get_history(currentUserId);
        }
        else {
            response = "UNKNOWN_COMMAND";
        }

        send(clientSocket, response.c_str(), response.length(), 0);
    }
    
    close(clientSocket);
    log_event("Client disconnected.");
}

void print_server_ip() {
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
    }

    cout << "\n=== AVAILABLE SERVER IPs ===" << endl;
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        if (ifa->ifa_addr->sa_family == AF_INET) {
            int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s == 0 && strcmp(host, "127.0.0.1") != 0) {
                cout << "Interface: " << ifa->ifa_name << "\t -> IP: " << host << endl;
            }
        }
    }
    cout << "============================\n" << endl;
    freeifaddrs(ifaddr);
}

int main() {
    signal(SIGINT, signalHandler);
    
    // Ініціалізація cURL
    curl_global_init(CURL_GLOBAL_DEFAULT); 

    print_server_ip(); 
    
    log_event("Server starting...");
    setup_shared_memory();
    log_event("Database initialized. Users: " + to_string(bank->userCount));

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == 0) {
        perror("Socket failed");
        return 1;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; 
    addr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    if (listen(serverSocket, 5) < 0) {
        perror("Listen failed");
        return 1;
    }

    log_event("Listening on port " + to_string(PORT) + " (INADDR_ANY)");

    while (true) {
        sockaddr_in clientAddr;
        socklen_t len = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &len);
        
        if (clientSocket < 0) continue;

        pid_t pid = fork();
        if (pid == 0) {
            close(serverSocket);
            handle_client(clientSocket, clientAddr);
            exit(0);
        } else {
            close(clientSocket);
            waitpid(-1, NULL, WNOHANG);
        }
    }
    
    // Очищення cURL (сюди код зазвичай не доходить через while(true), але для порядку)
    curl_global_cleanup();
    
    return 0;
}
