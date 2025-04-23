# Flex Sensor Glove
The Flex Sensor Glove project is for a glove with flex sensors and an IMU to detect and classify finger position, send the control signals via BLE, and control a host computer by emulating a keyboard. The PCB and 3D printed case was custom designed by me, Trevor O'Connell, to ensure a clean look.

# Hardware
- SEEED XIAO nrf52840 MCU for both the reciever and the transmitter
- FlexPoint Bend Sensors (3" and 4" lengths)
- GY-521 IMU
- 1S LiPo Battery
- SSD1306 OLED Screen 128x64

# How it works
- MCU reads and processes signals inputs
- Signals inputs mapped to discrete values
- Discrete control signals sent over BLE to reciever
- Reciever observes signals and triggers keystrokes when sensors are classified above their normal state

# Code Overview
- The code for the final version is in the /src folder and has the code for the transmitter (glove mounted device) and the reciever (host-computer device)
- Many different versions of the code were made to test and verify certain features prior to integration. They can be found in the other folder.
