# RTOS-Based Mini Robot Controller (ESP32 + FreeRTOS)

A multitasking mini robot controller built on **ESP32** using **FreeRTOS**, simulated in **Wokwi**. The robot measures distance with an ultrasonic sensor and decides whether to go forward, slow down, turn, or stop. LEDs are used in the simulation to represent the motors.

🔗 **Live simulation:** https://wokwi.com/projects/480540360623898625

## Features
- 5 independent FreeRTOS tasks with different priorities
- Inter-task communication using **queues**
- Emergency stop using a **hardware interrupt + binary semaphore**
- PWM speed control for left and right motors
- JSON telemetry printed on the Serial Monitor every 500 ms
- Button debounce handled in software

## Hardware (Simulated)
| Component | ESP32 Pin |
|---|---|
| HC-SR04 TRIG | GPIO 5 |
| HC-SR04 ECHO | GPIO 18 |
| Left motor PWM | GPIO 25 |
| Right motor PWM | GPIO 26 |
| Left motor DIR | GPIO 27 |
| Right motor DIR | GPIO 14 |
| Emergency stop button | GPIO 4 (INPUT_PULLUP) |

## How It Works

### Robot Behaviour
| Distance | Command | Action |
|---|---|---|
| > 25 cm | FORWARD | Both motors at full speed (PWM 220) |
| 10 – 25 cm | SLOW | Both motors at reduced speed (PWM 120) |
| < 10 cm | TURN | One motor reversed for 500 ms |
| Button pressed | STOP | Motors off until button is pressed again |

### RTOS Tasks
| Task | Priority | Job |
|---|---|---|
| EStop | 4 | Waits for semaphore from ISR, toggles emergency stop |
| Sensor | 3 | Reads distance every 100 ms, sends to queue |
| Control | 3 | Reads distance, decides the command |
| Motor | 2 | Reads command, sets PWM and direction |
| Telemetry | 1 | Prints status on Serial every 500 ms |

## Sample Serial Output
```
{"dist":42.3,"cmd":"FORWARD","estop":false}
{"dist":18.0,"cmd":"SLOW","estop":false}
{"dist":7.5,"cmd":"TURN","estop":false}
>>> EMERGENCY STOP
{"dist":7.5,"cmd":"STOP","estop":true}
```

## How to Run
1. Open the Wokwi link above.
2. Click the green **Start Simulation** button.
3. Change the distance on the HC-SR04 sensor to see FORWARD, SLOW, and TURN.
4. Press the button to toggle emergency stop.
5. Watch the output in the Serial Monitor.

## Concepts Used
FreeRTOS tasks, task priorities, queues, binary semaphore, ISR (`xSemaphoreGiveFromISR`), `vTaskDelay`, PWM (`ledcWrite`), ultrasonic distance measurement, debouncing.

## Future Improvements
- Add a real motor driver (L298N) and DC motors
- Add PID-based speed control
- Send telemetry over Wi-Fi / MQTT

## Author
**Saloni Kumari**
Electronics & Communication Engineering, GEC Rajkot (GTU)
heykumarisaloni@gmail.com
