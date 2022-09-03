#include "pio_example.h"
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "example.pio.h"
#include "pico/multicore.h"

namespace examples
{

const PIO pio = pio0;
const PIO pio_1 = pio1;
uint sm = -1;
uint sm1 = -1;

void PioExample::init()
{
	// setup_default_uart();

  // Get first free state machine in PIO 0
  sm = pio_claim_unused_sm(pio, true);
  sm1 = pio_claim_unused_sm(pio_1, true);

  // Add PIO program to PIO instruction memory. SDK will find location and
  // return with the memory offset of the program.
  const uint offset = pio_add_program(pio, &pio_example_program);
  const uint offset1 = pio_add_program(pio_1, &pio_example_wave_program);

  // Initialize the program using the helper function in our .pio file
  pio_example_program_init(pio, sm, offset, 6);
  pio_example_wave_program_init(pio_1, sm1, offset1, 7);
}

void write_wave_program_data()
{
  while (true) {
    pio_sm_put_blocking(pio_1, sm1, 1);
    pio_sm_put_blocking(pio_1, sm1, 0);
  }
}

void PioExample::update_y_reg()
{
  if (counter > 31) counter = 0;
  pio_sm_exec(pio_1, sm1, pio_encode_set(pio_y, counter));
}

void PioExample::run()
{
  multicore_launch_core1(write_wave_program_data);
  static uint16_t i = 20;
  while (true) {
    // --i;
    // if (i == false)
    //   increase_counter(31);
    pio_sm_put_blocking(pio, sm, 1);
    pio_sm_put_blocking(pio, sm, 0);
  }
}

} //namespace examples