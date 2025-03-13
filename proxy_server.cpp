#include <algorithm>
#include <cstring>
#include <ctime>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define PORT 8080 // Cổng của proxy server
#define BUFFER_SIZE 4096
#define IP_ADDRESS "0.0.0.0"

// Global variables
bool FLAG = true;      
int byteReceived = 0;                             
int byteSent = 0;                                  
int pivot = 0;   
int maxReceivedBytes = 100;
int maxSentBytes = 100;   
int ClientOut = 0;                                                                                                                                                                                                                                  
time_t now = time(0);                                                                                                                                                                                                                            
vector<thread> threads;                                                                                                                                                                                                                          
HWND hwnd, hDrawbox, hTextbox3, hTextbox1, hTextbox2 , 
    hSearchBar, hButtonAdd, hButtonStart, hButtonStop, hButtonBan, 
    hButtonUnban, hButtonRemove, hTextbox4, hSearchBar1, hSearchBar2, 
    Log_label, performance, stat, ban_label, Trace, TraceBox; 
string output, stringBan = ""; 
vector<pair<string, string>> user_ban_list;                                   
vector<string> ban_list;  
set<string> user_list;      
unordered_map<string, bool> addedStrings;                       
unordered_map<string, string> IPv4ConnectToDomain; // Lưu trữ địa chỉ IP của domain
deque<int> DataReicevedQueue = {0, 0, 0}; // Queue lưu trữ số byte nhận được và gửi đi
deque<int> DataSentedQueue = {0, 0, 0};   // Queue lưu trữ số byte nhận được và gửi đi
SOCKET proxySocket; 
RECT rect_drawbox;
struct INFO {
    string domain;
    string client_ip;
    string time_connect;
};

vector<INFO> info;

void relayData(SOCKET srcSocket, SOCKET destSocket) { // Hàm chuyển dữ liệu từ srcSocket sang destSocket
    char buffer[BUFFER_SIZE];                         // buffer dùng để chứa dữ liệu
    int bytesRead;                                    // Số byte đọc được từ srcSocket

    while ((bytesRead = recv(srcSocket, buffer, sizeof(buffer), 0)) > 0 && FLAG) { // Đọc dữ liệu từ srcSocket
        byteSent += bytesRead;                            // Cập nhật số byte nhận được
        if (send(destSocket, buffer, bytesRead, 0) < 0) { // Gửi dữ liệu đến destSocket
            cerr << "Error writing to socket" << endl;    // In ra lỗi nếu gửi dữ liệu không thành công
            break;
        }
    }
    ClientOut++;
}

void printReceivedAndSentBytes() {
    while (true) {
        Sleep(1000);
        // Tinh toan so byte nhan va gui
        int received = byteReceived / abs(time(0) - now);
        int sent = byteSent / abs(time(0) - now);

        string text = "Received: " + to_string(received) + " bytes/s\r\nSent: " + to_string(sent) + " bytes/s\r\n";
        if (FLAG) {
            SendMessage(hTextbox4, EM_SETSEL, 0, -1);
            SendMessage(hTextbox4, EM_REPLACESEL, FALSE, (LPARAM)text.c_str());
        }

        byteReceived = 0;
        byteSent = 0;
        now = time(0);
    }
    string text = "Received: " + to_string(0) + " bytes/s\r\nSent: " + to_string(0) + " bytes/s\r\n" + "Number of Opening port: " + to_string(0);
    SendMessage(hTextbox4, EM_SETSEL, 0, -1);
    SendMessage(hTextbox4, EM_REPLACESEL, FALSE, (LPARAM)text.c_str());
}
void printGraph() {
    
    while (true) {
        DataReicevedQueue.push_back(byteReceived);
        if (DataReicevedQueue.size() > 10)
            DataReicevedQueue.pop_front();
        maxReceivedBytes = *max_element(DataReicevedQueue.begin(), DataReicevedQueue.end());
        maxReceivedBytes = maxReceivedBytes == 0 ? 1000 : maxReceivedBytes;
        maxReceivedBytes = maxReceivedBytes * 1.5;

        DataSentedQueue.push_back(byteSent);
        if (DataSentedQueue.size() > 10)
            DataSentedQueue.pop_front();
        maxSentBytes = max(100, *max_element(DataSentedQueue.begin(), DataSentedQueue.end()));
        maxSentBytes = maxSentBytes * 3;

        PostMessage(hwnd, WM_USER + 1, 0, 0); // WM_USER + 1 is a custom message
        
        Sleep(1000); // Simulate some work
        int textLength = SendMessage(hTextbox1, WM_GETTEXTLENGTH, 0, 0);
        cout << textLength << endl;
        if (textLength > 20000) {
            SetWindowTextA(hTextbox1, "");
            SendMessage(hTextbox1, EM_SETSEL, 0, -1);
            SendMessage(hTextbox1, EM_REPLACESEL, FALSE, (LPARAM)output.c_str());
        }
    }
}

