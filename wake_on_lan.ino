#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiUdp.h>
#include <ESP8266Ping.h>

// Define Blynk template ID and name if not already defined
#ifndef BLYNK_TEMPLATE_ID
#define BLYNK_TEMPLATE_ID "YOUR_BLYNK_TEMPLATE_ID"
#endif
#ifndef BLYNK_TEMPLATE_NAME
#define BLYNK_TEMPLATE_NAME "WOL Server"
#define BLYNK_PRINT Serial
#endif

// Blynk authentication and WiFi credentials
const char auth[] = "YOUR_AUTH"; // Authentication in Blynk server
const char ssid[] = "WiFi name for ESP8266 internet connection"; // Your SSID to connect to the internet
const char pass[] = "YOUR WiFi Password"; // Your WiFi password

// Host and port for Wake-On-LAN device
const char* host = "IP WOL device"; // Static IP address of the WOL device
const uint16_t port = 8080; // Port for WOL

// Network configuration
bool shutdownCommand = false;
const IPAddress ip(192, 168, 1, 128);
const IPAddress gateway(192, 168, 1, 1);
const IPAddress bcastAddr(192, 168, 1, 255);
const IPAddress subnet(255, 255, 255, 0);
const IPAddress dns(8, 8, 8, 8);
const IPAddress server_ip(1, 1, 1, 1);
const IPAddress device_ip_windows(192, 168, 1, 120);

// MAC address of the Windows device to wake
byte macAddr_windows[6] = { 0x00, 0xE0, 0x4C, 0x18, 0x87, 0xBA };

// Alert configuration
const char device_name[] = "DESKTOP-ABVD0QL";
const uint16_t boot_time = 45; // Boot time in seconds

#define MAGIC_PACKET_LENGTH 102 // Magic packet length
#define PORT_WAKEONLAN 6461 // Port for sending Wake-On-LAN magic packet
byte magicPacket[MAGIC_PACKET_LENGTH];
unsigned int localPort = 6461;

WiFiUDP udp; // UDP instance

// Blynk virtual pin definitions
#define STATE_PIN V0
#define WAKEONLAN_PIN V1
#define PING_TIME_PIN V2
#define RSSI_PIN V3
#define RESTART_PIN V7
#define LED_PIN 16 
#define PING_VIRTUAL_PIN V5
#define SHUTDOWN_PIN V6 

// Terminal widget for Blynk
WidgetTerminal terminal(V4);

// State structure to hold server state
struct WOLServerState {
    bool IsOnline; // Is the WOL device online
    uint16_t boot_time; // Boot time counter
    bool boot_error; // Error during boot
    uint16_t ping; // Ping time
    uint32_t previousMillis; // Previous millis for interval timing
    uint32_t interval; // Interval for updating state
    bool ledState; // LED state
    uint32_t ledOffTime; // Time to turn off the LED
};
WOLServerState currentState = { false, 0, false, 0, 0, 5000UL, false, 0 }; // Initial state

void setup() {
    // Initialize Blynk and WiFi
    Blynk.begin("vLIUU3Alzcpa_lqUGCBYkoekUM0SvrXl","HOME65_2.4Gz","59454199");
    #ifdef DEBUG
    Serial.begin(115200);
    #endif

    pinMode(LED_PIN, OUTPUT); // Set LED pin as output
    digitalWrite(LED_PIN, LOW); // Ensure LED is off initially

    connectWiFi(); // Connect to WiFi
    connectBlynk(); // Connect to Blynk

    // Initialize UDP
    if (udp.begin(localPort) == 1) {
        BLYNK_LOG("UDP begin OK");
        buildMagicPacket(); // Build the magic packet for WOL
    } else {
        delay(500);
        ESP.restart(); // Restart if UDP initialization fails
    }
}

void connectWiFi() {
    // Configure and connect to WiFi
    WiFi.mode(WIFI_STA);
    WiFi.hostname("WOL server");
    WiFi.config(ip, dns, gateway, subnet);
    WiFi.begin(ssid, pass);

    int count = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(250);
        digitalWrite(LED_BUILTIN, LOW);

        count++;
        if (count > 40) {
            delay(5000);
            ESP.restart(); // Restart if WiFi connection fails after multiple attempts
        }
    }

    blinkLED(2, 500); // Blink LED 2 times when WiFi is connected
}

