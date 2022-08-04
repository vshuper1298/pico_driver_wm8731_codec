#include "pio_example.h"
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "example.pio.h"

void write_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq) {
  pio_example_program_init(pio, sm, offset, pin);
  pio_sm_set_enabled(pio, sm, true);

	pio_sm_put(pio, sm, 1);
  sleep_ms(500);
  pio_sm_put(pio, sm, 0);

  printf("Writing pin %d at %d Hz\n", pin, freq);

  // PIO counter program takes 3 more cycles in total than we pass as
  // input (wait for n + 1; mov; jmp)
  pio->txf[sm] = (clock_get_hz(clk_sys) / (2 * freq)) - 3;
}

namespace examples
{

void PioExample::init()
{
	setup_default_uart();

  // todo get free sm
  m_pio = pio0;
  m_offset = pio_add_program(m_pio, &pio_example_program);
  printf("Loaded program at %d\n", m_offset);
}

void PioExample::run()
{
  write_pin_forever(m_pio, 0, m_offset, 22, 3);
  write_pin_forever(m_pio, 1, m_offset, 24, 4);
  write_pin_forever(m_pio, 2, m_offset, 25, 1);
}

} //namespace examples