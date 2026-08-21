#include "api/HttpServer.h"
#include "api/ControlApi.h"

#include "eve-server.h"
#include "population/BotConfig.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <sstream>
#include <vector>

namespace EvEmuBots {

static bool ReadRequest(int fd, std::string& method, std::string& path,
                        std::string& auth, std::string& body)
{
    std::string raw;
    char buf[4096];
    while (raw.find("\r\n\r\n") == std::string::npos && raw.size() < 64 * 1024) {
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n <= 0)
            return false;
        raw.append(buf, static_cast<size_t>(n));
    }
    auto hdrEnd = raw.find("\r\n\r\n");
    if (hdrEnd == std::string::npos)
        return false;
    std::string headers = raw.substr(0, hdrEnd);
    body = raw.substr(hdrEnd + 4);

    std::istringstream hs(headers);
    std::string line;
    if (!std::getline(hs, line))
        return false;
    if (!line.empty() && line.back() == '\r')
        line.pop_back();
    std::istringstream req(line);
    std::string ver;
    req >> method >> path >> ver;

    size_t contentLength = 0;
    while (std::getline(hs, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        auto colon = line.find(':');
        if (colon == std::string::npos)
            continue;
        std::string k = line.substr(0, colon);
        std::string v = line.substr(colon + 1);
        while (!v.empty() && (v.front() == ' ' || v.front() == '\t'))
            v.erase(v.begin());
        if (strcasecmp(k.c_str(), "Authorization") == 0)
            auth = v;
        else if (strcasecmp(k.c_str(), "Content-Length") == 0)
            contentLength = strtoul(v.c_str(), nullptr, 10);
    }
    while (body.size() < contentLength && body.size() < 1024 * 1024) {
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n <= 0)
            break;
        body.append(buf, static_cast<size_t>(n));
    }
    if (contentLength && body.size() > contentLength)
        body.resize(contentLength);
    auto q = path.find('?');
    if (q != std::string::npos)
        path.resize(q);
    return !method.empty() && !path.empty();
}

static void WriteResponse(int fd, const HttpResponse& r)
{
    const char* reason = "OK";
    if (r.status == 400) reason = "Bad Request";
    else if (r.status == 401) reason = "Unauthorized";
    else if (r.status == 404) reason = "Not Found";
    else if (r.status >= 500) reason = "Internal Server Error";

    std::ostringstream o;
    o << "HTTP/1.1 " << r.status << " " << reason << "\r\n"
      << "Content-Type: " << r.contentType << "\r\n"
      << "Content-Length: " << r.body.size() << "\r\n"
      << "Connection: close\r\n"
      << "X-Service: evemu-bots\r\n"
      << "\r\n"
      << r.body;
    std::string out = o.str();
    send(fd, out.data(), out.size(), MSG_NOSIGNAL);
}

HttpServer& HttpServer::Instance()
{
    static HttpServer s;
    return s;
}

bool HttpServer::Start(const std::string& bind, uint16_t port, const std::string& token)
{
    if (m_run)
        return true;
    m_bind = bind;
    m_port = port;
    m_token = token;

    m_listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenFd < 0)
        return false;
    int opt = 1;
    setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, bind.c_str(), &addr.sin_addr) != 1)
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(m_listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(m_listenFd);
        m_listenFd = -1;
        return false;
    }
    if (listen(m_listenFd, 16) < 0) {
        close(m_listenFd);
        m_listenFd = -1;
        return false;
    }

    m_run = true;
    m_thread = std::thread([this]() { Loop(); });
    sLog.Green("evemu-bots", "Control API listening on %s:%u", bind.c_str(), port);
    return true;
}

void HttpServer::Stop()
{
    m_run = false;
    if (m_listenFd >= 0) {
        shutdown(m_listenFd, SHUT_RDWR);
        close(m_listenFd);
        m_listenFd = -1;
    }
    if (m_thread.joinable())
        m_thread.join();
}

void HttpServer::Loop()
{
    while (m_run) {
        sockaddr_in peer{};
        socklen_t len = sizeof(peer);
        int fd = accept(m_listenFd, reinterpret_cast<sockaddr*>(&peer), &len);
        if (fd < 0) {
            if (!m_run)
                break;
            continue;
        }
        HandleClient(fd);
        close(fd);
    }
}

void HttpServer::HandleClient(int fd)
{
    std::string method, path, auth, body;
    if (!ReadRequest(fd, method, path, auth, body)) {
        HttpResponse r;
        r.status = 400;
        r.body = "{\"error\":\"bad request\"}";
        WriteResponse(fd, r);
        return;
    }

    const bool metrics = (path == "/metrics");
    if (!metrics) {
        const std::string want = "Bearer " + m_token;
        if (m_token.empty() || auth != want) {
            HttpResponse r;
            r.status = 401;
            r.body = "{\"error\":\"unauthorized\"}";
            WriteResponse(fd, r);
            return;
        }
    }

    WriteResponse(fd, HandleApi(method, path, body));
}

} // namespace EvEmuBots
