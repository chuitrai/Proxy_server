#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

#define PORT 8080 // Cổng của proxy server
#define BUFFER_SIZE 4096
#define IP_ADDRESS "0.0.0.0"

stringstream output; // Biến lưu trữ dữ liệu để ghi vào file server.txt
int pivot = 0;       // Biến lưu trữ vị trí của dòng cuối cùng trong file server.txt

void relayData(SOCKET srcSocket, SOCKET destSocket) { // Hàm chuyển dữ liệu từ srcSocket sang destSocket
    char buffer[BUFFER_SIZE];                         // buffer dùng để chứa dữ liệu
    int bytesRead;                                    // Số byte đọc được từ srcSocket

    while ((bytesRead = recv(srcSocket, buffer, sizeof(buffer), 0)) > 0) { // Đọc dữ liệu từ srcSocket
        if (send(destSocket, buffer, bytesRead, 0) < 0) {                  // Gửi dữ liệu đến destSocket
            cerr << "Error writing to socket" << endl;                     // In ra lỗi nếu gửi dữ liệu không thành công
            break;
        }
    }
}
string stringBan;
vector<string> ban_list;    // Danh sách các trang web bị chặn
bool bann(char *hostName) { // Hàm kiểm tra hostName có nằm trong danh sách ban_list không
    // for (auto tmp : ban_list) { // Duyệt qua các phần tử trong ban_list
    //     cout <<"****" <<  tmp << endl;
    // }
    // Chuyển đổi char* sang string
    string host(hostName);    // host là hostName dưới dạng string
    for (auto s : ban_list) { // Duyệt qua các phần tử trong ban_list
        if (s == host)        // Nếu hostName nằm trong ban_list
            return false;
    }
    return true;
}

