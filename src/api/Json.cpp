#include "api/Json.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <sstream>

namespace EvEmuBots {

std::string JsonEscape(const std::string& s)
{
    std::string o;
    o.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"': o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n"; break;
            case '\r': o += "\\r"; break;
            case '\t': o += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned>(c));
                    o += buf;
                } else {
                    o += c;
                }
        }
    }
    return o;
}

std::string JsonString(const std::string& s)
{
    return std::string("\"") + JsonEscape(s) + "\"";
}

static const char* FindKey(const std::string& body, const char* key)
{
    std::string pat = std::string("\"") + key + "\"";
    auto pos = body.find(pat);
    if (pos == std::string::npos)
        return nullptr;
    pos = body.find(':', pos + pat.size());
    if (pos == std::string::npos)
        return nullptr;
    ++pos;
    while (pos < body.size() && isspace(static_cast<unsigned char>(body[pos])))
        ++pos;
    if (pos >= body.size())
        return nullptr;
    return body.c_str() + pos;
}

bool JsonExtractUint(const std::string& body, const char* key, uint32_t& out)
{
    const char* p = FindKey(body, key);
    if (!p)
        return false;
    char* end = nullptr;
    unsigned long v = strtoul(p, &end, 10);
    if (end == p)
        return false;
    out = static_cast<uint32_t>(v);
    return true;
}

bool JsonExtractDouble(const std::string& body, const std::string& dottedKey, double& out)
{
    const char* p = FindKey(body, dottedKey.c_str());
    if (!p) {
        auto dot = dottedKey.rfind('.');
        if (dot == std::string::npos)
            return false;
        p = FindKey(body, dottedKey.c_str() + dot + 1);
        if (!p)
            return false;
    }
    char* end = nullptr;
    double v = strtod(p, &end);
    if (end == p)
        return false;
    out = v;
    return true;
}

} // namespace EvEmuBots
