#pragma once
#include "component.h"


class RectifiedWave : public Component {
public:

	// Port index definitions
	static constexpr unsigned period = 0U;
	static constexpr unsigned duty   = 1U;
	static constexpr unsigned clk    = 2U;
	static constexpr unsigned out    = 0U;

	RectifiedWave() : m_counter(0), m_period(0), m_duty(100)
	{
		inputs.addPort<unsigned>(period, 1);
		inputs.addPort<unsigned>(duty, 1);
		inputs.addPort<bool>(clk, 1);
		outputs.addPort<double>(out);
	}

	virtual void initialize(void) override
	{
		m_period = inputs[period].receive<unsigned>();
		m_duty = inputs[duty].receive<unsigned>();
	}

	virtual void process(void) override
	{
		if(inputs[duty].has_message())
		{
			m_duty = inputs[duty].receive<unsigned>();
		}

		// Wait for clock
		//if(!inputs[clk].receive<bool>()) return;

		// Creating output based on counter value
		if(m_counter < (m_duty / 100.0f) * m_period)
		{
			//printf("%lf, ", 6.0f);
			outputs[out].send<double>(50.0f);
		}
		else
		{
			//printf("%lf, ", 0.0f);
			outputs[out].send<double>(0.0f);
		}

		// Increment sample counter
		m_counter = (m_counter + 1) % m_period;
	}

private:
	unsigned m_counter;
	unsigned m_period;
	unsigned m_duty;
};
