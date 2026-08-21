#ifndef EVEMU_BOTS_JSON_H
#define EVEMU_BOTS_JSON_H

#include <string>

namespace EvEmuBots {

std::string JsonEscape(const std::string& s);
std::string JsonString(const std::string& s);
bool JsonExtractUint(const std::string& body, const char* key, uint32_t& out);
bool JsonExtractDouble(const std::string& body, const std::string& dottedKey, double& out);

} // namespace EvEmuBots

#endif
