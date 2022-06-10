
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define ID_TEXTBOX 0
#define ID_BUTTON1 1
#define ID_BUTTON2 2
#define ID_BUTTON3 3
#define ID_CONFIRMCHOISE 4
#define SIZE_OF_NUM 6
#define RBUFFER_SIZE 7
#define GLOBAL_SERVER 0
#define GLOBAL_CLIENT 1
#define DEBUG 0
#define ERROR 1
#define ALL begin(arr), end(arr)

#include <string>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <filesystem>
#include <chrono>
#include <ctime>
#include "resource.h"
using std::string;


string title = "Game20";
string MainWindowClass = "MainWindow";
string YouWin = "You win";
string YourenemyWin = "Your enemy won";

const int MaxPixelsFSX = GetSystemMetrics(SM_CXFULLSCREEN);
const int MaxPixelsFSY = GetSystemMetrics(SM_CYFULLSCREEN);
int windowMaxX;
int windowMaxY;
RECT window;
const char* sendBuffer;
HDC WindowDC;
HINSTANCE globalhIst;
static HWND tbox, button1, button2, button3, confirmChoice, global_hWnd;
BOOL _bool;
WSADATA ws;
SOCKET sock = INVALID_SOCKET;
SOCKET lisen = INVALID_SOCKET;
SOCKET client = INVALID_SOCKET;

ADDRINFO hints;
ADDRINFO* addresult = nullptr;
HWND globalHWnd;
bool isServer;
struct Game {
	std::vector<int> arr; //array of numbers
	bool isOver;
	int AllSum;//sum of all numbers
	bool isMyTurn;
	string nickname;
	bool doIWin;
	int lastNum;
	string EncodeMessage();
	void DecodeMessage(const string&);
	void append(int);
	Game operator= (int c)
	{
		AllSum = c;
		lastNum = c;
		arr.erase(ALL);
		return *this;
	}
	string ToString()
	{
		string str;
		str += "game:";
		str += "\nisOver: "		+ isOver	? "true" : "false";
		str += "\nisMyTurn: "	+ isMyTurn	? "true" : "false";
		str += "\ndoIWin: "		+ doIWin	? "true" : "false";
		str += "\nAllSum: "		+ std::to_string(AllSum);
		str += "\nlastNum: "	+ std::to_string(lastNum);
		str += "\nArr: ";
		
		for (const auto& val : arr)
			str += std::to_string(val) + ", ";
		return str;
	}
} ;
Game game;
LRESULT CALLBACK WndProc(
	_In_ HWND   hWnd,
	_In_ UINT   message,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);
INT_PTR CALLBACK About(
	_In_ HWND   hWnd,
	_In_ UINT   message,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);
INT_PTR CALLBACK Results(
	_In_ HWND   hWnd,
	_In_ UINT   message,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);
