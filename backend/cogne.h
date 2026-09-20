#pragma once



/*
    TODO

        - Make the platform layer distinct from the application layer, Casey style
        - Create struct to represent a simple API's endpoint message
        - Add VERY BASIC UI to test getter user-specified data
        - Use Innosetup to pack and release the app and test it on second laptop
        - Send to Lucas for further testing
        - Grow it
        - $$$
*/

#include <stdint.h>

typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;
typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef float    real32;
typedef double   real64;
typedef int32    bool32;

typedef uint32   flag_type;
typedef uint8    enum_type;
constexpr uint8 FLAGS_COUNT = 32;

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

namespace utils
{
    namespace dex
    {
        struct TokenPairs;
    }
}

struct Dex;


namespace wnd
{
    struct Window
    {
        HWND        handle;
        const char* name;
        //HANDLE      start_node_process;
        //HANDLE      close_node_process;
    };
    void create_window(HINSTANCE hinstance, Window* out_wnd, gui::DearGUI* gui);
    bool process_start_node(Window* window);
    bool process_stop_node(Window* window);
}

struct AppState
{
    net::Network* network = nullptr;
    wnd::Window* window = nullptr;

    utils::dex::TokenPairs* tokenPairs = nullptr;

    Dex* dex = nullptr;
};