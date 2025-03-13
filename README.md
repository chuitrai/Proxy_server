# 🔥 Proxy Server - Computer Networks Project  

## 📌 Giới thiệu  
Đây là một **hệ thống Proxy Server** được phát triển bằng **C++**, sử dụng **`<windows.h>`** để xây dựng giao diện trên Windows. Proxy này giúp kiểm soát truy cập web, giám sát băng thông và bảo vệ người dùng trên mạng nội bộ.  

🔹 **Thành viên nhóm**  
| Họ và Tên | MSSV |
|-----------|------|
| Nguyễn Thanh Khôi | 23120009 |
| Vũ Đình Ngọc Bảo  | 23120114 |
| Võ Thành Nhân | 23120150 |

📌 **Công nghệ sử dụng**:  
- **Backend:** C++ (Windows API, socket programming)  
- **Frontend:** Windows API (`<windows.h>`)  
- **Mạng & Bảo mật:** HTTP, HTTPS, Proxy Forwarding  

---

## 🚀 Chức năng chính  
✅ **Forward Proxy:** Định tuyến và chuyển tiếp yêu cầu HTTP/HTTPS từ client đến server.  
✅ **Kiểm duyệt trang web:** Hỗ trợ chặn truy cập dựa trên domain/IP.  
✅ **Lưu vết người dùng:** Ghi lại lịch sử truy cập domain/IP.  
✅ **Hỗ trợ nhiều thiết bị:** Các máy trong mạng LAN có thể sử dụng chung proxy.  
✅ **Giám sát băng thông:** Hiển thị lưu lượng upload/download theo thời gian thực.  

---

## 🛠 Cài đặt & Chạy chương trình  

### 🔹 **Yêu cầu hệ thống**  
- Windows 10/11  
- Compiler: MinGW / MSVC  
- CMake (nếu cần)  

### 🔹 **Cách biên dịch & chạy**  
```sh
# Clone repository về máy
git clone https://github.com/your-repo/proxy-server.git
cd proxy-server

# Biên dịch chương trình (dùng MinGW)
g++ -o proxy_server.exe proxy.cpp -lws2_32

# Chạy chương trình trực tiếp
./proxy_server.exe
```
