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
uint32_t last_photoresistor_check[n] = {0}; // Last time the photoresistor value was read, modified in loop
int last_servo_degrees[n] = {0};            // Last angle the servomotor was set to, modified in loop
int servo_released_degrees[n] = {0};        // Angle at which the servo's handle doesn't touch the screen, modified in setup
uint32_t temp_time = 0;                     // Temporary variable that stores time passed between servo presses, modified in loop
uint16_t photoresistor_value = 0;           // Temporary variable that stores values read by the photoresistor, modified in loop
#ifdef DEBUG
const int bufsize = 128; // Size of the buffer for storing data coming from Serial
char buffer[bufsize];    // Input buffer for receiving data from Serial
#endif

// ╔───────────────────────╗
// │Adjust parameters below│
// ╚───────────────────────╝

// If the measurement value read by photoresistor 'i' is lower than the *threshold* then the servo 'i' moves.
uint16_t thresholds[n] = {600, 600, 600, 600}; // Default {500,500,500,500}, Possible integer values 0-1023

// Starting value of delay between reading from the same photoresistor in miliseconds (1/1000 s).
uint32_t start_delay_ms = 10; // Default 100, Possible integer values >0

// The angular displacement ( released angle - pressed angle) for each servomotor's handle when a black tile is detected. Servos spin counterclockwise.
int servo_pressed_degrees_delta[n] = {5, -16, 16, -8}; // Default {5, -5, 5, -5}, Possible values such that always (released+delta) in [0°; 180°]
// ─────────────────────────

#ifdef DEBUG

int wait_upto_10s_for_input()
{
  uint32_t time_start = millis();
  while (Serial.available() == 0)
  {
    // wait up to 10s for input on serial
    if (time_start + 10000 < millis())
    {
      return 0;
    }
  }
  return 1;
}

void receive_values_of_parameters_from_serial()
{
  if (wait_upto_10s_for_input())
  {
    memset(buffer, 0, bufsize * sizeof(char)); // clear buffer contents
    Serial.readBytesUntil('\n', buffer, bufsize);
    switch (buffer[0])
    {
    case '.': // 'p' for photoresistor thresholds
      // . 200,600,200,600 ; 10 ; 19,-25,17,-15
      sscanf(buffer, ". %u,%u,%u,%u ; %lu ; %d,%d,%d,%d",
             &thresholds[0], &thresholds[1], &thresholds[2], &thresholds[3],
             &start_delay_ms,
             &servo_pressed_degrees_delta[0], &servo_pressed_degrees_delta[1],
             &servo_pressed_degrees_delta[2], &servo_pressed_degrees_delta[3]);
      break;
    default:
      return; // abandon filling other values
    }
  }
}

#endif

// Only run once
void setup()
{
#ifdef DEBUG
  // Opening serial port for calibration and debugging
  Serial.begin(115200);

  receive_values_of_parameters_from_serial();

  for (size_t i = 0; i < n; i++)
  {
    Serial.print(String(thresholds[i]) + " ");
  }

  Serial.print("\n" + String(start_delay_ms) + "\n");

  for (size_t i = 0; i < n; i++)
  {
    Serial.print(String(servo_pressed_degrees_delta[i]) + " ");
  }

  Serial.print("\n");

#endif

  // Fill program specific arrays with all 0
  memset(last_photoresistor_check, 0, n * sizeof(uint32_t));
  memset(last_servo_degrees, 0, n * sizeof(int));
  memset(servo_released_degrees, 0, n * sizeof(int));

  for (size_t i = 0; i < n; i++)
  {
    // Assingning Analog pins to read data from photoresistor
    pinMode(photoresistor_pin[i], INPUT);
    // Attaching servo pins to allow for control
    servos[i].attach(servo_pin[i]);
    // Setting the default position
    servo_released_degrees[i] = servos[i].read();
    // Release all servos
    servos[i].write(servo_released_degrees[i]);
  }

#ifdef DEBUG
  for (size_t i = 0; i < n; i++)
  {
    Serial.print(String(servo_released_degrees[i]) + " ");
  }
  Serial.print("\n");
#endif

  delay(1000);

  // Press down all servo handles at once
  for (size_t i = 0; i < n; i++)
  {
    servos[i].write(servo_released_degrees[i] + servo_pressed_degrees_delta[i]);
  }

#ifdef DEBUG
  // Print starting time
  Serial.println(millis());
#endif

  // Hold shortly
  delay(100);

  // Release all servo handles and begin loop
  for (size_t i = 0; i < n; i++)
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
    // is less than start_delay_ms
    if (((temp_time = millis()) - last_photoresistor_check[i]) > start_delay_ms)
    {
      last_photoresistor_check[i] = temp_time; // reset timer

#ifdef DEBUG
      Serial.print(String(temp_time) + " ");
#endif

      // check if the photoresistor `i` value is less than the threshold parameter
      // Note that the threshold value should be adjusted
      // In other words, this if clause detects black tiles
      if ((photoresistor_value = analogRead(photoresistor_pin[i])) < thresholds[i])
      {
        // Move the servomotor handle to press on the screen
        last_servo_degrees[i] = servo_released_degrees[i] + servo_pressed_degrees_delta[i];
      }
      else
      {
        // Move back the servomotor handle to release the tap
        last_servo_degrees[i] = servo_released_degrees[i];
      }

#ifdef DEBUG
      // Print the value read by photoresistor to the serial monitor
      Serial.print(String(i) + " " + String(photoresistor_value) + "\n");
#endif
    }

    // save last handle position until the next check
    servos[i].write(last_servo_degrees[i]);
  }
}