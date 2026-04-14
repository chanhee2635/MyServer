🚀 C++ High-Performance Game Server
IOCP 기반의 고성능/확장성 게임 서버 엔진 개발 프로젝트

📌 Project Overview
본 프로젝트는 현대적인 C++ 표준과 Win32 API(IOCP)를 활용하여, 수만 명의 동시 접속자를 안정적으로 처리할 수 있는 **게임 서버 엔진(ServerCore)**과 이를 활용한 **컨텐츠 서버(GameServer)**를 구축하는 것을 목표로 합니다.

✨ Key Features
1. ⚡ Optimized Build System
Precompiled Header (PCH): CorePch.h를 통해 STL 및 자주 변하지 않는 외부 라이브러리를 미리 컴파일하여 빌드 속도를 최대 80% 단축.

Modern Type Aliasing: uint32, int64 등 명확한 고정 크기 정수형 정의로 유지보수성 향상.

2. 🧵 Multi-threading & Synchronization
jthread 기반 Thread Pool: C++20의 std::jthread를 활용해 자원 관리가 안전하고 자동화된 스레드 풀 구현.

RWLock Strategy: std::shared_mutex 기반의 Read-Write Lock을 도입하여 읽기 작업이 많은 게임 환경에서 컨텐션(Contention) 최소화.

3. 🛡️ Robustness & Stability
CrashDump System: 예기치 못한 종료 시 MiniDump를 자동 생성하여 사후 분석(Post-mortem)이 가능한 환경 구축.

Custom Exception Handler: 서버 크래시 발생 시 콜스택(Call Stack) 기록 및 사건 현장 보존.

📂 Project Structure
Plaintext
/MyServerProject
├── .gitignore
├── README.md
├── MyServer.sln
├── /ServerCore          # 고성능 서버 엔진 라이브러리
│   ├── /include         # 공용 헤더 및 타입 정의
│   ├── /src             # IOCP, 스레드 풀, 락 관리 핵심 로직
│   └── ServerCore.vcxproj
└── /GameServer          # 서버 엔진을 활용한 실 프로젝트
    ├── main.cpp         # 진입점 및 컨텐츠 로직
    └── GameServer.vcxproj

🛠️ Tech Stack
Language: C++20

API: Win32 API (IOCP)

Library: Standard Template Library (STL)

Tools: Visual Studio 2022, Git

👤 Author
Name: 김찬희

Email: cksgml0156@naver.com

GitHub: @chanhee2635