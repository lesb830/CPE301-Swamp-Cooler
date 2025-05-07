# CPE301-Swamp-Cooler-Final

## Final Project – CPE 301.1001  
**Authors**: Group 19 – Leslie Becerra, Kj Moreno, Khoa Minh Do

### Overview

This project is a fully functional, Arduino-based embedded system designed to control a swamp cooler. Built on the Arduino Mega2560 platform, the system integrates multiple sensors and actuators to simulate the behavior of a real-world evaporative cooling system. The cooler monitors environmental conditions and reacts to changes in temperature and water levels, while also allowing for user interaction through buttons and a vent angle control mechanism.

### Features

- Real-time temperature and humidity monitoring using the DHT11 sensor.
- Water level detection with alert handling.
- Fan control based on temperature thresholds.
- Stepper motor vent control using a potentiometer.
- On/Off button functionality using interrupt service routines (ISRs).
- Real-time clock module for timestamp logging of events.
- LCD display showing live temperature, humidity, and system status.
- Multi-state logic: DISABLED, IDLE, RUNNING, and ERROR.
- State-specific LED indicators (YELLOW, GREEN, BLUE, RED).
- Serial output for event logging to a host computer.

### System Components

- **Arduino Mega2560**
- **DHT11 Temperature/Humidity Sensor**
- **Water Level Sensor**
- **Stepper Motor + Potentiometer**
- **DC Fan Motor with Separate Power Board**
- **RTC Module (Real-Time Clock)**
- **16x2 LCD Display**
- **Multiple Pushbuttons (Start, Stop, Reset)**
- **Status LEDs (YELLOW, GREEN, RED, BLUE)**

### How It Works

The swamp cooler operates based on its current state. The main loop continuously calls a `checkState()` function (unless disabled), which reads the temperature and water level to determine the correct operational state. Depending on conditions, the system transitions between IDLE, RUNNING, or ERROR. All state changes and key actions are timestamped and printed via Serial communication. The system ensures responsive, low-level performance by avoiding most Arduino libraries and relying instead on direct register manipulation where required.

### Repository Contents

- `main.ino`: Core logic for the swamp cooler system.
- `README.md`: Project description and documentation.
- `Images/`: Folder for schematic and circuit photos (if available).
- `Video/`: Link or placeholder for demonstration video.
- `Docs/`: Supporting technical documentation and state diagrams.

### Demonstration Video

[Insert YouTube or Google Drive link here]

### Circuit Schematic

[Insert schematic image or PDF here or include in `Images/` folder]

### Notes

- All code complies with the course requirement to avoid restricted Arduino functions like `pinMode()` and `digitalWrite()` unless explicitly allowed.
- The system uses `millis()` instead of `delay()` to handle periodic updates like LCD refreshes.
- Only the motor uses `analogWrite()` as permitted.