bool bann(char *hostName) { // Hàm kiểm tra hostName có nằm trong danh sách ban_list không
    // Chuyển đổi char* sang string
    string host(hostName);    // host là hostName dưới dạng string
    for (auto s : ban_list) { // Duyệt qua các phần tử trong ban_list
        if (s == host)        // Nếu hostName nằm trong ban_list
            return false;
    }
    return true;
}
bool ban_1_user(char *hostName, char *ip) {
    string domain(hostName);
    string IP(ip);
    for (auto e : user_ban_list) {
        if (e.second == IP && e.first == domain) {
            return false;
        }
    }
    return true;
}

void UpdateTraceBox() {
    // Kiểm tra xem TraceBox có hợp lệ không
    if (!TraceBox)
        return;

    // Tạo một chuỗi để lưu toàn bộ dữ liệu
    ostringstream oss;

    // Duyệt qua vector để định dạng dữ liệu
    for (const auto &entry : info) {
        oss << setw(21) << left << (entry.domain.length() > 21 ? entry.domain.substr(0,18) + "..." : entry.domain) << " " << setw(15) << right << entry.client_ip << "\r\n";
    }

    // Lấy chuỗi định dạng
    string result = oss.str();
    // Xóa nội dung cũ và cập nhật nội dung mới
    SendMessage(TraceBox, EM_SETSEL, 0, -1);
    SendMessage(TraceBox, EM_REPLACESEL, FALSE, (LPARAM)result.c_str());
}

