#include "message_queue.h"


MessageQueue::MessageQueue(std::size_t element_size, std::size_t capacity, TaskHandle_t* p_reader_thread)
	: m_reader_thread(p_reader_thread),
	  m_capacity(capacity),
	  m_closed(false),
	  m_queue(xQueueCreate(capacity, element_size))
{
	// Nothing to do here...
}

MessageQueue::~MessageQueue()
{
	vQueueDelete(m_queue);
}

bool MessageQueue::has_message(void) const
{
	return !xQueueIsQueueEmptyFromISR(m_queue);
}

std::size_t MessageQueue::message_count(void) const
{
	return uxQueueMessagesWaiting(m_queue);
}

std::size_t MessageQueue::capacity(void) const
{
	return m_capacity;
}

void MessageQueue::close(void)
{
	m_closed = true;
}

bool MessageQueue::is_closed(void) const
{
	return m_closed;
}

bool MessageQueue::push_message(const void* p_message, uint32_t timeout_ms)
{
	// Sending the message to the target queue
	bool status = xQueueSendToBack(m_queue, p_message, timeout_ms / portTICK_RATE_MS) == pdTRUE;

	// Notifying task waiting for this queue
	if(status && *m_reader_thread) xTaskNotify(*m_reader_thread, MFLOW_NOTIFICATION_MASK_MESSAGE_ARRIVAL, eSetBits);

	return status;
}

void MessageQueue::pop_message(void* p_message)
{
	xQueueReceive(m_queue, p_message, portMAX_DELAY);
}
