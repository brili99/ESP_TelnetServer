#include "ESP_TelnetServer.h"

#include <Arduino.h>
#ifdef USE_ETHERNET
#include <Ethernet.h>
#endif
#include <WiFi.h>

static const uint16_t DEFAULT_TELNET_PORT = 23;

class ESP_TelnetServer::Impl
{
public:
#ifdef USE_ETHERNET
    class MyEthernetServer : public EthernetServer
    {
    public:
        MyEthernetServer(uint16_t p) : EthernetServer(p) {}
        virtual void begin(uint16_t port = 0) override { EthernetServer::begin(); }
    };
#endif

    Impl(uint16_t p)
#ifdef USE_ETHERNET
        : port(p), wifiServer(p), ethServer(p), useAuth(false), authenticated(false), authAttempts(0)
#else
        : port(p), wifiServer(p), useAuth(false), authenticated(false), authAttempts(0)
#endif
    {
    }

    uint16_t port;
    WiFiServer wifiServer;
#ifdef USE_ETHERNET
    MyEthernetServer ethServer;
#endif
    WiFiClient wifiClient;
#ifdef USE_ETHERNET
    EthernetClient ethClient;
#endif

    // Authentication state (single active-client model)
    bool useAuth;
    String password;       // empty == no auth
    bool authenticated;    // true when the current client has passed auth
    int authAttempts;
    String authBuffer;
    const int MAX_AUTH_ATTEMPTS = 3;

    // Return the currently active client (WiFi preferred).
    Client* activeClient()
    {
        Client *c = nullptr;
        if (wifiClient && wifiClient.connected())
            c = &wifiClient;
#ifdef USE_ETHERNET
        if (!c && ethClient && ethClient.connected())
            c = &ethClient;
#endif
        // Only return client if not using auth or if already authenticated
        if (c && useAuth && !authenticated)
            return nullptr;
        return c;
    }

    // Reset auth state for a new client
    void resetAuth()
    {
        authenticated = false;
        authAttempts = 0;
        authBuffer = "";
    }
};

ESP_TelnetServer::ESP_TelnetServer(uint16_t port)
{
    pimpl = new Impl(port);
}

void ESP_TelnetServer::setPassword(const char *pw)
{
    if (pw && pw[0])
    {
        pimpl->password = String(pw);
        pimpl->useAuth = true;
    }
    else
    {
        clearPassword();
    }
}

void ESP_TelnetServer::clearPassword()
{
    pimpl->password = String();
    pimpl->useAuth = false;
}

void ESP_TelnetServer::begin()
{
    pimpl->wifiServer.begin();
#ifdef USE_ETHERNET
    pimpl->ethServer.begin();
#endif
    Serial.println("Telnet server started on port 23");
}

static void sendPasswordPrompt(Client &c)
{
    c.print("Password: ");
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
            pimpl->resetAuth();
            if (pimpl->useAuth)
                sendPasswordPrompt(pimpl->wifiClient);
        }
    }

#ifdef USE_ETHERNET
    // Accept Ethernet client if none connected
    if ((!pimpl->ethClient || !pimpl->ethClient.connected()))
    {
        EthernetClient c = pimpl->ethServer.available();
        if (c)
        {
            pimpl->ethClient = c;
            pimpl->resetAuth();
            if (pimpl->useAuth)
                sendPasswordPrompt(pimpl->ethClient);
        }
    }
#endif

    // Handle authentication input for the not-yet-authenticated client
    if (pimpl->useAuth && !pimpl->authenticated)
    {
        Client *raw = nullptr;
        // use the same selection order as activeClient()
        if (pimpl->wifiClient && pimpl->wifiClient.connected())
            raw = &pimpl->wifiClient;
#ifdef USE_ETHERNET
        if (!raw && pimpl->ethClient && pimpl->ethClient.connected())
            raw = &pimpl->ethClient;
#endif

        if (raw)
        {
            while (raw->available())
            {
                int ch = raw->read();
                if (ch == '\r')
                    continue;
                if (ch == '\n')
                {
                    String attempt = pimpl->authBuffer;
                    attempt.trim();
                    if (attempt.equals(pimpl->password))
                    {
                        pimpl->authenticated = true;
                        raw->print("OK\r\n");
                    }
                    else
                    {
                        pimpl->authAttempts++;
                        if (pimpl->authAttempts >= pimpl->MAX_AUTH_ATTEMPTS)
                        {
                            raw->print("Too many failed attempts\r\n");
                            raw->stop();
                        }
                        else
                        {
                            raw->print("Invalid\r\n");
                            raw->print("Password: ");
                        }
                    }
                    pimpl->authBuffer = "";
                }
                else
                {
                    // Collect into buffer, do not echo
                    pimpl->authBuffer += (char)ch;
                }
            }
        }
    }

    // Cleanup disconnected clients
    if (pimpl->wifiClient && !pimpl->wifiClient.connected())
    {
        pimpl->wifiClient.stop();
        pimpl->resetAuth();
    }
#ifdef USE_ETHERNET
    if (pimpl->ethClient && !pimpl->ethClient.connected())
    {
        pimpl->ethClient.stop();
        pimpl->resetAuth();
    }
#endif
}

// Serial-like API
int ESP_TelnetServer::available()
{
    Client *c = pimpl->activeClient();
    return c ? c->available() : 0;
}

int ESP_TelnetServer::read()
{
    Client *c = pimpl->activeClient();
    return c ? c->read() : -1;
}

size_t ESP_TelnetServer::write(uint8_t b)
{
    Client *c = pimpl->activeClient();
    return c ? c->write(b) : 0;
}

size_t ESP_TelnetServer::write(const uint8_t *buffer, size_t size)
{
    Client *c = pimpl->activeClient();
    return c ? c->write(buffer, size) : 0;
}

// Use Print::print/println which call write()

int ESP_TelnetServer::peek()
{
    Client *c = pimpl->activeClient();
    return c ? c->peek() : -1;
}

void ESP_TelnetServer::flush()
{
    Client *c = pimpl->activeClient();
    if (c) c->flush();
}