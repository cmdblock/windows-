// Windows窗口.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "Windows窗口.h"
#include <map>
#include <CommCtrl.h>
#include <strsafe.h>
#include <objidl.h>
#include <gdiplus.h>
#include <shellapi.h>

#pragma comment(lib, "gdiplus.lib")

using namespace std;
using namespace Gdiplus;

#define MAX_LOADSTRING 100

//定义自己的宏
#define MAINWND_DEFAULT_WIDTH 225 //主窗口默认大小宽度
#define MAINWND_DEFAULT_HEIGHT 310 //主窗口默认大小高度

#define RIGHTSUBWND_DEFAULT_WIDTH 120 //右边窗口的默认宽度
#define RIGHTSUBWND_DEFAULT_HEIGHT 310 //右边窗口的默认高度

#define ABOUT_DEFAULT_WIDTH 180		//About默认宽度
#define ABOUT_DEFAULT_HEIGHT 230		//About默认高度

#define ID_BTN_SEPARATE 27

#define ID_BTN_MC 0
#define ID_BTN_MR 1
#define ID_BTN_MS 2
#define ID_BTN_M_ADD 3
#define ID_BTN_M_SUBSTRACT 4
#define ID_BTN_DELETE 5
#define ID_BTN_CE 6
#define ID_BTN_C 7
#define ID_BTN_AS 8
#define ID_BTN_TICE 9
#define ID_BTN_DIVIDE 10
#define ID_BTN_PERSENT 11
#define ID_BTN_MULTIPY 12
#define ID_BTN_RECIPROCAL 13
#define ID_BTN_SUBTRACT 14
#define ID_BTN_EQUAL 15
#define ID_BTN_POINT 16

#define ID_BTN_NUM0 17
#define ID_BTN_NUM1 18
#define ID_BTN_NUM2 19
#define ID_BTN_NUM3 20
#define ID_BTN_NUM4 21
#define ID_BTN_NUM5 22
#define ID_BTN_NUM6 23
#define ID_BTN_NUM7 24
#define ID_BTN_NUM8 25
#define ID_BTN_NUM9 26

#define ID_TIMER WM_USER + 1

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
map<HWND, RECT> mapSubHWndRect;		//子窗口区域集合
SIZE szPreWnd;					//窗口之前的大小

HWND hWndText = NULL;
HWND hRightSubWnd = NULL;
HWND hMainWnd = NULL;
HWND hAboutWnd = NULL;
HWND hSeparateWnd = NULL; //分离窗口

TCHAR szRightSubWindowClass[] = _T("RightSubCale");		//右边窗口类名

BOOL bDragging = FALSE; //右侧窗口是否被拖动
POINT ptBegingDragging;

BOOL bPlugIn = FALSE; //右侧窗口是否被插入主窗口中

HHOOK hKeyHook = NULL;
HHOOK hHook = NULL;		//退出弹出框

WNDPROC oldProc = NULL;		//窗口子类化

UINT WM_BROADCAST = 0;	//广播消息

TCHAR szAbout[] = _T("About");		//关于窗口类名

INT iTextBottomPos;		//滚动文字位置改变

HMENU hMenu = NULL;		//托盘菜单句柄
NOTIFYICONDATA nid;

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

//声明窗口函数
HWND CreateStaticForMainWnd(HWND hWnd, LPCTSTR strText, LONG lStyle, int x, int y, int iWidth, int iHeight);
void CreateButtonForMainWnd(HWND hWnd, int iID, LPCTSTR strText, int x, int y, int iWidth, int iHeight);
void CreateStaticForRightSubWnd(HWND hWnd, int x, int y, int iWidth, int iHeight);
void CreateRadioBoxForRightSubWnd(HWND hWnd, LPCTSTR strText, int x, int y, int iWidth, int iHeight, BOOL bGroup = FALSE);

