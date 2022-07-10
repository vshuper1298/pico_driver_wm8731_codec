#include "hardware.h"
#include <stdbool.h>
#include "hardware/adc.h"

static hw_interface intf;

bool hw_intf_init()
{
    // intf.read_cb = &adc_read;
}