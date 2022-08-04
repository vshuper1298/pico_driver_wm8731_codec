#ifndef PIO_EXAMPLE_H
#define PIO_EXAMPLE_H

#include "../example.h"
#include "hardware/pio.h"

namespace examples
{
	class PioExample : public Example
	{
	public:
		virtual void init() override;
		virtual void run() override;

	private:
		PIO m_pio;
		uint m_offset;
	};
}

#endif /* PIO_EXAMPLE_H */

