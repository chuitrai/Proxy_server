#include <windows.h>
#include <vector>

// Window Procedure Declaration
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Custom Data Points for the Chart
std::vector<POINT> dataPoints = {{10, 200}, {50, 180}, {90, 150}, {130, 220}, {170, 100}, {210, 130}, {250, 50}};

// Entry Point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "ChartWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "Chart Drawing Example",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Draw Axes
            HPEN hPenAxis = CreatePen(PS_SOLID, 2, RGB(200, 200, 200));
            SelectObject(hdc, hPenAxis);
            MoveToEx(hdc, 50, 350, NULL);
            LineTo(hdc, 550, 350);  // X-axis
            MoveToEx(hdc, 50, 50, NULL);
            LineTo(hdc, 50, 350);  // Y-axis
            DeleteObject(hPenAxis);

            // Draw Grid Lines
            HPEN hPenGrid = CreatePen(PS_DOT, 1, RGB(100, 100, 100));
            SelectObject(hdc, hPenGrid);
            for (int i = 100; i <= 500; i += 50) {
                MoveToEx(hdc, 50, i, NULL);
                LineTo(hdc, 550, i);
            }
            for (int i = 100; i <= 500; i += 50) {
                MoveToEx(hdc, i, 50, NULL);
                LineTo(hdc, i, 350);
            }
            DeleteObject(hPenGrid);

            // Draw Data Points and Chart Line
            HPEN hPenLine = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
            SelectObject(hdc, hPenLine);

            if (!dataPoints.empty()) {
                MoveToEx(hdc, 50 + dataPoints[0].x, 350 - dataPoints[0].y, NULL);
                for (size_t i = 1; i < dataPoints.size(); ++i) {
                    LineTo(hdc, 50 + dataPoints[i].x, 350 - dataPoints[i].y);
                }
            }

            DeleteObject(hPenLine);
            EndPaint(hwnd, &ps);
        }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
