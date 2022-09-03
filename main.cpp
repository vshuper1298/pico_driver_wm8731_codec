// #include "pico/stdlib.h"
// #include <stdio.h>
// #include "hardware/adc.h"
// #include "hardware/dma.h"
// #include "hardware.h"

// const uint LED_PIN = PICO_DEFAULT_LED_PIN;
// static int dma_channel = -1;
// static dma_channel_config ch_config;

// uint8_t capture_buf[256];

// void driver_setup()
// {
//     stdio_init_all();

//     printf("ADC Example, measuring GPIO26\n");
//     adc_init();
//     adc_gpio_init(26);
//     adc_select_input(0);
//     adc_fifo_setup(
//         true,    // Write each completed conversion to the sample FIFO
//         true,    // Enable DMA data request (DREQ)
//         1,       // DREQ (and IRQ) asserted when at least 1 sample present
//         false,   // We won't see the ERR bit because of 8 bit reads; disable.
//         true     // Shift each sample to 8 bits when pushing to FIFO
//     );

//     gpio_init(LED_PIN);
//     gpio_set_dir(LED_PIN, GPIO_OUT);

//     // DMA SETUP
//     dma_channel = dma_claim_unused_channel(true);
//     ch_config = dma_channel_get_default_config(dma_channel);
//     channel_config_set_transfer_data_size(&ch_config, DMA_SIZE_8);
//     channel_config_set_read_increment(&ch_config, false);
//     channel_config_set_write_increment(&ch_config, true);
//     channel_config_set_dreq(&ch_config, DREQ_ADC);

//     dma_channel_configure(dma_channel, &ch_config, capture_buf, &adc_hw->fifo, 256, true);
    
// }

// void adc_read_dma()
// {
//     printf("Starting capture\n");
//     adc_run(true);

//     // Once DMA finishes, stop any new conversions from starting, and clean up
//     // the FIFO in case the ADC was still mid-conversion.
//     dma_channel_wait_for_finish_blocking(dma_channel);
//     printf("Capture finished\n");
//     adc_run(false);
//     adc_fifo_drain();

//     // Print samples to stdout so you can display them in pyplot, excel, matlab
//     for (int i = 0; i < 256; ++i) {
//         printf("%-3d, ", capture_buf[i]);
//         if (i % 10 == 9)
//             printf("\n");
//     }
// }

// int main() {
//     driver_setup();

//     uint16_t result = 0;
//     // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
//     const float conversion_factor = 3.3f / 4096;
//     while (true) {
//         // if (result > 400) {
//             printf("BLINK\n");
//             gpio_put(LED_PIN, 1);
//         // }

//         // result = adc_read();
//         // printf("Raw value: 0x%03x, voltage: %f V\n", result, result * conversion_factor);

//         adc_read_dma();

//         sleep_ms(250);
//         gpio_put(LED_PIN, 0);
//         sleep_ms(250);
//     }
// }






// /**
//  * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
//  *
//  * SPDX-License-Identifier: BSD-3-Clause
//  */

// // Show how to reconfigure and restart a channel in a channel completion
// // interrupt handler.
// //
// // Our DMA channel will transfer data to a PIO state machine, which is
// // configured to serialise the raw bits that we push, one by one. We're going
// // to use this to do some crude LED PWM by repeatedly sending values with the
// // right balance of 1s and 0s. (note there are better ways to do PWM with PIO
// // -- see the PIO PWM example).
// //
// // Once the channel has sent a predetermined amount of data, it will halt, and
// // raise an interrupt flag. The processor will enter the interrupt handler in
// // response to this, where it will reconfigure and restart the channel. This
// // repeats.

// #include <stdio.h>
// #include "hardware/dma.h"
// #include "hardware/irq.h"
// #include "pio_serialiser.pio.h"

// // PIO sends one bit per 10 system clock cycles. DMA sends the same 32-bit
// // value 10 000 times before halting. This means we cycle through the 32 PWM
// // levels roughly once per second.
// #define PIO_SERIAL_CLKDIV 10.f
// #define PWM_REPEAT_COUNT 10000
// #define N_PWM_LEVELS 32

// int dma_chan;

// void dma_handler() {
//     static int pwm_level = 0;
//     static uint32_t wavetable[N_PWM_LEVELS];
//     static bool first_run = true;
//     // Entry number `i` has `i` one bits and `(32 - i)` zero bits.
//     if (first_run) {
//         first_run = false;
//         for (int i = 0; i < N_PWM_LEVELS; ++i)
//             wavetable[i] = ~(~0u << i);
//     }

//     // Clear the interrupt request.
//     dma_hw->ints0 = 1u << dma_chan;
//     // Give the channel a new wave table entry to read from, and re-trigger it
//     dma_channel_set_read_addr(dma_chan, &wavetable[pwm_level], true);

//     pwm_level = (pwm_level + 1) % N_PWM_LEVELS;
// }

// int main() {
// #ifndef PICO_DEFAULT_LED_PIN
// #warning dma/channel_irq example requires a board with a regular LED
// #else
//     // Set up a PIO state machine to serialise our bits
//     uint offset = pio_add_program(pio0, &pio_serialiser_program);
//     pio_serialiser_program_init(pio0, 0, offset, PICO_DEFAULT_LED_PIN, PIO_SERIAL_CLKDIV);

