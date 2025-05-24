#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiUdp.h>

#ifndef BLYNK_TEMPLATE_ID
#define BLYNK_TEMPLATE_ID "TMPL6nWnE8br_"
#endif
#ifndef BLYNK_TEMPLATE_NAME
#define BLYNK_TEMPLATE_NAME "WOL Server"
#define BLYNK_PRINT Serial
#endif

const char auth[] = "vLIUU3Alzcpa_lqUGCBYkoekUM0SvrXl";
const char ssid[] = "HOME65_2.4Gz";
const char pass[] = "59454199";

const IPAddress ip(192, 168, 1, 196);
const IPAddress gateway(192, 168, 1, 1);
const IPAddress subnet(255, 255, 255, 0);
const IPAddress dns(8, 8, 8, 8);
const IPAddress bcastAddr(192, 168, 1, 255);
byte macAddr_windows[6] = {0x00, 0xE0, 0x4C, 0x18, 0x87, 0xBA};

#define MAGIC_PACKET_LENGTH 102
#define PORT_WAKEONLAN 9
byte magicPacket[MAGIC_PACKET_LENGTH];
WiFiUDP udp;


#define WAKEONLAN_PIN V1
#define RESTART_PIN V7
#define STATUS_PIN V2        
#define TERMINAL_PIN V4
#define ONLINE_STATUS V6   

WidgetTerminal terminal(TERMINAL_PIN);

unsigned long lastStatusMillis = 0;
const unsigned long statusInterval = 10000;  

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.hostname("WOL-Server");
  WiFi.config(ip, dns, gateway, subnet);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Blynk.config(auth);
  while (!Blynk.connect()) {
    delay(500);
    Serial.print("~");
  }

  buildMagicPacket();
  udp.begin(6461);

  terminal.println(F("✅ WOL Server Initialized"));
  terminal.println("Device Name: WOL-Server");
  terminal.println("IP Address: " + WiFi.localIP().toString());
  terminal.println("WiFi Connected: " + String(ssid));
  terminal.flush();
}

void loop() {
  Blynk.run();

  if (millis() - lastStatusMillis >= statusInterval) {
    lastStatusMillis = millis();
    reportStatus();
  }
}

void buildMagicPacket() {
  memset(magicPacket, 0xFF, 6);
  for (int i = 1; i < 17; i++) {
    memcpy(&magicPacket[i * 6], macAddr_windows, 6);
  }
}

void sendMagicPacket() {
  if (udp.beginPacket(bcastAddr, PORT_WAKEONLAN)) {
    udp.write(magicPacket, MAGIC_PACKET_LENGTH);
    if (udp.endPacket()) {
      terminal.println("🚀 Magic Packet sent successfully (Wake-on-LAN)");
      Serial.println("Magic Packet sent");
    } else {
      terminal.println("❌ Failed to send Magic Packet (endPacket error)");
      Serial.println("Send failed");
    }
  } else {
    terminal.println("❌ Failed to start UDP transmission (beginPacket error)");
    Serial.println("UDP begin failed");
  }
  terminal.flush();
}

void reportStatus() {
  int rssi = WiFi.RSSI();
  String signalQuality;

  if (rssi >= -60) {
    signalQuality = "📶 Excellent";
  } else if (rssi >= -70) {
    signalQuality = "📶 Fair";
  } else {
    signalQuality = "⚠️ Weak";
  }

  Blynk.virtualWrite(STATUS_PIN, String(rssi) + " dBm (" + signalQuality + ")");
  Blynk.virtualWrite(ONLINE_STATUS, 1);   

  terminal.println("📡 WiFi RSSI: " + String(rssi) + " dBm - " + signalQuality);
  terminal.println("📍 IP: " + WiFi.localIP().toString());
  terminal.flush();
}

BLYNK_WRITE(WAKEONLAN_PIN) {
  if (param.asInt() == 1) {
    terminal.println("🛠️ Sending Magic Packet...");
    terminal.flush();
    sendMagicPacket();
  }
}

BLYNK_WRITE(RESTART_PIN) {
  if (param.asInt() == 1) {
    terminal.println("🔁 Restarting device...");
    terminal.flush();
    delay(1000);
    ESP.restart();
  }
}
