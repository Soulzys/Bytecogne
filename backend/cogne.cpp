#define WIN32_LEAN_AND_MEAN

#include "d3d11.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"

#include "cogne.h"
#include "utils.cpp"
#include "gui.cpp"
#include "networking.cpp"



LRESULT WINAPI main_window_callback(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
{
    // Setup our DearGUI ptr upon initializing the window so we can use it below
    if (message == WM_NCCREATE)
    {
        CREATESTRUCT* cs = (CREATESTRUCT*)lparam;
        gui::DearGUI* gui = (gui::DearGUI*)cs->lpCreateParams;
        SetWindowLongPtr(handle, GWLP_USERDATA, (LONG_PTR)gui);
    }


    if (ImGui_ImplWin32_WndProcHandler(handle, message, wparam, lparam))
    {
        return true;
    }

    switch (message)
    {
        case WM_SIZE:
        {
            gui::DearGUI* gui = (gui::DearGUI*)(GetWindowLongPtr(handle, GWLP_USERDATA));
            if (gui && gui->device && wparam != SIZE_MINIMIZED)
            {
                gui::destroy_render_target(gui);
                gui->swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
                gui::create_render_target(gui);
            }

            return 0;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(handle, message, wparam, lparam);
}


void wnd::create_window(HINSTANCE hinstance, Window* out_wnd, gui::DearGUI* gui)
{
    WNDCLASS wc      = {};
    wc.style         = CS_CLASSDC;
    wc.lpfnWndProc   = main_window_callback;
    wc.hInstance     = hinstance;
    wc.lpszClassName = "Cogne";

    RegisterClass(&wc);

    HWND handle = CreateWindowExA
    (
        0,
        wc.lpszClassName, 
        "Cogne (feat Dear ImGui & DirectX 11)",
        WS_OVERLAPPEDWINDOW,
        100, 100,
        1280, 720,
        nullptr,
        nullptr,
        hinstance,
        gui
    );

    out_wnd->handle = handle;
    out_wnd->name   = wc.lpszClassName;
}

void wnd::create_process()
{
    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);
    char command[] = "node C:\\Bytecogne\\src\\index.js";

    BOOL success = CreateProcessA
    (
        nullptr,
        command, 
        nullptr,
        nullptr,
        false,
        0, 
        nullptr,
        nullptr,
        &si,
        &pi
    );

    if (!success)
    {
        std::cerr << "Failed to start Node.js. Error: " << GetLastError() << "\n";
    }
}




int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, LPSTR, int)
{
    gui::DearGUI gui = {};

    // Create Win32 window
    //
    wnd::Window wnd = {};
    wnd::create_window(hinstance, &wnd, &gui);


    // Create D3D11
    //
    if (!gui::create_device_D3D(&gui, wnd.handle))
    {
        gui::cleanup_device_D3D(&gui);
        UnregisterClass(wnd.name, hinstance);

        return 1;
    }

    ShowWindow(wnd.handle, SW_SHOWDEFAULT);
    UpdateWindow(wnd.handle);


    // Initialize Dear ImGui
    //
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    gui.io = &ImGui::GetIO();
    (void)gui.io;

    ImGui::StyleColorsDark();
    
    ImGui_ImplWin32_Init(wnd.handle);
    ImGui_ImplDX11_Init(gui.device, gui.context);

    
    //
    // NETWORK CODE
    //
    net::Network network = {};
    if (!net::init(&network, 5000))
    {
        ::WSACleanup();
        return 1;
    }

    net::start(&network);


    AppState app = {};
    app.network = &network;


    bool running = true;
    while (running)
    {
        // Window messages
        //
        MSG message;
        while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);

            if (message.message == WM_QUIT)
            {
                running = false;
            }
        }

        if (!running) 
            break;


        // Network messages
        //
        net::process(&network);


        // Start ImGui frame
        //
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();


        // GUI
        //
        gui::draw_ui(&app);


        // Render
        //
        const real32 clear_color[] =
        {
            18.f / 255.f,
            30.f / 255.f,
            48.f / 255.f,
            1.0f
        };

        gui::render(&gui, clear_color);
    }




    net::stop(&network);
    ::WSACleanup();


    return 0;
}