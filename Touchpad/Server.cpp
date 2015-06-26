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

void Server::AcceptClients()
{
	TcpSocket c;
	Address from;
	if(c.Accept(server, from, false))
	{
		std::wstring name = c.GetPeer().ToString(false);

		// Check password.
		Packet p;
		if(c.Receive(&p, sizeof(p), 1000) == sizeof(p) && (p.Control == C_CONNECT || p.Control == C_RESUME))
		{
			if(password == 0 || password == ntohl(p.Password))
			{
				// Check if a client is being booted by the new client.
				if(client.IsValid())
				{
					std::wstring name = client.GetPeer().ToString(false);

					client.Close();
					if(p.Control != C_RESUME)
						Log(OL_INFO, L"Replacing client %s\r\n", name.c_str());
				}

				if(p.Control == C_CONNECT)
					Log(OL_NOTIFY | OL_INFO, L"Client connected from %s\r\n", name.c_str());
				else
					Log(OL_INFO, L"Client resumed\r\n");
				p.Control = C_CONNECT;
				c.Send(&p, sizeof(p));

				client.Take(c);
			}
			else
			{
				p.Control = C_DISCONNECT;
				p.Reason = 1;
				Log(OL_INFO, L"Rejected client %s: Bad password\r\n", name.c_str());
			}
		}
		else
		{
			p.Control = C_DISCONNECT;
			p.Reason = 0;
			Log(OL_INFO, L"Client failed to connect from %s\r\n", name.c_str());
		}
		if(p.Control == C_DISCONNECT)
		{
			c.Send(&p, sizeof(p));
			c.Close();
		}
	}
}

void Server::CheckBeacon(int i)
{
	Address from;
	Packet p;
	while(beacons[i].ReceiveFrom(&p, sizeof(p), from) == sizeof(p))
	{
		if(p.Control == C_PING)
		{
			// Reply with port the server is running on.
			p.Control = C_ACK;
			p.Port = htons(port);
			beacons[i].SendTo(&p, sizeof(p), from);

			std::wstring name = from.ToString(false);
			Log(OL_INFO, L"Responded to broadcast from %s\r\n", name.c_str());
		}
	}
}

void Server::Main(const volatile bool & run)
{
	while(run)
	{
		// Respond to client.
		if(client.IsValid())
		{
			try
			{
				HandlePackets();
			}
			catch(socket_exception & ex)
			{
				Log(OL_ERROR, L"%S", ex.what());
				client.Close();
			}
		}
		else
		{
			Sleep(10);
		}

		// Maybe accept client.
		if(server.IsValid())
		{
			try
			{
				AcceptClients();
			}
			catch(socket_exception & ex)
			{
				Log(OL_ERROR, L"%S", ex.what());
			}
		}

		// Check for broadcasts looking for the server.
		for(int i = 0; i < 2; ++i)
		{
			if(beacons[i].IsValid())
			{
				try
				{
					CheckBeacon(i);
				}
				catch(socket_exception & ex)
				{
					Log(OL_ERROR, L"%S", ex.what());
				}
			}
		}
	}
}

