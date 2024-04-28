#define DEBUG 1

#include <Arduino.h>
#include <Servo.h>

// Defining the count of sensors and motor pairs and their corresponding pins
// (Change only if the layout of the board has changed)
constexpr size_t n = 4;                                     // count of sensor + servo pairs
constexpr uint16_t servo_pin[n] = {6, 9, 10, 11};           // servo motor pins (must be PWM~)
constexpr uint16_t photoresistor_pin[n] = {A0, A1, A2, A3}; // photoresitor pins (must be analog)

// Program specific variables and objects
Servo servos[n];                            // Servo object used by the Servo library
uint64_t last_photoresistor_check[n] = {0}; // Last time the photoresistor value was read, modified in loop
int last_servo_degrees[n] = {0};            // Last angle the servomotor was set to, modified in loop
int servo_released_degrees[n] = {0};        // Angle at which the servo's handle doesn't touch the screen, modified in setup
uint16_t temp_time = 0;                     // Temporary variable that stores time passed between servo presses, modified in loop
uint16_t photoresistor_value = 0;           // Temporary variable that stores values read by the photoresistor, modified in loop

// ╔───────────────────────╗
// │Adjust parameters below│
// ╚───────────────────────╝

// If the measurement value read by photoresistor 'i' is lower than the *threshold* then the servo 'i' moves.
constexpr uint16_t thresholds[n] = {500, 500, 500, 500}; // Default {500,500,500,500}, Possible integer values 0-1023

// Minimum value of delay between reading from the same photoresistor in miliseconds (1/1000 s).
constexpr uint64_t min_delay_ms = 100; // Default 100, Possible integer values >0

// The angular displacement ( released angle - pressed angle) for each servomotor's handle when a black tile is detected. Servos spin counterclockwise.
constexpr int servo_pressed_degrees_delta[n] = {-45, 30, -30, 45}; // Default {-45, 30, -30, 45}, Possible values such that always (released+delta) in [0°; 180°]

// ─────────────────────────

// Only run once
void setup()
{
#ifdef DEBUG
  // Opening serial port for calibration and debugging
  Serial.begin(9600);
#endif

  // Fill program specific arrays with all 0
  memset(last_photoresistor_check, 0, n * sizeof(uint64_t));
  memset(last_servo_degrees, 0, n * sizeof(int));
  memset(servo_released_degrees, 0, n * sizeof(int));

  delay(5000); // wait 5s

  for (size_t i = 0; i < n; i++)
  {
    // Assingning Analog pins to read data from photoresistor
    pinMode(photoresistor_pin[i], INPUT);
    // Attaching servo pins to allow for control
    servos[i].attach(servo_pin[i]);
    // Setting the default position
    servo_released_degrees[i] = servos[i].read();
#ifdef DEBUG
    // Print the value read by servo to the serial monitor
    Serial.println("Servo " + String(i) + "default position: " + photoresistor_value);
#endif
    // Release all servos
    servos[i].write(servo_released_degrees[i]);
  }

  delay(5000); // wait 5s

  // Press down all servo handles at once
  for (int i = 0; i < n; i++)
  {
    servos[i].write(servo_released_degrees[i] + servo_pressed_degrees_delta[i]);
  }

  // Hold shortly
  delay(100);

  // Release all servo handles and begin loop
  for (int i = 0; i < n; i++)
  {
    servos[i].write(servo_released_degrees[i]);
  }
  delay(100);
}

// Run indefinitely in a loop
void loop()
{

  // For each pair `i` we check sequentially that ...
  for (size_t i = 0; i < n; i++)
  {

    // if the time since last value read for that specific photoresistor `i` was read
    // is less than min_delay_ms
    if (((temp_time = millis()) - last_photoresistor_check[i]) > min_delay_ms)
    {
      last_photoresistor_check[i] = temp_time; // reset timer

      // check if the photoresistor `i` value is less than the threshold parameter
      // Note that the threshold value should be adjusted
      // In other words, this if clause detects black tiles
      if ((photoresistor_value = analogRead(photoresistor_pin[i])) < thresholds[i])
      {

#ifdef DEBUG
        // Print the value read by photoresistor to the serial monitor
        Serial.println("Photores " + String(i) + ": " + String(photoresistor_value));
#endif

        // Move the servomotor handle to press on the screen
        last_servo_degrees[i] = servo_released_degrees[i] + servo_pressed_degrees_delta[i];
      }
      else
      {
        // Move back the servomotor handle to release the tap
        last_servo_degrees[i] = servo_released_degrees[i];
      }
    }
    // save last handle position until the next check
    servos[i].write(last_servo_degrees[i]);
  }
}