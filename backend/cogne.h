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

#include "utils.cpp"

class MessageQueue
{
public:

    void push(const std::string& message);
    bool try_pop(std::string& message);


private:

    std::mutex m_mutex;
    std::queue<std::string> m_queue;
};


class Application
{
public:

    Application(SOCKET socket);

    void start();
    void stop();
    void process_messages();


private:

    void network_thread();


private:

    SOCKET m_listen_socket;
    SOCKET m_client_socket;
    std::atomic<bool> m_running;
    MessageQueue m_messages;
    std::thread m_network_thread;
};