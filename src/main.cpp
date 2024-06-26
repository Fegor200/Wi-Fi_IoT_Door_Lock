/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-async-web-server-espasyncwebserver-library/
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include <Arduino.h>
#include <iostream>

#include <bits/stdc++.h> 
#include <random>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <string.h>

#include <ESP32Servo.h>
#include <Keypad.h>
#include <SPI.h>
#include "FS.h"

#include "WiFi.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#define LED 2
//set WiFi networ ID and password
#define WIFI_NETWORK "dlink-9728"
#define WIFI_PASSWORD "kugxb48486"

// #define WIFI_NETWORK1 "TELUS0377"
// #define WIFI_PASSWORD1 "yn99sbdzpd"

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
char keys[10]   = {'1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , '0'};

//initialize list with predefined passwords
std::vector<std::string> passwords = {"1111", "2250"};

// create hased based pasword management for better security/password management

//define string to hold keyed in keys
String passkey[1];

// creates servo object
Servo myservo; 

//webserver object
AsyncWebServer server(80);

// Variable to store the HTTP request
String header;

// Decode HTTP GET value
String valueString = String(0);
int pos1 = 0;
int pos2 = 0;

// define parameters to be passed to the URL (GPIO number and states)
const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

// webpage
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>ESP Web Server</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

String outputState(int output){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
}

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<h4>Board Led</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\" " + outputState(LED) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Servo Lock</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"12\" " + outputState(servopin) + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  return String();
}


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

  //define hardware modes
  pinMode(pushButton, INPUT_PULLUP);
  pinMode(GreenLED, OUTPUT);
  pinMode(RedLED, OUTPUT);
  pinMode(LED, OUTPUT);
  myservo.attach(servopin);  // Attach the servo to pin 12

  digitalWrite(GreenLED, LOW);
  digitalWrite(RedLED, LOW);
  digitalWrite(LED, LOW);
  myservo.write(90);   // Move servo to center position


  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>}
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());
      if(inputMessage2.toInt() == 1){
        myservo.write(180);
      }
      else if(inputMessage2.toInt() == 0){
        myservo.write(0);
      }
      else{
        
      }
    }
    else {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
    }
    request->send(200, "text/plain", "OK");
  });
  

  // Start server
  server.begin();
  Serial.println("HTTP server started");

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
}