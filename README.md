# Garage Door Control System

An RP2040-based embedded project using C/C++, with local controls and MQTT communication for remote commands and status reporting.

## Overview

The controller uses a stepper motor to move the door, a rotary encoder to track its position, and limit switches to calibrate the travel range.

A state machine manages initialization, calibration, door operation and error handling. Saved state is loaded from internal flash memory at startup.

## Key Features

- Tracks door position using a rotary encoder.
- Calibrates the travel range using limit switches.
- Handles local input and remote MQTT commands.
- Reports door status through MQTT.
- Saves system state to internal flash when idle.
- Separates hardware drivers, control logic, communication and storage into modules.

## System Architecture

`GarageDoorController` coordinates the main modules:

| Module | Responsibilities |
| --- | --- |
| Hardware | Stepper motor, rotary encoder, limit switches, buttons and LEDs |
| Control logic | Position tracking, calibration, state transitions and safety checks |
| Communication | MQTT connection, command parsing, status messages and Wi-Fi configuration |
| Storage | Loading and saving system state in internal flash |

The controller uses the following states:

`INIT`, `CALIBRATING`, `READY`, `OPEN`, `CLOSED`, `ERROR`

## Program Flow

At startup, the system initializes the hardware and loads the saved state.

The main loop then:

1. Reads buttons, encoder input and MQTT commands.
2. Updates the controller state.
3. Handles calibration and motor movement.
4. Updates the LEDs and publishes status messages.
5. Saves state to flash when idle.

## Technology

- **Language:** C/C++
- **Microcontroller:** RP2040
- **Networking:** MQTT using lwIP
- **Hardware:** Stepper motor, rotary encoder, limit switches, buttons and LEDs
- **Storage:** Internal flash memory