void connectBlynk() {
    // Connect to Blynk
    Blynk.config(auth);
    Blynk.disconnect();

    int count = 0;
    Serial.println("Attempting to connect to Blynk Cloud...");
    while (Blynk.connect(10000) == false) {
        delay(250);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(250);
        digitalWrite(LED_BUILTIN, LOW);

        count++;
        if (count > 20) {
            Serial.println("Failed to connect to Blynk Cloud, restarting...");
            delay(500);
            ESP.restart(); // Restart if Blynk connection fails after multiple attempts
        }
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
    // Reconnect WiFi if needed
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected, attempting to reconnect...");
        connectWiFi();
        return;
    }

    // Reconnect to Blynk Cloud if needed
    if (!Blynk.connected()) {
        Serial.println("Blynk disconnected, attempting to reconnect...");
        connectBlynk();
        return;
    }

    uint32_t currentMillis = millis();
    if (currentMillis - currentState.previousMillis >= currentState.interval) {
        currentState.previousMillis = currentMillis;

        if (currentState.boot_time == 0) {
            currentState.interval = 5000UL;
        } else {
            currentState.boot_time--;
            if (currentState.boot_time == 0) {
                currentState.boot_error = true;
            }
        }

        checkPing(); // Check server ping
        currentState.ping = Ping.ping(device_ip_windows, 1) ? Ping.averageTime() : 0; // Get ping time

        Blynk.virtualWrite(PING_TIME_PIN, currentState.ping); // Write ping time to Blynk
        Blynk.virtualWrite(RSSI_PIN, WiFi.RSSI()); // Write WiFi RSSI to Blynk

        currentState.IsOnline = Ping.ping(device_ip_windows, 1); // Check if device is online
        if (currentState.IsOnline) {
            currentState.boot_error = false;
            currentState.boot_time = 0;
        } else {
            currentState.IsOnline = false;
        }

        // Turn off LED if needed
        if (currentState.ledState && (currentMillis - currentState.ledOffTime >= 5000UL)) {
            digitalWrite(LED_PIN, LOW);
            currentState.ledState = false;
        }
    }

    Blynk.run(); // Run Blynk

    // Handle shutdown command
    if (shutdownCommand) {
        WiFiClient client;

        if (!client.connect(host, port)) {
            Serial.println("Connection failed");
            delay(5000);
            return;
        }

        client.print("shutdown"); // Send shutdown command
        Serial.println("Shutdown command sent");
        
        // Wait for a certain period before allowing another shutdown command
        delay(10000); // Adjust this delay as needed
        shutdownCommand = false;
    }
}

void checkPing() {
    // Check ping to the server and update Blynk
    bool pingResult = Ping.ping(server_ip, 1);
    uint32_t pingTime = pingResult ? Ping.averageTime() : 0;
    Blynk.virtualWrite(PING_VIRTUAL_PIN, pingTime);
}

void sendToTerminal() {
    // Send current state to the Blynk terminal
    String message = "Current Millis: " + String(millis()) + "\n";
    message += "Boot Time: " + String(currentState.boot_time) + "\n";
    message += "Ping: " + String(currentState.ping) + "\n";
    message += "RSSI: " + String(WiFi.RSSI()) + "\n";
    message += "Is Online: " + String(currentState.IsOnline ? "Yes" : "No") + "\n";

    terminal.println(message);
    terminal.flush();
}

// Generate magic packet for Wake-On-LAN
void buildMagicPacket() {
    memset(magicPacket, 0xFF, 6);

    for (int i = 1; i < 17; i++) {
        memcpy(&magicPacket[i * 6], macAddr_windows, 6);
    }
}

void blinkLED(int times, int interval) {
    // Blink LED specified number of times with specified interval
    for (int i = 0; i < times; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(interval);
        digitalWrite(LED_PIN, LOW);
        delay(interval);
    }
}

