#pragma once
#include "component.h"

class Plotter : public Component
{
public:

	static constexpr unsigned in = 1U;

	Plotter()
	{
		inputs.addPort<double>(in, 1);
	}

	virtual void initialize(void) override {

	}

	virtual void process(void) override {
		auto value = inputs[in].receive<double>();
		if(value) printf("%lf\n", (double) value);
	}
};
