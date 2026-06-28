#include "garage_door_controller.h"

#include <cstdio>
#include <cstring>

#include "status_serializer.h"

namespace garage {

namespace {

bool status_equal(const DoorStatus& a, const DoorStatus& b) {
    return a.door_state == b.door_state
        && a.error_state == b.error_state
        && a.calibration_state == b.calibration_state
        && a.top_limit_active == b.top_limit_active
        && a.bottom_limit_active == b.bottom_limit_active
        && a.current_position_steps == b.current_position_steps
        && a.travel_steps == b.travel_steps
        && a.motor_running == b.motor_running;
}

bool should_save_stable_state(const DoorStatus& s) {
    return !s.motor_running;
}

constexpr uint32_t kMotorStepIntervalUs = 2000;

} // namespace

void GarageDoorController::init() {
    //storage_.clear(); // only in a dedicated factory-reset mode.
    motor_.init();
    buttons_.init();
    leds_.init();
    storage_.init();

    encoder_.init();
    limits_.init();

    status_.door_state = DoorState::Closed;
    status_.error_state = ErrorState::Normal;
    status_.calibration_state = CalibrationState::NotCalibrated;
    status_.motor_running = false;

    status_.top_limit_active = false;
    status_.bottom_limit_active = false;

    status_.current_position_steps = 0;
    status_.travel_steps = 0;

    handle_boot();
    refresh_feedback();

    publish_needed_ = true;
    save_needed_ = false;
}

void GarageDoorController::tick() {
    const DoorStatus before = status_;

    process_encoder_events();
    status_.current_position_steps = tracker_.current_position();

    status_.top_limit_active = limits_.top_active();
    status_.bottom_limit_active = limits_.bottom_active();

    process_button_events();
    process_remote_events();

    motor_.tick();
    status_.motor_running = motor_.is_running();

    if (status_.door_state == DoorState::Calibrating) {
        tick_calibration();
    } else {
        process_limit_switches();

        if ((status_.door_state == DoorState::Opening ||
             status_.door_state == DoorState::Closing) &&
            safety_.is_stuck(status_.motor_running, status_.current_position_steps)) {
            enter_error_stuck();
        }
    }

    status_.current_position_steps = tracker_.current_position();
    status_.top_limit_active = limits_.top_active();
    status_.bottom_limit_active = limits_.bottom_active();
    status_.motor_running = motor_.is_running();

    refresh_feedback();
    leds_.tick();

    if (!status_equal(before, status_)) {
        publish_needed_ = true;
        if (should_save_stable_state(status_)) {
            save_needed_ = true;
        }
    }

    publish_status_if_needed();
    save_if_needed();
}

DoorStatus GarageDoorController::current_status() const {
    return status_;
}

CommandResponse GarageDoorController::submit_event(const Event& event) {
    switch (event.type) {
        case EventType::BootCompleted:
            handle_boot();
            return {true, "BOOT", "Boot completed"};

        case EventType::LocalSw1Pressed:
            return handle_local_sw1();

        case EventType::LocalCalibratePressed:
            return handle_local_calibrate();

        case EventType::RemoteToggle:
            return handle_remote_toggle();

        case EventType::RemoteOpen:
            return handle_remote_open();

        case EventType::RemoteClose:
            return handle_remote_close();

        case EventType::RemoteStop:
            return handle_remote_stop();

        case EventType::RemoteCalibrate:
            return handle_remote_calibrate();

        default:
            return {false, "UNSUPPORTED", "Event not supported"};
    }
}

void GarageDoorController::handle_boot() {
    auto saved = storage_.load();
    if (saved.has_value()) {
        status_.calibration_state = saved->calibration_state;
        status_.travel_steps = saved->travel_steps;
        status_.current_position_steps = saved->current_position_steps;
        status_.door_state = saved->last_known_door_state;
    } else {
        status_.door_state = DoorState::Closed;
        status_.calibration_state = CalibrationState::NotCalibrated;
        status_.current_position_steps = 0;
        status_.travel_steps = 0;
    }

    tracker_.reset_position(status_.current_position_steps);

    status_.error_state = ErrorState::Normal;
    status_.motor_running = false;

    if (status_.door_state == DoorState::Opening ||
        status_.door_state == DoorState::Closing ||
        status_.door_state == DoorState::Error ||
        status_.door_state == DoorState::Calibrating) {
        status_.door_state = DoorState::Stopped;
    }

    publish_needed_ = true;
}

CommandResponse GarageDoorController::handle_toggle_command(const char* command_name) {
    CommandResponse response{};
    response.success = true;
    response.command = command_name;

    if (status_.error_state != ErrorState::Normal) {
        status_.error_state = ErrorState::Normal;
        status_.door_state = DoorState::Stopped;
        response.message = "Error cleared";
        return response;
    }

    if (status_.door_state == DoorState::Calibrating) {
        stop_motion();
        calibration_.abort();
        enter_error_calibration_failed();
        response.message = "Calibration aborted";
        return response;
    }

    if (status_.calibration_state != CalibrationState::Calibrated) {
        response.success = false;
        response.message = "Not calibrated";
        return response;
    }

    switch (status_.door_state) {
        case DoorState::Closed:
            start_opening();
            response.message = "Door opening";
            break;

        case DoorState::Open:
            start_closing();
            response.message = "Door closing";
            break;

        case DoorState::Opening:
        case DoorState::Closing:
            stop_motion();
            status_.door_state = DoorState::Stopped;
            response.message = "Door stopped";
            break;

        case DoorState::Stopped:
            if (last_motion_direction_ == MoveDirection::Up) {
                start_closing();
                response.message = "Door closing";
            } else {
                start_opening();
                response.message = "Door opening";
            }
            break;

        default:
            response.success = false;
            response.message = "Command not allowed in current state";
            break;
    }

    return response;
}

CommandResponse GarageDoorController::handle_calibrate_command(const char* command_name) {
    CommandResponse response{};
    response.success = true;
    response.command = command_name;

    if (status_.door_state == DoorState::Calibrating) {
        response.success = false;
        response.message = "Calibration already running";
        return response;
    }

    if (status_.error_state != ErrorState::Normal) {
        response.success = false;
        response.message = "System is in error state";
        return response;
    }

    if (!(status_.door_state == DoorState::Closed ||
          status_.door_state == DoorState::Open ||
          status_.door_state == DoorState::Stopped)) {
        response.success = false;
        response.message = "Door is moving";
        return response;
    }

    const bool top = limits_.top_active();
    const bool bottom = limits_.bottom_active();

    if (top && bottom) {
        response.success = false;
        response.message = "Invalid limit state: both active";
        return response;
    }

    stop_motion();
    calibration_.begin();

    status_.error_state = ErrorState::Normal;
    status_.calibration_state = CalibrationState::NotCalibrated;
    status_.door_state = DoorState::Calibrating;

    printf("CALIB | Started\n");

    if (bottom) {
        tracker_.reset_position(0);
        status_.current_position_steps = 0;

        calibration_.notify_bottom_limit(0);
        if (calibration_.failed()) {
            enter_error_calibration_failed();
            response.success = false;
            response.message = "Calibration failed";
            return response;
        }

        printf("CALIB | In progress\n");
        continue_calibration_up();
    } else {
        printf("CALIB | In progress\n");
        start_calibration_down();
    }

    response.message = "Calibration started";
    return response;
}

CommandResponse GarageDoorController::handle_local_sw1() {
    return handle_toggle_command("SW1");
}

CommandResponse GarageDoorController::handle_local_calibrate() {
    return handle_calibrate_command("LOCAL_CALIBRATE");
}

CommandResponse GarageDoorController::handle_remote_toggle() {
    return handle_toggle_command("TOGGLE");
}

CommandResponse GarageDoorController::handle_remote_open() {
    CommandResponse response{};
    response.command = "OPEN";
    response.success = true;

    if (status_.error_state != ErrorState::Normal) {
        response.success = false;
        response.message = "System is in error state";
        return response;
    }

    if (status_.door_state == DoorState::Calibrating) {
        response.success = false;
        response.message = "Calibration in progress";
        return response;
    }

    if (status_.calibration_state != CalibrationState::Calibrated) {
        response.success = false;
        response.message = "Not calibrated";
        return response;
    }

    if (status_.door_state == DoorState::Open) {
        response.message = "Door already open";
        return response;
    }

    if (status_.door_state == DoorState::Opening) {
        response.message = "Door already opening";
        return response;
    }

    start_opening();//
    response.message = "Door opening";
    return response;
}

CommandResponse GarageDoorController::handle_remote_close() {
    CommandResponse response{};
    response.command = "CLOSE";
    response.success = true;

    if (status_.error_state != ErrorState::Normal) {
        response.success = false;
        response.message = "System is in error state";
        return response;
    }

    if (status_.door_state == DoorState::Calibrating) {
        response.success = false;
        response.message = "Calibration in progress";
        return response;
    }

    if (status_.calibration_state != CalibrationState::Calibrated) {
        response.success = false;
        response.message = "Not calibrated";
        return response;
    }

    if (status_.door_state == DoorState::Closed) {
        response.message = "Door already closed";
        return response;
    }

    if (status_.door_state == DoorState::Closing) {
        response.message = "Door already closing";
        return response;
    }

    start_closing();//
    response.message = "Door closing";
    return response;
}

CommandResponse GarageDoorController::handle_remote_stop() {
    CommandResponse response{};
    response.command = "STOP";
    response.success = true;

    if (status_.door_state == DoorState::Calibrating) {
        stop_motion();
        calibration_.abort();
        enter_error_calibration_failed();
        response.message = "Calibration aborted";
        return response;
    }

    if (status_.door_state == DoorState::Opening ||
        status_.door_state == DoorState::Closing) {
        stop_motion();
        status_.door_state = DoorState::Stopped;
        response.message = "Door stopped";
        return response;
    }

    response.message = "Door already stopped";
    return response;
}

CommandResponse GarageDoorController::handle_remote_calibrate() {
    return handle_calibrate_command("CALIBRATE");
}

void GarageDoorController::process_button_events() {
    auto btn_event = buttons_.poll_event();
    if (!btn_event.has_value()) {
        return;
    }

    switch (*btn_event) {
        case ButtonEvent::Sw1Pressed:
            (void)submit_event(Event{EventType::LocalSw1Pressed});
            break;

        case ButtonEvent::Sw0Sw2Pressed:
            (void)submit_event(Event{EventType::LocalCalibratePressed});
            break;

        case ButtonEvent::Sw0Pressed:
        case ButtonEvent::Sw2Pressed:
            break;
    }
}

void GarageDoorController::process_remote_events() {
    while (true) {
        auto maybe = mqtt_.poll_command();
        if (!maybe.has_value()) {
            break;
        }

        CommandResponse response = submit_event(maybe->event);
        mqtt_.publish_response(StatusSerializer::to_response_json(response));
    }
}

