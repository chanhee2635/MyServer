#pragma once

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <intrin.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

#include <iostream>
#include <vector>
#include <string>
#include <mutex>
#include <atomic>

#include "Types.h"
#include "CoreMacro.h"
#include "SocketUtils.h"
#include "CoreGlobal.h"