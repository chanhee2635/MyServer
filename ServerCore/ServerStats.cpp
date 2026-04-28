#include "pch.h"
#include "ServerStats.h"

void ServerStats::Report()
{
    auto& rb = recvBuffer;
    auto& net = network;
    auto& sess = session;
    auto& io = iocp;
    auto& mem = memory;

    uint64 iocpCalls = io.iocpCallCount.exchange(0);
    uint64 totalTime = io.totalProcessTimeUs.exchange(0);
    uint64 avgUs = (iocpCalls > 0) ? (totalTime / iocpCalls) : 0;

    std::cout
        << "=== Server Stats ===\n"
        << "[Session]  active=" << sess.currentActive
        << "  connected=" << sess.totalConnected
        << "  disconnected=" << sess.totalDisconnected << "\n"

        << "[Network]  recv=" << net.recvBytes.exchange(0) / 1024 << "KB/s"
        << "  send=" << net.sendBytes.exchange(0) / 1024 << "KB/s"
        << "  recvPkt=" << net.recvPackets.exchange(0) << "/s"
        << "  recvBatch=" << net.recvBatchCount.exchange(0) << "/s"
        << "  sendPkt=" << net.sendPackets.exchange(0) << "/s\n"

        << "[RecvBuf]  memmove=" << rb.memmoveCount.exchange(0)
        << "  linearize=" << rb.linearizeCount.exchange(0)
        << "  bufFull=" << rb.bufferFullCount.exchange(0) << "\n"

        << "[IOCP]     calls=" << iocpCalls
        << "/s  avgProcess=" << avgUs << "us\n"

        << "[Memory]"
        << "  hit=" << mem.poolHitCount.exchange(0)
        << "  miss=" << mem.poolMissCount.exchange(0)        
        << "  batch=" << mem.allocBatchCount.exchange(0)     
        << "  fetch=" << mem.fetchFromGlobalCount.exchange(0) 
        << "  return=" << mem.returnToGlobalCount.exchange(0)
        << "  live=" << mem.liveAllocCount << "\n"            
        << "====================\n";
}