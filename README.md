# ESP32 Web-Based Servo Motor Control

This project allows you to control a servo motor using an ESP32 microcontroller and a web interface. The project is based on the ESPAsyncWebServer library and includes a keypad for entering passcodes to grant access. The webpage can only be accessed if connected to the same wifi network with the ESP32.

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Hardware Required](#hardware-required)
- [Software Required](#software-required)
- [Installation](#installation)
- [Usage](#usage)
- [Credits](#credits)
- [License](#license)

## Introduction
This project was designed by Oghenefegor Enwa, starting on January 5th, 2024, and completed on June 26th, 2024. It provides a way to control a servo motor using a web interface served by an ESP32. The project also includes a keypad input system as an added system functionality.

## Features
- Control a servo motor via a web interface.
- Use a keypad to enter passcodes.
- LED indicators for status.
- WiFi connectivity to access the web page.

## Hardware Required
- ESP32 microcontroller
- Servo motor
- Keypad
- LEDs
- Push button
- Breadboard and connecting wires

## Software Required
- PlatformIO IDE
- ESP32 Board Library for Arduino IDE
- ESPAsyncWebServer Library
- ESP32Servo Library
- Additional dependencies included in the code

## Credits
Rui Santos for webserver design and code.
Tutorial can be found here: https://randomnerdtutorials.com/esp32-async-web-server-espasyncwebserver-library/