void handleClient(SOCKET clientSocket) { // Hàm xử lý dữ liệu từ client
    ofstream out("server.txt");          // Mở file server.txt để ghi dữ liệu
    char buffer[BUFFER_SIZE];            // buffer dùng để chứa dữ liệu

    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0); // Đọc dữ liệu từ clientSocket
    if (bytesRead <= 0) {                                              // Nếu không đọc được dữ liệu
        cerr << "Error reading from client socket" << endl;            // In ra lỗi
        closesocket(clientSocket);
        return;
    }
    buffer[bytesRead] = '\0'; // Kết thúc chuỗi buffer
                              // output << buffer << endl;

    int length = output.str().size();
    //////////////////////cout << pivot << " " << length << endl;

    if (pivot != length) {
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

    if (strcmp(method, "CONNECT") == 0) {            // Nếu method là CONNECT
        char hostname[256];                          // Biến lưu trữ hostname
        int port = 443;                              // Cổng mặc định
        sscanf(url, "%255[^:]:%d", hostname, &port); // Đọc hostname và port từ url
        if (!bann(hostname)) {                       // Nếu hostname nằm trong danh sách ban_list
            const char *response = "HTTP/1.1 403 Forbidden\r\n"
                                   "Content-Type: text/html\r\n"
                                   "Content-Length: 85\r\n"
                                   "Connection: close\r\n\r\n"
                                   "<html><body><h1>403 Forbidden</h1>"
                                   "<p>Your access to this resource has been denied.</p></body></html>"; // Gửi thông báo lỗi
            send(clientSocket, response, strlen(response), 0);                                           // Gửi thông báo lỗi đến client
            closesocket(clientSocket);
            return;
        }
        // cout << buffer << endl; // In ra thông tin kết nối
        // output << "hostname: " << hostname << endl; // In ra hostname
        // output << "url: " << url << endl; // In ra url
        SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Tạo socket server
        if (serverSocket == INVALID_SOCKET) {                  // Nếu không tạo được socket
            cerr << "Could not create server socket" << endl;
            closesocket(clientSocket);
            return;
        }

        struct hostent *host = gethostbyname(hostname); // Lấy thông tin host từ hostname
        if (host == nullptr) {                          // Nếu không lấy được thông tin host
            cerr << "Host resolution failed" << endl;
            closesocket(clientSocket);
            closesocket(serverSocket);
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
            return;
        }

        const char *response = "HTTP/1.1 200 Connection Established\r\n\r\n"; // Gửi thông báo kết nối thành công
        send(clientSocket, response, strlen(response), 0);                    // Gửi thông báo kết nối thành công đến client

        while (true) {                                        // Lặp để chuyển dữ liệu giữa client và server
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
                if (bytes <= 0)                                                  // Nếu không đọc được dữ liệu
                    break;
                if (send(serverSocket, buffer2, bytes, 0) != bytes) { // Gửi dữ liệu đến serverSocket
                    cerr << "Write to server error" << endl;
                    break;
                }
            }

            if (FD_ISSET(serverSocket, &fds)) {                                  // Nếu serverSocket có trong tập file descriptor
                int bytes = recv(serverSocket, buffer2, sizeof(buffer2) - 1, 0); // Đọc dữ liệu từ serverSocket
                if (bytes <= 0)                                                  // Nếu không đọc được dữ liệu
                    break;
                if (send(clientSocket, buffer2, bytes, 0) != bytes) { // Gửi dữ liệu đến clientSocket
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
            return;
        }

        struct hostent *host = gethostbyname(hostname); // Lấy thông tin host từ hostname
        if (host == nullptr) {                          // Nếu không lấy được thông tin host
            cerr << "Host resolution failed" << endl;
            closesocket(clientSocket);
            closesocket(serverSocket);
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
            return;
        }

        std::string request = std::string(method) + " " + path + " " + protocol + "\r\n"; // Tạo request
        request += "Host: " + std::string(hostname) + "\r\n";                             // Thêm Host vào request
        request += "Connection: close\r\n\r\n";                                           // Thêm Connection vào request

        send(serverSocket, buffer, bytesRead, 0); // Gửi request đến server

        while ((bytesRead = recv(serverSocket, buffer, sizeof(buffer), 0)) > 0) { // Đọc dữ liệu từ server
            send(clientSocket, buffer, bytesRead, 0);                             // Gửi dữ liệu đến client
            buffer[sizeof(buffer) - 1] = '\0';                                    // Kết thúc chuỗi buffer
            out << buffer;                                                        // Ghi dữ liệu vào file server.txt
        }
        closesocket(serverSocket);
    }
    closesocket(clientSocket);
}

int proxy_server(string &stringBan) {

    // cout << "Ban list: " << stringBan << '*' << endl; // In ra danh sách ban_list
    istringstream iss(stringBan); // Tạo một string stream từ stringBan
    string host;                  // Biến lưu trữ host
    while (iss >> host) {         // Đọc host từ string stream
        ban_list.push_back(host); // Thêm host vào ban_list
        // cout << host << '*' << endl; // In ra host
    }

    WSADATA wsaData;                                 // Biến lưu trữ thông tin Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { // Khởi tạo Winsock
        cerr << "WSAStartup failed" << endl;
        return 1;
    }

    SOCKET proxySocket = socket(AF_INET, SOCK_STREAM, 0); // Tạo proxy socket
    if (proxySocket == INVALID_SOCKET) {
        cerr << "Could not create proxy socket" << endl;
        WSACleanup();
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
        return 1;
    }

    if (listen(proxySocket, 10) < 0) { // Listen proxy socket
        cerr << "Listen failed" << endl;
        closesocket(proxySocket);
        WSACleanup();
        return 1;
    }

    cout << "Proxy server is running on port " << PORT << std::endl;

    while (true) { // Lặp để chấp nhận kết nối từ client
        // output << "_______________________________________________________\n";
        SOCKET clientSocket = accept(proxySocket, nullptr, nullptr); // Chấp nhận kết nối từ client
        // cerr << "Connected to client\n";
        if (clientSocket == INVALID_SOCKET) { // Nếu không chấp nhận được kết nối
            cerr << "Accept failed\n";
            continue;
        }
        // cout << "Client accepted for connection from: " << clientSocket << endl;
        vector<thread> th;
        th.emplace_back(handleClient, clientSocket); // Tạo một thread để xử lý dữ liệu từ client

        // cerr << "Thread created\n";
        th.back().detach(); // Tách thread
        // cout<<th.size()<<endl;
        // cerr << "Thread detached\n";
    }

    closesocket(proxySocket); // Đóng proxySocket
    WSACleanup();             // Kết thúc Winsock
    return 0;
}