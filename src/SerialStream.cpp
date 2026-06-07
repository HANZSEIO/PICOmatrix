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
    if (Serial.available() >= 2) {
        if (Serial.read() == 'F' && Serial.read() == 'R') {
            size_t bytesRead = Serial.readBytes((char*)packet_buffer, expected_size);
            if (bytesRead == expected_size) {
                return true;
            }
        } 
    }
    return false;
}

const uint8_t* SerialStream::getBuffer() const {
    return packet_buffer;
}