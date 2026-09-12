#pragma once

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


namespace net
{
    struct Network;
}

namespace gui
{
    struct DearGUI;
}


namespace wnd
{
    struct Window
    {
        HWND        handle;
        const char* name;
    };
    void create_window(HINSTANCE hinstance, Window* out_wnd, gui::DearGUI* gui);
    void create_process();
}

struct AppState
{
    net::Network* network = nullptr;
};