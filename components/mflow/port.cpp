#include "port.h"
#include "component.h"

Port::Port(const Component* parent, type_index type_id, std::shared_ptr<MessageQueue> p_queue)
	: m_parent(parent),
	  m_queue(p_queue),
	  m_type_id(type_id)
{
	// Nothing to do here...
}

bool Port::has_message(void) const
{
	// Checking if a message queue is attached
	if(m_queue != nullptr)
	{
		// Delegating the call to the attached message queue
		return m_queue->has_message();
	}

	// No message queue is attached
	else return false;
}

std::size_t Port::message_count(void) const
{
	// Checking if a message queue is attached
	if(m_queue != nullptr)
	{
		// Delegating the call to the attached message queue
		return m_queue->message_count();
	}

	// No message queue is attached
	else return 0;
}

std::size_t Port::capacity(void) const
{
	// Checking if a message queue is attached
	if(m_queue != nullptr)
	{
		// Delegating the call to the attached message queue
		return m_queue->capacity();
	}

	// No message queue is attached
	else return 0;
}

bool Port::is_closed(void) const
{
	// Checking if a message queue is attached
	if(m_queue != nullptr)
	{
		// Delegating the call to the attached message queue
		return m_queue->is_closed();
	}

	// No message queue is attached
	else return true;
}

type_index Port::type_id(void) const
{
	return m_type_id;
}

void Port::receive_from_message_queue(void* p_message)
{
	// Checking if a message queue is attached
	if(m_queue != nullptr)
	{
		// Delegating the call to the attached message queue
		m_queue->pop_message(p_message);
	}
}

bool Port::send_to_message_queue(const void* p_message, uint32_t timeout_ms)
{
	// Checking if a message queue is attached and not closed
	if(m_queue != nullptr && !m_queue->is_closed())
	{
		// Delegating the call to the attached message queue
		return m_queue->push_message(p_message, timeout_ms);
	}

	// No message queue is attached, message silently discarded
	return true;
}

bool Port::is_parent_terminating(void) const
{
	// Checking if the parent Component should be terminated
	return !m_parent->should_run();
}

void Port::close(void)
{
	// Checking if a message queue is attached
	if(m_queue != nullptr)
	{
		// Delegating the call to the attached message queue
		m_queue->close();
	}
}

InputPort::InputPort(Component* parent, std::size_t element_size, std::size_t capacity, type_index type_id)
	: Port(parent, type_id, std::make_shared<MessageQueue>(element_size, capacity, &parent->m_thread))
{
	// Nothing to do here...
}

InputPort::~InputPort()
{
	// Closing the attached message queue
	close();
}

bool InputPort::has_message(void) const
{
	return Port::has_message();
}

OutputPort::OutputPort(Component* parent, type_index type_id)
	: Port(parent, type_id)
{
	// Nothing to do here...
}

void connect(OutputPort& source, InputPort& target)
{
	// Preventing connections between input and output ports of the same
	// Component (potentially avoiding deadlock)
	if(source.m_parent == target.m_parent)
	{
		return;
	}

	// Checking if the source and target ports use the same type of messages
	if(source.type_id() == target.type_id()) {

		// Attaching the message queue of the target input port
		// to the source output port, so messages sent on the
		// output port arrive at the input port.
		source.m_queue = target.m_queue;
	}
}
