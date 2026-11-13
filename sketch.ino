// RTOS-Based Mini Robot Controller (ESP32 + FreeRTOS)
// Simulated in Wokwi. LEDs motors ko represent karte hain.

#include <Arduino.h>

// ---------- Pins ----------
const int TRIG_PIN = 5;
const int ECHO_PIN = 18;
const int LEFT_PWM = 25;    // left motor speed
const int RIGHT_PWM = 26;   // right motor speed
const int LEFT_DIR = 27;    // left motor direction
const int RIGHT_DIR = 14;   // right motor direction
const int BUTTON_PIN = 4;   // emergency stop

// ---------- Distance limits (cm) ----------
const int STOP_DIST = 10;   // isse kam = turn
const int SLOW_DIST = 25;   // isse kam = slow

// ---------- Robot commands ----------
#define CMD_FORWARD 0
#define CMD_SLOW    1
#define CMD_TURN    2
#define CMD_STOP    3

const char* cmdText[] = {"FORWARD", "SLOW", "TURN", "STOP"};

// telemetry ke liye data ka packet
typedef struct {
  float distance;
  int command;
  bool stopped;
} RobotStatus;

// ---------- RTOS objects ----------
QueueHandle_t distanceQueue;   // sensor -> control
QueueHandle_t commandQueue;    // control -> motor
QueueHandle_t statusQueue;     // control -> telemetry
SemaphoreHandle_t buttonSem;   // ISR -> estop task

volatile bool emergencyStop = false;

// ---------- Interrupt: button dabne par chalta hai ----------
void IRAM_ATTR buttonISR() {
  BaseType_t wake = pdFALSE;
  xSemaphoreGiveFromISR(buttonSem, &wake);
  if (wake) portYIELD_FROM_ISR();
}

// ---------- Helper: ultrasonic distance ----------
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long time = pulseIn(ECHO_PIN, HIGH, 30000);
  return time * 0.034 / 2;
}

// ---------- Helper: motors set karna ----------
void driveMotors(int leftSpeed, int rightSpeed, bool leftForward, bool rightForward) {
  digitalWrite(LEFT_DIR, leftForward);
  digitalWrite(RIGHT_DIR, rightForward);
  ledcWrite(LEFT_PWM, leftSpeed);
  ledcWrite(RIGHT_PWM, rightSpeed);
}

// ---------- Task 1: Sensor ----------
// Har 100 ms me distance naapta hai aur queue me daalta hai
void sensorTask(void *pv) {
  while (true) {
    float dist = getDistance();
    xQueueSend(distanceQueue, &dist, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// ---------- Task 2: Control ----------
// Distance dekh kar decide karta hai robot kya kare
void controlTask(void *pv) {
  float dist;
  while (true) {
    if (xQueueReceive(distanceQueue, &dist, portMAX_DELAY)) {
      int cmd;

      if (emergencyStop) {
        cmd = CMD_STOP;
      } else if (dist > 0 && dist < STOP_DIST) {
        cmd = CMD_TURN;
      } else if (dist > 0 && dist < SLOW_DIST) {
        cmd = CMD_SLOW;
      } else {
        cmd = CMD_FORWARD;
      }

      xQueueSend(commandQueue, &cmd, 0);

      RobotStatus status = {dist, cmd, emergencyStop};
      xQueueOverwrite(statusQueue, &status);  // hamesha latest status rakhta hai
    }
  }
}

// ---------- Task 3: Motor ----------
// Command ke hisaab se PWM aur direction set karta hai
void motorTask(void *pv) {
  int cmd;
  while (true) {
    if (xQueueReceive(commandQueue, &cmd, portMAX_DELAY)) {
      if (cmd == CMD_FORWARD) {
        driveMotors(220, 220, true, true);
      } else if (cmd == CMD_SLOW) {
        driveMotors(120, 120, true, true);
      } else if (cmd == CMD_TURN) {
        driveMotors(180, 180, false, true);   // ek side ulta = turn
        vTaskDelay(pdMS_TO_TICKS(500));
      } else {
        driveMotors(0, 0, true, true);
      }
    }
  }
}

// ---------- Task 4: Telemetry ----------
// Har 500 ms me status Serial pe bhejta hai
void telemetryTask(void *pv) {
  RobotStatus status;
  while (true) {
    if (xQueuePeek(statusQueue, &status, 0)) {
      Serial.printf("{\"dist\":%.1f,\"cmd\":\"%s\",\"estop\":%s}\n",
                    status.distance,
                    cmdText[status.command],
                    status.stopped ? "true" : "false");
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// ---------- Task 5: Emergency stop ----------
// ISR se semaphore milne par stop ko on/off karta hai
void estopTask(void *pv) {
  while (true) {
    if (xSemaphoreTake(buttonSem, portMAX_DELAY) == pdTRUE) {
      emergencyStop = !emergencyStop;
      Serial.println(emergencyStop ? ">>> EMERGENCY STOP" : ">>> RESUMED");

      vTaskDelay(pdMS_TO_TICKS(200));   // debounce
      xSemaphoreTake(buttonSem, 0);     // is dauran aaye extra press hata do
    }
  }
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LEFT_DIR, OUTPUT);
  pinMode(RIGHT_DIR, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  ledcAttach(LEFT_PWM, 1000, 8);    // 1 kHz, 8-bit PWM
  ledcAttach(RIGHT_PWM, 1000, 8);

  distanceQueue = xQueueCreate(5, sizeof(float));
  commandQueue  = xQueueCreate(5, sizeof(int));
  statusQueue   = xQueueCreate(1, sizeof(RobotStatus));
  buttonSem     = xSemaphoreCreateBinary();

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

  xTaskCreate(sensorTask,    "Sensor",    2048, NULL, 3, NULL);
  xTaskCreate(controlTask,   "Control",   2048, NULL, 3, NULL);
  xTaskCreate(motorTask,     "Motor",     2048, NULL, 2, NULL);
  xTaskCreate(telemetryTask, "Telemetry", 3072, NULL, 1, NULL);
  xTaskCreate(estopTask,     "EStop",     2048, NULL, 4, NULL);
}

void loop() {
  // sab kaam tasks karte hain, yahan kuch nahi
}
