# ESP_TelnetServer

A lightweight Telnet server library for ESP32/ESP32-S3 using PlatformIO. Easily add Telnet-based debugging and remote command capabilities to your projects.

## Features

- Simple Telnet server for ESP32/ESP32-S3
- Handles multiple clients
- Easy integration with Arduino framework
- Non-blocking operation

## Installation

1. Copy this library into your project's `lib/ESP_TelnetServer` directory.
2. Include the header in your code:

    ```cpp
    #include <ESP_TelnetServer.h>
    ```

## Usage

```cpp
#include <ESP_TelnetServer.h>

ESP_TelnetServer telnetServer(23); // Default Telnet port

void setup() {
  Serial.begin(115200);
  WiFi.begin("SSID", "PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(100);
  telnetServer.begin();
}

void loop() {
  telnetServer.loop();
  telnetServer.println("Hello from ESP32!");
  delay(1000);
}
```

## API Reference

- `begin()`: Starts the Telnet server.
- `loop()`: Handles incoming client connections and data.
- `print()`, `println()`: Send data to all connected clients.

## License

MIT License

## Credits

Inspired by various ESP32 Telnet server implementations.