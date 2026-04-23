#include "pch.h"
#include "Session.h"

int main()
{
    CoreGlobal::Init();

    SOCKET sock = SocketUtils::CreateSocket();

    // 서버 연결
    SOCKADDR_IN addr = NetAddress(L"127.0.0.1", 7777).GetSockAddr();
    ::connect(sock, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr));

    // PacketHeader + payload 전송
    struct TestPacket
    {
        PacketHeader header;
        char         msg[32] = "Hello Server!";
    };

    TestPacket packet;
    packet.header.size = sizeof(TestPacket);
    packet.header.type = 1;

    while (true)
    {
        ::send(sock, reinterpret_cast<char*>(&packet), sizeof(packet), 0);

        // 에코 수신
        char recvBuf[256] = {};
        int32 recvLen = ::recv(sock, recvBuf, sizeof(recvBuf), 0);
        if (recvLen > 0)
        {
            PacketHeader* header = reinterpret_cast<PacketHeader*>(recvBuf);
            std::cout << "Echo received — type: " << header->type
                << "  size: " << header->size << "\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    CoreGlobal::Clear();
	
}