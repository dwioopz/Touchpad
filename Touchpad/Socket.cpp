#include "Socket.h"

namespace ts
{
	template < int N >
	void AddressToString(wchar_t (&buffer)[N], const Address & addr, bool port);

	bool ThrowSocketException(const char * fn, int err = WSAGetLastError());
	
	// Inisialisasi socket
	void InitSockets()
	{
		WSADATA wsa;
		int err = WSAStartup(0x0202, &wsa);
		if(err)
			throw socket_exception("WSAStartup", err);
	}

	void CloseSockets()
	{
		WSACleanup();
	}

	// Address
	Address::Address(short port) : size(sizeof(addr)) 
	{ 
		in.sin_family = AF_INET;
		in.sin_addr.s_addr = htonl(INADDR_ANY);
		in.sin_port = htons(port);
	}

	int * Address::RefSize() { size = sizeof(addr); return &size; }
	sockaddr * Address::RefSockAddr()  {  return &addr; }
	const sockaddr * Address::RefSockAddr() const { return &addr; }
	int Address::Size() const { return size; }
	
	std::wstring Address::ToString(bool port) const
	{
		wchar_t buffer[256];
		AddressToString(buffer, *this, port);
		return buffer;
	}

	Address Address::LocalHost(short port)
	{
		char name[256];
		if(gethostname(name, 256) == SOCKET_ERROR)
			throw socket_exception("gethostname");
		hostent* host = gethostbyname(name);    
		if(!host)
			throw socket_exception("gethostbyname");

		Address addr;
		memcpy(&addr.in.sin_addr, host->h_addr, host->h_length);
		addr.in.sin_port = htons(port);

		return addr;
	}


}
