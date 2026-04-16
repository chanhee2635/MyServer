#pragma once
class NetAddress
{
public:
	NetAddress() = default;
	NetAddress(SOCKADDR_IN address);
	NetAddress(wstring ip, uint16 port);

	SOCKADDR_IN& GetSockAddr() { return _address; }
	wstring		 GetIpAddress();
	uint16		 GetPort() { return ::ntohs(_address.sin_port); }

	static IN_ADDR Ip2Address(const WCHAR* ip);

private:
	SOCKADDR_IN _address = {};
};