void CreateBoxes(HWND hWnd);
void SetMaxXY(int& x, int& y, RECT rt);
void SEND();
DWORD WINAPI Server(CONST LPVOID);
DWORD WINAPI Client(CONST LPVOID);
void EnableDisableButtons(HWND hWnd, bool _b);
void EnableDisableMenu(HWND hWnd, bool _b);
void log(const string& output);
string TransformLogMessage(int c, int mode, const string& message);
void Draw();

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	game.arr.reserve(20u);
	game.AllSum = 0;
	game.doIWin = false;
	game.lastNum =-1;
	game.isOver = false;
	WNDCLASSEX wcex;
	//fill window class
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wcex.lpszClassName = &MainWindowClass[0];
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	//register main window 
	const bool result(RegisterClassEx(&wcex));
	HWND hwnd;
	hwnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		MainWindowClass.c_str(),
		title.c_str(),
		WS_OVERLAPPEDWINDOW,
		//center window
		MaxPixelsFSX / 2 - MaxPixelsFSX / 4, MaxPixelsFSY / 2 - MaxPixelsFSY / 4,
		MaxPixelsFSX / 2, MaxPixelsFSY / 2,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	globalhIst = hInstance;
	globalHWnd = hwnd;
	WindowDC = GetDC(hwnd);
	ShowWindow(hwnd,
		nCmdShow);
	UpdateWindow(hwnd);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(
	_In_ HWND   hWnd,
	_In_ UINT   message,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	char num[8];
	string t;
	switch (message)
	{
	case WM_CREATE:
		GetClientRect(hWnd, &window);
		SetMaxXY(windowMaxX, windowMaxY, window);
		CreateBoxes(hWnd);
		EnableDisableButtons(hWnd, false);
		break;
	case WM_COMMAND:

		switch (wParam)
		{
		case ID_MENU_EXIT:
			PostQuitMessage(60);
			break;
		case ID_BECOME_HOST:			
			game = 0;
			game.isOver = false;
			isServer = true;
			game.isMyTurn = true;
			game.doIWin = false;
			InvalidateRect(hWnd, &window, 1);					
			CreateThread(0, 0, &Server, 0, 0, 0);			
			EnableDisableButtons(hWnd, game.isMyTurn);			
			break;
		case ID_CONNECTTO:
			game = 0;
			game.isOver = false;
			isServer = false;
			game.isMyTurn = false; 
			game.doIWin = false;
			InvalidateRect(hWnd,&window,1);			
			CreateThread(0, 0, &Client, 0, 0, 0);
			log(TransformLogMessage(isServer, DEBUG, game.ToString()));
			EnableDisableButtons(hWnd, game.isMyTurn);			
			break;
		case ID_HELP_ABOUT:
			
			DialogBoxParam(
				globalhIst,
				MAKEINTRESOURCE(IDD_DIALOG1),
				hWnd,
				About,
				0
			);
			
			break;
		case ID_RESULTS_SHOWRESULTS:
			DialogBoxParam(
				globalhIst,
				MAKEINTRESOURCE(IDD_DIALOG2),
				hWnd,
				Results,
				0
			);
			break;
		default:
			break;
		}
		//ZeroMemory(&num, sizeof(num));
		switch (LOWORD(wParam))
		{
		case ID_BUTTON1:
			SetWindowText(tbox, "1");
			break;
		case ID_BUTTON2:
			SetWindowText(tbox, "2");
			break;
		case ID_BUTTON3:
			SetWindowText(tbox, "3");
			break;
		case ID_CONFIRMCHOISE:

			GetWindowText(tbox, num, sizeof(num));
			t = num;
			game.append(std::stoi(t));
			SetWindowText(tbox, "1");
			Draw();
			isServer ? 
				_bool = send(sock, game.EncodeMessage().c_str(), RBUFFER_SIZE, 0) :
				_bool = send(client, game.EncodeMessage().c_str(), RBUFFER_SIZE, 0);
			if (_bool == SOCKET_ERROR)
			{
				MessageBox(0, "Failed sending data", isServer ? "Server" :"client" , 1l);
				isServer? closesocket(sock): closesocket(client) ;
				freeaddrinfo(addresult);
				WSACleanup();
			}

			break;
		default:
			break;
		}
		break;
	case WM_SIZE:
		GetClientRect(hWnd, &window);

		break;
	case WM_DESTROY:

		PostQuitMessage(69);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

}
void CreateBoxes(HWND hWnd)
{
	tbox = CreateWindow(
		"edit",										//textbox
		"1",										//value which shows by start
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_CENTER | ES_READONLY,
		(windowMaxX / 2) - 50, (windowMaxY / 2),
		110, 20,
		hWnd, (HMENU)ID_TEXTBOX, 0, 0);

	button1 = CreateWindow(
		"button",									//textbox
		"1",										//value which shows by start
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
		(windowMaxX / 2) - 165, (windowMaxY / 2) + 30,
		110, 20,
		hWnd, (HMENU)ID_BUTTON1, 0, 0);

	button2 = CreateWindow(
		"button",									//textbox
		"2",										//value which shows by start
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
		(windowMaxX / 2) - 50, (windowMaxY / 2) + 30,
		110, 20,
		hWnd, (HMENU)ID_BUTTON2, 0, 0);

	button3 = CreateWindow(
		"button",									//textbox
		"3",										//value which shows by start
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
		(windowMaxX / 2) + 63, (windowMaxY / 2) + 30,
		110, 20,
		hWnd, (HMENU)ID_BUTTON3, 0, 0);

	confirmChoice = CreateWindow(
		"button",									//textbox
		"Send",										//value which shows by start
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
		(windowMaxX / 2) - 112, (windowMaxY / 2) + 60,
		232, 21,
		hWnd, (HMENU)ID_CONFIRMCHOISE, 0, 0);
}
void SetMaxXY(int& x, int& y, RECT rt)
{
	x = rt.right - rt.left;
	y = rt.bottom - rt.top;
}
DWORD WINAPI Server(CONST LPVOID)
{
	//initializing sokit
	//		0 - succes  1-failed
	_bool = WSAStartup(MAKEWORD(2, 2), &ws);
	if (_bool)
	{
		MessageBox(0, "Soket failed", "Soket failed", 1);
		ExitThread(36);
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	_bool = getaddrinfo(NULL, "8080", &hints, &addresult);
	if (_bool)
	{
		MessageBox(0, "", "getaddr failed", 1);

		WSACleanup();
		freeaddrinfo(addresult);
		ExitThread(36);
	}

	lisen = socket(addresult->ai_family, addresult->ai_socktype, addresult->ai_protocol);
	if (lisen == INVALID_SOCKET)
	{
		MessageBox(0, "Socket creation failed", "Socket creation failed", 1l);
		WSACleanup();
		freeaddrinfo(addresult);

		ExitThread(36);
	}
	log(TransformLogMessage(!isServer, DEBUG, "Someone want to connect"));

	_bool = bind(lisen, addresult->ai_addr, static_cast<int>(addresult->ai_addrlen));
	if (_bool == SOCKET_ERROR)
	{
		MessageBox(0, "binding socket failed", "server error", 1l);

		closesocket(lisen);
		lisen = INVALID_SOCKET;
		freeaddrinfo(addresult);
		WSACleanup();
		ExitThread(36);
	}
	log(TransformLogMessage(!isServer, DEBUG, "Binding succesfull"));

	_bool = listen(lisen, SOMAXCONN);
	if (_bool == SOCKET_ERROR)
	{
		MessageBox(0, "Listening failed", "server error", 1l);
		closesocket(lisen);
		freeaddrinfo(addresult);
		WSACleanup();
		ExitThread(36);
	}
	log(TransformLogMessage(!isServer, DEBUG, "Listen succesfull"));

	sock = accept(lisen, 0, 0);
	if (sock == INVALID_SOCKET)
	{
		MessageBox(0, "Acception socket failed", "server error", 1l);
		closesocket(lisen);
		freeaddrinfo(addresult);
		WSACleanup();
		ExitThread(36);
	}
	log(TransformLogMessage(!isServer, DEBUG, "Acception succesfull"));

	closesocket(lisen);
	char rBuffer[RBUFFER_SIZE];
	while (!game.isOver)
	{
		ZeroMemory(&rBuffer, sizeof(rBuffer));
		_bool = recv(sock, rBuffer, RBUFFER_SIZE, 0);
		if (_bool > 0)
		{
			string t = "Message received: ";
			t += rBuffer;
			game.DecodeMessage(rBuffer);

			//MessageBox(0, t.c_str(), isServer ? "Server info" : "client info", 1);
			log(TransformLogMessage(isServer ? GLOBAL_SERVER : GLOBAL_CLIENT, DEBUG, t));
			game.AllSum >= 20 ? game.isOver = true : game.isOver = false;
			game.isOver ? EnableDisableButtons(globalHWnd, false) : EnableDisableButtons(globalHWnd, true);

			game.AllSum >= 20 ? game.doIWin ? TextOut(WindowDC, 0, 0, YouWin.c_str(), YouWin.size())
				: DrawTextA(WindowDC, YourenemyWin.c_str(), YourenemyWin.size(), &window, 1)
				: 0;
			Draw();


		}
	}
	_bool = shutdown(sock, SD_BOTH);
	if (_bool == SOCKET_ERROR)
	{
		MessageBox(0, "shutdown client socket failed", "connection", 1l);
		closesocket(sock);
		freeaddrinfo(addresult);
		WSACleanup();
		ExitThread(36);
	}
	closesocket(sock);
	freeaddrinfo(addresult);
	WSACleanup();

	ExitThread(36);
}
DWORD WINAPI Client(CONST LPVOID)
{
	//initializing socket
	//		0 - succes  1-failed
	_bool = WSAStartup(MAKEWORD(2, 2), &ws);
	if (_bool)
	{
		MessageBox(0, "Soket failed", "Soket failed", 1);
		ExitThread(69);

	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	_bool = getaddrinfo("localhost", "8080", &hints, &addresult);

	if (_bool)
	{
		MessageBox(0, "", "getaddr failed", 1);

		WSACleanup();
		freeaddrinfo(addresult);
		ExitThread(69);

	}

	client = socket(addresult->ai_family, addresult->ai_socktype, addresult->ai_protocol);
	if (client == INVALID_SOCKET)
	{
		MessageBox(0, "Socket creation failed", "Socket creation failed", 1l);
		WSACleanup();
		freeaddrinfo(addresult);

		ExitThread(69);
	}
	log(TransformLogMessage(!isServer, DEBUG, "Connection to server succesfull"));

	_bool = connect(client, addresult->ai_addr, static_cast<int>(addresult->ai_addrlen));
	if (_bool == SOCKET_ERROR)
	{
		MessageBox(0, "connection failed", "connection error", 1l);

		closesocket(client);
		client = INVALID_SOCKET;
		freeaddrinfo(addresult);
		WSACleanup();
		ExitThread(69);
	}
	log(TransformLogMessage(!isServer, DEBUG, "Connection succesfull"));
	char rBuffer[RBUFFER_SIZE];
	while (!game.isOver)
	{
		ZeroMemory(&rBuffer, sizeof(rBuffer));
		_bool = recv(client, rBuffer, RBUFFER_SIZE, 0);
		if (_bool > 0)
		{
			string t = "message received: ";
			t += rBuffer;
			game.DecodeMessage(rBuffer);
			//MessageBox(0, t.c_str(), isServer ? "Server info" : "client info", 1);
			log(TransformLogMessage(isServer ? GLOBAL_SERVER : GLOBAL_CLIENT, DEBUG, t));
			game.AllSum >= 20 ? game.isOver = true : game.isOver = false;
			game.isOver ? EnableDisableButtons(globalHWnd, false) : EnableDisableButtons(globalHWnd, true);
			game.AllSum >= 20 ? game.doIWin ? DrawTextA(WindowDC, YouWin.c_str(), YouWin.size(), &window, 1)
				: DrawTextA(WindowDC, YourenemyWin.c_str(), YourenemyWin.size(), &window, 1)
					: 0;
			Draw();
		}
	}

	closesocket(client);
	freeaddrinfo(addresult);
	WSACleanup();
	ExitThread(26);
}

void Game::DecodeMessage(const string& message)
// need to do [is your turn][AllSum(0 or 20)][do you win][num][isOver]
//            0             1->2             3           4    5
//6
{
	const char* a = message.c_str();
	
	isMyTurn = a[0] - '0';
	AllSum = ((a[1] - '0')*10) + (a[2] - '0');
	doIWin = a[3] - '0';
	
	arr.push_back(a[4] - '0');
	isOver = a[5] - '0';
	AllSum >= 20 ? isOver = true : doIWin = isOver = false;
	
	doIWin ? EnableDisableButtons(globalHWnd,false) 
		: EnableDisableButtons(globalHWnd, true);
}

string Game::EncodeMessage()
// need to do [is your turn][AllSum(0 or 20)][do you win][num][isOver]
//            0             1->2             3           4    5
//6
{
	string str /*= "hello"*/;
	EnableDisableButtons(globalHWnd, false);
	str.push_back(isMyTurn ? '1' : '0');
	str.push_back((AllSum/10)+'0');
	str.push_back((AllSum%10) + '0');
	str.push_back(/*doIWin ? '1':'0'*/'0');
	str.push_back(lastNum+'0');
	str.push_back(isOver ? '1' : '0');

	return str;
}
void Game::append(int num)
{
	arr.push_back(num);
	AllSum += num;
	AllSum >= 20 ? doIWin = true : 0;
	doIWin ? DrawTextA(WindowDC,YouWin.c_str(),YouWin.size(),&window,1) 
		: 0;

	lastNum = num;
}
void EnableDisableButtons(HWND hWnd, bool _b)
{
	EnableWindow(GetDlgItem(hWnd, ID_BUTTON1), _b);
	EnableWindow(GetDlgItem(hWnd, ID_BUTTON2), _b);
	EnableWindow(GetDlgItem(hWnd, ID_BUTTON3), _b);
	EnableWindow(GetDlgItem(hWnd, ID_CONFIRMCHOISE), _b);
}

void EnableDisableMenu(HWND hWnd, bool _b)
{
	EnableWindow(GetDlgItem(hWnd, ID_BECOME_HOST), _b);
	EnableWindow(GetDlgItem(hWnd, ID_CONNECTTO), _b);
}

void log(const string& output)
{
	string f = output + "\r\n";
	const string _path = (std::filesystem::current_path().generic_string() + "\\log.txt").c_str();
			
	HANDLE hf = CreateFile(
		_path.c_str(),
		FILE_APPEND_DATA,
		FILE_SHARE_WRITE,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);

	DWORD bytesWritten = 0;
	WriteFile(hf,
		f.c_str(),
		static_cast<DWORD>(f.size() - 1),
		&bytesWritten,
		0
	);

	if (bytesWritten == 0)
	{
		MessageBox(0, "Log hasn`t done", "Log error", 1);
	}
	CloseHandle(hf);
}
string TransformLogMessage(int c, int mode, const string& message)
{
	string str;
	c == GLOBAL_SERVER ? str = "[SERVER] " : str = "[CLIENT] ";
	mode == DEBUG ? str += "[DEBUG] " : str += "[ERROR] ";
	str += message;
	return str;
}
INT_PTR CALLBACK About(
	_In_ HWND   hWnd,
	_In_ UINT   message,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	switch (message)
	{
	case WM_INITDIALOG:

		break;
	case WM_COMMAND:
		if(wParam == IDOK)
			EndDialog(hWnd, 0);
		return FALSE;
		break;
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		
		break;
	default:
		break;
	}
	return FALSE;
}
INT_PTR CALLBACK Results(
	_In_ HWND   hWnd,
	_In_ UINT   message,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	switch (message)
	{
	case WM_INITDIALOG:

		break;
	case WM_COMMAND:
		if (wParam == IDOK)
			EndDialog(hWnd, 0);
		return FALSE;
		break;
	case WM_CLOSE:
		EndDialog(hWnd, 0);

		break;
	default:
		break;
	}
	return FALSE;
}
void Draw()
{
	using std::to_string;
	std::string str, strSum = "All sum: " + to_string(game.AllSum);
	
	for (const auto& val : game.arr)
		str += to_string(val) + ", ";
	PAINTSTRUCT ps;
	BeginPaint(globalHWnd, &ps);
	TextOut(WindowDC, (windowMaxX / 2) - 100, (windowMaxY / 2) - 100, strSum.c_str(), strSum.size());

	TextOut(WindowDC, (windowMaxX / 2) , (windowMaxY / 2) - 100, str.c_str(), str.size());
	EndPaint(globalHWnd, &ps);
}