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
		inline void increase_counter(int count = 0) { counter += count; update_y_reg(); }
		inline void decrease_counter(int count = 0) { counter -= count; update_y_reg(); }

	private:
		void update_y_reg();

	private:
		PIO m_pio;
		uint m_offset;
		uint counter = 0;
	};
}

#endif /* PIO_EXAMPLE_H */

