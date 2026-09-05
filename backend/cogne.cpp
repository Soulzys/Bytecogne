#define WIN32_LEAN_AND_MEAN

#include "cogne.h"
#include <thread>

#include "windows.h"
#include "d3d11.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"
#include <cstring>

#include "gui.h"




// D3D11 state
//
static ID3D11Device*           g_device             = nullptr;
static ID3D11DeviceContext*    g_device_context     = nullptr;
static IDXGISwapChain*         g_swap_chain         = nullptr;
static ID3D11RenderTargetView* g_render_target_view = nullptr;


// Create / Destroy render target
//
void create_render_target()
{
    ID3D11Texture2D* back_buffer = nullptr;

    g_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    g_device->CreateRenderTargetView(back_buffer, nullptr, &g_render_target_view);
    back_buffer->Release();
}

void destroy_render_target()
{
    if (g_render_target_view)
    {
        g_render_target_view->Release();
        g_render_target_view = nullptr;
    }
}


// D3D11 initialization
//
bool create_device_D3D(HWND handle)
{
    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};

    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.BufferDesc.Width = 0;
    swap_chain_desc.BufferDesc.Height = 0;
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;

    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow = handle;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.Windowed = TRUE;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL feature_level;
    const D3D_FEATURE_LEVEL feature_levels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };

    HRESULT hr = D3D11CreateDeviceAndSwapChain
    (
        nullptr, 
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr, 
        0, 
        feature_levels, 
        ARRAYSIZE(feature_levels),
        D3D11_SDK_VERSION,
        &swap_chain_desc, 
        &g_swap_chain, 
        &g_device,
        &feature_level,
        &g_device_context
    );

    if (FAILED(hr))
    {
        return false;
    }

    create_render_target();

    return true;
}

void cleanup_device_D3D()
{
    destroy_render_target();

    if (g_swap_chain)
    {
        g_swap_chain->Release();
        g_swap_chain = nullptr;
    }

    if (g_device_context)
    {
        g_device_context->Release();
        g_device_context = nullptr;
    }

    if (g_device)
    {
        g_device->Release();
        g_device = nullptr;
    }
}


