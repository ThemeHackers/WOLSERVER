# ESP8266-based Wake-On-LAN (WOL) Server

This project utilizes an ESP8266 microcontroller to create a Wake-On-LAN (WOL) server, which can be controlled via the Blynk app. The server can wake up a specified device on your network and provide real-time monitoring and control.

## Code Explanation

### Libraries
- **ESP8266WiFi**: Manages WiFi connectivity for the ESP8266.
- **BlynkSimpleEsp8266**: Allows integration with the Blynk app for remote control and monitoring.
- **WiFiUdp**: Provides UDP communication capabilities.
- **ESP8266Ping**: Allows the ESP8266 to send ping requests and measure response time.

### Definitions
- **BLYNK_TEMPLATE_ID, BLYNK_TEMPLATE_NAME**: Define the Blynk template ID and name.
- **auth**: Authentication token for connecting to the Blynk server.
- **ssid, pass**: Credentials for connecting the ESP8266 to your WiFi network.
- **host, port**: IP address and port number of the target device for sending shutdown commands.
- **ip, gateway, bcastAddr, subnet, dns, server_ip, device_ip_windows**: Network configuration details, including IP addresses and subnet mask.
- **macAddr_windows**: MAC address of the target device for the Wake-On-LAN packet.

### Pins
- **STATE_PIN**: Virtual pin for device status.
- **WAKEONLAN_PIN**: Virtual pin for the Wake-On-LAN button.
- **PING_TIME_PIN**: Virtual pin for displaying ping time.
- **RSSI_PIN**: Virtual pin for displaying WiFi signal strength.
- **RESTART_PIN**: Virtual pin for restarting the ESP8266.
- **LED_PIN**: GPIO pin for status LED.
- **PING_VIRTUAL_PIN**: Virtual pin for displaying ping time of a server.
- **SHUTDOWN_PIN**: Virtual pin for sending a shutdown command.

### Variables and Structures
- **WOLServerState**: Structure to store the current state of the WOL server, including online status, boot time, ping time, and LED status.
- **currentState**: Instance of `WOLServerState` to manage and track server state.

### Functions

- **setup()**: Initializes the ESP8266, connects to WiFi and Blynk, and sets up UDP communication for sending Wake-On-LAN packets.
  - **connectWiFi()**: Connects to the WiFi network and handles reconnection if necessary.
  - **connectBlynk()**: Connects to the Blynk server and handles reconnection if needed.
  - **buildMagicPacket()**: Constructs the Wake-On-LAN magic packet to wake the target device.

- **loop()**: Main function that runs continuously.
  - Reconnects to WiFi and Blynk if disconnected.
  - Checks and updates the ping time and device status.
  - Sends shutdown commands if requested.
  - Updates the LED state based on the current server status.

- **checkPing()**: Sends a ping request to a server and updates the ping time displayed in the Blynk app.

- **sendToTerminal()**: Sends a status message to the Blynk terminal widget, including device information, boot time, ping time, and WiFi RSSI.

- **buildMagicPacket()**: Generates the magic packet used for Wake-On-LAN by filling it with the MAC address of the target device.

- **blinkLED(int times, int interval)**: Blinks the status LED a specified number of times with a given interval.

### Blynk Virtual Pin Handlers

- **BLYNK_READ(STATE_PIN)**: Reads the device status and updates the virtual pin with the device's online status, boot time, and ping time. Updates button labels based on the current state.

- **BLYNK_WRITE(WAKEONLAN_PIN)**: Handles the Wake-On-LAN button press event. Sends the magic packet if the target device is offline and updates the boot time.

- **BLYNK_WRITE(RESTART_PIN)**: Restarts the ESP8266 when the restart button is pressed.

- **BLYNK_WRITE(SHUTDOWN_PIN)**: Sets the shutdown command flag when the shutdown button is pressed, triggering the shutdown command to be sent to the target device.

## Hardware Required
- ESP8266 (e.g., NodeMCU)
- LED (optional, for status indication)

## Software Required
- Arduino IDE
- Blynk library
- ESP8266WiFi library
- WiFiUdp library
- ESP8266Ping library
## Troubleshooting

### Common Issues

- **WiFi Connection Issues**: If you are experiencing problems connecting to WiFi, ensure that your WiFi credentials are correct and that the ESP8266 is within range of your router. Also, verify that the WiFi network is functioning properly.

- **Blynk Connection Issues**: If the ESP8266 is unable to connect to the Blynk server, check that the Blynk authentication token is correctly configured and that the Blynk server is accessible from your network.

- **Magic Packet Not Working**: If the Wake-On-LAN magic packet is not waking up the target device, ensure that the MAC address and IP address of the device are correctly set. Additionally, make sure that Wake-On-LAN is enabled in the BIOS settings and network adapter settings of the target device.

### Error Handling

In case you encounter errors or issues not covered above, please feel free to reach out for support. We apologize for any inconvenience caused by these issues and appreciate your patience as we work to address them. Contributions and feedback are always welcome to help improve the project.
