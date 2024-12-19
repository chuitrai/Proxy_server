#include <algorithm>
#include <cstring>
#include <ctime>
#include <deque>
#include <fstream>
#include <iostream>
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
bool FLAG = true;                                                                                                                                                                                                         // Biến kiểm tra việc chạy proxy server
time_t now = time(0);                                                                                                                                                                                                     // Biến lưu trữ thời gian hiện tại
vector<thread> threads;                                                                                                                                                                                                   // Vector lưu trữ các thread
HWND hwnd, hDrawbox, hTextbox3, hTextbox1, hTextbox2, hSearchBar, hButtonAdd, hButtonStart, hButtonStop, hButtonBan, hButtonUnban, hButtonRemove, hTextbox4, hSearchBar1, hSearchBar2, Log_label, performance, stat, ban_label; // Các control// Khai báo unordered_map để lưu các dòng đã được thêm vào
// Khai báo unordered_map để lưu các dòng đã được thêm vào
unordered_map<string, bool> addedStrings;
string stringBan = ""; // Danh sách các trang web bị chặn
stringstream output;
vector<pair<string, string>> user_ban_list;                                           // Biến lưu trữ dữ liệu để ghi vào file server.txt
int byteReceived = 0;                                                                 // Biến lưu trữ số byte nhận được
int byteSent = 0;                                                                     // Biến lưu trữ số byte gửi đi
int pivot = 0;                                                                        // Biến lưu trữ vị trí của dòng cuối cùng trong file server.txt
vector<string> ban_list;                                                              // Danh sách các trang web bị chặn
unordered_map<string, string> IPv4ConnectToDomain;                                    // Lưu trữ địa chỉ IP của domain
int ClientOut = 0;                                                                    // Số client đã ngắt kết nối
SOCKET proxySocket;                                                                   // Socket của proxy server
deque<int> DataDisplayQueue = {12354, 50463, 24653, 42361, 62346, 10350, 2314, 5493}; // Queue lưu trữ số byte nhận được và gửi đi
int maxData = 1000;
RECT rect_drawbox;

void relayData(SOCKET srcSocket, SOCKET destSocket) { // Hàm chuyển dữ liệu từ srcSocket sang destSocket
    char buffer[BUFFER_SIZE];                         // buffer dùng để chứa dữ liệu
    int bytesRead;                                    // Số byte đọc được từ srcSocket

    while ((bytesRead = recv(srcSocket, buffer, sizeof(buffer), 0)) > 0 && FLAG) { // Đọc dữ liệu từ srcSocket
        //---------------------------------------------------------
        byteSent += bytesRead;                            // Cập nhật số byte nhận được
        if (send(destSocket, buffer, bytesRead, 0) < 0) { // Gửi dữ liệu đến destSocket
            cerr << "Error writing to socket" << endl;    // In ra lỗi nếu gửi dữ liệu không thành công
            break;
        }
    }
    ClientOut++;
}

