#pragma once
class NetAddress
{
public:
	NetAddress() = default;
	NetAddress(SOCKADDR_IN address);
	NetAddress(wstring ip, uint16 port);

	const SOCKADDR_IN& GetSockAddr() const { return _address; }
	wstring			   GetIpAddress() const;
	uint16			   GetPort() const { return ::ntohs(_address.sin_port); }

	static IN_ADDR Ip2Address(const WCHAR* ip);

private:
	SOCKADDR_IN _address = {};
};

