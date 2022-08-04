#ifndef EXAMPLE_H
#define EXAMPLE_H

namespace examples
{
	class Example
	{
	public:
		virtual void init() = 0;
		virtual void run() = 0;
	};
}

#endif /* EXAMPLE_H */