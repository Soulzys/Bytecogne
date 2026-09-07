#include <stdint.h>

typedef int8_t   int8   ;
typedef int16_t  int16  ;
typedef int32_t  int32  ;
typedef int64_t  int64  ;
typedef uint8_t  uint8  ;
typedef uint16_t uint16 ;
typedef uint32_t uint32 ;
typedef uint64_t uint64 ;
typedef float    real32 ;
typedef double   real64 ;
typedef int32    bool32 ;

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <iostream>
#include <thread>
#include <cstring>

#include "utils.cpp"

#include "d3d11.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"


namespace net
{
    struct Network;
}

namespace wnd
{
    struct Window
    {
        HWND        handle;
        const char* name;
    };
    void create_window(HINSTANCE hinstance, Window* out_wnd);
}

struct AppState
{
    net::Network* network = nullptr;
};