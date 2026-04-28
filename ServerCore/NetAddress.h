#pragma once
class NetAddress
{
public:
	NetAddress() = default;
	NetAddress(SOCKADDR_IN address);
	NetAddress(string ip, uint16 port);

	const SOCKADDR_IN& GetSockAddr()  const { return _address; }
	string			   GetIpAddress() const;
	uint16			   GetPort()	  const { return ::ntohs(_address.sin_port); }

	static IN_ADDR Ip2Address(const char* ip);

private:
	SOCKADDR_IN _address = {};
};

