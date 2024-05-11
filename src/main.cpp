#include <Arduino.h>
#include <iostream>
#include <bits/stdc++.h> 
#include <random>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <string.h>
#include "WiFi.h"
#include <ESP32Servo.h>
#include <Keypad.h>


#define WIFI_NETWORK "dlink-9728"
#define WIFI_PASSWORD "kugxb48486"
#define WIFI_TIMEOUT_MS 10000

#define GreenLED 13
#define servopin 12
#define pushButton 4
#define RedLED 27
#define A1 34

int position = 0;
int bias = 40;
int keyIN = 0;

const int maxKey = 3;  // Maximum number of keys to detect
char detectedKeys[maxKey];  // Array to store detected keys
int keyCount = 0;  // Counter for detected keys

int keyVals[10] = {2080, 2520, 3030, 2280, 2650, 3150, 2400, 2770, 3210, 2860};
char keys[10] = {'1'   ,'2'  ,'3'  ,'4'  ,'5'  ,'6'  ,'7'  ,'8'  ,'9'  ,'0'};


//initialize list with predefined passwords
std::vector<std::string> passwords = {"1111", "2250"};

//define string to hold keyed in keys
String passkey[1];

Servo myservo; // creates servo object

void connectToWiFi(){
 
  Serial.print("Connecting to WiFi.....");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();

  while(WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS){
    Serial.print(".");
    delay(100);
  }
  if(WiFi.status() != WL_CONNECTED){
      Serial.print("\nConnection Failed :(");
  }
  else{
    Serial.print("\nWifi connection established");
    Serial.print("\nLocal ESP32 IP: ");
    Serial.println(WiFi.localIP());
  }
}

int generatePasscode(){
    // Use the current time as a seed for the random number generator
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);

    // Define a uniform distribution of integers from 1000 to 9999 (inclusive)
    std::uniform_int_distribution<> dis(1000, 9999);

    // Generate and print the random 4-digit number
    int fourDigitNumber = dis(gen);
    Serial.println(fourDigitNumber);

    return fourDigitNumber;
}

// create function for checking password
int passwordChecker(const std::string& userinput, const std::vector<std::string>& passwords){
    for (const auto& password : passwords){
        if (userinput == password){
            Serial.println("Access Granted");
            return 1;         
        }
    }
    Serial.println("Access denied, wrong password entered");
    return 0;
}

void setup() {
  Serial.begin(9600); // Start the serial communication
  delay(1000);        // Wait for serial port to initialize (useful for some boards)
  connectToWiFi();    // Function to connect to WiFi

  pinMode(pushButton, INPUT_PULLUP);
  pinMode(GreenLED, OUTPUT);
  pinMode(RedLED, OUTPUT);
  myservo.attach(12);  // Attach the servo to pin 12
  digitalWrite(GreenLED, LOW);
  digitalWrite(RedLED, LOW);
  myservo.write(90);   // Move servo to center position

  // Generate and add a temporary passcode
  int tempPasscode = generatePasscode();
  passwords.push_back(std::to_string(tempPasscode)); // Convert integer to string and add to vector

}

void loop() {
  if (keyCount < maxKey + 1) {
    keyIN = analogRead(A1); // Read the analog value
    for (int i = 0; i < 10; i++) {
      if (keyIN > keyVals[i] - bias && keyIN < keyVals[i] + bias) {
        detectedKeys[keyCount++] = keys[i];  // Store the key in the array
        Serial.print(keyIN);
        Serial.print(" "); 
        Serial.print("Key Pressed: ");
        Serial.println(keys[i]);  // Print the key pressed
        
        delay(200);
        digitalWrite(RedLED, HIGH);
        delay(1000);  // Delay to debounce and to limit the speed of input
        digitalWrite(RedLED, LOW);  
        break;
      }
    }
  } else {
    Serial.print("Detected Keys: ");
    passkey[0] = String(detectedKeys[0]) + detectedKeys[1] + detectedKeys[2] + detectedKeys[3];  //convert detected keys to string
    Serial.print(passkey[0]);
    Serial.println();
    keyCount = 0;
    delay(200);
    digitalWrite(GreenLED, HIGH);
    delay(1000);  // Long delay to stop further processing
    digitalWrite(GreenLED, LOW);

   
        
    // Pass integer to passwordChecker
    // Convert Arduino String to std::string
    std::string passkeyStd = passkey[0].c_str();

    // Call passwordChecker with the converted std::string
    if (passwordChecker(passkeyStd, passwords)) {
      digitalWrite(GreenLED, HIGH);
      delay(100); // Indicate access granted
      digitalWrite(GreenLED, LOW);
      delay(100);
      digitalWrite(GreenLED, HIGH);
      delay(100); // Indicate access granted
      digitalWrite(GreenLED, LOW);
    }
  }
  // if(digitalRead(pushButton) == LOW){
  //   digitalWrite(RedLED, LOW);
  //   delay(100);
  //   myservo.write(0); // moves servo far right, unlocked
  //   delay(200);
  //   digitalWrite(GreenLED, HIGH);
  //   delay(3000);
  // }
  // if(digitalRead(pushButton) == HIGH){
  //   digitalWrite(GreenLED, LOW);
  //   delay(100);
  //   myservo.write(180); // move all the way left, locked position
  //   delay(200);
  //   digitalWrite(RedLED, HIGH);
  //   delay(2000);
  // }
}