void handleClient(SOCKET clientSocket, char *ip) { // Hàm xử lý dữ liệu từ client
    cout << "Num client: " << user_list.size() << endl;
    string CIP(ip);
    ofstream out("server.txt"); // Mở file server.txt để ghi dữ liệu
    char buffer[BUFFER_SIZE];   // buffer dùng để chứa dữ liệu

    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0); // Đọc dữ liệu từ clientSocket
    byteReceived += bytesRead;                                         // Cập nhật số byte nhận được
    if (bytesRead <= 0) {                                              // Nếu không đọc được dữ liệu
        cerr << "Error reading from client socket" << endl;            // In ra lỗi
        closesocket(clientSocket);
        user_list.erase(CIP);
        ClientOut++;
        return;
    }
    buffer[bytesRead] = '\0'; // Kết thúc chuỗi buffer
    user_list.insert(CIP);

    // Xử lý các dòng dữ liệu từ client
    istringstream stream(buffer);
    string line, connect_line, host_line, port;
    int line_number = 0;

    while (getline(stream, line)) {
        line_number++;
        if (line_number == 1) {
            connect_line = line; // Dòng CONNECT
        } else if (line_number == 2) {
            host_line = line; // Dòng Host
            // Tách port từ dòng Host
            size_t colon_pos = host_line.find(":");
            if (colon_pos != string::npos) {
                port = host_line.substr(colon_pos + 1);
                port = port.substr(0, port.find(" ")); // Xóa phần dư nếu có
            }
        }
    }

    // Đưa kết quả về 1 văn bản (chuỗi)
    string buff = connect_line + "\r\n" +
                       host_line + "\r\n" + "Client IP: " + CIP + "\r\n----------------\r\n";
    
    output += buff;
    int length = output.length();
    
    if (pivot != length) {
        // cout<<0<<endl;
        int dis = length - pivot;
        SendMessage(hTextbox1, EM_SETSEL, length, length);
        // cout << output.str() << endl;
        SendMessage(hTextbox1, EM_REPLACESEL, FALSE, (LPARAM)output.c_str());
        //cout << output.str().length() << endl;

        // Scroll to the end of the content
        SendMessage(hTextbox1, EM_SCROLL, SB_BOTTOM, 0);
        pivot = length;
    }
    char method[16], url[256], protocol[16];           // Biến lưu trữ method, url, protocol
    sscanf(buffer, "%s %s %s", method, url, protocol); // Đọc method, url, protocol từ buffer

    if (strcmp(method, "CONNECT") == 0) {            // Nếu method là CONNECT
        char hostname[256];                          // Biến lưu trữ hostname
        int port = 443;                              // Cổng mặc định
        sscanf(url, "%255[^:]:%d", hostname, &port); // Đọc hostname và port từ url
        string host_string(hostname);
        bool toAdd = true;
        for (auto e : info) {
            if (e.domain == host_string && e.client_ip == CIP)
                toAdd = false;
        }
        if (toAdd) {
            info.push_back({host_string, CIP, "NAN"});
            UpdateTraceBox();
        }
        if (!bann(hostname) || !ban_1_user(hostname, ip)) { // Nếu hostname nằm trong danh sách ban_list
            const char *response = "HTTP/1.1 403 Forbidden\r\n"
                                   "Content-Type: text/html\r\n"
                                   "Content-Length: 85\r\n"
                                   "Connection: close\r\n\r\n"
                                   "<html><body><h1>403 Forbidden</h1>"
                                   "<p>Your access to this resource has been denied.</p></body></html>"; // Gửi thông báo lỗi
            send(clientSocket, response, strlen(response), 0);                                           // Gửi thông báo lỗi đến client
            //---------------------------------------------------------
            byteSent += strlen(response); // Cập nhật số byte nhận được
            closesocket(clientSocket);
            user_list.erase(CIP);
            ClientOut++;
            return;
        }
        
        SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Tạo socket server
        if (serverSocket == INVALID_SOCKET) {                  // Nếu không tạo được socket
            cerr << "Could not create server socket" << endl;
            closesocket(clientSocket);
            user_list.erase(CIP);
            ClientOut++;
            return;
        }

        struct hostent *host = gethostbyname(hostname); // Lấy thông tin host từ hostname
        if (host == nullptr) {                          // Nếu không lấy được thông tin host
            cerr << "Host resolution failed" << endl;
            closesocket(clientSocket);
            user_list.erase(CIP);
            closesocket(serverSocket);
            ClientOut++;
            return;
        }

        struct sockaddr_in serverAddr;                              // Địa chỉ server
        serverAddr.sin_family = AF_INET;                            // Giao thức AF_INET
        serverAddr.sin_port = htons(port);                          // Cổng server
        memcpy(&serverAddr.sin_addr, host->h_addr, host->h_length); // Sao chép địa chỉ IP của host

        if (connect(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) { // Kết nối đến server
            cerr << "Connection to server failed" << endl;
            closesocket(clientSocket);
            user_list.erase(CIP);
            closesocket(serverSocket);
            ClientOut++;
            return;
        }

        const char *response = "HTTP/1.1 200 Connection Established\r\n\r\n"; // Gửi thông báo kết nối thành công
        send(clientSocket, response, strlen(response), 0);                    // Gửi thông báo kết nối thành công đến client
        //---------------------------------------------------------
        byteSent += strlen(response); // Cập nhật số byte nhận được

        while (FLAG) {                                        // Lặp để chuyển dữ liệu giữa client và server
            char buffer2[BUFFER_SIZE];                        // buffer2 dùng để chứa dữ liệu
            fd_set fds;                                       // Tập file descriptor
            FD_ZERO(&fds);                                    // Xóa tập file descriptor
            FD_SET(clientSocket, &fds);                       // Thêm clientSocket vào tập file descriptor
            FD_SET(serverSocket, &fds);                       // Thêm serverSocket vào tập file descriptor
            int max_fd = max(clientSocket, serverSocket) + 1; // Số file descriptor tối đa

            if (select(0, &fds, nullptr, nullptr, nullptr) == SOCKET_ERROR) { // Kiểm tra lỗi select
                cerr << "Select error" << endl;
                break;
            }

            if (FD_ISSET(clientSocket, &fds)) {                                  // Nếu clientSocket có trong tập file descriptor
                int bytes = recv(clientSocket, buffer2, sizeof(buffer2) - 1, 0); // Đọc dữ liệu từ clientSocket
                //---------------------------------------------------------
                byteReceived += bytes; // Cập nhật số byte nhận được
                if (bytes <= 0)        // Nếu không đọc được dữ liệu
                    break;
                if (send(serverSocket, buffer2, bytes, 0) != bytes) { // Gửi dữ liệu đến serverSocket
                    //---------------------------------------------------------
                    byteSent += bytes; // Cập nhật số byte nhận được
                    cerr << "Write to server error" << endl;
                    break;
                }
            }
            if (!bann(hostname) || !ban_1_user(hostname, ip)) { // Nếu hostname không nằm trong danh sách ban_list
                break;
            }

            if (FD_ISSET(serverSocket, &fds)) {                                  // Nếu serverSocket có trong tập file descriptor
                int bytes = recv(serverSocket, buffer2, sizeof(buffer2) - 1, 0); // Đọc dữ liệu từ serverSocket
                //---------------------------------------------------------
                byteReceived += bytes; // Cập nhật số byte nhận được
                if (bytes <= 0)        // Nếu không đọc được dữ liệu
                    break;
                if (send(clientSocket, buffer2, bytes, 0) != bytes) { // Gửi dữ liệu đến clientSocket
                    //---------------------------------------------------------
                    byteSent += bytes; // Cập nhật số byte nhận được
                    cerr << "Write to client error" << endl;
                    break;
                }
            }
        }
        closesocket(serverSocket); // Đóng serverSocket

    } else {
        char hostname[256], path[256] = "/";                 // Biến lưu trữ hostname và path
        sscanf(url, "http://%255[^/]%255s", hostname, path); // Đọc hostname và path từ url

        SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Tạo socket server
        if (serverSocket == INVALID_SOCKET) {                  // Nếu không tạo được socket
            cerr << "Could not create server socket" << endl;
            closesocket(clientSocket);
            user_list.erase(CIP);
            ClientOut++;
            return;
        }

        struct hostent *host = gethostbyname(hostname); // Lấy thông tin host từ hostname
        if (host == nullptr) {                          // Nếu không lấy được thông tin host
            cerr << "Host resolution failed" << endl;
            closesocket(clientSocket);
            user_list.erase(CIP);
            closesocket(serverSocket);
            ClientOut++;
            return;
        }

        struct sockaddr_in serverAddr; // Địa chỉ server
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(80);
        memcpy(&serverAddr.sin_addr, host->h_addr, host->h_length); // Sao chép địa chỉ IP của host

        if (connect(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) { // Kết nối đến server
            cerr << "Connection to server failed" << endl;
            closesocket(clientSocket);
            user_list.erase(CIP);
            closesocket(serverSocket);
            ClientOut++;
            return;
        }

        string request = string(method) + " " + path + " " + protocol + "\r\n"; // Tạo request
        request += "Host: " + string(hostname) + "\r\n";                             // Thêm Host vào request
        request += "Connection: close\r\n\r\n";                                           // Thêm Connection vào request

        send(serverSocket, buffer, bytesRead, 0); // Gửi request đến server
        byteSent += bytesRead; // Cập nhật số byte nhận được

        while ((bytesRead = recv(serverSocket, buffer, sizeof(buffer), 0)) > 0 && FLAG) { // Đọc dữ liệu từ server

            byteReceived += bytesRead;                // Cập nhật số byte nhận được
            send(clientSocket, buffer, bytesRead, 0); // Gửi dữ liệu đến client

            byteSent += bytesRead;                              // Cập nhật số byte nhận được
            buffer[sizeof(buffer) - 1] = '\0';                  // Kết thúc chuỗi buffer
            if (!bann(hostname) || !ban_1_user(hostname, ip)) { // Nếu hostname không nằm trong danh sách ban_list
                break;
            }
            // out << buffer; // Ghi dữ liệu vào file server.txt
        }
        closesocket(serverSocket);
    }
    closesocket(clientSocket);
    user_list.erase(CIP);
    ClientOut++;
}
int proxy_server() {
    WSADATA wsaData;                                 // Biến lưu trữ thông tin Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { // Khởi tạo Winsock
        cerr << "WSAStartup failed" << endl;
        ClientOut++;
        return 1;
    }

    proxySocket = socket(AF_INET, SOCK_STREAM, 0); // Tạo proxy socket
    if (proxySocket == INVALID_SOCKET) {
        cerr << "Could not create proxy socket" << endl;
        WSACleanup();
        ClientOut++;
        return 1;
    }

    struct sockaddr_in proxyAddr; // Địa chỉ proxy
    proxyAddr.sin_family = AF_INET;
    proxyAddr.sin_addr.s_addr = INADDR_ANY; // Địa chỉ IP của proxy
    proxyAddr.sin_port = htons(PORT);

    if (::bind(proxySocket, (struct sockaddr *)&proxyAddr, sizeof(proxyAddr)) < 0) { // Bind proxy socket
        cerr << "Bind failed" << endl;
        closesocket(proxySocket);
        WSACleanup();
        ClientOut++;
        return 1;
    }

    if (listen(proxySocket, 10) < 0) { // Listen proxy socket
        cerr << "Listen failed" << endl;
        closesocket(proxySocket);
        WSACleanup();
        ClientOut++;
        return 1;
    }

    cout << "Proxy server is running on port " << PORT << endl;

    while (FLAG) {

        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        // Lặp để chấp nhận kết nối từ client
        SOCKET clientSocket = accept(proxySocket, (struct sockaddr *)&client_addr, &client_addr_len); // Chấp nhận kết nối từ client

        // cerr << "Connected to client\n";
        if (clientSocket == INVALID_SOCKET) { // Nếu không chấp nhận được kết nối
            cerr << "Accept failed\n";
            ClientOut++;
            continue;
        }
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        
        threads.emplace_back(handleClient, clientSocket, client_ip); // Tạo một thread để xử lý dữ liệu từ client
        threads.back().detach(); // Tách thread
        
    }

    closesocket(proxySocket); // Đóng proxySocket
    WSACleanup();             // Kết thúc Winsock
    ClientOut++;
    return 0;
}