uint32_t countdownStart = 0;
const uint32_t countdownDuration = 15000;

// Blynk read function for state pin
BLYNK_READ(STATE_PIN) {
    Blynk.virtualWrite(RSSI_PIN, WiFi.RSSI());
    Blynk.virtualWrite(PING_TIME_PIN, currentState.ping);

    String deviceInfo = String(device_name) + " (" + WiFi.localIP().toString() + ")";

    if (currentState.IsOnline) {
        Blynk.virtualWrite(STATE_PIN, deviceInfo + " is Online");
        Blynk.setProperty(WAKEONLAN_PIN, "offLabel", deviceInfo + " running...");
        Blynk.setProperty(WAKEONLAN_PIN, "onLabel", deviceInfo + " running...");
    } else if (!currentState.IsOnline && currentState.boot_time > 0) {
        Blynk.virtualWrite(STATE_PIN, "Waiting for ping...");
        Blynk.setProperty(WAKEONLAN_PIN, "offLabel", String(currentState.boot_time));
        Blynk.setProperty(WAKEONLAN_PIN, "onLabel", "Waiting for ping...");
    } else if (!currentState.IsOnline && currentState.boot_time == 0 && currentState.boot_error) {
        Blynk.virtualWrite(STATE_PIN, "Oops! Something happened, Try It Again!");
        Blynk.setProperty(WAKEONLAN_PIN, "offLabel", "Try It Again");
        Blynk.setProperty(WAKEONLAN_PIN, "onLabel", "Magic Packet has been sent");
    } else if (currentState.boot_time > 0 && millis() - countdownStart < countdownDuration) {
        uint32_t remainingTime = countdownDuration - (millis() - countdownStart);
        Blynk.virtualWrite(STATE_PIN, "Countdown: " + String(remainingTime / 1000) + "s");
        Blynk.setProperty(WAKEONLAN_PIN, "offLabel", String(currentState.boot_time));
        Blynk.setProperty(WAKEONLAN_PIN, "onLabel", "Waiting for ping...");
    } else {
        currentState.boot_time = 0;
        currentState.boot_error = false;
        Blynk.virtualWrite(STATE_PIN, deviceInfo + " is Offline");
        Blynk.setProperty(WAKEONLAN_PIN, "offLabel", "Turn On");
        Blynk.setProperty(WAKEONLAN_PIN, "onLabel", "Magic Packet has been sent");
    }
}

// Blynk write function for wake-on-LAN pin
BLYNK_WRITE(WAKEONLAN_PIN) {
    if (!currentState.IsOnline && currentState.boot_time == 0) {
        int value = param.asInt();
        BLYNK_LOG("AppButtonWakeOnLan: value=%d", value);
        if (value == 1) {
            udp.beginPacket(bcastAddr, PORT_WAKEONLAN);
            udp.write(magicPacket, MAGIC_PACKET_LENGTH);
            if (udp.endPacket() == 1) {
                terminal.println("Magic Packet sent successfully.");
                blinkLED(3, 1000); // Blink LED 3 times for 1 second each when Magic Packet is sent
                currentState.ledState = true;
                digitalWrite(LED_PIN, HIGH); // Turn LED on
                currentState.ledOffTime = millis(); // Track time when LED should turn off
            } else {
                Serial.println("Failed to send Magic Packet.");
                terminal.println("Failed to send Magic Packet.");
            }
            terminal.flush();

            currentState.boot_time = boot_time;
            currentState.interval = 1000UL;
            countdownStart = millis(); 
        }
    }
}

// Blynk write function for restart pin
BLYNK_WRITE(RESTART_PIN) {
    int value = param.asInt(); 

    if (value == 1) {
        ESP.restart(); // Restart ESP8266 if button is pressed
    }
}

// Blynk write function for shutdown pin
BLYNK_WRITE(SHUTDOWN_PIN) {
    int value = param.asInt();

    if (value == 1) {
        Serial.println("Shutdown command received");
        shutdownCommand = true; // Set shutdown command flag
    }
}
