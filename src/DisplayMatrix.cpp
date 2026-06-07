#include "DisplayMatrix.h"
#include "config.h"

DisplayMatrix::DisplayMatrix() {
    clear();
}

void DisplayMatrix::init() {
    pinMode(A_PIN, OUTPUT); pinMode(B_PIN, OUTPUT); pinMode(C_PIN, OUTPUT); pinMode(D_PIN, OUTPUT);
    pinMode(LAT_PIN, OUTPUT); pinMode(OE_PIN, OUTPUT);
    digitalWrite(OE_PIN, HIGH);
    digitalWrite(LAT_PIN, LOW);

    offset = pio_add_program(pio, &hub75_data_program);
    hub75_data_program_init(pio, sm, offset, R1_PIN, CLK_PIN);

    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));

    dma_channel_configure(dma_chan, &c, &pio->txf[sm], NULL, 64, false);
}

inline void DisplayMatrix::setRow(int row) {
    digitalWrite(A_PIN, row & 0x01);
    digitalWrite(B_PIN, (row >> 1) & 0x01);
    digitalWrite(C_PIN, (row >> 2) & 0x01);
    digitalWrite(D_PIN, (row >> 3) & 0x01);
}

void DisplayMatrix::renderFrame(const uint8_t *buffer) {
    int back_buffer_idx = 1 - current_buffer_idx;

    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 64; col++) {
            int idx1 = (row * 64 + col) * 3;
            int idx2 = ((row + 16) * 64 + col) * 3;

            for (int plane = 0; plane < NUM_BITPLANES; plane++) {
                uint8_t pixel = 0;
                int shift = (8 - NUM_BITPLANES) + plane;

                if ((buffer[idx1]     >> shift) & 0x01) pixel |= 0x01; // R1
                if ((buffer[idx1 + 1] >> shift) & 0x01) pixel |= 0x02; // G1
                if ((buffer[idx1 + 2] >> shift) & 0x01) pixel |= 0x04; // B1
                
                if ((buffer[idx2]     >> shift) & 0x01) pixel |= 0x08; // R2
                if ((buffer[idx2 + 1] >> shift) & 0x01) pixel |= 0x10; // G2
                if ((buffer[idx2 + 2] >> shift) & 0x01) pixel |= 0x20; // B2

                packed_buffers[back_buffer_idx][plane][row * 64 + col] = pixel;
            }
        }
    }
    swap_requested = true;
}

void DisplayMatrix::updateDisplay() {
    if (swap_requested) {
        current_buffer_idx = 1 - current_buffer_idx;
        swap_requested = false;
    }

    for (int plane = 0; plane < NUM_BITPLANES; plane++) {
        int delay_time = (1 << plane) * 4;

        for (int row = 0; row < 16; row++) {
            dma_channel_set_read_addr(dma_chan, &packed_buffers[current_buffer_idx][plane][row * 64], true);
            dma_channel_wait_for_finish_blocking(dma_chan);
            
            while (!pio_sm_is_tx_fifo_empty(pio, sm));
            delayMicroseconds(1);

            digitalWrite(OE_PIN, HIGH);
            setRow(row);
            digitalWrite(LAT_PIN, HIGH);
            asm volatile("nop");
            digitalWrite(LAT_PIN, LOW);
            digitalWrite(OE_PIN, LOW);
            
            if (delay_time > 0) delayMicroseconds(delay_time);
        }
    }
}

void DisplayMatrix::drawPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= 64 || y < 0 || y >= 32) return;
    int idx = (y * 64 + x) * 3;
    matrix_buffer[idx] = r;
    matrix_buffer[idx + 1] = g;
    matrix_buffer[idx + 2] = b;
}

void DisplayMatrix::drawSplash() {
    clear();
    for(int x=22; x<42; x++) drawPixel(x, 8, 255, 0, 0);
    for(int i=0; i<10; i++) drawPixel(41-i, 9+i, 255, 0, 0);
    for(int i=0; i<6; i++) drawPixel(31-i, 19+i, 255, 0, 0);
    for(int x=22; x<42; x++) drawPixel(x, 24, 255, 0, 0);

    drawPixel(25, 27, 100, 100, 100); drawPixel(25, 28, 100, 100, 100); drawPixel(25, 29, 100, 100, 100); drawPixel(26, 29, 100, 100, 100);
    drawPixel(29, 27, 100, 100, 100); drawPixel(30, 27, 100, 100, 100); drawPixel(29, 28, 100, 100, 100); drawPixel(30, 28, 100, 100, 100); drawPixel(29, 29, 100, 100, 100); drawPixel(30, 29, 100, 100, 100);
    drawPixel(33, 27, 100, 100, 100); drawPixel(33, 28, 100, 100, 100); drawPixel(33, 29, 100, 100, 100); drawPixel(34, 27, 100, 100, 100); drawPixel(34, 29, 100, 100, 100);
    drawPixel(37, 27, 100, 100, 100); drawPixel(37, 28, 100, 100, 100); drawPixel(37, 29, 100, 100, 100); drawPixel(38, 27, 100, 100, 100); drawPixel(38, 29, 100, 100, 100);
    
    renderFrame(matrix_buffer);
}

void DisplayMatrix::clear() {
    memset(matrix_buffer, 0, sizeof(matrix_buffer));
    memset(packed_buffers, 0, sizeof(packed_buffers));
}