    // Due to current wiring / phase convention,
    // motor MoveDirection::Down drives the door upward (opening).
void GarageDoorController::start_opening() {
    motor_.start(MoveDirection::Down, kMotorStepIntervalUs);
    safety_.on_motor_started(MoveDirection::Up, status_.current_position_steps);

    status_.door_state = DoorState::Opening;
    status_.motor_running = true;
    last_motion_direction_ = MoveDirection::Up;
}

void GarageDoorController::start_closing() {
    motor_.start(MoveDirection::Up, kMotorStepIntervalUs);
    safety_.on_motor_started(MoveDirection::Down, status_.current_position_steps);

    status_.door_state = DoorState::Closing;
    status_.motor_running = true;
    last_motion_direction_ = MoveDirection::Down;
}

void GarageDoorController::stop_motion() {
    motor_.stop();
    safety_.on_motor_stopped();
    status_.motor_running = false;
}

void GarageDoorController::start_calibration_down() {
    motor_.start(MoveDirection::Up, kMotorStepIntervalUs);
    safety_.on_motor_started(MoveDirection::Down, status_.current_position_steps);

    status_.door_state = DoorState::Calibrating;
    status_.motor_running = true;
    last_motion_direction_ = MoveDirection::Down;
}

void GarageDoorController::continue_calibration_up() {
    motor_.start(MoveDirection::Down, kMotorStepIntervalUs);
    safety_.on_motor_started(MoveDirection::Up, status_.current_position_steps);

    status_.door_state = DoorState::Calibrating;
    status_.motor_running = true;
    last_motion_direction_ = MoveDirection::Up;
}

void GarageDoorController::tick_calibration() {
    if (safety_.is_stuck(status_.motor_running, status_.current_position_steps)) {
        calibration_.abort();
        enter_error_stuck();
        return;
    }

    const char* phase = calibration_.phase_name();

    if (std::strcmp(phase, "FindingBottom") == 0) {
        if (limits_.bottom_active()) {
            stop_motion();

            calibration_.notify_bottom_limit(status_.current_position_steps);

            if (calibration_.failed()) {
                enter_error_calibration_failed();
                return;
            }

            tracker_.reset_position(0);
            status_.current_position_steps = 0;

            printf("CALIB | In progress\n");
            continue_calibration_up();
        }
        return;
    }

    if (std::strcmp(phase, "FindingTop") == 0) {
        if (limits_.top_active()) {
            stop_motion();

            calibration_.notify_top_limit(status_.current_position_steps);

            if (calibration_.failed()) {
                enter_error_calibration_failed();
                return;
            }

            if (calibration_.finished() && !calibration_.failed()) {
                const CalibrationResult r = calibration_.result();
                if (r.valid) {
                    status_.travel_steps = r.travel_steps;
                    status_.calibration_state = CalibrationState::Calibrated;
                    status_.door_state = DoorState::Open;
                    status_.error_state = ErrorState::Normal;
                    status_.motor_running = false;
                    status_.current_position_steps = r.travel_steps;
                    tracker_.reset_position(r.travel_steps);

                    printf("CALIB | Completed | success=Yes | travel=%ld\n",
                           static_cast<long>(status_.travel_steps));
                    return;
                }
            }

            enter_error_calibration_failed();
        }
        return;
    }
}

void GarageDoorController::process_encoder_events() {
    while (true) {
        auto ev = encoder_.poll_event();
        if (!ev.has_value()) {
            break;
        }
        tracker_.on_encoder_event(*ev);
    }
}

void GarageDoorController::process_limit_switches() {
    if (status_.door_state == DoorState::Opening && limits_.top_active()) {
        stop_motion();

        if (status_.calibration_state == CalibrationState::Calibrated &&
            status_.travel_steps > 0) {
            tracker_.reset_position(status_.travel_steps);
            status_.current_position_steps = status_.travel_steps;
        }

        status_.door_state = DoorState::Open;
    }

    if (status_.door_state == DoorState::Closing && limits_.bottom_active()) {
        stop_motion();

        tracker_.reset_position(0);
        status_.current_position_steps = 0;
        status_.door_state = DoorState::Closed;
    }
}

void GarageDoorController::enter_error_stuck() {
    motor_.stop();
    safety_.on_motor_stopped();

    status_.door_state = DoorState::Error;
    status_.error_state = ErrorState::DoorStuck;
    status_.calibration_state = CalibrationState::NotCalibrated;
    status_.motor_running = false;

    printf("CALIB | Completed | success=No | reason=Door stuck\n");
}

void GarageDoorController::enter_error_calibration_failed() {
    motor_.stop();
    safety_.on_motor_stopped();

    status_.door_state = DoorState::Error;
    status_.error_state = ErrorState::DoorStuck;
    status_.calibration_state = CalibrationState::NotCalibrated;
    status_.motor_running = false;

    printf("CALIB | Completed | success=No | reason=Calibration failed\n");
}

void GarageDoorController::refresh_feedback() {
    leds_.show_status(status_);
}

void GarageDoorController::publish_status_if_needed() {
    if (!publish_needed_) {
        return;
    }

    if (mqtt_.publish_status(StatusSerializer::to_status_json(status_))) {
        publish_needed_ = false;
    }
}

void GarageDoorController::save_if_needed() {
    if (!save_needed_) {
        return;
    }

    PersistentData data{};
    data.calibration_state = status_.calibration_state;
    data.travel_steps = status_.travel_steps;
    data.current_position_steps = status_.current_position_steps;
    data.last_known_door_state = status_.door_state;

    if (storage_.save(data)) {
        save_needed_ = false;
    }
}

} // namespace garage