#ifndef SERIAL_STREAM_H
#define SERIAL_STREAM_H

#include <Arduino.h>

class SerialStream {
    private:
    uint8_t *packet_buffer;
    size_t expected_size;

    public:
    SerialStream(size_t buffer_size);
    ~SerialStream();
    void init(unsigned long baudrate);
    bool update();
    const uint8_t* getBuffer() const;
};

#endif