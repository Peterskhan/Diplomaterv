#include "component.h"

Component::Component()
	: inputs(this),
	  outputs(this),
	  m_thread(nullptr),
	  m_should_run(false),
	  m_is_running(false)
{
	// Nothing to do here...
}

void Component::start_process(void)
{
	// Indicating the task that it should run
	m_should_run = true;

	// Creating the task to execute this component
	xTaskCreate(Component::run_process, "", 5000, (void*) this, 10, &m_thread);

	// Releasing the process for execution
	xTaskNotify(m_thread, MFLOW_NOTIFICATION_MASK_PROCESS_START, eSetBits);
}

void Component::stop_process(void)
{
	// Indicating the task that it should terminate
	m_should_run = false;

	// Notifying the process about the termination request
	xTaskNotify(m_thread, MFLOW_NOTIFICATION_MASK_PROCESS_SHUTDOWN, eSetBits);
}

bool Component::should_run(void) const
{
	return m_should_run;
}

bool Component::is_running(void) const
{
	return m_is_running;
}

optional<unsigned> Component::await(std::initializer_list<unsigned> input_indices)
{
	// Wait for a message to arrive on an input port or process termination
	while(true) {

		// Checking if the Component has been asked to terminate
		if(!should_run())
		{
			// Indicating process termination, returning no input port index
			return optional<unsigned>(MessageStatus::Terminated);
		}

		// Checking if one of the input ports has a message available
		for(auto index : input_indices)
		{
			if(inputs[index].has_message())
			{
				// Found a message, return with the input port index
				return optional<unsigned>(index, MessageStatus::Okay);
			}
		}

		// Notification value to read into
		uint32_t notification;

		// Blocking until a message arrival notification is received
		xTaskNotifyWait(0x00000000, MFLOW_NOTIFICATION_MASK_MESSAGE_ARRIVAL, &notification, portMAX_DELAY);
	}
}

Component::InputArray::InputArray(Component* parent)
	: m_parent(parent)
{
	// Nothing to do here...
}

InputPort& Component::InputArray::operator[](unsigned index)
{
	return m_ports.at(index);
}

Component::OutputArray::OutputArray(Component* parent)
	: m_parent(parent)
{
	// Nothing to do here...
}

OutputPort& Component::OutputArray::operator[](unsigned index)
{
	return m_ports.at(index);
}

void Component::run_process(void* p_process)
{
	Component* process = static_cast<Component*>(p_process);

	// Notification value to read into
	uint32_t notification = 0x00000000;

	// Blocking until the process is released for execution
	while(!(notification & MFLOW_NOTIFICATION_MASK_PROCESS_START)) {
		xTaskNotifyWait(MFLOW_NOTIFICATION_MASK_PROCESS_START, MFLOW_NOTIFICATION_MASK_PROCESS_START,
					    &notification, portMAX_DELAY);
	}

	process->m_is_running = true;

	ESP_LOGI("", "Component initializing.");

	process->initialize();

	ESP_LOGI("", "Component running.");

	while(process->m_should_run)
	{
		process->process();
	}

	process->m_is_running = false;

	ESP_LOGI("", "Component shutting down.");

	vTaskDelete(nullptr);
}

void connect(Component& source, unsigned source_index, Component& target, unsigned target_index)
{
	connect(source.outputs[source_index], target.inputs[target_index]);
}