// Win32 window procedure
//
// Forward declare based on the instructions in imgui_impl_win32.cpp
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (ImGui_ImplWin32_WndProcHandler(handle, message, wparam, lparam))
    {
        return true;
    }

    switch (message)
    {
        case WM_SIZE:
        {
            if (g_device && wparam != SIZE_MINIMIZED)
            {
                destroy_render_target();
                g_swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
                create_render_target();
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
    //*network = {};
    network->listen_socket = INVALID_SOCKET;
    network->client_socket = INVALID_SOCKET;
    network->running = false;
    network->connected = false;
    network->receive.read = 0;
    network->receive.write = 0;
    network->incoming.read = 0;
    network->incoming.write = 0;

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

    network->thread = std::thread(network_thread2, network);
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

void network_thread2(Network* network)
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

    char receive_data[4096];
    while (network->running)
    {
        int bytes_received = ::recv(network->client_socket, receive_data, sizeof(receive_data), 0);

        if (bytes_received > 0)
        {
            NetworkReceiveBuffer* buffer = &network->receive;

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

                    size++;
                    position = (position + 1) % NETWORK_RECEIVE_BUFFER_SIZE;
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

                network_message_push(&network->incoming, message, size);

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

    while (network_message_pop(&network->incoming, &message))
    {
        std::cout << "Main thread received:";

        // The message isn't null terminated
        std::cout.write(message.data, message.size);

        std::cout << "\n";



        // Convert network data into application data here.
    }
}






int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, LPSTR, int)
{
    // Create Win32 window
    //
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hinstance;
    wc.lpszClassName = L"Cogne";

    RegisterClassExW(&wc);

    HWND handle = CreateWindowW
    (
        wc.lpszClassName, L"Cogne (feat Dear ImGui & DirectX 11",
        WS_OVERLAPPEDWINDOW,
        100, 100,
        1280, 720,
        nullptr,
        nullptr,
        hinstance,
        nullptr
    );


    // Create D3D11
    //
    if (!create_device_D3D(handle))
    {
        cleanup_device_D3D();
        UnregisterClassW(wc.lpszClassName, hinstance);

        return 1;
    }

    ShowWindow(handle, SW_SHOWDEFAULT);
    UpdateWindow(handle);


    // Initialize Dear ImGui
    //
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();
    
    ImGui_ImplWin32_Init(handle);
    ImGui_ImplDX11_Init(g_device, g_device_context);

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

        g_device_context->OMSetRenderTargets(1, &g_render_target_view, nullptr);
        g_device_context->ClearRenderTargetView(g_render_target_view, clear_color);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        
        g_swap_chain->Present(1, 0);
    }




    //// Cleanup
    //::closesocket(l);
    //::WSACleanup();

    //app.stop();
    network_stop(&network);
    ::WSACleanup();


    return 0;
}



/*
void MessageQueue::push(const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(message);
}

bool MessageQueue::try_pop(std::string& outMessage)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_queue.empty()) return false;

    outMessage = m_queue.front();
    m_queue.pop();

    return true;
}









Application::Application(SOCKET socket)
    : m_listen_socket(socket)
    , m_client_socket(INVALID_SOCKET)
    , m_running(true)
{
}


void Application::start()
{
    std::cout << "Running !\n";
    m_network_thread = std::thread(&Application::network_thread, this);
}

void Application::stop()
{
    m_running = false;

    // Wake up accept()
    if (m_listen_socket != INVALID_SOCKET)
    {
        ::shutdown(m_listen_socket, SD_BOTH);
        ::closesocket(m_listen_socket);
        m_listen_socket = INVALID_SOCKET;
    }

    // Wake up recv(), if JV has connected
    if (m_client_socket != INVALID_SOCKET)
    {
        ::shutdown(m_client_socket, SD_BOTH);
        ::closesocket(m_client_socket);
        m_client_socket = INVALID_SOCKET;
    }

    if (m_network_thread.joinable())
    {
        m_network_thread.join();
    }
}


void Application::network_thread()
{
    // Wait for JS to connect
    //
    m_client_socket = ::accept(m_listen_socket, nullptr, nullptr);
    if (m_client_socket == INVALID_SOCKET)
    {
        std::cerr << "client_socket failed\n";
        ::closesocket(m_listen_socket);
        ::WSACleanup();
        return;
    }

    std::cout << "JS application connected !\n";

    char buffer[4096];
    std::string receive_buffer;

    while (m_running)
    {
        int bytes_received = ::recv(m_client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) // We receive data
        {
            std::cout << "Data received\n";
            receive_buffer.append(buffer, bytes_received);
            while (true)
            {
                size_t newline = receive_buffer.find('\n');
                std::cout << "newline: " << newline << "\n";
                if (newline == std::string::npos) break;

                std::string message = receive_buffer.substr(0, newline);
                std::cout << "message: " << message << "\n";
                receive_buffer.erase(0, newline + 1);
                m_messages.push(message);
            }
        }
        else if (bytes_received == 0) // Remote side closed the connection
        {
            std::cout << "Connection closed by peer\n";
            break;
        }
        else // recv failed
        {
            int error = WSAGetLastError();
            if (m_running)
            {
                std::cout << "recv() failed: " << error << "\n";
            }

            break;
        }
    }

    std::cout << "Network thread exiting\n";
}


void Application::process_messages()
{
    std::string message;

    // IMPORTANT
    // try_pop() does not block, so the main_loop() remains responsive if there is no network data!

    while (m_messages.try_pop(message))
    {
        std::cout << "Main thread received: " << message << "\n";

        utils::dex::parse_token_pairs(message);

        // Update the application state here based on the message received
    }
}
*/