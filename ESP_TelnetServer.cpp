#include "ESP_TelnetServer.h"

#include <Arduino.h>
#include <Ethernet.h>
#include <WiFi.h>
#include <Client.h>

static const uint16_t DEFAULT_TELNET_PORT = 23;

class ESP_TelnetServer::Impl
{
public:
    Impl(uint16_t p)
        : port(p), wifiServer(p), ethServer(p)
    {
    }

    class MyEthernetServer : public EthernetServer
    {
    public:
        MyEthernetServer(uint16_t p) : EthernetServer(p) {}
        virtual void begin(uint16_t port = 0) override { EthernetServer::begin(); }
    };

    uint16_t port;
    WiFiServer wifiServer;
    MyEthernetServer ethServer;
    WiFiClient wifiClient;
    EthernetClient ethClient;
    // No internal command handling — this Impl only stores servers/clients.
};

ESP_TelnetServer::ESP_TelnetServer(uint16_t port)
{
    pimpl = new Impl(port);
}

void ESP_TelnetServer::begin()
{
    pimpl->wifiServer.begin();
    pimpl->ethServer.begin();
    Serial.println("Telnet server started on port 23");
}

void ESP_TelnetServer::loop()
{
    // Accept WiFi client if none connected
    if ((!pimpl->wifiClient || !pimpl->wifiClient.connected()))
    {
        WiFiClient c = pimpl->wifiServer.available();
        if (c)
        {
            pimpl->wifiClient = c;
        }
    }

    // Accept Ethernet client if none connected
    if ((!pimpl->ethClient || !pimpl->ethClient.connected()))
    {
        EthernetClient c = pimpl->ethServer.available();
        if (c)
        {
            pimpl->ethClient = c;
        }
    }

    // Cleanup disconnected clients
    if (pimpl->wifiClient && !pimpl->wifiClient.connected())
    {
        pimpl->wifiClient.stop();
    }
    if (pimpl->ethClient && !pimpl->ethClient.connected())
    {
        pimpl->ethClient.stop();
    }
}

// Serial-like API
int ESP_TelnetServer::available()
{
    if (pimpl->wifiClient && pimpl->wifiClient.connected())
        return pimpl->wifiClient.available();
    if (pimpl->ethClient && pimpl->ethClient.connected())
        return pimpl->ethClient.available();
    return 0;
}

int ESP_TelnetServer::read()
{
    if (pimpl->wifiClient && pimpl->wifiClient.connected())
        return pimpl->wifiClient.read();
    if (pimpl->ethClient && pimpl->ethClient.connected())
        return pimpl->ethClient.read();
    return -1;
}

size_t ESP_TelnetServer::write(uint8_t b)
{
    if (pimpl->wifiClient && pimpl->wifiClient.connected())
        return pimpl->wifiClient.write(b);
    if (pimpl->ethClient && pimpl->ethClient.connected())
        return pimpl->ethClient.write(b);
    return 0;
}

size_t ESP_TelnetServer::write(const uint8_t *buffer, size_t size)
{
    if (pimpl->wifiClient && pimpl->wifiClient.connected())
        return pimpl->wifiClient.write(buffer, size);
    if (pimpl->ethClient && pimpl->ethClient.connected())
        return pimpl->ethClient.write(buffer, size);
    return 0;
}

// Use Print::print/println which call write()

int ESP_TelnetServer::peek()
{
    if (pimpl->wifiClient && pimpl->wifiClient.connected())
        return pimpl->wifiClient.peek();
    if (pimpl->ethClient && pimpl->ethClient.connected())
        return pimpl->ethClient.peek();
    return -1;
}

void ESP_TelnetServer::flush()
{
    if (pimpl->wifiClient && pimpl->wifiClient.connected())
        pimpl->wifiClient.flush();
    if (pimpl->ethClient && pimpl->ethClient.connected())
        pimpl->ethClient.flush();
}