#pragma once

#include <Arduino.h>

class ESP_TelnetServer : public Stream
{
public:
	ESP_TelnetServer(uint16_t port = 23);
	void begin();
	void loop();

	// Stream/Print overrides
	int available() override;
	int read() override;
	int peek() override;
	void flush() override;
	size_t write(uint8_t b) override;
	size_t write(const uint8_t *buffer, size_t size);

private:
private:
	class Impl;
	Impl *pimpl;
};