//回调函数
LRESULT CALLBACK HookKeyProc(int iCode, WPARAM wParam, LPARAM lParam);		//截获键盘按键
LRESULT CALLBACK RightSubWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);	//右边窗口创建的回调函数
LRESULT CALLBACK HookProc(int iCode, WPARAM wParam, LPARAM lParam);		//截获窗口
LRESULT CALLBACK HookWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);	//子窗口
LRESULT CALLBACK AboutWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//
void OnShowNumber(int iNum);
void DrawScrollTextBackground(HDC& hdc);		//绘制背景
void DrawScrollText(RECT rectText, HDC& hdc); //双缓冲绘制滚动背景

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此处放置代码。

	// Initialize GDI+.
	ULONG_PTR           uToken = 0;
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&uToken, &gdiplusStartupInput, NULL);

	// 初始化全局字符串
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WINDOWS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWS));

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

	GdiplusShutdown(uToken);

	return (int)msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWS));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WINDOWS);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	ATOM wResult = RegisterClassExW(&wcex);

	wcex.lpfnWndProc = RightSubWndProc;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szRightSubWindowClass;
	wResult = RegisterClassExW(&wcex);

	wcex.lpfnWndProc = AboutWndProc;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szAbout;
	wResult = RegisterClassExW(&wcex);

	return wResult;
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

	hMainWnd = CreateWindowW(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		200,
		200,
		MAINWND_DEFAULT_WIDTH,
		MAINWND_DEFAULT_HEIGHT,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	if (!hMainWnd)
	{
		return FALSE;
	}

	// 设置窗口标题
	SetWindowText(hMainWnd, L"计算器");

	//禁止最大化按钮
	DWORD dwStyle = GetWindowLong(hMainWnd, GWL_STYLE);
	dwStyle &= ~WS_MAXIMIZEBOX;
	SetWindowLong(hMainWnd, GWL_STYLE, dwStyle);

	//修改菜单名字
	HMENU hMenuBar = GetMenu(hMainWnd);
	HMENU hHelpMenu = GetSubMenu(hMenuBar, 1); //得到第二个子菜单
	UINT_PTR uItemID = GetMenuItemID(hHelpMenu, 0);
	ModifyMenu(hHelpMenu, 0, MF_BYPOSITION, uItemID, _T("About"));

	//修改窗口大小
	RECT rect;
	GetWindowRect(hMainWnd, &rect);
	//在这里使用MoveWindows函数会触发WM_SIZE消息
	MoveWindow(hMainWnd, rect.left, rect.top, MAINWND_DEFAULT_WIDTH, MAINWND_DEFAULT_HEIGHT, TRUE);

	ShowWindow(hMainWnd, nCmdShow);
	UpdateWindow(hMainWnd);

	//右边子窗口的创建
	hRightSubWnd = CreateWindowEx(
		WS_EX_LAYERED,
		szRightSubWindowClass,
		_T(""),
		WS_POPUP,
		rect.right,
		rect.top,
		RIGHTSUBWND_DEFAULT_WIDTH,
		RIGHTSUBWND_DEFAULT_HEIGHT,
		hMainWnd,
		NULL,
		hInstance,
		NULL
	);
	ShowWindow(hRightSubWnd, nCmdShow);
	UpdateWindow(hRightSubWnd);

	SetLayeredWindowAttributes(hRightSubWnd, 0, 255, LWA_ALPHA);

	//注册广播消息
	WM_BROADCAST = RegisterWindowMessage(_T("BroadCastCommand"));

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
		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			int wmEvent = HIWORD(wParam);

			//按键
			if (wmId == ID_BTN_MC)
			{
				SetWindowText(hWndText, _T("MC"));
			}
			else if (wmId == ID_BTN_MR)
			{
				SetWindowText(hWndText, _T("MR"));
			}
			else if (wmId == ID_BTN_MS)
			{
				SetWindowText(hWndText, _T("MS"));
			}
			else if (wmId == ID_BTN_M_ADD)
			{
				SetWindowText(hWndText, _T("M+"));
			}
			else if (wmId == ID_BTN_M_SUBSTRACT)
			{
				SetWindowText(hWndText, _T("M-"));
			}
			else if (wmId == ID_BTN_DELETE)
			{
				SetWindowText(hWndText, _T("Del"));
			}
			else if (wmId == ID_BTN_CE)
			{
				SetWindowText(hWndText, _T("CE"));
			}
			else if (wmId == ID_BTN_C)
			{
				SetWindowText(hWndText, _T("C"));
			}
			else if (wmId == ID_BTN_AS)
			{
				SetWindowText(hWndText, _T("+-"));
			}
			else if (wmId == ID_BTN_TICE)
			{
				SetWindowText(hWndText, _T("V"));
			}
			else if (wmId == ID_BTN_DIVIDE)
			{
				SetWindowText(hWndText, _T("/"));
			}
			else if (wmId == ID_BTN_PERSENT)
			{
				SetWindowText(hWndText, _T("%"));
			}
			else if (wmId == ID_BTN_RECIPROCAL)
			{
				SetWindowText(hWndText, _T("1/x"));
			}
			else if (wmId == ID_BTN_SUBTRACT)
			{
				SetWindowText(hWndText, _T("-"));
			}
			else if (wmId == ID_BTN_EQUAL)
			{
				SetWindowText(hWndText, _T("="));
			}
			else if (wmId == ID_BTN_POINT)
			{
				SetWindowText(hWndText, _T("."));
			}
			else if (wmId == ID_BTN_EQUAL)
			{
				SetWindowText(hWndText, _T("+"));
			}
			else if (wmId >= ID_BTN_NUM0 && wmId <= ID_BTN_NUM9)
			{
				OnShowNumber(wmId - ID_BTN_NUM0 + 48);
			}

			// 分析菜单选择:
			switch (wmId)
			{
				case IDM_ABOUT:
					RECT rect;
					GetWindowRect(hWnd, &rect);
					if (hAboutWnd == NULL)
					{
						hAboutWnd = CreateWindowEx(
							WS_EX_LAYERED,
							szAbout,
							_T(""),
							WS_POPUP | WS_VISIBLE,
							rect.left + 350,
							rect.top + 80,
							ABOUT_DEFAULT_WIDTH,
							ABOUT_DEFAULT_HEIGHT,
							hWnd,
							NULL,
							hInst,
							NULL);
						ShowWindow(hAboutWnd, SW_SHOWNORMAL);
						UpdateWindow(hAboutWnd);
						SetLayeredWindowAttributes(hAboutWnd, 0, 80, LWA_ALPHA);
						iTextBottomPos = 0;
					}
					break;
				case IDM_EXIT:
					SendMessage(hWnd, WM_CLOSE, wParam, lParam);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
		case WM_CLOSE:
			hHook = SetWindowsHookEx(WH_CALLWNDPROC, HookProc, NULL, GetCurrentThreadId());		//捕获messagebox弹出来的消息

			if (MessageBox(hWnd, _T("是否关闭"), _T("提示"), MB_YESNO) == IDYES)
			{
				DWORD dw = BSM_APPLICATIONS;
				BroadcastSystemMessage(BSF_QUERY, &dw, WM_BROADCAST, NULL, NULL);

				UnhookWindowsHookEx(hHook);
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			UnhookWindowsHookEx(hHook);
			break;
		case WM_MOVE:
			if (hRightSubWnd != NULL && IsWindow(hRightSubWnd))
			{
				RECT rect;
				GetWindowRect(hWnd, &rect);
				SetWindowPos(hRightSubWnd, hWnd, rect.right, rect.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
			}
			break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: 在此处添加使用 hdc 的任何绘图代码...
			EndPaint(hWnd, &ps);
		}
		break;
		case WM_CREATE:
		{
			RECT rect;
			GetWindowRect(hWnd, &rect);
			szPreWnd.cx = rect.right - rect.left;
			szPreWnd.cy = rect.bottom - rect.top;

			CreateStaticForMainWnd(hWnd, _T(""), WS_CHILD | WS_VISIBLE, 10, 15, 195, 50);
			hWndText = CreateStaticForMainWnd(hWnd, _T("1234"), WS_CHILD | WS_VISIBLE | SS_RIGHT, 12, 35, 191, 20);
			CreateStaticForMainWnd(hWnd, _T(""), WS_CHILD | WS_VISIBLE | SS_GRAYFRAME, 10, 15, 195, 50);

			CreateButtonForMainWnd(hWnd, ID_BTN_MC, _T("MC"), 10, 75, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_MR, _T("MR"), 50, 75, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_MS, _T("MS"), 90, 75, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_M_ADD, _T("M+"), 130, 75, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_M_SUBSTRACT, _T("M-"), 170, 75, 35, 25);

			CreateButtonForMainWnd(hWnd, ID_BTN_DELETE, _T("Del"), 10, 105, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_CE, _T("CE"), 50, 105, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_C, _T("C"), 90, 105, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_AS, _T("+-"), 130, 105, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_TICE, _T("V"), 170, 105, 35, 25);

			CreateButtonForMainWnd(hWnd, ID_BTN_NUM7, _T("7"), 10, 135, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_NUM8, _T("8"), 50, 135, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_NUM9, _T("9"), 90, 135, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_DIVIDE, _T("/"), 130, 135, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_PERSENT, _T("%"), 170, 135, 35, 25);

			CreateButtonForMainWnd(hWnd, ID_BTN_NUM4, _T("4"), 10, 165, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_NUM5, _T("5"), 50, 165, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_NUM6, _T("6"), 90, 165, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_MULTIPY, _T("*"), 130, 165, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_RECIPROCAL, _T("1/x"), 170, 165, 35, 25);

			CreateButtonForMainWnd(hWnd, ID_BTN_NUM1, _T("1"), 10, 195, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_NUM2, _T("2"), 50, 195, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_NUM3, _T("3"), 90, 195, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_SUBTRACT, _T("-"), 130, 195, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_EQUAL, _T("="), 170, 195, 35, 55);

			CreateButtonForMainWnd(hWnd, ID_BTN_NUM0, _T("0"), 10, 225, 75, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_POINT, _T("."), 90, 225, 35, 25);
			CreateButtonForMainWnd(hWnd, ID_BTN_EQUAL, _T("+"), 130, 225, 35, 25);

			//安装钩子
			hKeyHook = SetWindowsHookEx(WH_KEYBOARD, HookKeyProc, NULL, GetCurrentThreadId());

			//创建托盘菜单
			nid.cbSize = sizeof(nid);
			nid.hWnd = hWnd;
			nid.uID = 0;
			nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
			nid.uCallbackMessage = WM_USER;
			nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));

			hMenu = CreatePopupMenu();
			AppendMenu(hMenu, MF_STRING, IDM_EXIT, _T("退出"));
		}
		break;
		case WM_USER:
			if (lParam == WM_LBUTTONDOWN)		//点击左键恢复原窗口
			{
				ShowWindow(hWnd, SW_SHOWNORMAL);
				Shell_NotifyIcon(NIM_DELETE, &nid);
				SetForegroundWindow(hWnd);
			}
			
			if (lParam == WM_RBUTTONDOWN) //点击右键
			{
				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow(hWnd);
				TrackPopupMenu(hMenu, NULL, pt.x, pt.y, NULL, hWnd, NULL);	//显示菜单
			}
			break;
		case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED)
			{
				Shell_NotifyIcon(NIM_ADD, &nid);
				ShowWindow(hWnd, SW_HIDE);
			}
			else
			{
				//自适应控件大小
				WORD x = LOWORD(lParam);
				WORD y = HIWORD(lParam);

				//右侧窗口被拖进来了
				if (bPlugIn)
				{
					x -= RIGHTSUBWND_DEFAULT_WIDTH;
				}

				double dx = (double)x / szPreWnd.cx;
				double dy = (double)y / szPreWnd.cy;

				for (map<HWND, RECT>::iterator it = mapSubHWndRect.begin(); it != mapSubHWndRect.end(); ++it)
				{
					pair<HWND, RECT> pi = *it;
					MoveWindow(pi.first, it->second.left * dx, pi.second.top * dy,
						(pi.second.right - pi.second.left) * dx, (pi.second.bottom - pi.second.top) * dy, TRUE);
				}

				if (hRightSubWnd != NULL && IsWindow(hRightSubWnd))
				{
					RECT rect;
					GetWindowRect(hWnd, &rect);
					SetWindowPos(hRightSubWnd, hWnd, rect.right, rect.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
				}
			}
		}
		break;
		case WM_DESTROY:
			Shell_NotifyIcon(NIM_DELETE, &nid);
			UnhookWindowsHookEx(hKeyHook);
			PostQuitMessage(0);
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

