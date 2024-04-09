#define DEBUG 1

#include <Arduino.h>
#include <Servo.h>

// Defining the count of sensors and motor pairs and their corresponding pins
// (depends on the layout)
constexpr size_t n = 4;                                     // count of sensor + servo pairs
constexpr uint16_t servo_pin[n] = {6, 9, 10, 11};           // servo motor pins (must be PWM~)
constexpr uint16_t photoresistor_pin[n] = {A0, A1, A2, A3}; // photoresitor pins (must be analog)

// Program specific variables and objects
Servo servos[n];
uint64_t last_lit[n] = {0, 0, 0, 0};
int last_servo_degrees[n] = {0, 0, 0, 0};
uint16_t temp_time = 0;
uint16_t photoresistor_value = 0;
// Change parameters depending on empirical measurements
constexpr uint16_t thresholds[n] = {500, 500, 500, 500};
constexpr uint64_t min_delay_ms = 100;
constexpr int servo_pressed_degrees = 45;
constexpr int servo_released_degrees = 0;

void setup()
{
#ifdef DEBUG
  // Opening serial port for calibration and debugging
  Serial.begin(9600);
#endif

  // Assingning Analog pins to read data from photoresistor
  pinMode(photoresistor_pin[0], INPUT);
  pinMode(photoresistor_pin[1], INPUT);
  pinMode(photoresistor_pin[2], INPUT);
  pinMode(photoresistor_pin[3], INPUT);

  // Attaching servo pins to allow for control
  servos[0].attach(servo_pin[0]);
  servos[1].attach(servo_pin[1]);
  servos[2].attach(servo_pin[2]);
  servos[3].attach(servo_pin[3]);
}

void loop()
{
  // For each pair `i` we check sequentially that ...
  for (size_t i = 0; i < n; i++)
  {

    // if the time since last value read for that specific photoresistor `i` was read
    // is less than min_delay_ms
    if (((temp_time = millis()) - last_lit[i]) > min_delay_ms)
    {
      last_lit[i] = temp_time; // reset timer

      // check if the photoresistor `i` value is less than the threshold parameter
      // Note that the threshold value should be adjusted
      // In other words, this if clause detects black tiles
      if ((photoresistor_value = analogRead(photoresistor_pin[i])) < thresholds[i])
      {

// Print the value read by photoresistor to the serial monitor
#ifdef DEBUG
        Serial.print(i);
        Serial.print(" ");
        Serial.print(photoresistor_value);
#endif

        // Move the servomotor handle to press on the screen (45° default)
        servos[i].write(servo_pressed_degrees);
        last_servo_degrees[i] = servo_pressed_degrees;
      }
      else
      {
        // Move back the servomotor handle to release the tap (0° default)
        servos[i].write(servo_released_degrees);
        last_servo_degrees[i] = servo_released_degrees;
      }
    }
    // save last handle position until the next check
    servos[i].write(last_servo_degrees[i]);
  }
}