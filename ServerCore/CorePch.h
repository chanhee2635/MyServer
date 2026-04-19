#pragma once

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <intrin.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

#include <iostream>
#include <string>
#include <mutex>
#include <atomic>

#include "Types.h"
#include "Utils.h"
#include "Config.h"
#include "CoreMacro.h"
#include "SocketUtils.h"
#include "CoreGlobal.h"
#include "Container.h"