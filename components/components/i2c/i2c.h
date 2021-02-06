#pragma once
#ifndef MFLOW_ESP_IDF_I2C_H_INCLUDED
#define MFLOW_ESP_IDF_I2C_H_INCLUDED

// FreeRTOS includes
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// ESP-IDF includes
#include "driver/i2c.h"

// Project includes
#include "component.h"

class I2C_CommandChain {
public:

	// The I2C master component needs access to the command chain handle
	friend class I2C_Master;

	I2C_CommandChain(void)
		: m_commands(i2c_cmd_link_create()),
		  m_synch(xSemaphoreCreateBinary()),
		  m_status(false)
	{}

	void queue_start(void) {
		i2c_master_start(m_commands);
	}

	void queue_stop(void)  {
		i2c_master_stop(m_commands);
	}

	void queue_read(uint8_t data[], uint8_t length) {
		i2c_master_read(m_commands, data, length, I2C_MASTER_LAST_NACK);
	}

	void queue_read_byte(uint8_t* byte, uint8_t length) {
		i2c_master_read_byte(m_commands, byte, I2C_MASTER_LAST_NACK);
	}

	void queue_write(uint8_t data[], uint8_t length) {
		i2c_master_write(m_commands, data, length, true);
	}

	void queue_write_byte(uint8_t byte) {
		i2c_master_write_byte(m_commands, byte, true);
	}

	bool wait_for_execute(void) {

		xSemaphoreTake(m_synch, portMAX_DELAY);

		i2c_cmd_link_delete(m_commands);
		vSemaphoreDelete(m_synch);

		return m_status;
	}

	void set_execution_result(bool status) {
		m_status = status;
		xSemaphoreGive(m_synch);
	}

private:
	i2c_cmd_handle_t  m_commands;
	SemaphoreHandle_t m_synch;
	bool              m_status;
};

class I2C_Master : public Component
{
public:

	// Input port index definitions
	static constexpr unsigned command  = 0U;
	static constexpr unsigned port     = 1U;
	static constexpr unsigned sda_pin  = 2U;
	static constexpr unsigned scl_pin  = 3U;
	static constexpr unsigned speed_hz = 4U;

	I2C_Master() : m_port(0)
	{
		// Adding input port for receiving command chains
		inputs.addPort<I2C_CommandChain>(command, 10);

		// Adding configuration inputs
		inputs.addPort<unsigned>(port,     1);
		inputs.addPort<unsigned>(sda_pin,  1);
		inputs.addPort<unsigned>(scl_pin,  1);
		inputs.addPort<unsigned>(speed_hz, 1);
	}

	virtual void initialize(void) override {

		// Reading configuration values
		auto port_config  = inputs[port].receive<unsigned>();
		auto sda_config   = inputs[sda_pin].receive<unsigned>();
		auto scl_config   = inputs[scl_pin].receive<unsigned>();
		auto speed_config = inputs[speed_hz].receive<unsigned>();

		// Checking configuration validity
		if(!port_config || !sda_config || !scl_config || !speed_config) return;

		// Saving port identifier
		m_port = port_config;

		// Configuring I2C bus parameters
		i2c_config_t config;
		config.mode = I2C_MODE_MASTER;
		config.sda_io_num = sda_pin;
		config.sda_pullup_en = GPIO_PULLUP_ENABLE;
		config.scl_io_num = scl_pin;
		config.scl_pullup_en = GPIO_PULLUP_ENABLE;
		config.master.clk_speed = speed_hz;

		// Applying configuration
		i2c_param_config(port, &config);
		i2c_driver_install(port, config.mode, 0, 0, 0);
	}

	virtual void process(void) override {

		// Waiting for a command chain to arrive and execute
		auto commands = inputs[command].receive<I2C_CommandChain>();

		// Checking validity of received message
		if(!commands) return;

		// Executing command chain
		int status = i2c_master_cmd_begin(m_port, commands.value().m_commands, 100 / portTICK_RATE_MS);

		// Sending signal to the caller that the operation finished
		commands.value().set_execution_result(status == ESP_OK);
	}

private:
	unsigned m_port;
};

#endif // MFLOW_ESP_IDF_I2C_H_INCLUDED
