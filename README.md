# ESP8266-based Wake-On-LAN (WOL) Server

This project uses an ESP8266 microcontroller to create a Wake-On-LAN (WOL) server, allowing you to remotely wake up a device on your network. It integrates with Blynk for easy control and monitoring.

## Features
- Wake up a device on your network using a Magic Packet.
- Monitor device status, ping time, and WiFi signal strength.
- Control the WOL server via Blynk.
- Restart the ESP8266 or send a shutdown command to the target device.

## Hardware Required
- ESP8266 (e.g., NodeMCU)
- LED (optional, for status indication)

## Software Required
- Arduino IDE
- Blynk library
- ESP8266WiFi library
- WiFiUdp library
- ESP8266Ping library

## Getting Started

### 1. Clone the repository
```sh
git clone https://github.com/yourusername/wol-server.git
cd wol-server
