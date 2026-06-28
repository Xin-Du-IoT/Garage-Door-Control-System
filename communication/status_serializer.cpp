#include "status_serializer.h"
#include <cstdio>

namespace garage {

std::string StatusSerializer::to_status_json(const DoorStatus& s) {
    char buf[320];
    snprintf(buf, sizeof(buf),
        "{"
        "\"door_state\":\"%s\","
        "\"error_state\":\"%s\","
        "\"calibration_state\":\"%s\","
        "\"position_steps\":%ld,"
        "\"travel_steps\":%ld,"
        "\"motor_running\":%s"
        "}",
        door_state_str(s.door_state),
        error_state_str(s.error_state),
        calibration_state_str(s.calibration_state),
        static_cast<long>(s.current_position_steps),
        static_cast<long>(s.travel_steps),
        s.motor_running ? "true" : "false"
    );
    return std::string(buf);
}

std::string StatusSerializer::to_response_json(const CommandResponse& r) {
    char buf[512];
    snprintf(buf, sizeof(buf),
        "{"
        "\"command\":\"%s\","
        "\"result\":\"%s\","
        "\"message\":\"%s\""
        "}",
        escape(r.command).c_str(),
        r.success ? "success" : "error",
        escape(r.message).c_str()
    );
    return std::string(buf);
}

// Open / Closed / In between
const char* StatusSerializer::door_state_str(DoorState s) {
    switch (s) {
        case DoorState::Open:
            return "Open";
        case DoorState::Closed:
            return "Closed";
        case DoorState::InBetween:
        case DoorState::Opening:
        case DoorState::Closing:
        case DoorState::Stopped:
        case DoorState::Calibrating:
        case DoorState::Error:
        default:
            return "In between";
    }
}

// private: error_state_str
const char* StatusSerializer::error_state_str(ErrorState s) {
    switch (s) {
        case ErrorState::Normal:    return "Normal";
        case ErrorState::DoorStuck: return "Door stuck";
        default:                    return "Normal";
    }
}

// private: calibration_state_str
const char* StatusSerializer::calibration_state_str(CalibrationState s) {
    switch (s) {
        case CalibrationState::Calibrated:    return "Calibrated";
        case CalibrationState::NotCalibrated: return "Not calibrated";
        default:                              return "Not calibrated";
    }
}

std::string StatusSerializer::escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() * 2);
    for (unsigned char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04X",
                             static_cast<unsigned>(c));
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
                break;
        }
    }
    return out;
}

} // namespace garage
