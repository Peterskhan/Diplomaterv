#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/uart.h"

#include "moving_avg.h"
#include "rect_wave.h"
#include "plotter.h"
#include "runtime.h"

#include "i2c/i2c.h"
#include "sine.h"

extern "C" {
	void app_main(void);
}

class Adder : public Component {
public:
	Adder() {
		inputs.addPort<double>(0, 10);
		inputs.addPort<double>(1, 10);
		outputs.addPort<double>(0);
	}

	virtual void initialize(void) { return; }

	virtual void process(void) override {
		outputs[0].send<double>((double) inputs[0].receive<double>() + (double) inputs[1].receive<double>());
	}
};

void runtime_test(void)
{
	register_component("RectifiedWave", [](){ return (Component*) new RectifiedWave(); });
	register_component("MovingAverage", [](){ return (Component*) new MovingAverage(); });
	register_component("Plotter",       [](){ return (Component*) new Plotter();       });
	//register_component("I2C",           [](){ return (Component*) new I2C_Master();    });
	register_component("SineWave",      [](){ return (Component*) new SineWave();      });
	register_component("Adder",         [](){ return (Component*) new Adder();         });

	add_node("RectifiedWave", "PWM");
	//add_node("MovingAverage",  "MA");
	add_node("Plotter",      "PLOT");
	//add_node("I2C",           "I2C");
	add_node("SineWave", "SIN1");
	add_node("SineWave", "SIN2");
	add_node("Adder",    "ADD");
	add_node("Adder",    "ADD2");

	add_initial("PWM", RectifiedWave::period, (unsigned) 600);
	add_initial("PWM", RectifiedWave::duty,   (unsigned) 40);
	//add_initial("MA",  MovingAverage::width,  (unsigned)  4);
	//add_initial("MA2", MovingAverage::width,  (unsigned)  4);
	//add_initial("I2C", I2C_Master::port,      (unsigned)  0);
	//add_initial("I2C", I2C_Master::sda_pin,   (unsigned) 10);
	//add_initial("I2C", I2C_Master::scl_pin,   (unsigned) 11);
	//add_initial("I2C", I2C_Master::speed_hz,  (unsigned) 400000);
	add_initial("SIN1", SineWave::period, (unsigned) 200);
	add_initial("SIN2", SineWave::period, (unsigned) 5);
	add_initial("SIN1", SineWave::amplitude, (unsigned) 3);
	add_initial("SIN2", SineWave::amplitude, (unsigned) 1);

	add_edge("SIN1", SineWave::out, "ADD", 0);
	add_edge("SIN2", SineWave::out, "ADD", 1);
	add_edge("ADD", 0, "ADD2", 1);
	add_edge("PWM", RectifiedWave::out, "ADD2", 0);
	add_edge("ADD2", 0, "PLOT", Plotter::in);

	//add_edge("PWM", RectifiedWave::out, "MA", MovingAverage::in);

	start_network();

	while(true) {
	//for(int i = 0; i < 1000; i++) {

		//add_initial("PWM", RectifiedWave::clk, (bool) true);

		// Waiting for the next clock cyle
		// vTaskDelay(10 / portTICK_RATE_MS);
	}

	stop_network();

	vTaskSuspend(nullptr);
}

void app_main(void)
{
	{
		runtime_test();
	}

	// Creating components
	RectifiedWave source;
	MovingAverage sink;

	// Sending initial messages to Components
	send_message<unsigned>(source.inputs[RectifiedWave::period], 10);
	send_message<unsigned>(source.inputs[RectifiedWave::duty],   40);
	send_message<unsigned>(sink.inputs[MovingAverage::width],     4);

	// Connecting the output of the first Component to the input of the second Component
	connect(source, RectifiedWave::out, sink, MovingAverage::in);

	// Starting the Component's processes
	source.start_process();
	sink.start_process();

	// Manually sending clock messages to the first component
	for(int i = 0; i < 1000; i++) {

		// Sending a message to act as a clock tick
		send_message<bool>(source.inputs[RectifiedWave::clk], true);

		// Waiting for the next clock cyle
		vTaskDelay(10 / portTICK_RATE_MS);
	}

	// Signalling the Components to stop
	source.stop_process();
	sink.stop_process();

	// Suspending this task, waiting for Components to shut down
	vTaskSuspend(nullptr);
}