//
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

    static HWND hwndTextBox;

    switch (msg) {
    
    case WM_MOVE: {
        UpdateWindow(hwnd);
    } break;
    case WM_SIZE: {
        UpdateWindow(hwnd);
    } break;
    case WM_DESTROY: {
        PostQuitMessage(0);
    } break;

    case WM_USER + 1: {
        PAINTSTRUCT ps;
        HDC hds = BeginPaint(hDrawbox, &ps);
        GetClientRect(hDrawbox, &rect_drawbox);
        InvalidateRect(hDrawbox, &rect_drawbox, TRUE);

        HBRUSH hbrWhite = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hds, &rect_drawbox, hbrWhite);
        HPEN hPenAxis = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));     // Black axes
        HPEN hPenGrid = CreatePen(PS_DOT, 1, RGB(200, 200, 200)); // Light gray grid
        HPEN hPenChart = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));  // Red chart line
        HPEN hPenChart2 = CreatePen(PS_DOT, 1, RGB(0, 0, 150)); // Green chart line

        // Draw the grid
        SelectObject(hds, hPenGrid);
        for (int i = 1; i < 10; i++) {
            MoveToEx(hds, rect_drawbox.left, rect_drawbox.bottom - i * (rect_drawbox.bottom - rect_drawbox.top) / 10, NULL);
            LineTo(hds, rect_drawbox.right, rect_drawbox.bottom - i * (rect_drawbox.bottom - rect_drawbox.top) / 10); // Horizontal lines
            MoveToEx(hds, rect_drawbox.left + i * (rect_drawbox.right - rect_drawbox.left) / 10, rect_drawbox.bottom, NULL);
            LineTo(hds, rect_drawbox.left + i * (rect_drawbox.right - rect_drawbox.left) / 10, rect_drawbox.top); // Vertical lines
        }

        // Draw the char
        SelectObject(hds, hPenChart);
        for (int i = 1; i < DataReicevedQueue.size(); i++) {
            MoveToEx(hds, rect_drawbox.left + (i - 1) * (rect_drawbox.right - rect_drawbox.left) / 10, rect_drawbox.bottom - DataReicevedQueue[i - 1] * (rect_drawbox.bottom - rect_drawbox.top) / maxReceivedBytes, NULL);
            LineTo(hds, rect_drawbox.left + i * (rect_drawbox.right - rect_drawbox.left) / 10, rect_drawbox.bottom - DataReicevedQueue[i] * (rect_drawbox.bottom - rect_drawbox.top) / maxReceivedBytes); // Chart line
        }

        // Draw the char2
        SelectObject(hds, hPenChart2);
        for (int i = 1; i < DataSentedQueue.size(); i++) {
            MoveToEx(hds, rect_drawbox.left + (i - 1) * (rect_drawbox.right - rect_drawbox.left) / 10, rect_drawbox.bottom - DataSentedQueue[i - 1] * (rect_drawbox.bottom - rect_drawbox.top) / maxSentBytes, NULL);
            LineTo(hds, rect_drawbox.left + i * (rect_drawbox.right - rect_drawbox.left) / 10, rect_drawbox.bottom - DataSentedQueue[i] * (rect_drawbox.bottom - rect_drawbox.top) / maxSentBytes); // Chart line
        }
        
        SelectObject(hds, hPenAxis);
        MoveToEx(hds, rect_drawbox.left, rect_drawbox.bottom, NULL);
        LineTo(hds, rect_drawbox.right, rect_drawbox.bottom); // X-axis
        MoveToEx(hds, rect_drawbox.left, rect_drawbox.bottom, NULL);
        LineTo(hds, rect_drawbox.left, rect_drawbox.top); // X-axis

        DeleteObject(hbrWhite);
        DeleteObject(hPenAxis);
        DeleteObject(hPenGrid);
        DeleteObject(hPenChart);

        EndPaint(hDrawbox, &ps);
    } break;

    case WM_COMMAND: {
        if ((HWND)lp == hButtonStart) {
            Sleep(500);
            FLAG = true;

            threads.emplace_back(proxy_server);
            threads.back().detach();
            ClientOut++;
            EnableWindow(hButtonStart, FALSE);
            EnableWindow(hButtonStop, TRUE);
        }

        if ((HWND)lp == hButtonStop) {
            EnableWindow(hButtonStop, FALSE);
            
            FLAG = false;
            ClientOut = threads.size() - 1;
            closesocket(proxySocket);
           
            SendMessage(hTextbox1, EM_SETSEL, 0, -1);
            SendMessage(hTextbox1, EM_SCROLL, SB_BOTTOM, 0);
            SendMessage(hTextbox1, EM_REPLACESEL, FALSE, (LPARAM) "");
            EnableWindow(hButtonStart, TRUE);
        }
        if ((HWND)lp == hButtonBan) {
            // Lấy nội dung từ hSearchBar1 và hSearchBar2
            wchar_t buffer1[256] = L"", buffer2[256] = L"";
            GetWindowTextW(hSearchBar1, buffer1, 256);
            GetWindowTextW(hSearchBar2, buffer2, 256);
            cout << wcslen(buffer1) << endl
                      << wcslen(buffer2) << endl;

            // Kiểm tra điều kiện: cả hai ô không rỗng
            if (wcslen(buffer1) > 0 && wcslen(buffer2) > 0) {
                // Chuyển đổi từ wchar_t* sang string
                wstring wstr1(buffer1), wstr2(buffer2);
                string str1(wstr1.begin(), wstr1.end());
                string str2(wstr2.begin(), wstr2.end());

                // Kiểm tra cặp (str1, str2) trong user_ban_list
                auto it = find(user_ban_list.begin(), user_ban_list.end(), make_pair(str1, str2));
                if (it == user_ban_list.end()) {
                    // Nếu cặp chưa tồn tại, thêm vào user_ban_list
                    user_ban_list.emplace_back(str1, str2);
                    stringstream displaytext;
                    string str1_tmp = str1;
                    string str2_tmp = str2;

                    if (str1.length() > 24) {
                        str1_tmp = str1.substr(0, 20) + "...";
                    }
                    displaytext << setw(24) << left << str1_tmp<< " "<< setw(15) << left << str2;
                    cout << displaytext.str() << endl;
                    // Thêm chuỗi vào ListBox
                    SendMessage(hTextbox2, LB_ADDSTRING, 0, (LPARAM)displaytext.str().c_str());

                    // In ra thông báo đã thêm thành công (nếu cần)
                } else {
                    // Nếu cặp đã tồn tại, in ra cảnh báo
                    cout << "Pair already exists in user_ban_list!" << endl;
                }
            } else if (wcslen(buffer1) > 0 && wcslen(buffer2) == 0) {
                // Trường hợp chỉ có hSearchBar1 được điền
                wstring wstr1(buffer1), wstr2 = L"all";
                string str1(wstr1.begin(), wstr1.end());
                string str2(wstr2.begin(), wstr2.end());

                // Kiểm tra cặp (str1, "all") trong user_ban_list
                auto it = find(user_ban_list.begin(), user_ban_list.end(), make_pair(str1, str2));
                if (it == user_ban_list.end()) {
                    // Nếu cặp chưa tồn tại, thêm vào user_ban_list
                    user_ban_list.emplace_back(str1, str2);
                    ban_list.push_back(str1);

                    // Tạo chuỗi định dạng với căn chỉnh
                    ostringstream oss;
                    oss << left << setw(24) << str1
                        << " " << left << setw(15) << str2;
                    string displayText = oss.str();

                    // Thêm chuỗi vào ListBox
                    SendMessage(hTextbox2, LB_ADDSTRING, 0, (LPARAM)displayText.c_str());
                } else {
                    // Nếu cặp đã tồn tại, in ra cảnh báo
                    cout << "Pair already exists in user_ban_list!" << endl;
                }
            } else {
                // Nếu một trong hai ô rỗng và không phải trường hợp trên, in ra cảnh báo
                cout << "Both fields must be filled!" << endl;
            }

            // Xóa nội dung trong hSearchBar1 và hSearchBar2
            SetWindowTextW(hSearchBar1, L"");
            SetWindowTextW(hSearchBar2, L"");
            InvalidateRect(hSearchBar1, NULL, TRUE);
            UpdateWindow(hSearchBar1);
            InvalidateRect(hSearchBar2, NULL, TRUE);
            UpdateWindow(hSearchBar2);
            InvalidateRect(hTextbox2, NULL, TRUE);
            UpdateWindow(hTextbox2);
        }

        if ((HWND)lp == hButtonUnban) {
            //  Lấy chỉ số mục được chọn trong ListBox
            int selectedIndex = (int)SendMessage(hTextbox2, LB_GETCURSEL, 0, 0);

            // Nếu không có mục nào được chọn
            if (selectedIndex == LB_ERR) {
                cout << "No item selected to unban!" << endl;
                break;
            }

            // Lấy chuỗi từ mục được chọn
            int textLen = SendMessage(hTextbox2, LB_GETTEXTLEN, selectedIndex, 0);
            // cout << textLen << endl;
            char *selectedText = new char[textLen + 1];
            SendMessage(hTextbox2, LB_GETTEXT, selectedIndex, (LPARAM)selectedText);

            // Xóa item đã chọn từ ListBox
            if (LB_ERR == SendMessage(hTextbox2, LB_DELETESTRING, selectedIndex, 0)) {
                cout << "Error deleting item from ListBox!" << endl;
                break;
            }

            // InvalidateRect(hTextbox2, NULL, TRUE);
            UpdateWindow(hTextbox2);
            string itemToRemove = selectedText;
            // Phân tách chuỗi
            string part1, part2;
            istringstream iss(itemToRemove);
            iss >> part1 >> part2; // Đọc từng phần, tự động bỏ qua khoảng trắng thừa
            ban_list.erase(
                remove(ban_list.begin(), ban_list.end(), part1),
                ban_list.end());

            user_ban_list.erase(
                remove_if(
                    user_ban_list.begin(), user_ban_list.end(),
                    [&part1, &part2](const pair<string, string> &p) {
                        return p.first == part1 && p.second == part2;
                    }),
                user_ban_list.end());
        }

    } break;
    default: {
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    }
    return 0;
}

