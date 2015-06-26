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

void Server::HandlePackets()
{
	Packet p;
	if(client.Receive(&p, sizeof(p), 10) == sizeof(p))
	{		
		std::vector < INPUT > input;
		switch(p.Control)
		{
		case C_MOUSE_MOVE:		
			Log(OL_VERBOSE, L"MOUSE_MOVE %i %i\r\n", (int)p.Delta2D.dx, (int)p.Delta2D.dy);
			input.push_back(MouseMove(p.Delta2D.dx, p.Delta2D.dy));
			break;
		case C_MOUSE_BUTTONDOWN:
			Log(OL_VERBOSE, L"MOUSE_BUTTONDOWN %i\r\n", (int)p.Button);
			input.push_back(MouseButtonDown(p.Button));
			break;
		case C_MOUSE_BUTTONUP:
			Log(OL_VERBOSE, L"MOUSE_BUTTONUP %i\r\n", (int)p.Button);
			input.push_back(MouseButtonUp(p.Button));
			break;
		case C_MOUSE_SCROLL:
			Log(OL_VERBOSE, L"MOUSE_SCROLL %i\r\n", (int)p.Delta);
			input.push_back(MouseWheel(p.Delta));
			break;
		case C_MOUSE_SCROLL2:
			Log(OL_VERBOSE, L"MOUSE_SCROLL2 %i %i\r\n", (int)p.Delta2D.dx, (int)p.Delta2D.dy);
			if(p.Delta2D.dx != 0)
				input.push_back(MouseHWheel(p.Delta2D.dx));
			if(p.Delta2D.dy != 0)
				input.push_back(MouseWheel(p.Delta2D.dy));
			break;
		
		case C_CHAR:
			Log(OL_VERBOSE, L"CHAR %c\r\n", ntohs(p.Char));
			input.push_back(CharDown(ntohs(p.Char)));
			input.push_back(CharUp(ntohs(p.Char)));
			break;
		case C_KEYPRESS:	
			Log(OL_VERBOSE, L"KEYPRESS %i 0x%x\r\n", (int)ntohs(p.Key.keycode), (int)ntohs(p.Key.meta));
			input.push_back(KeyDown(MapKeycode((ANDROID_KEYCODE)ntohs(p.Key.keycode))));
			input.push_back(KeyUp(MapKeycode((ANDROID_KEYCODE)ntohs(p.Key.keycode))));
			break;
		case C_KEYDOWN:	
			Log(OL_VERBOSE, L"KEYDOWN %i 0x%x\r\n", (int)ntohs(p.Key.keycode), (int)ntohs(p.Key.meta));
			input.push_back(KeyDown(MapKeycode((ANDROID_KEYCODE)ntohs(p.Key.keycode))));
			break;
		case C_KEYUP:
			Log(OL_VERBOSE, L"KEYUP %i 0x%x\r\n", (int)ntohs(p.Key.keycode), (int)ntohs(p.Key.meta));
			input.push_back(KeyUp(MapKeycode((ANDROID_KEYCODE)ntohs(p.Key.keycode))));
			break;

		case C_NULL:
			Log(OL_VERBOSE, L"NULL %i\r\n", ntohl(p.Count));
			break;
		
		case C_DISCONNECT:
			Log(OL_VERBOSE, L"DISCONNECT\r\n");
			client.Close();
			Log(OL_NOTIFY | OL_INFO, L"Client disconnected\r\n");
			break;
		case C_SUSPEND:
			Log(OL_VERBOSE, L"SUSPEND\r\n");
			client.Close();
			Log(OL_INFO, L"Client suspended\r\n");
			break;
		default:
			Log(OL_VERBOSE, L"UNKNOWN\r\n");
			break;
		}
		if(!input.empty() && SendInput(input.size(), &input[0], sizeof(input[0])) != input.size())
			Log(OL_ERROR, L"SendInput Failed!\r\n");
	}
}