HWND CreateStaticForMainWnd(HWND hWnd, LPCTSTR strText, LONG lStyle, int x, int y, int iWidth, int iHeight)
{
	HWND hWndResult = CreateWindow(_T("STATIC"), strText, lStyle, x, y,
		iWidth, iHeight, hWnd, NULL, hInst, NULL);

	RECT rect;
	rect.left = x;
	rect.top = y;
	rect.right = x + iWidth;
	rect.bottom = y + iHeight;

	mapSubHWndRect.insert(pair<HWND, RECT>(hWndResult, rect));
	return hWndResult;
}

void CreateButtonForMainWnd(HWND hWnd, int iID, LPCTSTR strText, int x, int y, int iWidth, int iHeight)
{
	HWND hWndTemp = CreateWindow(_T("Button"), strText, WS_CHILD | WS_VISIBLE,
		x, y, iWidth, iHeight, hWnd, (HMENU)iID, hInst, NULL);

	RECT rect;
	rect.left = x;
	rect.top = y;
	rect.right = x + iWidth;
	rect.bottom = y + iHeight;

	mapSubHWndRect.insert(pair<HWND, RECT>(hWndTemp, rect));
}

LRESULT CALLBACK HookKeyProc(int iCode, WPARAM wParam, LPARAM lParam)
{
	if (iCode == HC_ACTION)
	{
		UINT nFlags = HIWORD(lParam);
		BOOL bKeyDown = (0 == (nFlags & KF_UP));

		if (bKeyDown)
		{
			//数字键的0-9
			if (wParam >= VK_NUMPAD0 && wParam <= VK_NUMPAD9)
			{
				//转换成主键盘的按键
				wParam -= 0x30;
			}
			//主键盘的0-9
			if (wParam >= 0x30 && wParam <= 0x39)
			{
				OnShowNumber(wParam);
			}
		}
	}
	return CallNextHookEx(hKeyHook, iCode, lParam, wParam);
}

