// Game_huapai.cpp : 定义应用程序的入口点。
//
#include "framework.h"
#include "Game_huapai.h"
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <locale>
#include <codecvt>
#include <cstring>
#include <string>
#pragma comment (lib,"Ws2_32.lib")
using std::vector;
using std::thread;
using std::wstring;

#define MAX_LOADSTRING 100
#define WM_RECEIVED_MESSAGE (WM_USER + 1)

// 全局变量:
wchar_t name[22][2] = {
    L"乙",L"二",L"三",L"四",L"五",L"六",L"七",
    L"八",L"九",L"十",L"化",L"千",L"孔",L"己",
    L"土",L"子",L"上",L"大",L"人",L"可",L"知",L"礼"
};
enum type { hand = '0', mydzf, nextdzf, enddzf, notis, throwncd, errorturn, errorop,statuss };
std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
std::wstring wideString;
bool test = true;
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HWND mhWnd;
HWND ghwnd;
HWND edit,dzf1,dzf2,dzf3,thrown,messg,status;
HWND button[10];
HWND hcd[22];
RECT mrect;
int width = 1000;
int height=700;
const int PORT = 12345;
const char* SERVER_IP = "127.0.0.1";
SOCKET clientSocket;
sockaddr_in serverAddr;
WSADATA wsaData;
const int BUFFER_SIZE = 1024;
char receiveBuffer[BUFFER_SIZE];
std::wstring Buffer;
int bytesSent;
bool g_terminateThread = false;
int bytesRead;
char msgtype;
type mtype;

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndProc_game(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void ReceiveMessages(HWND hwnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

  
    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GAMEHUAPAI, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GAMEHUAPAI));
    
    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
   
    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(0, 150, 0));
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GAMEHUAPAI);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

    WNDCLASSEX gameWcex = {};
    gameWcex.cbSize = sizeof(WNDCLASSEX);
    gameWcex.style = CS_HREDRAW | CS_VREDRAW;
    gameWcex.lpfnWndProc = WndProc_game;
    gameWcex.hInstance = hInstance;
    gameWcex.lpszClassName = L"game_window";
    gameWcex.hbrBackground = CreateSolidBrush(RGB(0, 150, 0));
    gameWcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

    if (!RegisterClassExW(&gameWcex))
    {
        MessageBox(NULL, L"游戏窗口类注册失败！", L"错误", MB_ICONERROR);
        return 0;
    }

    
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        MessageBox(NULL, L"Failed to initialize Winsock.", L"Error", MB_ICONERROR);
        return 0;
    }

    // 初始化服务器地址
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(PORT);

    return RegisterClassExW(&wcex);
}


