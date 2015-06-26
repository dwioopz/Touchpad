#include "Server.h"

#include <vector>

using namespace ts;

// Map android keycode to VK.
UINT MapKeycode(ANDROID_KEYCODE keycode);

bool Server::IsRunning()
{
	return server.IsValid() && Thread::IsRunning();
}

bool Server::Run(short port, int password)
{
	Thread::Stop();	

	// Close sockets.
	client.Close();
	server.Close();
	for(int i = 0; i < 2; ++i)
		beacons[i].Close();
	
	try
	{
		// Listen for clients.
		server.Listen(port, 3, false);
		
		// Bind beacons.
		try { beacons[0].Bind(DefaultPort, false); } 
		catch(socket_exception & ex) { Log(OL_ERROR, L"%S", ex.what()); }
		if(port != DefaultPort)
		{
			try { beacons[1].Bind(port, false); } 
			catch(socket_exception & ex) { Log(OL_ERROR, L"%S", ex.what()); }
		}

		this->port = port;
		this->password = password;

		Thread::Run();

		std::wstring host = Address::LocalHost(port).ToString();
		Log(OL_NOTIFY | OL_STATUS | OL_INFO, L"Server running at %s\r\n", host.c_str());
		return true;
	}
	catch(socket_exception & ex)
	{
		Log(OL_ERROR, L"%S", ex.what());

		server.Close();

		Log(OL_NOTIFY | OL_STATUS | OL_ERROR, L"Error initializing server on port %i!\r\n", port);
		return false;
	}
}

// Mouse input helpers.
INPUT MouseMove(int dx, int dy)
{
	INPUT in = { 0 };
	in.type = INPUT_MOUSE;
	in.mi.dx = dx;
	in.mi.dy = dy;
	in.mi.dwFlags = MOUSEEVENTF_MOVE;
	return in;
}
INPUT MouseButtonDown(int button)
{
	INPUT in = { 0 };
	in.type = INPUT_MOUSE;
	switch(button)
	{
	case 0: in.mi.dwFlags = MOUSEEVENTF_LEFTDOWN; break;
	case 1:
	case 2: in.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN; break;
	}
	return in;
}
INPUT MouseButtonUp(int button)
{
	INPUT in = { 0 };
	in.type = INPUT_MOUSE;
	switch(button)
	{
	case 0: in.mi.dwFlags = MOUSEEVENTF_LEFTUP; break;
	case 1:
	case 2: in.mi.dwFlags = MOUSEEVENTF_RIGHTUP; break;
	}
	return in;
}
INPUT MouseWheel(int delta)
{
	INPUT in = { 0 };
	in.type = INPUT_MOUSE;
	in.mi.dwFlags = MOUSEEVENTF_WHEEL;
	in.mi.mouseData = delta;
	return in;
}
INPUT MouseHWheel(int delta)
{
	INPUT in = { 0 };
	in.type = INPUT_MOUSE;
	in.mi.dwFlags = MOUSEEVENTF_HWHEEL;
	in.mi.mouseData = delta;
	return in;
}

// Key input helpers.
INPUT KeyDown(WORD vk)
{
	INPUT in = { 0 };
	in.type = INPUT_KEYBOARD;
	in.ki.wVk = vk;
	in.ki.dwFlags = 0;
	return in;
}
INPUT KeyUp(WORD vk)
{
	INPUT in = { 0 };
	in.type = INPUT_KEYBOARD;
	in.ki.wVk = vk;
	in.ki.dwFlags = KEYEVENTF_KEYUP;
	return in;
}
INPUT CharDown(WORD ch)
{
	INPUT in = { 0 };
	in.type = INPUT_KEYBOARD;
	in.ki.wScan = ch;
	in.ki.dwFlags = KEYEVENTF_UNICODE;
	return in;
}
INPUT CharUp(WORD ch)
{
	INPUT in = { 0 };
	in.type = INPUT_KEYBOARD;
	in.ki.wScan = ch;
	in.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
	return in;
}


