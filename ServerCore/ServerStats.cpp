#include "pch.h"
#include "ServerStats.h"
#include "Logger.h"

// Stats 패널 라인 수 (헤더+데이터+푸터)
static constexpr int STATS_PANEL_LINES = 9;

void ServerStats::Init()
{
    Logger::ReserveStatsPanel(STATS_PANEL_LINES);
}

void ServerStats::Report()
{
    auto& rb   = recvBuffer;
    auto& net  = network;
    auto& sess = session;
    auto& io   = iocp;
    auto& mem  = memory;

    uint64 iocpCalls = io.iocpCallCount.exchange(0);
    uint64 totalTime = io.totalProcessTimeUs.exchange(0);
    uint64 avgUs     = (iocpCalls > 0) ? (totalTime / iocpCalls) : 0;

    // Logger와 같은 락으로 보호 → 커서 이동 중 로그 끼어들기 방지
    std::lock_guard<std::mutex> guard(Logger::GetConsoleLock());

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // 현재 커서 위치 저장
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);
    COORD logPos = csbi.dwCursorPosition;

    // Stats 패널 상단으로 이동
    SetConsoleCursorPosition(hOut, { 0, 0 });

    // \033[K : 줄 끝까지 지우기 (이전 Stats 잔상 제거)
    std::cout
        << "=== Server Stats ===              \033[K\n"
        << "[Session]  active=" << sess.currentActive
        << "  connected="    << sess.totalConnected
        << "  disconnected=" << sess.totalDisconnected << "\033[K\n"

        << "[Network]  recv="  << net.recvBytes.exchange(0) / 1024 << "KB/s"
        << "  send="           << net.sendBytes.exchange(0) / 1024 << "KB/s"
        << "  recvPkt="        << net.recvPackets.exchange(0) << "/s"
        << "  recvBatch="      << net.recvBatchCount.exchange(0) << "/s"
        << "  sendPkt="        << net.sendPackets.exchange(0) << "/s\033[K\n"

        << "[RecvBuf]  memmove="   << rb.memmoveCount.exchange(0)
        << "  linearize="          << rb.linearizeCount.exchange(0)
        << "  bufFull="            << rb.bufferFullCount.exchange(0) << "\033[K\n"

        << "[IOCP]     calls="     << iocpCalls
        << "/s  avgProcess="       << avgUs << "us\033[K\n"

        << "[Memory]   hit="    << mem.poolHitCount.exchange(0)
        << "  miss="            << mem.poolMissCount.exchange(0)
        << "  batch="           << mem.allocBatchCount.exchange(0)
        << "  fetch="           << mem.fetchFromGlobalCount.exchange(0)
        << "  return="          << mem.returnToGlobalCount.exchange(0)
        << "  live="            << mem.liveAllocCount << "\033[K\n"

        << "[Job]      queued=" << job.jobsQueued.exchange(0)
        << "/s  exec="          << job.jobsExecuted.exchange(0)
        << "/s  slice="         << job.timeSlices.exchange(0)
        << "/s  timer="         << job.timerFired.exchange(0) << "/s\033[K\n"

        << "====================\033[K\n";

    std::cout.flush();

    // 로그 영역으로 커서 복원 (Stats 패널 아래로 제한)
    if (logPos.Y < STATS_PANEL_LINES)
        logPos.Y = STATS_PANEL_LINES;
    SetConsoleCursorPosition(hOut, logPos);
}