//     // Configure a channel to write the same word (32 bits) repeatedly to PIO0
//     // SM0's TX FIFO, paced by the data request signal from that peripheral.
//     dma_chan = dma_claim_unused_channel(true);
//     dma_channel_config c = dma_channel_get_default_config(dma_chan);
//     channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
//     channel_config_set_read_increment(&c, false);
//     channel_config_set_dreq(&c, DREQ_PIO0_TX0);

//     dma_channel_configure(
//         dma_chan,
//         &c,
//         &pio0_hw->txf[0], // Write address (only need to set this once)
//         NULL,             // Don't provide a read address yet
//         PWM_REPEAT_COUNT, // Write the same value many times, then halt and interrupt
//         false             // Don't start yet
//     );

//     // Tell the DMA to raise IRQ line 0 when the channel finishes a block
//     dma_channel_set_irq0_enabled(dma_chan, true);

//     // Configure the processor to run dma_handler() when DMA IRQ 0 is asserted
//     irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
//     irq_set_enabled(DMA_IRQ_0, true);

//     // Manually call the handler once, to trigger the first transfer
//     dma_handler();

//     // Everything else from this point is interrupt-driven. The processor has
//     // time to sit and think about its early retirement -- maybe open a bakery?
//     while (true)
//         tight_loop_contents();
// #endif
// }





/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <math.h>

#if PICO_ON_DEVICE

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"

#endif

#include "examples/pio/pio_example.h"

#include "pico/stdlib.h"
#include "pico/audio_i2s.h"

// #if PICO_ON_DEVICE
#include "pico/binary_info.h"
bi_decl(bi_3pins_with_names(PICO_AUDIO_I2S_DATA_PIN, "I2S DIN", PICO_AUDIO_I2S_CLOCK_PIN_BASE, "I2S BCK", PICO_AUDIO_I2S_CLOCK_PIN_BASE+1, "I2S LRCK"));
// #endif

#define SINE_WAVE_TABLE_LEN 2048
#define SAMPLES_PER_BUFFER 256

static int16_t sine_wave_table[SINE_WAVE_TABLE_LEN];

struct audio_buffer_pool *init_audio() {

    static audio_format_t audio_format = {
        24000,
        AUDIO_BUFFER_FORMAT_PCM_S16,
        1
    };

    static struct audio_buffer_format producer_format = {
            .format = &audio_format,
            .sample_stride = 2
    };

    struct audio_buffer_pool *producer_pool = audio_new_producer_pool(&producer_format, 3, SAMPLES_PER_BUFFER); // todo correct size
    bool __unused ok;
    struct audio_i2s_config config = {
            .data_pin = PICO_AUDIO_I2S_DATA_PIN,
            .clock_pin_base = PICO_AUDIO_I2S_CLOCK_PIN_BASE,
            .dma_channel = 0,
            .pio_sm = 0,
    };

    const struct audio_format *output_format = audio_i2s_setup(&audio_format, &config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }

    ok = audio_i2s_connect(producer_pool);
    assert(ok);
    audio_i2s_set_enabled(true);
    return producer_pool;
}

int main() {

    stdio_init_all();
    volatile const uint LED_PIN = 20;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    for (int i = 0; i < SINE_WAVE_TABLE_LEN; i++) {
        sine_wave_table[i] = 32767 * cosf(i * 2 * (float) (M_PI / SINE_WAVE_TABLE_LEN));
    }

    // struct audio_buffer_pool *ap = init_audio();
    uint32_t step = 0x200000;
    uint32_t pos = 0;
    uint32_t pos_max = 0x10000 * SINE_WAVE_TABLE_LEN;
    uint vol = 128;

    examples::PioExample ex;
    ex.init();
    ex.run();

    // while (true) {
    //     // int c = getchar_timeout_us(0);
    //     // if (c >= 0) {
    //     //     if (c == '-' && vol) vol -= 4;
    //     //     if ((c == '=' || c == '+') && vol < 255) vol += 4;
    //     //     if (c == '[' && step > 0x10000) step -= 0x10000;
    //     //     if (c == ']' && step < (SINE_WAVE_TABLE_LEN / 16) * 0x20000) step += 0x10000;
    //     //     if (c == 'q') break;
    //     //     printf("vol = %d, step = %d      \r", vol, step >> 16);
    //     // }
    //     // struct audio_buffer *buffer = take_audio_buffer(ap, true);
    //     // int16_t *samples = (int16_t *) buffer->buffer->bytes;
    //     // for (uint i = 0; i < buffer->max_sample_count; i++) {
    //     //     samples[i] = (vol * sine_wave_table[pos >> 16u]) >> 8u;
    //     //     pos += step;
    //     //     if (pos >= pos_max) pos -= pos_max;
    //     // }
    //     // buffer->sample_count = buffer->max_sample_count;
    //     // give_audio_buffer(ap, buffer);

    //     // gpio_put(LED_PIN, 0);
    //     // sleep_ms(250);
    //     gpio_put(LED_PIN, 1);
    // }
    puts("\n");
    return 0;
}