int main() {
    const wchar_t *g_szClassName = L"myWindowClass";
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WindowProcedure;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = g_szClassName;
    RegisterClassW(&wc);

    hwnd = CreateWindowW(
        g_szClassName, L"Window with Textboxes and Buttons", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1300, 700, NULL, NULL, wc.hInstance, NULL);

    if (hwnd == NULL) {
        return 0;
    }

    // Lấy kích thước cửa sổ để tính toán vị trí các ô
    RECT rect;
    GetClientRect(hwnd, &rect);
    //

    // Định nghĩa vị trí và kích thước các textbox
    int margin = 20; // Lề giữa các thành phần

    hTextbox1 = CreateWindowW(L"EDIT", L"",
                              WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL,
                              margin, margin + 50, 400, 500, hwnd, (HMENU)3, wc.hInstance, NULL);

    hTextbox2 = CreateWindowW(L"LISTBOX", L"",
                              WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_STANDARD | WS_VSCROLL,
                              2 * margin + 400, 2 * margin + 230 + 50, 325, 220, hwnd, (HMENU)3, wc.hInstance, NULL);

    hTextbox4 = CreateWindowW(L"EDIT", L"",
                              WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL | ES_READONLY,
                              800 - margin, margin + 50, 325, 240 - 50, hwnd, (HMENU)3, wc.hInstance, NULL);
    hDrawbox = CreateWindowW(L"STATIC", L"",
                             WS_CHILD | WS_VISIBLE,
                             2 * margin + 400, margin + 50, 325, 240 - 50, hwnd, (HMENU)3, wc.hInstance, NULL);

    TraceBox = CreateWindowW(L"EDIT", L"",
                             WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL | ES_READONLY,
                             800 - margin, 2 * margin + 230 + 50, 325, 320, hwnd, (HMENU)3, wc.hInstance, NULL);
    // Tao font cho textbox
    static HFONT hFont;
    hFont = CreateFontA(
            16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, FIXED_PITCH, "Courier New");

        // Gán font cho ListBox
    SendMessage(hTextbox2, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hTextbox1, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hTextbox4, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(TraceBox, WM_SETFONT, (WPARAM)hFont, TRUE);    

    // Tạo thanh tìm kiếm ngang trên các nút
    int searchBarWidth = 300;
    int searchBarHeight = 30;

    hButtonStart = CreateWindowW(L"BUTTON", L"Start",
                                 WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                 2 * margin + 15, margin + 500 + margin + 50, 150, 50, hwnd, (HMENU)101, wc.hInstance, NULL);
    hButtonStop = CreateWindowW(L"BUTTON", L"Stop",
                                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                2 * margin + 170 + margin, margin + 500 + margin + 50, 150, 50, hwnd, (HMENU)101, wc.hInstance, NULL);
    EnableWindow(hButtonStop, FALSE);

    hButtonBan = CreateWindowW(L"BUTTON", L"Ban",
                               WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                               2 * margin + 400, margin + 500 + margin + 50, 150, 50, hwnd, (HMENU)101, wc.hInstance, NULL);

    hButtonUnban = CreateWindowW(L"BUTTON", L"Unban", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                 2 * margin + 400 + 325 / 2 + 10, margin + 500 + margin + 50, 150, 50, hwnd, (HMENU)101, wc.hInstance, NULL);
    hSearchBar1 = CreateWindowW(L"EDIT", L"",
                                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                                2 * margin + 400, 2 * margin + 480 + 20, 325 / 2 - 10, 25, hwnd, NULL, wc.hInstance, NULL);
    hSearchBar2 = CreateWindowW(L"EDIT", L"",
                                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                                2 * margin + 400 + 325 / 2 + 10, 2 * margin + 480 + 20, 325 / 2 - 10, 25, hwnd, NULL, wc.hInstance, NULL);

    Log_label = CreateWindowW(L"STATIC", L"Log",
                              WS_CHILD | WS_VISIBLE | SS_CENTER,
                              margin, margin + 20, 50, 20, hwnd, NULL, wc.hInstance, NULL);

    performance = CreateWindowW(L"STATIC", L"Performance",
                                WS_CHILD | WS_VISIBLE | SS_CENTER,
                                2 * margin + 400, margin + 20, 120, 20, hwnd, NULL, wc.hInstance, NULL);

    stat = CreateWindowW(L"STATIC", L"Stat",
                         WS_CHILD | WS_VISIBLE | SS_CENTER,
                         800 - margin, margin + 20, 50, 20, hwnd, NULL, wc.hInstance, NULL);

    Trace = CreateWindowW(L"STATIC", L"Trace",
                          WS_CHILD | WS_VISIBLE | SS_CENTER,
                          800 - margin, margin + 50 + 220, 50, 20, hwnd, NULL, wc.hInstance, NULL);

    ban_label = CreateWindowW(L"STATIC", L"Ban",
                              WS_CHILD | WS_VISIBLE | SS_CENTER,
                              2 * margin + 400, margin + 50 + 220, 50, 20, hwnd, NULL, wc.hInstance, NULL);

    // Tạo màu nền xám cho cửa sổ
    HBRUSH hbrGray = CreateSolidBrush(RGB(211, 211, 211)); // Màu xám nhạt
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrGray);
    threads.emplace_back(printReceivedAndSentBytes);
    threads.back().detach();
    threads.emplace_back(printGraph);
    threads.back().detach();

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // ClientOut++;

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
        if (msg.message != WM_PAINT) {
            if (msg.message == 1025) {
                // cout << "_______" << endl;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

    return 0;
}