//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   mhWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, hInstance, nullptr);
   if (!mhWnd)
   {
      return FALSE;
   }

   ShowWindow(mhWnd, nCmdShow);
   UpdateWindow(mhWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateWindowEx(0, L"BUTTON", L"开始游戏", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
            width/2-50, 300, 100, 35, hWnd, (HMENU)IDM_START, hInst, nullptr);
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_START:
                GetWindowRect(hWnd, &mrect);
                ShowWindow(hWnd, SW_HIDE);
                ghwnd = CreateWindowEx(0, L"game_window",
                    L"花牌游戏",
                    WS_VISIBLE | WS_OVERLAPPEDWINDOW,
                    mrect.left, mrect.top, width, height,
                    NULL,
                    NULL,
                    NULL,
                    NULL);
                ShowWindow(ghwnd, SW_SHOWNORMAL);
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK WndProc_game(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int xbegin = 840, ybegin = 400;
    int width = 30, hight = 25;
    switch (message)
    {
    case WM_CREATE:
        // 创建文本框和按钮
        edit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE ,
            200, 500, 600, 150, hWnd, (HMENU)IDC_EDIT, GetModuleHandle(NULL), NULL);   //手牌
        dzf1= CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE,
            30, 500, 150, 150, hWnd, (HMENU)IDC_EDIT2, GetModuleHandle(NULL), NULL);   //对，招，贩
        dzf2 = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE,
            30, 200, 150, 150, hWnd, (HMENU)IDC_EDIT2, GetModuleHandle(NULL), NULL);   //对，招，贩
        dzf3 = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE,
            820, 200, 150, 150, hWnd, (HMENU)IDC_EDIT2, GetModuleHandle(NULL), NULL);   //对，招，贩
        thrown = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE,
            250, 150, 500, 250, hWnd, (HMENU)IDC_EDIT3, GetModuleHandle(NULL), NULL);  //弃牌
        messg = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE,
            300, 90, 400,30, hWnd, (HMENU)IDC_EDIT4, GetModuleHandle(NULL), NULL);  //通知
        status = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE,
            300, 50, 400, 30, hWnd, (HMENU)IDC_EDIT5, GetModuleHandle(NULL), NULL);  //通知

        button[0]=CreateWindowEx(0,L"BUTTON", L"过", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            350, 460, 40, 25, hWnd, (HMENU)IDC_PASS_BUTTON, GetModuleHandle(NULL), NULL);
        button[1] = CreateWindowEx(0, L"BUTTON", L"统", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            400, 460, 40, 25, hWnd, (HMENU)IDC_TONG_BUTTON, GetModuleHandle(NULL), NULL);
        button[2] = CreateWindowEx(0, L"BUTTON", L"对", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            450, 460, 40, 25, hWnd, (HMENU)IDC_DUI_BUTTON, GetModuleHandle(NULL), NULL);
        button[3] = CreateWindowEx(0, L"BUTTON", L"开招", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            500, 460, 40, 25, hWnd, (HMENU)IDC_ZHAO_BUTTON, GetModuleHandle(NULL), NULL);
        button[4] = CreateWindowEx(0, L"BUTTON", L"开贩", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            550, 460, 40, 25, hWnd, (HMENU)IDC_FAN_BUTTON, GetModuleHandle(NULL), NULL);
        button[5] = CreateWindowEx(0, L"BUTTON", L"赶踏", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            600, 460, 40, 25, hWnd, (HMENU)IDC_TA_BUTTON, GetModuleHandle(NULL), NULL);
        button[6] = CreateWindowEx(0, L"BUTTON", L"胡", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            650, 460, 40, 25, hWnd, (HMENU)IDC_HU_BUTTON, GetModuleHandle(NULL), NULL);

        for (int i = 0; i < 7; i++) {
            for (int j = 0; j < 3; j++) {
                hcd[3*i+j]= CreateWindowEx(0, L"BUTTON", name[i*3+j], WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                    xbegin + j * 40, ybegin + i * 30, width, hight, hWnd, (HMENU)(140 + 3 * i + j), hInst, NULL);
            }
        }
        hcd[21] = CreateWindowEx(0, L"BUTTON", L"礼", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            xbegin, ybegin + 7 * 30, width, hight, hWnd, (HMENU)(140 + 21), hInst, NULL);
        
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            MessageBox(NULL, L"Failed to create client socket.", L"Error", MB_ICONERROR);
            PostQuitMessage(0);
        }

        if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            MessageBox(NULL, L"Failed to connect to the server.", L"Error", MB_ICONERROR);
            closesocket(clientSocket);
            PostQuitMessage(0);
        }

        // 启动接收服务器消息的线程
        thread(ReceiveMessages,hWnd).detach();
        break;
    
    case WM_CLOSE:
        // 关闭客户端socket
        g_terminateThread = true;
        closesocket(clientSocket);
        PostQuitMessage(0);
        //DestroyWindow(hWnd);
        //DestroyWindow(mhWnd);
        WSACleanup();
        
        SendMessage(mhWnd, WM_QUIT, 0, 0);
        break;

        break;
    case WM_RECEIVED_MESSAGE:
        mtype = static_cast<type>(wParam); // 从 WPARAM 中检索 msgtype
        Buffer = converter.from_bytes(reinterpret_cast<const char*>(lParam) + 1);
        switch (mtype) {
        case hand:
            SetWindowText(edit, L"");            
            SetWindowText(edit, Buffer.c_str());
            break;
        case notis:
            SetWindowText(messg, L"");           
            SetWindowText(messg, Buffer.c_str());
            break;
        case throwncd:
            SendMessage(thrown, EM_SETSEL, (WPARAM)-1, (LPARAM)-1); // 设置光标到文本末尾
            SendMessage(thrown, EM_REPLACESEL, 0, (LPARAM)Buffer.c_str());
            break;
        case errorturn:
            MessageBox(hWnd, L"现在不是你的回合",L"错误", 0);
            break;
        case errorop:
            MessageBox(hWnd, L"错误的操作", L"错误", 0);
            break;
        case statuss:
            SetWindowText(status, L"");
            SetWindowText(status, Buffer.c_str());
            break;
        case mydzf:
            SetWindowText(dzf1, L"");
            SetWindowText(dzf1, Buffer.c_str());
            break;
        case nextdzf:
            SetWindowText(dzf2, L"");
            SetWindowText(dzf2, Buffer.c_str());
            break;
        case enddzf:
            SetWindowText(dzf3, L"");
            SetWindowText(dzf3, Buffer.c_str());
            break;

        default:
            break;
        }
        // 释放分配的内存
        delete[] reinterpret_cast<char*>(lParam);
        break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        const char* msg[7] = { "pass","tong","dui","zhao","fan","ta","hu" };
        const char* msg2[22] = { "0","1","2","3","4","5","6","7","8","9","10","11",
            "12","13","14","15","16","17","18","19","20","21" };
        if (wmId >= 111 && wmId <= 117) {
            send(clientSocket, msg[wmId-111], sizeof(msg[wmId - 111]), 0);
        }
        else if (wmId >= 140 && wmId <= 161) {
            send(clientSocket, msg2[wmId - 140], sizeof(msg2[wmId - 140]), 0);
        }
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// 接收服务器消息的函数
void ReceiveMessages(HWND hWnd) {
    while (!g_terminateThread) {
        bytesRead = recv(clientSocket, receiveBuffer, sizeof(receiveBuffer), 0);
        if (bytesRead <= 0) {
            break;
        }
        // 处理接收到的消息
        receiveBuffer[bytesRead] = '\0'; // 添加字符串结束符
        msgtype = receiveBuffer[0];

        // Allocate memory for a copy of the received message and msgtype
        char* messageCopy = new char[BUFFER_SIZE];
        memcpy(messageCopy, receiveBuffer, BUFFER_SIZE);

        // 发送自定义消息到主窗口，将复制的消息指针和msgtype传递给PostMessage
        PostMessage(hWnd, WM_RECEIVED_MESSAGE, static_cast<WPARAM>(msgtype), reinterpret_cast<LPARAM>(messageCopy));
    }
}
