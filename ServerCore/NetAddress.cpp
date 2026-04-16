#include "pch.h"
#include "NetAddress.h"

NetAddress::NetAddress(SOCKADDR_IN address) : _address(address)
{
}

NetAddress::NetAddress(wstring ip, uint16 port)
{
	::memset(&_address, 0, sizeof(_address));
	_address.sin_family = AF_INET;
	_address.sin_addr = Ip2Address(ip.c_str());
	_address.sin_port = ::htons(port);
}

wstring NetAddress::GetIpAddress()
{
	WCHAR buffer[100];
	::InetNtopW(AF_INET, &_address.sin_addr, buffer, len32(buffer));
	return wstring(buffer);
}

IN_ADDR NetAddress::Ip2Address(const WCHAR* ip)
{
	IN_ADDR address;
	::InetPtonW(AF_INET, ip, &address);
	return address;
}
