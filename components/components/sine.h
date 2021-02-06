#pragma once

#include <cmath>
#include "component.h"

class SineWave : public Component {
public:

	static constexpr unsigned amplitude = 0U;
	static constexpr unsigned period    = 1U;
	static constexpr unsigned phase     = 2U;
	static constexpr unsigned out       = 0U;

	SineWave() : m_period(0), m_tick(0), m_ampl(1)
	{
		inputs.addPort<unsigned>(amplitude, 1);
		inputs.addPort<unsigned>(period, 1);
		inputs.addPort<unsigned>(phase,  1);
		outputs.addPort<double>(out);
	}

	virtual void initialize(void) override {
		m_period = inputs[period].receive<unsigned>();
		m_ampl   = inputs[amplitude].receive<unsigned>();
		//m_tick = inputs[phase].receive<unsigned>();
	}

	virtual void process(void) override {
		double output = m_ampl * sin(2 * 3.14159265f * (double) m_tick++ / (double) m_period);
		outputs[out].send<double>(output);

		vTaskDelay(10 / portTICK_RATE_MS);
	}

private:
	unsigned m_period;
	unsigned m_tick;
	unsigned   m_ampl;
};
