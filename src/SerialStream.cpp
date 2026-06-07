#include "SerialStream.h"

SerialStream::SerialStream(size_t buffer_size) {
    packet_buffer = new uint8_t[buffer_size];
    expected_size = buffer_size;
}

SerialStream::~SerialStream() {
    delete[] packet_buffer;
}

void SerialStream::init(unsigned long baudrate) {
    Serial.begin(baudrate);
}

bool SerialStream::update() {

    Serial.write('G');
    Serial.flush();

    int header = Serial.read();
        if (header == 0xAA) {
            size_t bytesRead = Serial.readBytes((char*)packet_buffer, expected_size);
                return (bytesRead == expected_size);
    }
    return false;
}

const uint8_t* SerialStream::getBuffer() const {
    return packet_buffer;
}