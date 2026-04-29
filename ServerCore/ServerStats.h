#pragma once

struct RecvBufferStats
{
    std::atomic<uint64> memmoveCount = 0;
    std::atomic<uint64> linearizeCount = 0;
    std::atomic<uint64> bufferFullCount = 0;
};

struct NetworkStats
{
    std::atomic<uint64> recvBytes = 0;
    std::atomic<uint64> sendBytes = 0;
    std::atomic<uint64> recvPackets = 0;
    std::atomic<uint64> recvBatchCount = 0;
    std::atomic<uint64> sendPackets = 0;
};

struct SessionStats
{
    std::atomic<int32>  currentActive = 0;
    std::atomic<uint64> totalConnected = 0;
    std::atomic<uint64> totalDisconnected = 0;
};

struct IocpStats
{
    std::atomic<uint64> iocpCallCount = 0;
    std::atomic<uint64> totalProcessTimeUs = 0;
};

struct MemoryStats
{
    std::atomic<uint64> poolHitCount = 0;
    std::atomic<uint64> poolMissCount = 0;
    std::atomic<uint64> allocBatchCount = 0;
    std::atomic<uint64> fetchFromGlobalCount = 0;
    std::atomic<uint64> returnToGlobalCount = 0;
    std::atomic<int64>  liveAllocCount = 0;
};

struct JobStats
{
    std::atomic<uint64> jobsQueued = 0; 
    std::atomic<uint64> jobsExecuted = 0; 
    std::atomic<uint64> timeSlices = 0; 
    std::atomic<uint64> timerFired = 0; 
};

class ServerStats
{
public:
    static ServerStats& Get() { static ServerStats instance; return instance; }

    RecvBufferStats recvBuffer;
    NetworkStats    network;
    SessionStats    session;
    IocpStats       iocp;
    MemoryStats     memory;
    JobStats        job;

    void Report();
};
