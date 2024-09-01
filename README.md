# ESP8266 Wake-On-LAN (WOL) Server with Blynk

This project sets up an ESP8266 as a Wake-On-LAN (WOL) server that interacts with the Blynk IoT platform. The server can send a magic packet to wake up a Windows device on the network, check the device's status via ping, and manage the server's state, including restart and shutdown functionalities.

## Features

- **Wake-On-LAN Functionality**: Sends a magic packet to wake up a specific device on the local network.
- **Ping Monitoring**: Pings a specified DNS server (default: `8.8.4.4`) and a remote server (`1.1.1.1`) to check network connectivity and device status.
- **Blynk Integration**: Displays real-time server status and controls the server through the Blynk app.
- **Automatic Reconnection**: Attempts to reconnect to Wi-Fi and Blynk if the connection is lost.
- **LED Indicator**: An LED connected to the ESP8266 blinks to indicate various states like connection status, WOL packet sent, etc.
- **Restart and Shutdown Control**: The server can be restarted or send a shutdown command to the target device via Blynk.

## Hardware Requirements

- **ESP8266 Module**: A microcontroller with built-in Wi-Fi capabilities.
- **LED**: Connected to GPIO pin 16, used as an indicator for various statuses.
- **Blynk App**: A Blynk account and a corresponding app project with virtual pins configured.

## Software Requirements

- **Arduino IDE**: With the ESP8266 board manager installed.
- **Blynk Library**: Installed via Arduino Library Manager.
- **ESP8266Ping Library**: For ping functionality.
- **WiFiUdp Library**: For sending UDP packets (Magic Packet).

## Configuration

1. **Blynk Authentication**: Replace the `auth[]` string with your Blynk authentication token.
2. **Wi-Fi Credentials**: Update the `ssid[]` and `pass[]` with your Wi-Fi network's SSID and password.
3. **Device IP and MAC Address**: Configure the IP address and MAC address of the target Windows device you wish to wake up.
4. **Local IP Configuration**: Set the ESP8266's static IP, subnet mask, gateway, and DNS server.

## Blynk Virtual Pins

- `V0` - Server state (Online/Offline).
- `V1` - Wake-On-LAN button.
- `V2` - Ping time to DNS server.
- `V3` - Wi-Fi RSSI (Signal Strength).
- `V4` - Terminal widget for debugging and status messages.
- `V5` - Ping time to `1.1.1.1`.
- `V6` - Shutdown button.
- `V7` - Restart button.

## Usage

1. **Setup**: Upload the code to your ESP8266 module using the Arduino IDE.
2. **Blynk App**: Create a Blynk project with the virtual pins as described above. Link the project to your ESP8266 using the authentication token.
3. **Operation**:
   - Use the Blynk app to monitor the ESP8266's status.
   - Press the WOL button to send a magic packet and wake up the target device.
   - Monitor the ping times to ensure network connectivity.
   - Restart or shut down the ESP8266 server via the Blynk app.

## Code Overview

- **`setup()`**: Initializes Wi-Fi, Blynk, and UDP for WOL. It also starts the LED indicator and checks connections.
- **`connectWiFi()`**: Handles Wi-Fi connection with automatic retries and restart if the connection fails.
- **`connectBlynk()`**: Manages Blynk connection with retry logic and auto-restart upon failure.
- **`loop()`**: Manages the main logic, including LED blinking, checking connections, sending pings, and handling Blynk commands.
- **`sendMagicPacket()`**: Sends the WOL magic packet to the target device.
- **`buildMagicPacket()`**: Constructs the magic packet based on the target device's MAC address.
- **`checkPing()` and `pingServer()`**: Functions to ping the DNS and server IPs and report the results to Blynk.

## Notes

- The ESP8266 will automatically restart if it fails to connect to Wi-Fi or Blynk after multiple attempts.
- The `LED_PIN` is used to indicate the connection status and blink when the magic packet is sent.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
