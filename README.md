# RTOS-Based Mini Robot Controller (ESP32 + FreeRTOS)

A multitasking robot controller built with FreeRTOS on ESP32.
**Simulated in Wokwi. Hardware testing pending.**

**Live simulation:** https://wokwi.com/projects/480540360623898625

## Demo
[Watch the demo video](demo.mp4)

## Features
- Ultrasonic obstacle detection (HC-SR04)
- PWM motor speed control (LEDs represent motors in simulation)
- Emergency stop using a button interrupt and semaphore
- UART telemetry in JSON format

## RTOS Tasks
| Task | Job |
|------|-----|
| Sensor | Reads distance and sends it to a queue |
| Control | Decides FORWARD / SLOW / TURN / STOP |
| Motor | Sets PWM speed and direction |
| Telemetry | Prints status over UART every 500 ms |
| E-Stop | Woken by button ISR through a binary semaphore |

## Behavior
| Distance | Action |
|----------|--------|
| 25 cm or more | FORWARD (full speed) |
| 10 to 25 cm | SLOW |
| Below 10 cm | TURN |
| Button pressed | STOP (press again to resume) |

## Pin Mapping
| Pin | Use |
|-----|-----|
| GPIO5 / GPIO18 | HC-SR04 TRIG / ECHO |
| GPIO25 / GPIO26 | Left / Right motor PWM |
| GPIO27 / GPIO14 | Left / Right direction |
| GPIO4 | E-Stop button |

## Tools
ESP32, FreeRTOS (Arduino core), Wokwi