void OnShowNumber(int iNum)
{
	TCHAR tc[2] = { 0 };
	tc[0] = (TCHAR)iNum;

	TCHAR tcCur[100] = { 0 };
	GetWindowText(hWndText, tcCur, 100);

	LPCTSTR str = lstrcat(tcCur, tc);

	if (lstrlen(str) > 50)
	{
		str = _T("");
	}

	SetWindowText(hWndText, str);
}

LRESULT CALLBACK RightSubWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			CreateStaticForRightSubWnd(hWnd, 10, 20, 90, 90);
			CreateRadioBoxForRightSubWnd(hWnd, _T("16进制"), 15, 25, 80, 20, TRUE);
			CreateRadioBoxForRightSubWnd(hWnd, _T("10进制"), 15, 45, 80, 20);
			CreateRadioBoxForRightSubWnd(hWnd, _T("8进制"), 15, 65, 80, 20);
			CreateRadioBoxForRightSubWnd(hWnd, _T("2进制"), 15, 85, 80, 20);

			//
			CreateStaticForRightSubWnd(hWnd, 10, 120, 90, 90);
			CreateRadioBoxForRightSubWnd(hWnd, _T("四字"), 15, 125, 80, 20, TRUE);
			CreateRadioBoxForRightSubWnd(hWnd, _T("双字"), 15, 145, 80, 20);
			CreateRadioBoxForRightSubWnd(hWnd, _T("字"), 15, 165, 80, 20);
			CreateRadioBoxForRightSubWnd(hWnd, _T("字节"), 15, 185, 80, 20);

			//创建分离窗口
			hSeparateWnd = CreateWindow(_T("Button"), _T("分离"), WS_CHILD, 10, 230, 80, 20,
				hWnd, (HMENU)ID_BTN_SEPARATE, hInst, NULL);

			//创建提示窗口
			HWND hWndTip;
			hWndTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
				WS_POPUP | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT,
				CW_USEDEFAULT, CW_USEDEFAULT, hRightSubWnd, NULL, hInst, NULL);

			TCHAR tcContentAssistant[40];
			StringCchPrintf(tcContentAssistant, 20, _T("主窗口和右侧窗口分离"));
			TOOLINFO toolInfo;
			toolInfo = { 0 };
			toolInfo.cbSize = 44;
			toolInfo.hwnd = hRightSubWnd;
			toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
			toolInfo.uId = (UINT_PTR)hSeparateWnd;
			toolInfo.lpszText = tcContentAssistant;
			SendMessage(hWndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);

			break;
		case WM_LBUTTONDOWN:
			bDragging = TRUE;
			ptBegingDragging.x = LOWORD(lParam);
			ptBegingDragging.y = HIWORD(lParam);
			SetCapture(hWnd);		//鼠标移除窗口也可以起作用
			break;
		case WM_MOUSEMOVE:
			if (!bPlugIn && bDragging)
			{
				RECT rect;
				GetWindowRect(hWnd, &rect);
				TCHAR tc[100] = { 0 };
				_itot_s(rect.left, tc, 100, 10);

				SIZE sz = { 0 };
				sz.cx = (SHORT)LOWORD(lParam) - ptBegingDragging.x;
				sz.cy = (SHORT)HIWORD(lParam) - ptBegingDragging.y;

				rect.left += sz.cx;
				rect.top += sz.cy;

				SetLayeredWindowAttributes(hWnd, 0, 128, LWA_ALPHA);

				RECT rectMainWnd;
				GetWindowRect(hMainWnd, &rectMainWnd);

				RECT rectRightSubWnd;
				GetWindowRect(hRightSubWnd, &rectRightSubWnd);

				if (rectRightSubWnd.left > rectMainWnd.right)
				{
					if (rectRightSubWnd.left - rectMainWnd.right < 5)
					{
						SetWindowPos(hWnd, hMainWnd, rectMainWnd.right, rectMainWnd.top, 0, 0, SWP_NOSIZE);
						if (sz.cx > 5 || sz.cx < 0)
						{
							SetWindowPos(hWnd, hMainWnd, rect.left, rect.top, 0, 0, SWP_NOSIZE);
						}
						break;
					}
				}
				SetWindowPos(hWnd, hMainWnd, rect.left, rect.top, 0, 0, SWP_NOSIZE);
			}
			break;
		case WM_LBUTTONUP:
			bDragging = FALSE;
			ReleaseCapture();
			SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);

			RECT rectMainWnd;
			GetWindowRect(hMainWnd, &rectMainWnd);

			RECT rectRightSubWnd;
			GetWindowRect(hRightSubWnd, &rectRightSubWnd);

			POINT ptRightSubWnd;
			ptRightSubWnd.x = rectRightSubWnd.left;
			ptRightSubWnd.y = rectRightSubWnd.top;

			if (PtInRect(&rectMainWnd, ptRightSubWnd))		//指定点是否在矩形框内
			{
				ShowWindow(hSeparateWnd, SW_SHOWDEFAULT);		//显示分离按钮
				RECT rect;

				bPlugIn = TRUE;
				DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
				dwStyle &= ~WS_POPUP;
				dwStyle |= WS_CHILD;			//去掉弹出窗口，变成子窗口
				SetWindowLong(hWnd, GWL_STYLE, dwStyle);

				//层窗口是扩展属性
				DWORD dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
				dwExStyle &= ~GWL_EXSTYLE;
				SetWindowLong(hWnd, GWL_EXSTYLE, dwExStyle);

				//改变主窗口大小
				SetWindowPos(hMainWnd, NULL, 0, 0, MAINWND_DEFAULT_WIDTH + RIGHTSUBWND_DEFAULT_WIDTH,
					MAINWND_DEFAULT_HEIGHT, SWP_NOMOVE);
				SetWindowPos(hWnd, hMainWnd, MAINWND_DEFAULT_WIDTH, 0,
					RIGHTSUBWND_DEFAULT_WIDTH, RIGHTSUBWND_DEFAULT_HEIGHT, NULL);

				//记录右侧窗口新的区域
				rect.left = MAINWND_DEFAULT_WIDTH;
				rect.top = 0;
				rect.right = MAINWND_DEFAULT_WIDTH + RIGHTSUBWND_DEFAULT_WIDTH;
				rect.bottom = RIGHTSUBWND_DEFAULT_HEIGHT;

				mapSubHWndRect.insert(pair<HWND, RECT>(hWnd, rect));
				SetParent(hWnd, hMainWnd);
			}
			break;
		case WM_COMMAND:
			WORD cmd;
			cmd = LOWORD(wParam);
			if (cmd == ID_BTN_SEPARATE)
			{
				mapSubHWndRect.erase(hWnd);

				//还原风格
				LONG lStyle = GetWindowLong(hWnd, GWL_STYLE);
				lStyle &= ~WS_CHILD;
				lStyle |= WS_POPUP;
				SetWindowLong(hWnd, GWL_STYLE, lStyle);

				bPlugIn = FALSE;

				SetWindowPos(hMainWnd, NULL, 0, 0, MAINWND_DEFAULT_WIDTH,
					MAINWND_DEFAULT_HEIGHT, SWP_NOMOVE);		//改变主窗口大小
				SetParent(hWnd, NULL);
				RECT rectMainWnd;
				GetWindowRect(hMainWnd, &rectMainWnd);
				SetWindowPos(hWnd, hMainWnd, rectMainWnd.right, rectMainWnd.top,
					RIGHTSUBWND_DEFAULT_WIDTH, RIGHTSUBWND_DEFAULT_HEIGHT, NULL);		//改变右侧窗口位置
				ShowWindow(hSeparateWnd, SW_HIDE);		//隐藏分离按钮

				//右侧窗口从普通窗口恢复为层窗口
				lStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
				lStyle |= WS_EX_LAYERED;
				SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_LAYERED);
				SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);
			}
			break;
		default:
			if (message == WM_BROADCAST)
			{
				SendMessage(hWnd, WM_CLOSE, 0, 0);
				break;
			}
			else
			{
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
	}
	return 0;
}

