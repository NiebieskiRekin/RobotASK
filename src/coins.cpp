// #define DEBUG 1

// #include <Arduino.h>

// // Defining the count of sensors and motor pairs and their corresponding pins
// // (depends on the layout)
// constexpr size_t n = 4;                                     // count of sensor + servo pairs
// constexpr uint16_t coin_pin[n] = {6, 9, 10, 11};           // servo motor pins (must be PWM~)
// constexpr uint16_t photoresistor_pin[n] = {A0, A1, A2, A3}; // photoresitor pins (must be analog)

// // Program specific variables and objects
// uint64_t last_lit[n] = {0, 0, 0, 0};
// int last_coin_voltage[n] = {0,0,0,0};
// uint16_t temp_time = 0;
// uint16_t photoresistor_value = 0;

// // Change parameters depending on empirical measurements
// constexpr uint16_t thresholds[n] = {500, 500, 500, 500};
// constexpr uint64_t min_delay_ms = 100;

// void setup()
// {
// #ifdef DEBUG
//   // Opening serial port for calibration and debugging
//   Serial.begin(9600);
// #endif

//   for (int i=0; i<n; i++){
//     // Assingning Analog pins to read data from photoresistor
//     pinMode(photoresistor_pin[i], INPUT);
//     // Assinging pins to coins to allow for screen control
//    pinMode(coin_pin[i],OUTPUT);
//   }

// }

// void loop()
// {
//   // For each pair `i` we check sequentially that ...
//   for (size_t i = 0; i < n; i++)
//   {

//     // if the time since last value read for that specific photoresistor `i` was read
//     // is less than min_delay_ms
//     if (((temp_time = millis()) - last_lit[i]) > min_delay_ms)
//     {
//       last_lit[i] = temp_time; // reset timer

//       // check if the photoresistor `i` value is less than the threshold parameter
//       // Note that the threshold value should be adjusted
//       // In other words, this if clause detects black tiles
//       if ((photoresistor_value = analogRead(photoresistor_pin[i])) < thresholds[i])
//       {

// // Print the value read by photoresistor to the serial monitor
// #ifdef DEBUG
//         Serial.print(i);
//         Serial.print(" ");
//         Serial.print(photoresistor_value);
// #endif

//         // Apply voltage to a coin
//         digitalWrite(coin_pin[i],HIGH);
//         last_coin_voltage[i] = HIGH;
//       }
//       else
//       {
//         // Drop the voltage to release the tap
//         digitalWrite(coin_pin[i],LOW);
//         last_coin_voltage[i] = LOW;
//       }
//     }
//     // save last position until the next check
//     digitalWrite(coin_pin[i],last_coin_voltage[i]);
//   }
// }
