#pragma once

struct RecvBufferStats
{
    // Linear 전용: CleanLinear() 에서 memmove 발생 횟수
    // 높으면 → RECV_BUFFER_COUNT 를 늘리거나 Circular 버퍼로 교체 고려
    std::atomic<uint64> memmoveCount = 0;

    // Circular 전용: wrap-around 된 데이터를 앞으로 정렬한 횟수
    // 높으면 → 패킷 처리 속도가 수신 속도를 못 따라가는 신호, OnRecv 로직 점검
    std::atomic<uint64> linearizeCount = 0;

    // 버퍼 여유 공간 부족으로 OnWrite 실패 → Disconnect 된 횟수
    // 0 이어야 정상. 발생 시 → RECV_BUFFER_SIZE 증가 또는 패킷 처리 속도 개선 필요
    std::atomic<uint64> bufferFullCount = 0;
};

struct NetworkStats
{
    // 초당 수신 바이트 (Report 에서 exchange(0) 으로 리셋)
    // 낮으면 → 클라이언트 송신 문제 또는 네트워크 병목
    std::atomic<uint64> recvBytes = 0;

    // 초당 송신 바이트 (Report 에서 exchange(0) 으로 리셋)
    // 낮으면 → Send 로직 미구현 또는 병목
    std::atomic<uint64> sendBytes = 0;

    // 초당 수신 패킷 수
    // recvBytes 대비 너무 낮으면 → 패킷이 크거나 분할 수신 중
    std::atomic<uint64> recvPackets = 0;

    // 초당 송신 패킷 수
    std::atomic<uint64> sendPackets = 0;
};

struct SessionStats
{
    // 현재 연결 중인 세션 수 (Connect +1 / Disconnect -1)
    // 예상 최대치를 초과하면 → 서버 수용량 검토 또는 로드밸런싱 필요
    std::atomic<int32>  currentActive = 0;

    // 서버 시작 이후 누적 접속 횟수 (리셋 안 함)
    std::atomic<uint64> totalConnected = 0;

    // 서버 시작 이후 누적 접속 종료 횟수 (리셋 안 함)
    // totalConnected - totalDisconnected == currentActive 여야 정상
    // 불일치 시 → Disconnect 누락 또는 이중 호출 버그 의심
    std::atomic<uint64> totalDisconnected = 0;
};

struct IocpStats
{
    // 초당 IOCP 이벤트 처리 횟수
    // 낮으면 → 이벤트가 적거나 처리 스레드 부족
    std::atomic<uint64> iocpCallCount = 0;

    // IOCP 이벤트 처리에 걸린 총 시간(us), iocpCallCount 로 나누면 평균
    // 평균이 높으면 → OnRecv / OnSend 처리 로직이 무거운 것, 분리 필요
    std::atomic<uint64> totalProcessTimeUs = 0;
};

struct MemoryStats
{
    // TLS 또는 글로벌 풀에서 할당 성공 횟수
    // poolHitCount / (poolHitCount + poolMissCount) 가 풀 효율
    // 낮으면 → 자주 사용하는 객체 크기가 MAX_POOL_SIZE 초과인지 점검
    std::atomic<uint64> poolHitCount = 0;

    // 요청 크기가 MAX_POOL_SIZE 초과 → 힙 직접 할당한 횟수
    // 높으면 → MAX_POOL_SIZE 상향 또는 해당 객체 크기 줄이기 고려
    std::atomic<uint64> poolMissCount = 0;

    // 글로벌 풀이 고갈되어 OS 에서 새 블록을 배치 할당한 횟수
    // 높으면 → ALLOC_COUNT 를 늘려 배치 크기 확대, 또는 풀 워밍업 고려
    std::atomic<uint64> allocBatchCount = 0;

    // TLS 캐시가 비어 글로벌 풀에서 배치로 가져온 횟수
    // 높으면 → TLS_MAX_COUNT 또는 TLS_BATCH_COUNT 증가 고려
    std::atomic<uint64> fetchFromGlobalCount = 0;

    // TLS 캐시가 가득 차 글로벌 풀로 반환한 횟수
    // 높으면 → TLS_MAX_COUNT 증가 고려
    std::atomic<uint64> returnToGlobalCount = 0;

    // 현재 살아있는 할당 수 (Allocate +1 / Release -1, 리셋 안 함)
    // 서버 정상 종료 후에도 0 이 아니면 → 메모리 누수 버그 존재
    // 음수가 되면 → Release 이중 호출 또는 외부 메모리를 Release 에 넘긴 것
    std::atomic<int64>  liveAllocCount = 0;
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

    void Report();
};