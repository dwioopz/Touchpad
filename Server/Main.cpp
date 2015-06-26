#include "Server.h"

#include "resource.h"

using namespace ts;
#include "Server.h"

#include "resource.h"

using namespace ts;

// konstanta
const wchar_t * RegistryKey = L"Software\\Things & Stuff\\Touchpad Server";
const OUTPUT_LEVEL OutputLevel = OL_INFO;

// notif
NOTIFYICONDATA Notify;
// Port.
int Port = DefaultPort;

// Password hash
int Password = 0;

// Server thread.
Server server;

// Log & message
HWND LogWnd = NULL;
HWND StatusWnd = NULL;

// Compute hash of string.
int Hash(const wchar_t * str);

