#pragma once
#include <cstdint>
#include <string>

namespace garage {

    enum class DoorState {
        Open,
        Closed,
        InBetween,
        Opening,
        Closing,
        Stopped,
        Calibrating,
        Error
    };

    enum class ErrorState {
        Normal,
        DoorStuck
    };

    enum class CalibrationState {
        NotCalibrated,
        Calibrated
    };

    enum class MoveDirection {
        Up,
        Down
    };

    enum class EventType {
        LocalSw1Pressed,
        LocalCalibratePressed,
        RemoteToggle,
        RemoteOpen,
        RemoteClose,
        RemoteStop,
        RemoteCalibrate,
        BootCompleted
    };

    struct Event {
        EventType type;
    };

    struct CalibrationResult {
        bool valid {false};
        int32_t travel_steps {0};
    };

    struct DoorStatus {
        //controller
        DoorState door_state {DoorState::Closed};
        ErrorState error_state {ErrorState::Normal};
        CalibrationState calibration_state {CalibrationState::NotCalibrated};
       // limit_switch
        bool top_limit_active {false};
        bool bottom_limit_active {false};
        //tracker
        int32_t current_position_steps {0};
        //calibration
        int32_t travel_steps {0};
        //motor
        bool motor_running {false};
    };

    struct CommandResponse {
        bool success {false};
        std::string command;
        std::string message;
    };

    struct PersistentData {
        CalibrationState calibration_state {CalibrationState::NotCalibrated};
        int32_t travel_steps {0};
        int32_t current_position_steps {0};
        DoorState last_known_door_state {DoorState::Closed};
    };

} // namespace garage