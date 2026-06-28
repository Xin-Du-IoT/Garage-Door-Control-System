#pragma once
#include <optional>
#include <string>
#include <string_view>
#include "garage_types.h"

namespace garage {

struct RemoteCommand {
    Event       event;
    std::string raw_command;
};

class CommandParser {
public:
    CommandParser() = delete;
    static std::optional<RemoteCommand> parse(std::string_view payload);

private:
    static std::string extract_keyword(std::string_view payload);
};

} // namespace garage
