#pragma once
#include <string>
#include "garage_types.h"

namespace garage {

class StatusSerializer {
public:
    StatusSerializer() = delete;

    static std::string to_status_json(const DoorStatus& status);
    static std::string to_response_json(const CommandResponse& response);

private:
    static const char* door_state_str(DoorState s);
    static const char* error_state_str(ErrorState s);
    static const char* calibration_state_str(CalibrationState s);
    static std::string escape(const std::string& s);
};

} // namespace garage
