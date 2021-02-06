#pragma once
#ifndef MFLOW_MESSAGE_QUEUE_H_INCLUDED
#define MFLOW_MESSAGE_QUEUE_H_INCLUDED

// Standard includes
#include <cstdint>

// FreeRTOS includes
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

// Project includes
#include "mflow_config.h"


/**
 * @brief   The MessageQueue class encapsulates a FreeRTOS queue.
 * @details This class is used by components to pass data between
 *          running processes. The message queue is created by an
 *          input port and referenced by output ports. The message
 *          types must be plain old data (POD) types. This class
 *          uses raw memory copying for message passing, type-checks
 *          are performed by higher level interfaces.
 */
class MessageQueue {
public:

	/**
	 * @brief Creates a message queue with the specified capacity.
	 * @param element_size    [in] The size of each message in bytes.
	 * @param capacity        [in] The maximum number of messages in the queue.
	 * @param p_reader_thread [in] Pointer to the thread handle of the queue reader.
	 */
	MessageQueue(std::size_t element_size, std::size_t capacity, TaskHandle_t* p_reader_thread);

	/**
	 * @brief Destroys the MessageQueue and releases allocated FreeRTOS resources.
	 */
	~MessageQueue();

	/**
	 * @brief  Queries whether the queue contains readable messages.
	 * @retval True when the queue contains any message, false otherwise.
	 */
	bool has_message(void) const;

	/**
	 * @brief  Queries the current number of messages in the queue.
	 * @retval The number of messages currently in the queue.
	 */
	std::size_t message_count(void) const;

	/**
	 * @brief  Queries the maximum number of messages the queue can store.
	 * @retval The maximum number of messages the queue can store.
	 */
	std::size_t capacity(void) const;

	/**
	 * @brief Set by the reader on shutdown, flags the queue as closed.
	 */
	void close(void);

	/**
	 * @brief  Checks whether the message queue is closed.
	 * @retval True when the message queue is closed, false otherwise.
	 */
	bool is_closed(void) const;

	/**
	 * @brief  Pushes a message into the queue (shallow copy).
	 * @param  p_message  [in] Pointer to the message to push into the queue.
	 * @param  timeout_ms [in] Timeout for the pushing operation in milliseconds.
	 * @retval True when pushed successfully, false on timeout.
	 */
	bool push_message(const void* p_message, uint32_t timeout_ms);

	/**
	 * @brief Pops a message from the queue (shallow copy).
	 * @param p_message [out] Pointer where the popped message will be stored.
	 */
	void pop_message(void* p_message);

private:
	TaskHandle_t* m_reader_thread; /**< Pointer to the thread reading from the queue.       */
	std::size_t   m_capacity;      /**< The maximum number of messages the queue can store. */
	volatile bool m_closed;        /**< Flag indicating that the reader thread stopped.     */
	QueueHandle_t m_queue;         /**< Handle for the underlying FreeRTOS queue.           */
};

#endif // MFLOW_MESSAGE_QUEUE_H_INCLUDED
