#define WIN32_LEAN_AND_MEAN

#include "cogne.h"
#include <thread>

#include <cstring>

#include "gui.cpp"







// Win32 window procedure
//
// Forward declare based on the instructions in imgui_impl_win32.cpp
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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










bool network_init(Network* network, uint16 port)
{
    // >NOTE: we cannot use network = {} because of Network.thread, thus we reset it manually
    network->listen_socket = INVALID_SOCKET;
    network->client_socket = INVALID_SOCKET;
    network->running = false;
    network->connected = false;
    network->receive_buffer.read = 0;
    network->receive_buffer.write = 0;
    network->incoming_buffer.read = 0;
    network->incoming_buffer.write = 0;

    SOCKET socket_handle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_handle == INVALID_SOCKET)
    {
        std::cerr << "socket() failed\n";
        return false;
    }

    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_port = ::htons(port);

    ::inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

    if (::bind(socket_handle, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR)
    {
        std::cerr << "bind() failed\n";
        ::closesocket(socket_handle);
        return false;
    }

    if (::listen(socket_handle, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "listen() failed\n";
        ::closesocket(socket_handle);
        return false;
    }

    network->listen_socket = socket_handle;
    return true;
}

bool network_message_push(NetworkMessageBuffer* buffer, const char* data, uint32 size)
{
    if (size > NETWORK_MESSAGE_SIZE)
    {
        std::cerr << "Size is too large\n";
        return false;
    }

    std::lock_guard<std::mutex> lock(buffer->mutex);

    uint32 next_write = (buffer->write + 1) % NETWORK_MESSAGE_CAPACITY;

    // Buffer is full
    if (next_write == buffer->read)
    {
        std::cerr << "Buffer is full\n";
        return false;
    }

    NetworkMessage* message = &buffer->messages[buffer->write];
    message->size = size;

    memcpy(message->data, data, size);
    buffer->write = next_write;
    
    return true;
}

bool network_message_pop(NetworkMessageBuffer* buffer, NetworkMessage* out_message)
{
    std::lock_guard<std::mutex> lock(buffer->mutex);

    if (buffer->read == buffer->write)
    {
        return false;
    }

    *out_message = buffer->messages[buffer->read];

    buffer->read = (buffer->read + 1) % NETWORK_MESSAGE_CAPACITY;

    return true;
}

void network_start(Network* network)
{
    network->running = true;
    std::cout << "Waiting for JS application...\n";

    network->thread = std::thread(network_thread, network);
}

void network_stop(Network* network)
{
    network->running = false;

    // Closing the listening socket wakes accept()
    if (network->listen_socket != INVALID_SOCKET)
    {
        ::shutdown(network->listen_socket, SD_BOTH);
        ::closesocket(network->listen_socket);
        network->listen_socket = INVALID_SOCKET;
    }

    // Closing the client socket wakes recv()
    if (network->client_socket != INVALID_SOCKET)
    {
        ::shutdown(network->client_socket, SD_BOTH);
        ::closesocket(network->client_socket);
        network->client_socket = INVALID_SOCKET;
    }

    if (network->thread.joinable())
    {
        network->thread.join();
    }
}

void network_thread(Network* network)
{
    SOCKET client = ::accept(network->listen_socket, nullptr, nullptr);
    if (client == INVALID_SOCKET)
    {
        if (network->running)
        {
            std::cerr << "accept() failed\n";
        }

        return;
    }

    network->client_socket = client;
    network->connected = true;

    std::cout << "JS application connected !\n";

    char receive_data[NETWORK_MESSAGE_SIZE];
    while (network->running)
    {
        int bytes_received = ::recv(network->client_socket, receive_data, sizeof(receive_data), 0);

        if (bytes_received > 0)
        {
            NetworkReceiveBuffer* buffer = &network->receive_buffer;

            // Append bytes to our persistent receive buffer
            //
            for (int i = 0; i < bytes_received; i++)
            {
                uint32 next_write = (buffer->write + 1) % NETWORK_RECEIVE_BUFFER_SIZE;

                // Receive buffer is full
                if (next_write == buffer->read)
                {
                    std::cerr << "Network receive buffer is full\n";
                    break;
                }

                buffer->data[buffer->write] = receive_data[i];
                buffer->write = next_write;
            }

            // Extract complete newline-delimited messages
            //
            while (buffer->read != buffer->write)
            {
                uint32 position = buffer->read;
                uint32 size = 0;
                bool found_newline = false;

                while (position != buffer->write)
                {
                    char c = buffer->data[position];
                    if (c == '\n')
                    {
                        found_newline = true;
                        break;
                    }

                    position = (position + 1) % NETWORK_RECEIVE_BUFFER_SIZE;
                    size++;
                    if (size >= NETWORK_MESSAGE_SIZE)
                    {
                        std::cerr << "Network message too large\n";
                        break;
                    }
                }

                // We haven't received the complete message yet
                if (!found_newline)
                {
                    break;
                }

                // Copy the complete message into the message buffer
                //
                char message[NETWORK_MESSAGE_SIZE];
                uint32 source = buffer->read;

                for (uint32 i = 0; i < size; i++)
                {
                    message[i] = buffer->data[source];
                    source = (source + 1) % NETWORK_RECEIVE_BUFFER_SIZE;
                }

                network_message_push(&network->incoming_buffer, message, size);

                // Consume message + newline
                buffer->read = (source + 1) % NETWORK_RECEIVE_BUFFER_SIZE;
            }
        }
        // Connection closed
        else if (bytes_received == 0)
        {
            std::cout << "Connection closed by peer\n";
            break;
        }
        // recv() failed
        else
        {
            int error = WSAGetLastError();
            if (network->running)
            {
                std::cerr << "recv() failed: " << error << "\n";
            }

            break;
        }
    }

    network->connected = false;

    ::shutdown(network->client_socket, SD_BOTH);
    ::closesocket(network->client_socket);
    network->client_socket = INVALID_SOCKET;

    std::cout << "Network thread exiting\n";
}

void network_process(Network* network)
{
    NetworkMessage message = {};

    while (network_message_pop(&network->incoming_buffer, &message))
    {
        std::cout << "Main thread received:";

        // The message isn't null terminated
        std::cout.write(message.data, message.size);

        std::cout << "\n";



        // Convert network data into application data here.
    }
}



void wnd::create_window(HINSTANCE hinstance, Window* out_wnd)
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
        nullptr
    );

    out_wnd->handle = handle;
    out_wnd->name   = wc.lpszClassName;
}


