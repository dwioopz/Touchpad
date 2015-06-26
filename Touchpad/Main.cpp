#include "Server.h"

#include "resource.h"

using namespace ts;

// Constants.
const wchar_t * RegistryKey = L"Software\\Things & Stuff\\Touchpad Server";
const OUTPUT_LEVEL OutputLevel = OL_INFO;

// Notification icon.
NOTIFYICONDATA Notify;
// Port.
int Port = DefaultPort;
// Password hash.
int Password = 0;

// Server thread.
Server server;

// Log and message control
HWND LogWnd = NULL;
HWND StatusWnd = NULL;

// Compute hash of string.
int Hash(const wchar_t * str);

// Load preferences from globals.
void LoadPreferences(HWND hWnd)
{
	HKEY key;	
	if(RegCreateKeyEx(HKEY_CURRENT_USER, RegistryKey, 0, NULL, 0, KEY_READ, NULL, &key, NULL) == ERROR_SUCCESS)
	{
		DWORD dwType, dwSize;

		dwType = REG_DWORD;
		dwSize = sizeof(DWORD);
		RegQueryValueEx(key, L"Port", NULL, &dwType, (BYTE *)&Port, &dwSize);

		dwType = REG_DWORD;
		dwSize = sizeof(DWORD);
		RegQueryValueEx(key, L"Password", NULL, &dwType, (BYTE *)&Password, &dwSize);

		RegCloseKey(key);
	}

	SetDlgItemInt(hWnd, IDC_PORT, Port, FALSE);
	if(Password != 0)
	{
		SetDlgItemText(hWnd, IDC_PASSWORD1, L"*****");
		SetDlgItemText(hWnd, IDC_PASSWORD2, L"*****");
	}
	else
	{
		SetDlgItemText(hWnd, IDC_PASSWORD1, L"");
		SetDlgItemText(hWnd, IDC_PASSWORD2, L"");
	}
}

// Validate and save preferences to globals.
bool SavePreferences(HWND hWnd)
{
	wchar_t password1[256] = { 0 };
	wchar_t password2[256] = { 0 };

	// Validation.
	GetDlgItemText(hWnd, IDC_PASSWORD1, password1, sizeof(password1) / sizeof(password1[0]));
	GetDlgItemText(hWnd, IDC_PASSWORD2, password2, sizeof(password2) / sizeof(password2[0]));

	if(wcscmp(password1, password2) != 0)
	{
		MessageBox(hWnd, L"Password does not match. Please re-enter your password.", L"Error", MB_OK | MB_ICONERROR);
		SetFocus(GetDlgItem(hWnd, IDC_PASSWORD1));
		return false;
	}
	int password = Password;
	if(wcscmp(password1, L"") == 0)
		password = 0;
	else if(wcscmp(password1, L"*****") != 0)
		password = Hash(password1);

	int port = GetDlgItemInt(hWnd, IDC_PORT, NULL, FALSE);
	if(port <= 0 || port > 65535)
	{
		MessageBox(hWnd, L"Port must be a number between 1 and 65535.", L"Error", MB_OK | MB_ICONERROR);
		SetFocus(GetDlgItem(hWnd, IDC_PORT));
		return false;
	}

	// Validation passed, set options.
	bool init = !server.IsRunning();
	if(port != Port)
		init = true;
	if(password != Password)
		init = true;

	if(init && !server.Run(port, password))
	{
		MessageBox(hWnd, L"Error starting server!", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	Port = port;
	Password = password;

	// Write preferences to the registry
	HKEY key;
	if(RegCreateKeyEx(HKEY_CURRENT_USER, RegistryKey, 0, NULL, 0, KEY_WRITE, NULL, &key, NULL) == ERROR_SUCCESS)
	{
		DWORD dwType, dwSize;

		dwType = REG_DWORD;
		dwSize = sizeof(DWORD);
		RegSetValueEx(key, L"Port", 0, dwType, (BYTE *)&Port, dwSize);
		
		dwType = REG_DWORD;
		dwSize = sizeof(DWORD);
		RegSetValueEx(key, L"Password", 0, dwType, (BYTE *)&Password, dwSize);

		RegCloseKey(key);
	}

	return true;
}

// Show log and resize window based on check box.
void ShowLog(HWND hWnd)
{
	bool show = IsDlgButtonChecked(hWnd, IDC_SHOWLOG) == BST_CHECKED;
	ShowWindow(LogWnd, show ? SW_SHOW : SW_HIDE);

	RECT rcLog;
	GetWindowRect(LogWnd, &rcLog);
	ScreenToClient(hWnd, (POINT *)&rcLog + 0);
	ScreenToClient(hWnd, (POINT *)&rcLog + 1);

	RECT rcWnd, rcClient;
	GetWindowRect(hWnd, &rcWnd);

	GetClientRect(hWnd, &rcClient);
	ClientToScreen(hWnd, (POINT *)&rcClient + 0);
	ClientToScreen(hWnd, (POINT *)&rcClient + 1);

	MoveWindow(
		hWnd, 
		rcWnd.left, 
		rcWnd.top, 
		rcWnd.right - rcWnd.left, 
		(show ? rcLog.bottom + 12 : rcLog.top) + rcClient.top - rcWnd.top,
		TRUE);
}