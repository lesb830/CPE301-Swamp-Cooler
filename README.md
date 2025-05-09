# CPE301-Swamp-Cooler

## Final Project – CPE 301.1001  
**Authors**: Group 19 – Leslie Becerra, Kj Moreno, Khoa Minh Do

### Overview

This project is a functional swamp cooler built on the Arduino Mega 2560 platform. The system uses various sensors and actuators to simulate a real-world evaporative cooling system, adjusting temperature and airflow based on environmental conditions. It features a user interface with buttons, LCD display, and visual status indicators.


### Features

- Temperature & Humidity Monitoring via DHT11 sensor
- Water Level Detection with alerts
- Fan Control based on temperature thresholds
- Stepper Motor Control for vent adjustment via potentiometer
- User Interaction through buttons and LCD screen
- State Indicators using LEDs (Yellow, Green, Red, Blue)
- Real-time Logging via Serial output
  
### System Components
- Arduino Mega 2560
- DHT11 Temperature/Humidity Sensor
- Water Level Sensor
- Step Motors & Potentiometer
- DC Motor & Motor Driver Modules
- 16x2 LCD Display (I2C)
- Pushbuttons (Start, Stop, Reset)
- Status LEDs

### How It Works
The cooler adjusts its operation based on the readings from the sensors. The system runs in states like IDLE, RUNNING, and ERROR, with transitions logged in real-time. The stepper motor adjusts the vent, while the fan operates based on environmental conditions. Key functions are timestamped and displayed on the LCD.