int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, LPSTR, int)
{
    // Create Win32 window
    //
    wnd::Window wnd = {};
    wnd::create_window(hinstance, &wnd);


    // Create D3D11
    //
    gui::DearGUI gui = {};
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

    // Initialize WinSock
    //
    WSAData wsa_data;
    if (::WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }



    //
    // NETWORK CODE
    //
    Network network = {};
    if (!network_init(&network, 5000))
    {
        ::WSACleanup();
        return 1;
    }

    network_start(&network);


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
        network_process(&network);


        // Start ImGui frame
        //
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();


        // GUI
        //
        ImGui::Begin("Cogne");
        ImGui::Text("Hello from Cogne!");
        ImGui::TextColored(ImVec4(0.f, 255.f, 0.f, 1.f), "Connected");
        if (ImGui::Button("Test"))
        {
            std::cout << "Button pressed\n";
            OutputDebugString("----------- WSH\n");
        }

        ImGui::End();


        // Render
        //
        ImGui::Render();
        
        const float clear_color[] =
        {
            18.f / 255.f, 
            30.f / 255.f, 
            48.f / 255.f,
            1.0f
        };

        gui.context->OMSetRenderTargets(1, &gui.render_target_view, nullptr);
        gui.context->ClearRenderTargetView(gui.render_target_view, clear_color);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        
        gui.swap_chain->Present(1, 0);
    }




    network_stop(&network);
    ::WSACleanup();


    return 0;
}