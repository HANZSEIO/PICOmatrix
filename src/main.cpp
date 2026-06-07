#include <Arduino.h>
#include "DisplayMatrix.h"
#include "SerialStream.h"

DisplayMatrix matrix;
SerialStream stream(6144);

void setup() {
  matrix.init();
  matrix.drawSplash();
  stream.init(921600);

  Serial.setTimeout(100);
}

void loop() {
  if (stream.update()) {
    matrix.renderFrame(stream.getBuffer());
  }
}

void setup1() {
}

void loop1() {
  matrix.updateDisplay();
}

