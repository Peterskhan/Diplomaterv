#pragma once
#include "component.h"


class MovingAverage : public Component {
public:

	// Port index definitions
	static constexpr unsigned in    = 0U;
	static constexpr unsigned width = 1U;
	static constexpr unsigned out   = 0U;

	MovingAverage() : m_previous_values(nullptr), m_width(0)
	{
		inputs.addPort<double>(in, 1);
		inputs.addPort<unsigned>(width, 1);
		outputs.addPort<double>(out);
	}

	// Component initialization
	virtual void initialize(void) override
	{
		// Reading window width
		m_width = inputs[width].receive<unsigned>();

		// Creating array for previous values
		m_previous_values = new double[m_width];

		// Clearing array of previous values
		for(int i = 0; i < m_width; i++) m_previous_values[i] = 0.0f;
	}

	// Component behaviour
	virtual void process(void) override
	{
		// Checking if window width changed
		if(inputs[width].has_message()) {

			// Reading new window width
			m_width = inputs[width].receive<unsigned>();

			// Deleting current buffer
			delete m_previous_values;

			// Allocating new buffer
			m_previous_values = new double[m_width];

			// Initializing buffer
			for(int i = 0; i < m_width; i++)
			{
				m_previous_values[i] = 0.0f;
			}
		}

		// Reading input
		auto input = inputs[in].receive<double>();

		// Checking input
		if(!input) return;

		// Initialize sum of elements
		double sum = 0.0f;

		// Shifting previous values array
		for(int i = 0; i < m_width - 1; i++)
		{
			m_previous_values[i] = m_previous_values[i + 1];
		}

		// Appending latest value
		m_previous_values[m_width - 1] = input;

		for(int i = 0; i < m_width; i++)
		{
			sum += m_previous_values[i];
		}

		// Calculating average
		double output = sum / m_width;

		// Sending output message
		outputs[out].send<double>(output);
	}

private:
	double*  m_previous_values;
	unsigned m_width;
};
