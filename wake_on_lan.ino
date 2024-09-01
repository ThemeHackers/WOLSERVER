#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiUdp.h>
#include <ESP8266Ping.h>

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
const char* host = "192.168.1.120";
const uint16_t port = 8080;

bool shutdownCommand = false;
const IPAddress ip(192, 168, 1, 196);
const IPAddress gateway(192, 168, 1, 1);
const IPAddress bcastAddr(192, 168, 1, 255);
const IPAddress subnet(255, 255, 255, 0);
const IPAddress dns(8, 8, 4, 4);
const IPAddress server_ip(1, 1, 1, 1);
const IPAddress device_ip_windows(192, 168, 1, 174);
byte macAddr_windows[6] = { 0x00, 0xE0, 0x4C, 0x18, 0x87, 0xBA };

const char device_name[] = "DESKTOP-ABVD0QL";
const uint16_t boot_time = 45;

#define MAGIC_PACKET_LENGTH 102
#define PORT_WAKEONLAN 9
byte magicPacket[MAGIC_PACKET_LENGTH];
unsigned int localPort = 6461;

WiFiUDP udp;

#define STATE_PIN V0
#define WAKEONLAN_PIN V1
#define PING_TIME_PIN V2
#define RSSI_PIN V3
#define RESTART_PIN V7
#define LED_PIN 16 
#define PING_VIRTUAL_PIN V5
#define SHUTDOWN_PIN V6 

WidgetTerminal terminal(V4);

struct WOLServerState {
    bool IsOnline;
    uint16_t boot_time;
    bool boot_error;
    uint16_t ping;
    uint32_t previousMillis;
    uint32_t interval;
    bool ledState;
    uint32_t ledOffTime;
};
WOLServerState currentState = { false, 0, false, 0, 0, 5000UL, false, 0 };

uint32_t ledBlinkInterval = 500;
uint32_t previousLedMillis = 0;

void setup() {
    Blynk.begin(auth, ssid, pass);
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    connectWiFi();
    connectBlynk();

    if (udp.begin(localPort) == 1) {
        BLYNK_LOG("UDP begin OK");
        buildMagicPacket();
    } else {
        delay(500);
        ESP.restart();
    }
}

void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.hostname("WOL server");
    WiFi.config(ip, dns, gateway, subnet);
    WiFi.begin(ssid, pass);

    int count = 0;
    while (WiFi.status() != WL_CONNECTED && count <= 40) {
        delay(500);
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        count++;
    }
    if (WiFi.status() != WL_CONNECTED) {
        delay(5000);
        ESP.restart();
    }
}

void connectBlynk() {
    Blynk.config(auth);
    Blynk.disconnect();

    int count = 0;
    Serial.println("Attempting to connect to Blynk Cloud...");
    while (Blynk.connect(10000) == false && count <= 20) {
        delay(500);
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        count++;
    }
    if (!Blynk.connected()) {
        Serial.println("Failed to connect to Blynk Cloud, restarting...");
        delay(500);
        ESP.restart();
    }

    Serial.println("Blynk connected successfully!");
    BLYNK_LOG("Blynk connected\n");
    terminal.println("Blynk connecting...");
    delay(2000);
    terminal.println("Starting the server...");
    delay(5000);
    terminal.println("Server is available...");
    delay(500);
}

void loop() {
    uint32_t currentMillis = millis();

    if (currentMillis - previousLedMillis >= ledBlinkInterval) {
        previousLedMillis = currentMillis;
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));  
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected, attempting to reconnect...");
        connectWiFi();
        return;
    }

    if (!Blynk.connected()) {
        Serial.println("Blynk disconnected, attempting to reconnect...");
        connectBlynk();
        return;
    }

    if (currentMillis - currentState.previousMillis >= currentState.interval) {
        currentState.previousMillis = currentMillis;

        if (currentState.boot_time > 0) {
            currentState.boot_time--;
            if (currentState.boot_time == 0) {
                currentState.boot_error = true;
            }
        }

        checkPing(); 
        pingServer(); // Ping 1.1.1.1 and send the result to Blynk

        Blynk.virtualWrite(RSSI_PIN, WiFi.RSSI());

        if (currentState.IsOnline) {
            currentState.boot_error = false;
            currentState.boot_time = 0;
        }

        if (currentState.ledState && (currentMillis - currentState.ledOffTime >= 5000UL)) {
            digitalWrite(LED_PIN, LOW);
            currentState.ledState = false;
        }
    }

    Blynk.run();

    if (shutdownCommand) {
        WiFiClient client;
        if (client.connect(host, port)) {
            client.print("shutdown");
            Serial.println("Shutdown command sent");
        } else {
            Serial.println("Connection failed");
        }
        delay(10000);
        shutdownCommand = false;
    }
}

void checkPing() {
    bool pingResult = Ping.ping(dns, 1);
    currentState.ping = pingResult ? Ping.averageTime() : 0;

    Serial.print("Ping to ");
    Serial.print(dns);
    Serial.print(" : ");
    if (pingResult) {
        Serial.print(currentState.ping);
        Serial.println(" ms");
    } else {
        Serial.println("Failed");
    }

    Blynk.virtualWrite(PING_TIME_PIN, currentState.ping);

    currentState.IsOnline = pingResult;
}

void pingServer() {
    IPAddress pingIP(1, 1, 1, 1);
    bool pingResult = Ping.ping(pingIP, 1);
    uint16_t pingTime = pingResult ? Ping.averageTime() : 0;

    Serial.print("Ping to 1.1.1.1: ");
    if (pingResult) {
        Serial.print(pingTime);
        Serial.println(" ms");
    } else {
        Serial.println("Failed");
    }

    Blynk.virtualWrite(PING_VIRTUAL_PIN, pingTime);
}

void sendMagicPacket() {
    udp.beginPacket(bcastAddr, PORT_WAKEONLAN);
    udp.write(magicPacket, MAGIC_PACKET_LENGTH);
    if (udp.endPacket() == 1) {
        terminal.println("Magic Packet sent successfully.");
        currentState.ledState = true;
        digitalWrite(LED_PIN, HIGH);
        currentState.ledOffTime = millis();
    } else {
        Serial.println("Failed to send Magic Packet.");
        terminal.println("Failed to send Magic Packet.");
    }
    terminal.flush();
}

void buildMagicPacket() {
    memset(magicPacket, 0xFF, 6);
    for (int i = 1; i < 17; i++) {
        memcpy(&magicPacket[i * 6], macAddr_windows, 6);
    }
}

uint32_t countdownStart = 0;
const uint32_t countdownDuration = 15000;

BLYNK_WRITE(WAKEONLAN_PIN) {
    if (!currentState.IsOnline && currentState.boot_time == 0) {
        int value = param.asInt();
        BLYNK_LOG("AppButtonWakeOnLan: value=%d", value);
        if (value == 1) {
            sendMagicPacket();
            currentState.boot_time = boot_time;
            currentState.interval = 1000UL;
            countdownStart = millis(); 
            Serial.println("Magic Packet sent, boot timer started.");
        }
    }
}

BLYNK_WRITE(RESTART_PIN) {
    if (param.asInt() == 1) {
        ESP.restart(); 
    }
}

BLYNK_WRITE(SHUTDOWN_PIN) {
    if (param.asInt() == 1) {
        Serial.println("Shutdown command received");
        shutdownCommand = true;
    }
}