void CreateStaticForRightSubWnd(HWND hWnd, int x, int y, int iWidth, int iHeight)
{
	CreateWindow(_T("STATIC"), _T(""), WS_CHILD | SS_GRAYFRAME | WS_VISIBLE,
		x, y, iWidth, iHeight, hWnd, NULL, hInst, NULL);
}

void CreateRadioBoxForRightSubWnd(HWND hWnd, LPCTSTR strText, int x, int y, int iWidth, int iHeight, BOOL bGroup)
{
	LONG lStyle = WS_CHILD | BS_AUTORADIOBUTTON | WS_VISIBLE;
	if (bGroup)
	{
		lStyle |= lStyle | WS_GROUP;
	}

	CreateWindow(_T("Button"), strText, lStyle, x, y, iWidth, iHeight, hWnd, NULL, hInst, NULL);
}

LRESULT CALLBACK HookProc(int iCode, WPARAM wParam, LPARAM lParam)
{
	if (iCode == HC_ACTION)
	{
		CWPSTRUCT* pwp = (CWPSTRUCT*)lParam;
		if (pwp->message == WM_INITDIALOG)
		{
			oldProc = (WNDPROC)SetWindowLong(pwp->hwnd, GWL_WNDPROC, (LONG)HookWndProc);		//窗口子类化
		}
	}
	return CallNextHookEx(hHook, iCode, wParam, lParam);
}

