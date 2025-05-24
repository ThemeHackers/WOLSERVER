# 📡 ESP8266 Wake-on-LAN (WOL) Server with Blynk

This project allows you to remotely wake up a Windows PC on your local network using Wake-on-LAN (WOL) via an ESP8266 module and the **Blynk IoT** platform.

---

## 🚀 Features

- ✅ Sends a **Magic Packet** to wake up a Windows PC
- 📡 Displays real-time **WiFi signal strength (RSSI)** in the Blynk app
- 🔁 Remotely **restart the ESP8266**
- 💬 Real-time logs via **Blynk Terminal Widget**
- 🌐 Configures **static IP, DNS, and gateway**
- 📲 Blynk app UI integration using virtual pins

---

## 🧰 Hardware Requirements

- NodeMCU / ESP8266 Board
- USB Cable for flashing and power
- Mobile Device with [Blynk IoT App](https://blynk.io)

---

## 🔧 Software Requirements

- Arduino IDE
- ESP8266 Board Package installed
- Blynk Library installed
- Blynk Template with:
  - V1: Button (Wake-on-LAN)
  - V2: Value Display (WiFi Signal)
  - V4: Terminal
  - V6: Online Status Indicator
  - V7: Button (Restart ESP)

---

## 🔐 Configuration

### WiFi Credentials

```cpp
const char ssid[] = "YOUR_WIFI_SSID";
const char pass[] = "YOUR_WIFI_PASSWORD";
