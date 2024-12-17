#include "proxy_server.h"
#include <iostream>
#include <unordered_map>
#include <string>
#include <ctime>
//using namespace std;

// 
HWND hTextbox3, hTextbox1, hTextbox2, hSearchBar, hButtonAdd, hButtonStart, hButtonStop;

// Khai báo unordered_map để lưu các dòng đã được thêm vào
unordered_map<string, bool> addedStrings;

//   
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    vector <thread> threads;
    static HWND hwndTextBox;

    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_COMMAND: {
        // Kiểm tra nếu nút Add (ID của hButtonAdd) được nhấn
        if ((HWND)lp == hButtonAdd) {
            // Lấy văn bản từ ô tìm kiếm (hSearchBar)
            int len = GetWindowTextLength(hSearchBar) + 1;
            char *text = new char[len];
            GetWindowTextA(hSearchBar, text, len);

            // Kiểm tra xem văn bản đã tồn tại trong unordered_map chưa
            string textStr = text;
            if (addedStrings.find(textStr) == addedStrings.end()) {
                // Nếu chưa tồn tại, thêm vào unordered_map và vào hTextbox3
                addedStrings[textStr] = true;

                // Lấy văn bản hiện tại trong ô hTextbox3
                int currentTextLength = GetWindowTextLength(hTextbox3) + 1;
                char *currentText = new char[currentTextLength];
                GetWindowTextA(hTextbox1, currentText, currentTextLength);

                // Tạo chuỗi mới, nối văn bản hiện tại và văn bản mới
                string newText = currentText;
                newText += "\r\n";  // Thêm dấu xuống dòng
                newText += textStr;   // Thêm văn bản từ ô tìm kiếm

                // Đưa văn bản mới vào ô textbox bên phải (hTextbox3)
                SetWindowTextA(hTextbox3, newText.c_str());

                // Giải phóng bộ nhớ đã cấp phát
                delete[] text;
                delete[] currentText;
            }
            else {
                // Nếu văn bản đã tồn tại trong unordered_map, không làm gì cả
                cout << "The path was existed!" << endl;
                delete[] text;
            }
        }

        if ((HWND)lp == hButtonStart) {
            string stringBan = " fuck";
            for (auto it = addedStrings.begin(); it != addedStrings.end(); ++it) {
                stringBan += it->first + "\n";
            }
            //cout<<threads.size()<<endl;
            threads.emplace_back(proxy_server, std::ref(stringBan));
            threads.back().detach();
            //cout << "--->" << threads.size() << endl;
        }

    }
    default: {
        int length = output.str().size();
        //////////////////////cout << pivot << " " << length << endl;
        
        if (pivot != length){
            int dis = length - pivot;
            SendMessage(hTextbox1, EM_SETSEL, length , length);
            //cout << output.str() << endl;
            SendMessage(hTextbox1, EM_REPLACESEL, FALSE, (LPARAM)output.str().c_str());

                // Scroll to the end of the content
            SendMessage(hTextbox1, EM_SCROLL, SB_BOTTOM, 0);
            pivot = length;
        }
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

    HWND hwnd = CreateWindowW(
        g_szClassName, L"Window with Textboxes and Buttons", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 500, NULL, NULL, wc.hInstance, NULL);

    if (hwnd == NULL) {
        return 0;
    }

    // Lấy kích thước cửa sổ để tính toán vị trí các ô
    RECT rect;
    GetClientRect(hwnd, &rect);

    // Định nghĩa vị trí và kích thước các textbox
    int leftWidth = rect.right * 5 / 10; // 50% chiều rộng cửa sổ cho phần bên trái
    int leftHeight = rect.bottom / 3;    // Chiều cao mỗi textbox là 1/3 chiều cao của cửa sổ
    int rightWidth = 200;                // Chiều rộng cố định cho ô bên phải (textbox 3)

    // Chiều cao của ô bên phải được căn chỉnh sao cho đáy dưới của nó ngang hàng với đáy dưới của 2 ô bên trái
    int rightHeight = leftHeight * 2; // Chiều cao của ô bên phải = 2 lần chiều cao của ô bên trái

    // Tạo 2 ô textbox ở phần bên trái
    hTextbox1 = CreateWindowW(L"EDIT", L"Display function output here",
                                   WS_CHILD |ES_LEFT | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL | ES_READONLY | ES_AUTOVSCROLL | EM_SETSEL,
                                   30, 50, leftWidth - 40, leftHeight - 20, hwnd, (HMENU)1, wc.hInstance, NULL);

    hTextbox2 = CreateWindowW(L"EDIT", L"",
                                   WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL | ES_READONLY | EM_SETSEL,
                                   30, 50 + leftHeight, leftWidth - 40, leftHeight - 20, hwnd, (HMENU)2, wc.hInstance, NULL);

    // Tạo 1 ô textbox ở phần bên phải, với chiều ngang 200 và chiều cao đã điều chỉnh
    hTextbox3 = CreateWindowW(L"EDIT", L"",
                              WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL | ES_READONLY | EM_SETSEL,
                              leftWidth + 20, 50, 180, rightHeight - 20, hwnd, (HMENU)3, wc.hInstance, NULL);

    // Tạo một textbox nằm ngang (giống search bar) ngay trên nút Add
    int searchBarHeight = 30; // Chiều cao của search bar
    hSearchBar = CreateWindowW(L"EDIT", L"Search...",
                               WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE,
                               (rect.right - 300) / 2, rect.bottom - 115, 300, searchBarHeight, hwnd, NULL, wc.hInstance, NULL);

    // Đẩy nút Add sang góc phải
    hButtonAdd = CreateWindowW(L"BUTTON", L"Add",
                               WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_BORDER,
                               rect.right - 120, rect.bottom - 75, 100, 30, hwnd, (HMENU)100, wc.hInstance, NULL);
    
    // Tạo nút Start
    hButtonStart = CreateWindowW(L"BUTTON", L"Start",
                               WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_BORDER,
                               rect.right - 120, rect.bottom - 50, 100, 30, hwnd, (HMENU)100, wc.hInstance, NULL);

    // Tạo màu nền xám cho cửa sổ
    HBRUSH hbrGray = CreateSolidBrush(RGB(211, 211, 211)); // Màu xám nhạt
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrGray);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (pivot != output.str().size() || GetMessage(&msg, NULL, 0, 0) > 0) {
        cout << pivot << "  "<< output.str().size();
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
