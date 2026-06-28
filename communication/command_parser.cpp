#include "command_parser.h"
#include <algorithm>
#include <cctype>

namespace garage {

std::optional<RemoteCommand> CommandParser::parse(std::string_view payload) {
    const std::string keyword = extract_keyword(payload);
    if (keyword.empty()) return std::nullopt;

    Event ev{};
    if      (keyword == "OPEN")      { ev.type = EventType::RemoteOpen;      }
    else if (keyword == "CLOSE")     { ev.type = EventType::RemoteClose;     }
    else if (keyword == "TOGGLE")    { ev.type = EventType::RemoteToggle;    }
    else if (keyword == "STOP")      { ev.type = EventType::RemoteStop;      }
    else if (keyword == "CALIBRATE") { ev.type = EventType::RemoteCalibrate; }
    else { return std::nullopt; }

    RemoteCommand cmd;
    cmd.event       = ev;
    cmd.raw_command = std::string(payload);
    return cmd;
}

std::string CommandParser::extract_keyword(std::string_view payload) {
    auto trim = [](std::string_view sv) -> std::string_view {
        auto s = sv.find_first_not_of(" \t\r\n");
        if (s == std::string_view::npos) return {};
        auto e = sv.find_last_not_of(" \t\r\n");
        return sv.substr(s, e - s + 1);
    };

    std::string_view t = trim(payload);
    if (t.empty()) return {};

    std::string keyword;

    if (t.front() == '{') {
        // json
        std::string upper(t);
        std::transform(upper.begin(), upper.end(), upper.begin(),
                       [](unsigned char c){ return (char)std::toupper(c); });

        const size_t key_pos = upper.find("\"COMMAND\"");
        if (key_pos == std::string::npos) return {};
        const size_t colon = upper.find(':', key_pos + 9);
        if (colon == std::string::npos) return {};
        const size_t vq1 = upper.find('"', colon + 1);
        if (vq1 == std::string::npos) return {};
        const size_t vq2 = upper.find('"', vq1 + 1);
        if (vq2 == std::string::npos) return {};

        keyword = upper.substr(vq1 + 1, vq2 - vq1 - 1);
    } else {
        keyword = std::string(t);
        std::transform(keyword.begin(), keyword.end(), keyword.begin(),
                       [](unsigned char c){ return (char)std::toupper(c); });
    }

    return keyword;
}

} // namespace garage
