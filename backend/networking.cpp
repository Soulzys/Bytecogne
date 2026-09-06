#include "networking.h"

bool net::init(Network* network, uint16 port)
{
    // Initialize WinSock
    WSAData wsa_data;
    if (::WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    // >NOTE: we cannot use network = {} because of Network.thread, thus we reset it manually
    network->listen_socket         = INVALID_SOCKET;
    network->client_socket         = INVALID_SOCKET;
    network->running               = false;
    network->connected             = false;
    network->receive_buffer.read   = 0;
    network->receive_buffer.write  = 0;
    network->incoming_buffer.read  = 0;
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

bool net::message_push(NetworkMessageBuffer* buffer, const char* data, uint32 size)
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

bool net::message_pop(NetworkMessageBuffer* buffer, NetworkMessage* out_message)
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

void net::start(Network* network)
{
    network->running = true;
    std::cout << "Waiting for JS application...\n";

    network->thread = std::thread(thread, network);
}

void net::stop(Network* network)
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

void net::thread(Network* network)
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

                message_push(&network->incoming_buffer, message, size);

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

void net::process(Network* network)
{
    NetworkMessage message = {};

    while (message_pop(&network->incoming_buffer, &message))
    {
        std::cout << "Main thread received:";

        // The message isn't null terminated
        std::cout.write(message.data, message.size);

        std::cout << "\n";



        // Convert network data into application data here.
    }
}