# ESP_TelnetServer

A lightweight Telnet server library for ESP32/ESP32-S3 using PlatformIO. Adds a Stream-compatible Telnet interface for remote logging and simple command I/O.

## Features

- Stream/Print-compatible API (use like `Serial`).
- Optional password authentication (`setPassword` / `clearPassword`).
- WiFi and optional Ethernet support (build-guarded with `USE_ETHERNET`).
- Non-blocking operation and single active-client model.

## Installation

1. Copy this library into your project's `lib/ESP_TelnetServer` directory.
2. Include the header in your code:

    ```cpp
    #include <ESP_TelnetServer.h>
    ```

## Usage

Basic example (no authentication):

```cpp
#include <ESP_TelnetServer.h>

ESP_TelnetServer telnet; // default port 23

void setup() {
  Serial.begin(115200);
  WiFi.begin("SSID", "PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(100);
  telnet.begin();
}

void loop() {
  telnet.loop();
  if (telnet.available()) {
    int b = telnet.read();
    // handle incoming byte
  }
  telnet.println("Hello from ESP32!");
  delay(1000);
}
```

Require a password (clients must authenticate before the Stream is exposed):

```cpp
telnet.setPassword("mypassword"); // prompt: "Password: " on connect
// later: telnet.clearPassword(); // disable auth
```

Behavior notes
- When a password is set the server sends a `Password: ` prompt to the client on connect and collects input (no echo). On successful match the client receives `OK` and the telnet stream becomes available. After 3 failed attempts the connection is closed.
- If no password is set, telnet behaves as a normal Stream immediately after connect.
- The library uses a single active client at a time (WiFi preferred, Ethernet if enabled and no WiFi client).

## Enabling Ethernet support

Ethernet code is compiled only when `USE_ETHERNET` is defined. Add the define in `platformio.ini` for the environment you build:

```ini
[env:esp32-usb]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
build_flags = -DUSE_ETHERNET
```

## API Reference

- `begin()`: Starts the Telnet server. Call in `setup()`.
- `loop()`: Poll in `loop()` to accept clients and process authentication.
- `setPassword(const char*)`: Require clients to authenticate with given password.
- `clearPassword()`: Disable authentication.
- `print()/println()/write()`: Use as with `Serial` — data is sent to the active, authenticated client.

## License

MIT License

## Credits

Inspired by various ESP32 Telnet server implementations.
