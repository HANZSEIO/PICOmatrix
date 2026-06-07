#ifndef DISPLAYMATRIX_H
#define DISPLAYMATRIX_H
#define NUM_BITPLANES 4

#include <Arduino.h>
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hub75.pio.h"

class DisplayMatrix
{
private:
    uint8_t matrix_buffer[6144]; // Raw RGB
    uint8_t packed_buffer[1024];
    uint8_t packed_buffers[2][NUM_BITPLANES][1024];

    volatile int current_buffer_idx = 0;
    volatile bool swap_requested = false;
    
    PIO pio = pio0;
    uint sm = 0;
    uint offset;
    int dma_chan;

    inline void setRow(int row);

public:
    DisplayMatrix();
    void init();
    void renderFrame(const uint8_t *buffer);
    void updateDisplay();
    void clear();
    void drawPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
    void drawSplash();
};

#endif