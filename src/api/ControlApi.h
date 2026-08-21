#ifndef EVEMU_BOTS_CONTROL_API_H
#define EVEMU_BOTS_CONTROL_API_H

#include <string>

namespace EvEmuBots {

struct HttpResponse {
    int status = 200;
    std::string contentType = "application/json";
    std::string body;
};

HttpResponse HandleApi(const std::string& method, const std::string& path, const std::string& body);

} // namespace EvEmuBots

#endif