// Menerjemahkan key android ke virtual
UINT MapKeycode(ANDROID_KEYCODE keycode)
{
	switch(keycode)
	{
	case KEYCODE_A: return 'A';
	case KEYCODE_B: return 'B';
	case KEYCODE_C: return 'C';
	case KEYCODE_D: return 'D';
	case KEYCODE_E: return 'E';
	case KEYCODE_F: return 'F';
	case KEYCODE_G: return 'G';
	case KEYCODE_H: return 'H';
	case KEYCODE_I: return 'I';
	case KEYCODE_J: return 'J';
	case KEYCODE_K: return 'K';
	case KEYCODE_L: return 'L';
	case KEYCODE_M: return 'M';
	case KEYCODE_N: return 'N';
	case KEYCODE_O: return 'O';
	case KEYCODE_P: return 'P';
	case KEYCODE_Q: return 'Q';
	case KEYCODE_R: return 'R';
	case KEYCODE_S: return 'S';
	case KEYCODE_T: return 'T';
	case KEYCODE_U: return 'U';
	case KEYCODE_V: return 'V';
	case KEYCODE_W: return 'W';
	case KEYCODE_X: return 'X';
	case KEYCODE_Y: return 'Y';
	case KEYCODE_Z: return 'Z';

	case KEYCODE_0: return '0';
	case KEYCODE_1: return '1';
	case KEYCODE_2: return '2';
	case KEYCODE_3: return '3';
	case KEYCODE_4: return '4';
	case KEYCODE_5: return '5';
	case KEYCODE_6: return '6';
	case KEYCODE_7: return '7';
	case KEYCODE_8: return '8';
	case KEYCODE_9: return '9';

	case KEYCODE_F1: return VK_F1;
	case KEYCODE_F2: return VK_F2;
	case KEYCODE_F3: return VK_F3;
	case KEYCODE_F4: return VK_F4;
	case KEYCODE_F5: return VK_F5;
	case KEYCODE_F6: return VK_F6;
	case KEYCODE_F7: return VK_F7;
	case KEYCODE_F8: return VK_F8;
	case KEYCODE_F9: return VK_F9;
	case KEYCODE_F10: return VK_F10;
	case KEYCODE_F11: return VK_F11;
	case KEYCODE_F12: return VK_F12;

	case KEYCODE_SHIFT_LEFT: return VK_LSHIFT;
	case KEYCODE_SHIFT_RIGHT: return VK_RSHIFT;
	case KEYCODE_CTRL_LEFT: return VK_LCONTROL;
	case KEYCODE_CTRL_RIGHT: return VK_RCONTROL;
	case KEYCODE_ALT_LEFT: return VK_LMENU;
	case KEYCODE_ALT_RIGHT: return VK_RMENU;
	case KEYCODE_MENU: return VK_MENU;
		
	//case KEYCODE_DPAD_CENTER: return VK_DPAD_CENTER;
	case KEYCODE_DPAD_DOWN: return VK_DOWN;
	case KEYCODE_DPAD_LEFT: return VK_LEFT;
	case KEYCODE_DPAD_RIGHT: return VK_RIGHT;
	case KEYCODE_DPAD_UP: return VK_UP;
		
	case KEYCODE_VOLUME_DOWN: return VK_VOLUME_DOWN;
	case KEYCODE_VOLUME_MUTE: return VK_VOLUME_MUTE;
	case KEYCODE_VOLUME_UP: return VK_VOLUME_UP;
	
	case KEYCODE_SEARCH: return VK_BROWSER_SEARCH;	
	case KEYCODE_HOME: return VK_BROWSER_HOME;
	case KEYCODE_BACK: return VK_BROWSER_BACK;
	case KEYCODE_FORWARD: return VK_BROWSER_FORWARD;
		
	case KEYCODE_INSERT: return VK_INSERT;
	case KEYCODE_MOVE_END: return VK_END;
	case KEYCODE_MOVE_HOME: return VK_HOME;
	case KEYCODE_PAGE_DOWN: return VK_NEXT;
	case KEYCODE_PAGE_UP: return VK_PRIOR;

	case KEYCODE_DEL: return VK_BACK;
	case KEYCODE_FORWARD_DEL: return VK_DELETE;
	case KEYCODE_CLEAR: return VK_CLEAR;

	case KEYCODE_CAPS_LOCK: return VK_CAPITAL;
				
	case KEYCODE_TAB: return VK_TAB;
	case KEYCODE_SPACE: return VK_SPACE;
	case KEYCODE_ENTER: return VK_RETURN;
		
	case KEYCODE_ESCAPE: return VK_ESCAPE;
				
	case KEYCODE_SCROLL_LOCK: return VK_SCROLL;
	case KEYCODE_NUM_LOCK: return VK_NUMLOCK;

	//case KEYCODE_MEDIA_CLOSE: return VK_MEDIA_CLOSE;
	//case KEYCODE_MEDIA_EJECT: return VK_MEDIA_EJECT;
	//case KEYCODE_MEDIA_FAST_FORWARD: return VK_MEDIA_FAST_FORWARD;
	case KEYCODE_MEDIA_NEXT: return VK_MEDIA_NEXT_TRACK;
	//case KEYCODE_MEDIA_PAUSE: return VK_MEDIA_PAUSE;
	//case KEYCODE_MEDIA_PLAY: return VK_MEDIA_PLAY;
	case KEYCODE_MEDIA_PLAY_PAUSE: return VK_MEDIA_PLAY_PAUSE;
	case KEYCODE_MEDIA_PREVIOUS: return VK_MEDIA_PREV_TRACK;
	//case KEYCODE_MEDIA_RECORD: return VK_MEDIA_RECORD;
	//case KEYCODE_MEDIA_REWIND: return VK_MEDIA_REWIND;
	case KEYCODE_MEDIA_STOP: return VK_MEDIA_STOP;
	case KEYCODE_MUTE: return VK_VOLUME_MUTE;

	//case KEYCODE_SOFT_LEFT: return VK_LEFT;
	//case KEYCODE_SOFT_RIGHT: return VK_RIGHT;
	//case KEYCODE_ZOOM_IN: return VK_ZOOM;
	//case KEYCODE_ZOOM_OUT: return VK_ZOOM_OUT;

	default: return 0;
	}
}


