#ifndef EVEMU_BOTS_HTTP_SERVER_H
#define EVEMU_BOTS_HTTP_SERVER_H

#include <atomic>
#include <string>
#include <thread>

namespace EvEmuBots {

class HttpServer {
public:
    static HttpServer& Instance();
    bool Start(const std::string& bind, uint16_t port, const std::string& token);
    void Stop();

private:
    HttpServer() = default;
    void Loop();
    void HandleClient(int fd);

    int m_listenFd = -1;
    uint16_t m_port = 0;
    std::string m_bind;
    std::string m_token;
    std::atomic<bool> m_run{false};
    std::thread m_thread;
};

} // namespace EvEmuBots

#endif
