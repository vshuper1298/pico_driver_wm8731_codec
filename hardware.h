#include <stdbool.h>

typedef int(*read_cb)(void);

typedef struct
{
    read_cb read_cb;
} hw_interface;

bool hw_intf_init();