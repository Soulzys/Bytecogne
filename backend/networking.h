#pragma once

constexpr uint32 NETWORK_RECEIVE_BUFFER_SIZE = 64 * 1024;
constexpr uint32 NETWORK_MESSAGE_SIZE        = 4096;
constexpr uint32 NETWORK_MESSAGE_CAPACITY    = 32;


namespace net
{
    struct NetworkReceiveBuffer
    {
        char data[NETWORK_RECEIVE_BUFFER_SIZE];
        uint32 read;
        uint32 write;
    };

    struct NetworkMessage
    {
        char data[NETWORK_MESSAGE_SIZE];
        uint32 size;
    };

    struct NetworkMessageBuffer
    {
        NetworkMessage messages[NETWORK_MESSAGE_CAPACITY];
        //NetworkMessage messages*;
        uint32 read;
        uint32 write;
        std::mutex mutex;
    };

    struct Network
    {
        SOCKET listen_socket;
        SOCKET client_socket;

        std::atomic<bool> connected;
        std::atomic<bool> running;

        NetworkReceiveBuffer receive_buffer;
        NetworkMessageBuffer incoming_buffer;

        std::thread thread;
    };


    bool message_push (NetworkMessageBuffer* buffer, const char* data, uint32 size);
    bool message_pop  (NetworkMessageBuffer* buffer, NetworkMessage* out_message);
    bool init         (Network* network, uint16 port);
    void start        (Network* network);
    void stop         (Network* network);
    void thread       (Network* network);
    void process      (Network* network);
}