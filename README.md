# Garage-Door-Control-System

Embedded garage door control system based on RP2040. The system integrates hardware drivers, control logic, and IoT communication to implement a reliable and state-driven garage door controller.

## Overview

This project implements a full embedded control system for a garage door. It combines hardware abstraction, real-time control logic, and MQTT-based communication to enable both local and remote operation.

The system supports encoder-based position tracking, limit switch calibration, and persistent state storage using internal flash memory.

## Key Features

- Encoder-based position tracking for accurate door movement measurement  
- Limit switch calibration for automatic travel range detection  
- State machine-based control logic (INIT, CALIBRATING, READY, OPEN, CLOSED, ERROR)  
- MQTT communication for remote control and status reporting  
- Flash memory persistence for saving system state after reboot  
- Modular architecture separating hardware, logic, and communication layers  

## System Architecture

The system is structured into multiple layers:

- **Core Controller**
  - GarageDoorController manages overall system behavior

- **Hardware Layer**
  - Stepper motor control
  - Rotary encoder input
  - Limit switches
  - Buttons and LEDs

- **Logic Layer**
  - Position tracking
  - Calibration engine
  - Safety monitoring

- **Communication Layer**
  - MQTT client
  - Command parser
  - Status serializer
  - WiFi configuration

- **Storage Layer**
  - Internal flash persistence for system state

## Project Flow

Boot → Initialization → Load state → Main loop

Main loop includes:
- Reading inputs (buttons, encoder, MQTT commands)
- Updating door state
- Handling calibration and movement logic
- Updating LEDs
- Publishing MQTT status
- Saving state to flash (when idle)

## Tech Stack

- Language: C / C++
- Platform: RP2040
- Communication: MQTT (lwIP)
- Hardware: Stepper motor, rotary encoder, limit switches
- Storage: Internal flash memory

## Purpose

This project demonstrates embedded system design principles including modular architecture, state machine control, hardware abstraction, and IoT integration.

## Future Improvements

- Add OTA firmware update support  
- Improve MQTT message structure with JSON schema  
- Add real-time diagnostics dashboard  
- Expand safety monitoring system  