LRESULT CALLBACK HookWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_PAINT)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 88));
		HBRUSH hBrushOld = (HBRUSH)SelectObject(hdc, hBrush);

		RECT rect;
		GetClientRect(hWnd, &rect);
		Rectangle(hdc, 0, 0, rect.right, rect.bottom);

		SelectObject(hdc, hBrushOld);
		DeleteObject(hBrush);

		EndPaint(hWnd, &ps);
	}
	else if (message == WM_DESTROY)
	{
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)oldProc);
	}
	return CallWindowProc(oldProc, hWnd, message, wParam, lParam);
}

LRESULT CALLBACK AboutWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rectText;
	switch (message)
	{
		case WM_CREATE:
			SetTimer(hWnd, ID_TIMER, USER_TIMER_MINIMUM + 20, NULL);
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			GetWindowRect(hWnd, &rectText);
			DrawScrollTextBackground(hdc);
			EndPaint(hWnd, &ps);
			break;
		case WM_TIMER:
			hdc = GetDC(hWnd);
			GetWindowRect(hWnd, &rectText);
			DrawScrollText(rectText, hdc);
			ReleaseDC(hWnd, hdc);
			break;
		case WM_LBUTTONUP:
			SendMessage(hWnd, WM_CLOSE, wParam, lParam);
			break;
		case WM_DESTROY:
			KillTimer(hWnd, ID_TIMER);
			hAboutWnd = NULL;
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void DrawScrollTextBackground(HDC& hdc)
{
	Gdiplus::Bitmap bitmap(ABOUT_DEFAULT_WIDTH, ABOUT_DEFAULT_HEIGHT, PixelFormat32bppARGB);
	Gdiplus::Graphics graphics(&bitmap);

	Gdiplus::SolidBrush solidBrush(Gdiplus::Color(255, 255, 0, 0));

	Gdiplus::Rect gdiRect(0, 0, ABOUT_DEFAULT_WIDTH, ABOUT_DEFAULT_HEIGHT);
	graphics.FillRectangle(&solidBrush, gdiRect);

	Gdiplus::Graphics graphicsHdc(hdc);
	Gdiplus::CachedBitmap cachedBmp(&bitmap, &graphicsHdc);
	graphicsHdc.DrawCachedBitmap(&cachedBmp, 0, 0);
};

void DrawScrollText(RECT rectText, HDC& hdc)
{
	SIZE sz;
	sz.cx = rectText.right - rectText.left;
	sz.cy = rectText.bottom - rectText.top;

	Gdiplus::Bitmap bitmap(sz.cx, sz.cy, PixelFormat32bppARGB);
	Gdiplus::Graphics graphics(&bitmap);

	Gdiplus::SolidBrush solidBrush(Gdiplus::Color(255, 255, 222, 173));

	Gdiplus::Rect gdiRect(0, 0, sz.cx, sz.cy);
	graphics.FillRectangle(&solidBrush, gdiRect);

	Gdiplus::Font font(L"微软雅黑", 12, FontStyleBold);
	SolidBrush brush(Color(255, 0, 0, 0));
	StringFormat format;
	format.SetAlignment(StringAlignmentCenter);

	PointF ptF(sz.cx / 2, sz.cy - 40 - iTextBottomPos);
	++iTextBottomPos;
	if (ptF.Y <= 0)
	{
		ptF.Y = sz.cy - 40;
		iTextBottomPos = 0;
	}

	graphics.DrawString(L"点击本窗口可以退出\n谢谢大家！", -1, &font, ptF, &format, &brush);

	Gdiplus::Graphics graphicsHdc(hdc);
	Gdiplus::CachedBitmap cachedBmp(&bitmap, &graphicsHdc);
	graphicsHdc.DrawCachedBitmap(&cachedBmp, 0, 0);
};