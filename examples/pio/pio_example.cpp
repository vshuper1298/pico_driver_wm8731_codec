#include "pio_example.h"
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "example.pio.h"
#include "pico/multicore.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

namespace examples
{

const PIO pio = pio0;
// const PIO pio_1 = pio1;
uint sm = 0;
uint sm1 = 1;
uint irq_ctrl_sm = 2;
uint offset1 = 0;

void PioExample::init()
{
	// setup_default_uart();

  // Get first free state machine in PIO 0
  // sm = pio_claim_unused_sm(pio, true);
  // sm1 = pio_claim_unused_sm(pio, true);

  // Add PIO program to PIO instruction memory. SDK will find location and
  // return with the memory offset of the program.
  const uint offset = pio_add_program(pio, &pio_example_program);
  uint offset1 = pio_add_program(pio, &pio_example_wave_program);
  uint offset2 = pio_add_program(pio, &interrupt_controller_program);


  // pio_set_irq0_source_enabled(pio, pis_interrupt0, true);
  // irq_set_enabled(PIO0_IRQ_0, false);

  float div = clock_get_hz(clk_sys) / 134000;
  // Initialize the program using the helper function in our .pio file
  pio_example_program_init(pio, sm, offset, 6, 5);
  // sleep_ms(20);
  pio_example_wave_program_init(pio, sm1, offset1, 7, 5);
  interrupt_controller_program_init(pio, irq_ctrl_sm, offset2, 7, 5);

  #define H_ACTIVE   1 //655    // (active + frontporch - 1) - one cycle delay for mov
  #define V_ACTIVE   7 //  1 + n * 6 (high/low part has a 6 cycles in addition to the delay)

  pio_sm_put_blocking(pio, sm, H_ACTIVE);
  pio_sm_put_blocking(pio, sm1, V_ACTIVE);

  // pio_set_irq1_source_enabled(pio, static_cast<uint>(pis_interrupt0) + sm1, true);

  pio_enable_sm_mask_in_sync(pio, (1u << sm) | (1u << sm1) | (1u << irq_ctrl_sm));
  // irq_set_enabled(PIO0_IRQ_1, true);
}

void write_wave_program_data()
{
  // pio_sm_put_blocking(pio_1, sm1, 1);
  // pio_sm_put_blocking(pio_1, sm1, 0);
}

void write_example_program_data()
{
  pio_sm_put_blocking(pio, sm, 1);
  pio_sm_put_blocking(pio, sm, 0);
}

void PioExample::update_y_reg()
{
  if (counter > 31) counter = 0;
  pio_sm_exec(pio, sm1, pio_encode_set(pio_y, counter));
}

QueueHandle_t queue = NULL;
// static QueueHandle_t xQueue = NULL;

void wave_task(void*)
{
  // This variable will take a copy of the value
  // added to the FreeRTOS xQueue
  uint8_t passed_value_buffer = 0;
    
  // Configure the GPIO LED
    
  while (true) {
    // Check for an item in the FreeRTOS xQueue
    // if (xQueueReceive(xQueue, &passed_value_buffer, portMAX_DELAY) == pdPASS) {
      // Received a value so flash the GPIO LED accordingly
      // (NOT the sent value)
      //if (passed_value_buffer) log_debug("GPIO LED FLASH");
      write_example_program_data();
    // }
  }
}

void wave2_task(void*)
{
  // This variable will take a copy of the value
  // added to the FreeRTOS xQueue
  uint8_t passed_value_buffer = 0;
    
  while (true) {
    // Check for an item in the FreeRTOS xQueue
    // if (xQueueReceive(xQueue, &passed_value_buffer, portMAX_DELAY) == pdPASS) {
      // Received a value so flash the GPIO LED accordingly
      // (NOT the sent value)
      //if (passed_value_buffer) log_debug("GPIO LED FLASH");
      write_wave_program_data();
    // }
  }
}

#define	mainQUEUE_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

#define mainQUEUE_LENGTH					( 1 )

void move_waveform_right()
{
  pio_sm_exec(pio0, sm1, pio_encode_set(pio_y, 1));
}

void PioExample::run()
{
  /* Create the queue. */
	queue = xQueueCreate( mainQUEUE_LENGTH, sizeof( uint32_t ) );

	if( false && queue != NULL )
	{
		/* Start the two tasks as described in the comments at the top of this
		file. */
		xTaskCreate( wave_task,				/* The function that implements the task. */
					"Rx", 								/* The text name assigned to the task - for debug only as it is not used by the kernel. */
					configMINIMAL_STACK_SIZE, 			/* The size of the stack to allocate to the task. */
					NULL, 								/* The parameter passed to the task - not used in this case. */
					mainQUEUE_TASK_PRIORITY, 	/* The priority assigned to the task. */
					NULL );								/* The task handle is not required, so NULL is passed. */

		xTaskCreate( wave2_task, "TX", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_TASK_PRIORITY, NULL );

		/* Start the tasks and timer running. */
		vTaskStartScheduler();
	}

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was insufficient FreeRTOS heap memory available for the Idle and/or
	timer tasks to be created.  See the memory management section on the
	FreeRTOS web site for more details on the FreeRTOS heap
	http://www.freertos.org/a00111.html. */

  // pio_set_irq0_source_enabled(pio, pis_interrupt0, true);
  // pio_set_irq0_source_enabled(pio_1, pis_interrupt0, true);

  // irq_set_mask_enabled(PIO0_IRQ_0, true);
  // irq_set_enabled(PIO0_IRQ_0, true);

  while(true) {
    int c = getchar_timeout_us(0);
    if (c >= 0 && (c == '=' || c == '+')) {
      move_waveform_right();
      printf("Waveform moved right for 1 pio cycle\n");
    }
  }
}

} //namespace examples