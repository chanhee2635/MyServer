#include "pch.h"
#include "NetAddress.h"

NetAddress::NetAddress(SOCKADDR_IN address) : _address(address)
{
}

NetAddress::NetAddress(string ip, uint16 port)
{
	::memset(&_address, 0, sizeof(_address));
	_address.sin_family = AF_INET;
	_address.sin_addr	= Ip2Address(ip.c_str());
	_address.sin_port	= ::htons(port);
}

std::string NetAddress::GetIpAddress() const
{
	char buffer[46];
	::InetNtopA(AF_INET, &_address.sin_addr, buffer, sizeof(buffer));
	return buffer;
}

IN_ADDR NetAddress::Ip2Address(const char* ip)
{
	IN_ADDR address;
	::InetPtonA(AF_INET, ip, &address);
	return address;
}