void printReceivedAndSentBytes() {
    while (FLAG) {
        Sleep(1000);
        // ________________________________________________________________________________________________________________________
        // Tinh toan so byte nhan va gui
        int received = byteReceived / abs(time(0) - now);
        int sent = byteSent / abs(time(0) - now);

        string text = "Received: " + to_string(received) + " bytes/s\r\nSent: " + to_string(sent) + " bytes/s\r\n" + "Number of Opening port: " + to_string((threads.size() - 1 - ClientOut) > 0 ? 1 : threads.size() - 1 - ClientOut) + " clients\r\n";
        SendMessage(hTextbox4, EM_SETSEL, 0, -1);
        SendMessage(hTextbox4, EM_REPLACESEL, FALSE, (LPARAM)text.c_str());
        byteReceived = 0;
        byteSent = 0;
        now = time(0);

        // __________________________________________________________________________________________________________________
    }
    string text = "Received: " + to_string(0) + " bytes/s\r\nSent: " + to_string(0) + " bytes/s\r\n" + "Number of Opening port: " + to_string(0) + " clients\r\n";
    SendMessage(hTextbox4, EM_SETSEL, 0, -1);
    SendMessage(hTextbox4, EM_REPLACESEL, FALSE, (LPARAM)text.c_str());
}
void printGraph() {
    // Perform calculations or data preparation here
    while(FLAG) {
        Sleep(2000); // Simulate some work
        cout << "Print graph" << endl;
    // Send a message to the main thread to trigger a GUI update
        PostMessage(hwnd, WM_USER + 1, 0, 0); // WM_USER + 1 is a custom message
    //Sleep(2000);
        //Sleep(2000); // Simulate some work
        //PostMessage(hwnd, WM_USER + 2, 0, 0); // WM_USER + 2 is a custom message
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

void handleClient(SOCKET clientSocket, char *ip) { // Hàm xử lý dữ liệu từ client
    // if (user_ban_list.size() > 0)

    //     cout << user_ban_list[0].first << " " << user_ban_list[0].second << endl;
    ofstream out("server.txt"); // Mở file server.txt để ghi dữ liệu
    char buffer[BUFFER_SIZE];   // buffer dùng để chứa dữ liệu

    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0); // Đọc dữ liệu từ clientSocket
    //---------------------------------------------------------
    byteReceived += bytesRead;                              // Cập nhật số byte nhận được
    if (bytesRead <= 0) {                                   // Nếu không đọc được dữ liệu
        cerr << "Error reading from client socket" << endl; // In ra lỗi
        closesocket(clientSocket);
        ClientOut++;
        return;
    }
    buffer[bytesRead] = '\0'; // Kết thúc chuỗi buffer
    // cout << buffer << endl; // In ra dữ liệu đọc được

    std::istringstream stream(buffer);
    std::string line, connect_line, host_line, port;
    int line_number = 0;

    while (std::getline(stream, line)) {
        line_number++;
        if (line_number == 1) {
            connect_line = line; // Dòng CONNECT
        } else if (line_number == 2) {
            host_line = line; // Dòng Host
            // Tách port từ dòng Host
            size_t colon_pos = host_line.find(":");
            if (colon_pos != std::string::npos) {
                port = host_line.substr(colon_pos + 1);
                port = port.substr(0, port.find(" ")); // Xóa phần dư nếu có
            }
        }
    }

    // Đưa kết quả về 1 văn bản (chuỗi)
    std::string buff = connect_line + "\r\n"
                                      "Host: " +
                       host_line + "\r\n" + "----------------\r\n";

    output << buff << endl;
    int length = output.str().size();
    //////////////////////cout << pivot << " " << length << endl;

    if (pivot != length) {
        // cout<<0<<endl;
        int dis = length - pivot;
        SendMessage(hTextbox1, EM_SETSEL, length, length);
        // cout << output.str() << endl;
        SendMessage(hTextbox1, EM_REPLACESEL, FALSE, (LPARAM)output.str().c_str());

        // Scroll to the end of the content
        SendMessage(hTextbox1, EM_SCROLL, SB_BOTTOM, 0);
        pivot = length;
    }
    char method[16], url[256], protocol[16];           // Biến lưu trữ method, url, protocol
    sscanf(buffer, "%s %s %s", method, url, protocol); // Đọc method, url, protocol từ buffer

    if (strcmp(method, "CONNECT") == 0) {                   // Nếu method là CONNECT
        char hostname[256];                                 // Biến lưu trữ hostname
        int port = 443;                                     // Cổng mặc định
        sscanf(url, "%255[^:]:%d", hostname, &port);        // Đọc hostname và port từ url
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
            ClientOut++;
            return;
        }
        // cout << buffer << endl; // In ra thông tin kết nối
        // output << "hostname: " << hostname << endl; // In ra hostname
        // output << "url: " << url << endl; // In ra url
        SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Tạo socket server
        if (serverSocket == INVALID_SOCKET) {                  // Nếu không tạo được socket
            cerr << "Could not create server socket" << endl;
            closesocket(clientSocket);
            ClientOut++;
            return;
        }

        struct hostent *host = gethostbyname(hostname); // Lấy thông tin host từ hostname
        if (host == nullptr) {                          // Nếu không lấy được thông tin host
            cerr << "Host resolution failed" << endl;
            closesocket(clientSocket);
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
        // cout << "Disconnected from: " << url << endl; // In ra thông báo đã ngắt kết nối
        closesocket(serverSocket); // Đóng serverSocket

    } else {
        char hostname[256], path[256] = "/";                 // Biến lưu trữ hostname và path
        sscanf(url, "http://%255[^/]%255s", hostname, path); // Đọc hostname và path từ url

        SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Tạo socket server
        if (serverSocket == INVALID_SOCKET) {                  // Nếu không tạo được socket
            cerr << "Could not create server socket" << endl;
            closesocket(clientSocket);
            ClientOut++;
            return;
        }

        struct hostent *host = gethostbyname(hostname); // Lấy thông tin host từ hostname
        if (host == nullptr) {                          // Nếu không lấy được thông tin host
            cerr << "Host resolution failed" << endl;
            closesocket(clientSocket);
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
            closesocket(serverSocket);
            ClientOut++;
            return;
        }

        std::string request = std::string(method) + " " + path + " " + protocol + "\r\n"; // Tạo request
        request += "Host: " + std::string(hostname) + "\r\n";                             // Thêm Host vào request
        request += "Connection: close\r\n\r\n";                                           // Thêm Connection vào request

        send(serverSocket, buffer, bytesRead, 0); // Gửi request đến server
        //---------------------------------------------------------
        byteSent += bytesRead; // Cập nhật số byte nhận được

        while ((bytesRead = recv(serverSocket, buffer, sizeof(buffer), 0)) > 0 && FLAG) { // Đọc dữ liệu từ server
            //---------------------------------------------------------
            byteReceived += bytesRead;                // Cập nhật số byte nhận được
            send(clientSocket, buffer, bytesRead, 0); // Gửi dữ liệu đến client
            //---------------------------------------------------------
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
    ClientOut++;
}
int proxy_server() {

    // cout << "Ban list: " << stringBan << '*' << endl; // In ra danh sách ban_list

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

    cout << "Proxy server is running on port " << PORT << std::endl;

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
        // std::cout << "CLient IPv4 " << client_ip << " and Port: " << ntohs(client_addr.sin_port) << "\n";
        // cout << "Client accepted for connection from: " << clientSocket << endl;
        threads.emplace_back(handleClient, clientSocket, client_ip); // Tạo một thread để xử lý dữ liệu từ client

        // cerr << "Thread created\n";
        threads.back().detach(); // Tách thread
        // cout<<th.size()<<endl;
        // cerr << "Thread detached\n";
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
        case WM_DESTROY: {
            PostQuitMessage(0);
        } break;

        // case WM_USER+2: {
        // cout << "WM_USER+2 called" << endl;
        // InvalidateRect(hDrawbox, &rect_drawbox, TRUE);}
        // break;
        
        case WM_USER+1: {
            std::cout << "WM_PAINT called" << std::endl;
            PAINTSTRUCT ps;
            HDC hds = BeginPaint(hDrawbox, &ps);
            GetClientRect(hDrawbox, &rect_drawbox);
            //Sleep(1000);
            InvalidateRect(hDrawbox, &rect_drawbox, TRUE);
            //cout << rect_drawbox.left << " " << rect_drawbox.top << " " << rect_drawbox.right << " " << rect_drawbox.bottom << endl;
            // cout << rect.left << " " << rect.top << " " << rect.right << " " << rect.bottom << endl;

            DataDisplayQueue.push_back(byteReceived);
            if (DataDisplayQueue.size() > 10)
                DataDisplayQueue.pop_front();
            maxData = *max_element(DataDisplayQueue.begin(), DataDisplayQueue.end());
            maxData = maxData == 0 ? 1000 : maxData;
            maxData = maxData * 1.5;
            // cout << "DataDisplayQueue: "; for (auto i : DataDisplayQueue) cout << i << " ";
            HBRUSH hbrWhite = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(hds, &rect_drawbox, hbrWhite);
            HPEN hPenAxis = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));     // Black axes
            HPEN hPenGrid = CreatePen(PS_DOT, 1, RGB(200, 200, 200)); // Light gray grid
            HPEN hPenChart = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));  // Red chart line

            // Select pens for drawing
            SelectObject(hds, hPenAxis);
            MoveToEx(hds, rect_drawbox.left, rect_drawbox.bottom, NULL);
            LineTo(hds, rect_drawbox.right, rect_drawbox.bottom); // X-axis
            MoveToEx(hds, rect_drawbox.left, rect_drawbox.bottom, NULL);
            LineTo(hds, rect_drawbox.left, rect_drawbox.top); // X-axis

            // Draw the grid
            SelectObject(hds, hPenGrid);
            for (int i = 0; i < 10; i++) {
                MoveToEx(hds, rect_drawbox.left, rect_drawbox.bottom - i * (rect_drawbox.bottom - rect_drawbox.top) / 10, NULL);
                LineTo(hds, rect_drawbox.right, rect_drawbox.bottom - i * (rect_drawbox.bottom - rect_drawbox.top) / 10); // Horizontal lines
                MoveToEx(hds, rect_drawbox.left + i * (rect_drawbox.right - rect_drawbox.left) / 10, rect_drawbox.bottom, NULL);
                LineTo(hds, rect_drawbox.left + i * (rect_drawbox.right - rect_drawbox.left) / 10, rect_drawbox.top); // Vertical lines
            }

            // Draw the char
            SelectObject(hds, hPenChart);
            //cout << DataDisplayQueue.size() << endl;
            for (int i = 1; i < DataDisplayQueue.size(); i++) {
                //cout << DataDisplayQueue[i] << " ";
                MoveToEx(hds, rect_drawbox.left + (i - 1) * (rect_drawbox.right - rect_drawbox.left) / 10, rect_drawbox.bottom - DataDisplayQueue[i - 1] * (rect_drawbox.bottom - rect_drawbox.top) / maxData, NULL);
                LineTo(hds, rect_drawbox.left + i * (rect_drawbox.right - rect_drawbox.left) / 10, rect_drawbox.bottom - DataDisplayQueue[i] * (rect_drawbox.bottom - rect_drawbox.top) / maxData); // Chart line
            }

            //

            DeleteObject(hbrWhite);
            DeleteObject(hPenAxis);
            DeleteObject(hPenGrid);
            DeleteObject(hPenChart);

            EndPaint(hDrawbox, &ps);
            //ReleaseDC(hDrawbox, hds);
           //InvalidateRect(hwnd, NULL, TRUE);
        } break;

    case WM_COMMAND: {
        // Kiểm tra nếu nút Add (ID của hButtonAdd) được nhấn
        // if ((HWND)lp == hButtonAdd) {
        //     // Lấy văn bản từ ô tìm kiếm (hSearchBar)
        //     int len = GetWindowTextLength(hSearchBar) + 1;
        //     char *text = new char[len];
        //     GetWindowTextA(hSearchBar, text, len);
        //     SetWindowTextA(hSearchBar, "");

        //     // Kiểm tra xem văn bản đã tồn tại trong unordered_map chưa
        //     string textStr = text;
        //     if (addedStrings.find(textStr) == addedStrings.end()) {
        //         // Nếu chưa tồn tại, thêm vào unordered_map và vào ban_list
        //         addedStrings[textStr] = true;
        //         ban_list.push_back(textStr);
        //         // Thêm văn bản vào ListBox (hTextbox3)
        //         SendMessage(hTextbox3, LB_ADDSTRING, 0, (LPARAM)textStr.c_str());

        //         // Giải phóng bộ nhớ đã cấp phát
        //         delete[] text;
        //     } else {
        //         // Nếu văn bản đã tồn tại trong unordered_map, không làm gì cả
        //         delete[] text;
        //     }
        // }

        // if ((HWND)lp == hButtonRemove) {
        //     // Lấy dòng được chọn trong ListBox và xóa nó
        //     int selectedIndex = SendMessage(hTextbox3, LB_GETCURSEL, 0, 0);
        //     if (selectedIndex != LB_ERR) {
        //         // Lấy văn bản của item được chọn từ ListBox để cập nhật danh sách
        //         int textLen = SendMessage(hTextbox3, LB_GETTEXTLEN, selectedIndex, 0);
        //         char *selectedText = new char[textLen + 1];
        //         SendMessage(hTextbox3, LB_GETTEXT, selectedIndex, (LPARAM)selectedText);

        //         // Xóa item đã chọn từ ListBox
        //         SendMessage(hTextbox3, LB_DELETESTRING, selectedIndex, 0);

        //         // Cập nhật lại danh sách ban_list và addedStrings
        //         string itemToRemove = selectedText;
        //         addedStrings.erase(itemToRemove);
        //         auto it = find(ban_list.begin(), ban_list.end(), itemToRemove);
        //         if (it != ban_list.end()) {
        //             ban_list.erase(it);
        //         }

        //         // Giải phóng bộ nhớ
        //         delete[] selectedText;
        //     }
        // }
        if ((HWND)lp == hButtonStart) {
            FLAG = true;
            // cout<<threads.size()<<endl;

            threads.emplace_back(proxy_server);
            threads.back().detach();
            threads.emplace_back(printReceivedAndSentBytes);

            threads.back().detach();
            threads.emplace_back(printGraph);
            threads.back().detach();
            ClientOut++;
            EnableWindow(hButtonStart, FALSE);
            EnableWindow(hButtonStop, TRUE);
            // cout << "--->" << threads.size() << endl;
        }

        if ((HWND)lp == hButtonStop) {
            EnableWindow(hButtonStop, FALSE);
            EnableWindow(hButtonStart, TRUE);
            FLAG = false;
            ClientOut = threads.size() - 1;
            closesocket(proxySocket);
            // cout << "Stop" << endl;
            //  Dua con tro xuong cuoi cung cua textbox1
            SendMessage(hTextbox1, EM_SETSEL, 0, -1);
            // Scroll to the end of the content
            SendMessage(hTextbox1, EM_SCROLL, SB_BOTTOM, 0);
            SendMessage(hTextbox1, EM_REPLACESEL, FALSE, (LPARAM) "");
            // std::string output_Command = executeCommand("netstat -ano | findstr :8080"); // Use "dir" on Windows
            // cout << output_Command << endl;
            // Get the number at the end of the string
            // std::string number = output_Command.substr(output_Command.find_last_of(" ") + 1);
            // output_Command = executeCommand("taskkill /F /PID " + number);
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
                // Chuyển đổi từ wchar_t* sang std::string
                std::wstring wstr1(buffer1), wstr2(buffer2);
                std::string str1(wstr1.begin(), wstr1.end());
                std::string str2(wstr2.begin(), wstr2.end());

                // Kiểm tra cặp (str1, str2) trong user_ban_list
                auto it = std::find(user_ban_list.begin(), user_ban_list.end(), std::make_pair(str1, str2));
                if (it == user_ban_list.end()) {
                    // Nếu cặp chưa tồn tại, thêm vào user_ban_list
                    user_ban_list.emplace_back(str1, str2);

                    // Tạo chuỗi định dạng "first                     second"
                    std::string displayText = str1 + "              " + str2;

                    // Thêm chuỗi vào ListBox
                    SendMessage(hTextbox2, LB_ADDSTRING, 0, (LPARAM)displayText.c_str());

                    // In ra thông báo đã thêm thành công (nếu cần)
                } else {
                    // Nếu cặp đã tồn tại, in ra cảnh báo
                    std::cout << "Pair already exists in user_ban_list!" << std::endl;
                }
            } else if (wcslen(buffer1) > 0 && wcslen(buffer2) == 0) {
                // Trường hợp chỉ có hSearchBar1 được điền
                std::wstring wstr1(buffer1), wstr2 = L"all";
                std::string str1(wstr1.begin(), wstr1.end());
                std::string str2(wstr2.begin(), wstr2.end());

                // Kiểm tra cặp (str1, "all") trong user_ban_list
                auto it = std::find(user_ban_list.begin(), user_ban_list.end(), std::make_pair(str1, str2));
                if (it == user_ban_list.end()) {
                    // Nếu cặp chưa tồn tại, thêm vào user_ban_list
                    user_ban_list.emplace_back(str1, str2);
                    ban_list.push_back(str1);
                    // Tạo chuỗi định dạng "first                     all"
                    std::string displayText = str1 + "              " + str2;

                    // Thêm chuỗi vào ListBox
                    SendMessage(hTextbox2, LB_ADDSTRING, 0, (LPARAM)displayText.c_str());
                } else {
                    // Nếu cặp đã tồn tại, in ra cảnh báo
                    std::cout << "Pair already exists in user_ban_list!" << std::endl;
                }
            } else {
                // Nếu một trong hai ô rỗng và không phải trường hợp trên, in ra cảnh báo
                std::cout << "Both fields must be filled!" << std::endl;
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
            cout << HWND(lp) << endl;
            // Lấy chỉ số mục được chọn trong ListBox
            int selectedIndex = (int)SendMessage(hTextbox2, LB_GETCURSEL, 0, 0);

            // Nếu không có mục nào được chọn
            if (selectedIndex == LB_ERR) {
                std::cout << "No item selected to unban!" << std::endl;
                break;
            }

            // Lấy chuỗi từ mục được chọn
            int textLen = SendMessage(hTextbox2, LB_GETTEXTLEN, selectedIndex, 0);
            cout << textLen << endl;
            char *selectedText = new char[textLen + 1];
            SendMessage(hTextbox2, LB_GETTEXT, selectedIndex, (LPARAM)selectedText);

            // Xóa item đã chọn từ ListBox
            if (LB_ERR == SendMessage(hTextbox2, LB_DELETESTRING, selectedIndex, 0) ) {
                std::cout << "Error deleting item from ListBox!" << std::endl;
                break;
            }
           
            //InvalidateRect(hTextbox2, NULL, TRUE);
            UpdateWindow(hTextbox2);
            string itemToRemove = selectedText;
            // Phân tách chuỗi
            string part1, part2;
            std::istringstream iss(itemToRemove);
            iss >> part1 >> part2; // Đọc từng phần, tự động bỏ qua khoảng trắng thừa
            ban_list.erase(
                std::remove(ban_list.begin(), ban_list.end(), part1),
                ban_list.end());

            user_ban_list.erase(
                std::remove_if(
                    user_ban_list.begin(), user_ban_list.end(),
                    [&part1, &part2](const std::pair<std::string, std::string> &p) {
                        return p.first == part1 && p.second == part2;
                    }),
                user_ban_list.end());
            cout << "Ban: \n";
            for (auto e : ban_list) {
                cout << e << " ";
            }
            // cout<<"------------\n";
            // for(auto e : user_ban_list){
            //     cout<<e.first<<" "<<e.second<<endl;
            // }
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
        CW_USEDEFAULT, CW_USEDEFAULT, 1040, 700, NULL, NULL, wc.hInstance, NULL);

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
                              WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL | ES_READONLY,
                              margin, margin + 50, 400, 500, hwnd, (HMENU)3, wc.hInstance, NULL);
    hTextbox2 = CreateWindowW(L"LISTBOX", L"",
                              WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_STANDARD | WS_VSCROLL,
                              2 * margin + 400, 2 * margin + 230 + 50, 325, 220, hwnd, (HMENU)3, wc.hInstance, NULL);
    hTextbox4 = CreateWindowW(L"EDIT", L"",
                              WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL | ES_READONLY,
                              800 - margin, margin + 50, 200, 150, hwnd, (HMENU)3, wc.hInstance, NULL);
    // hTextbox3 = CreateWindowW(L"LISTBOX", L"",
    //                           WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_STANDARD | WS_VSCROLL,
    //                           810 - margin, margin + 200, 200, 300, hwnd, (HMENU)3, wc.hInstance, NULL);
    hDrawbox = CreateWindowW(L"STATIC", L"",
                             WS_CHILD | WS_VISIBLE,
                             2 * margin + 400, margin + 50, 325, 240 - 50, hwnd, (HMENU)3, wc.hInstance, NULL);

    // Tạo thanh tìm kiếm ngang trên các nút
    int searchBarWidth = 300;
    int searchBarHeight = 30;
    // hSearchBar = CreateWindowW(L"EDIT", L"",
    //                            WS_CHILD | WS_VISIBLE | WS_BORDER,
    //                            810 - margin, 2 * margin + 480, 200, 25, hwnd, NULL, wc.hInstance, NULL);
    // hButtonAdd = CreateWindowW(L"BUTTON", L"Add",
    //                            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    //                            820 - margin, 2 * margin + 500 + 10, 80, 30, hwnd, (HMENU)100, wc.hInstance, NULL);
    // hButtonRemove = CreateWindowW(L"BUTTON", L"Remove",
    //                               WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    //                               820 - margin + 80 + 10, 2 * margin + 500 + 10, 80, 30, hwnd, (HMENU)100, wc.hInstance, NULL);
    // Tạo nút Start và Add nằm ngang hàng nhau
    // int buttonWidth = 100;
    // int buttonHeight = 30;
    hButtonStart = CreateWindowW(L"BUTTON", L"Start",
                                 WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                 2 * margin + 15, margin + 500 + margin + 50, 150, 50, hwnd, (HMENU)101, wc.hInstance, NULL);
    hButtonStop = CreateWindowW(L"BUTTON", L"Stop",
                                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                2 * margin + 170 + margin, margin + 500 + margin + 50, 150, 50, hwnd, (HMENU)101, wc.hInstance, NULL);

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

    ban_label = CreateWindowW(L"STATIC", L"Ban",
                              WS_CHILD | WS_VISIBLE | SS_CENTER,
                              2 * margin + 400, margin + 50 + 220, 50, 20, hwnd, NULL, wc.hInstance, NULL);

    // Tạo màu nền xám cho cửa sổ
    HBRUSH hbrGray = CreateSolidBrush(RGB(211, 211, 211)); // Màu xám nhạt
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrGray);

    ShowWindow(hwnd, SW_SHOW);  
    UpdateWindow(hwnd);

    // ClientOut++;

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) 
    if (msg.message != WM_PAINT){ 
        if (msg.message == 1025) {
            cout << "_______" << endl;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}