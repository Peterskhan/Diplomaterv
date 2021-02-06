#pragma once
#ifndef MFLOW_PORT_H_INCLUDED
#define MFLOW_PORT_H_INCLUDED

// Standard includes
#include <memory>

// FreeRTOS includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Project includes
#include "mflow_config.h"
#include "message_queue.h"
#include "utility.hpp"


// Forward declaration of classes
class Component;
class InputPort;
class OutputPort;

/**
 * @brief   The Port class is the base class used for communication
 *          between components.
 * @details This class is the parent of input and output ports and
 *          implements common functionality to the underlying message
 *          queue. It also provides the notion of type hashes, ports
 *          are message type aware.
 */
class Port {
public:

	// The connect function needs access to the type index and the message queue
	friend void connect(OutputPort& source, InputPort& target);

	// The send_message function needs access to the type index and the message queue
	template <class Type>
	friend MessageStatus send_message(InputPort& target, const Type& message);

	/**
	 * @brief Creates a port with the specified type index.
	 * @param p_parent [in] Pointer to the parent Component of the Port.
	 * @param type_id  [in] The identifier of the Port's message type.
	 * @param p_queue  [in] Pointer to the attached message queue.
	 */
	Port(const Component* parent, type_index type_id, std::shared_ptr<MessageQueue> p_queue = nullptr);

	/**
	 * @brief Destroys the port.
	 */
	virtual ~Port(void) = default;

	/**
	 * @brief  Queries the number of messages in the attached queue.
	 * @retval The number of messages currently in the queue, false otherwise.
	 */
	std::size_t message_count(void) const;

	/**
	 * @brief  Queries the capacity of the attached queue.
	 * @retval The maximum number of messages the attached queue can store.
	 */
	std::size_t capacity(void) const;

	/**
	 * @brief  Checks whether the attached message queue is closed.
	 * @retval True when the message queue is closed, false otherwise.
	 */
	bool is_closed(void) const;

	/**
	 * @brief  Queries the type index of the Port's message type.
	 * @retval The identifier of the Port's message type.
	 */
	type_index type_id(void) const;

protected:

	// The following methods are used by the InputPort and OutputPort
	// classes, which expose and supplement this functionality to the
	// user in a type-checked fashion, taking Component shutdown into
	// account.

	/**
	 * @brief  Queries whether the attached queue contains messages.
	 * @retval True when the queue contains any message, false otherwise.
	 */
	bool has_message(void) const;

	/**
	 * @brief Receives a message from the attached message queue.
	 * @param p_message [in] Pointer to where the received message will be stored.
	 */
	void receive_from_message_queue(void* p_message);

	/**
	 * @brief  Sends a message to the attached message queue.
	 * @param  p_message  [in] Pointer where the message to send is stored.
	 * @param  timeout_ms [in] The timeout for the sending in milliseconds.
	 * @retval True when the message was sent successfully, false on timeout.
	 */
	bool send_to_message_queue(const void* p_message, uint32_t timeout_ms);

	/**
	 * @brief  Queries whether the parent Component should be terminating.
	 * @retval True when the parent Component is terminating, false otherwise.
	 */
	bool is_parent_terminating(void) const;

	/**
	 * @brief Called by the InputPort upon destruction, flags the attached
	 *        message queue as closed.
	 */
	void close(void);

private:
	const Component*              m_parent;  /**< Pointer to the parent Component.           */
	std::shared_ptr<MessageQueue> m_queue;   /**< Pointer to the attached MessageQueue.      */
	const type_index              m_type_id; /**< The identifier of the Port's message type. */
};

/**
 * @brief   The InputPort class implements the receiver interface
 *          for a message queue.
 * @details The input port creates and attaches a message queue upon
 *          construction. Output ports can be connected to input ports
 *          and this attaches the message queue to the output port as
 *          well. Ownership of the message queue is shared between the
 *          initial input port creating the message queue and the output
 *          ports connected to it.
 */
class InputPort : public Port {
public:

	/**
	 * @brief Creates an input port with the specified message queue parameters.
	 * @param element_size [in] The size of the port type messages in bytes.
	 * @param capacity     [in] The capacity of the underlying message queue.
	 * @param type_id      [in] The hash ID of the port type.
	 */
	InputPort(Component* parent, std::size_t element_size, std::size_t capacity, type_index type_id);

	/**
	 * @brief Destroys the input port and closes it's message queue.
	 */
	virtual ~InputPort() override;

	/**
	 * @brief  Queries whether the attached queue contains messages.
	 * @retval True when the queue contains any message, false otherwise.
	 */
	bool has_message(void) const;

	/**
	 * @brief   Receives a message from the attached message queue.
	 * @details When the message is received successfully, the status is "Okay".
	 *          Status values of "TypeMismatch", "Error" and "Terminated" are
	 *          corresponding with an unsuccessful receive. If the status is
	 *          "Terminated", the receiving Component should return from the
	 *          process(void) method gracefully, and all subsequent receive
	 *          operations will fail with the same status code.
	 * @retval  An optional value which contains the message and receive status.
	 */
	template <class Type>
	optional<Type> receive(void) {

		// Checking if the received type matches with the InputPort's type
		if(type_id() != ::type_id<Type>())
		{
			// Indicating type mismatch, returning no value
			return optional<Type>(MessageStatus::TypeMismatch);
		}

		// Repeat the receiving procedure until it succeeds or the parent Component is terminated
		while(true) {

			// Checking if the receiving process should already terminate
			if(is_parent_terminating())
			{
				// Indicating parent process termination, returning no value
				return optional<Type>(MessageStatus::Terminated);
			}

			// Checking if a message is available already
			else if(has_message()) {

				// Temporary to read the message value into
				Type message;

				// Receiving the message in raw bytes
				receive_from_message_queue(&message);

				// Returning message successfully
				return optional<Type>(message, MessageStatus::Okay);
			}

			// FreeRTOS task notification value to read into
			uint32_t notification;

			// Waiting for the receiving task to receive a notification (either message arrival or shutdown request)
			// The message arrival notification bit is cleared when receiving the notification
			xTaskNotifyWait(0x00000000, MFLOW_NOTIFICATION_MASK_MESSAGE_ARRIVAL, &notification, portMAX_DELAY);
		}
	}
};

/**
 * @brief   The OutputPort class implements the sender interface
 *          for a message queue.
 * @details The output port is initialized with no message queue,
 *          and can be connected to one by the application. The
 *          connected output port is referenced by the output
 *          port with shared ownership.
 */
class OutputPort : public Port {
public:

	/**
	 * @brief Creates an output port with the specified type.
	 * @param type_id [in] The hash ID of the port type.
	 */
	OutputPort(Component* parent, type_index type_id);

	/**
	 * @brief   Sends a message to the attached message queue.
	 * @details When the message is sent successfully, the status is "Okay".
	 *          Status values of "TypeMismatch", "Error" and "Terminated" are
	 *          corresponding with an unsuccessful send operation. If the status
	 *          is "Terminated", the sending Component should return from the
	 *          process(void) method gracefully, and all subsequent send operations
	 *          will fail with the same status code.
	 * @param   value [in] The message to send to the attached message queue.
	 * @retval  Status of the sending operation.
	 */
	template <class Type>
	MessageStatus send(const Type& value) {

		// Checking if the type of the message sent matches with the OutputPort's type
		if(type_id() != ::type_id<Type>())
		{
			// Indicate unsuccessful sending due to type-mismatch
			return MessageStatus::TypeMismatch;
		}

		// Repeat the sending procedure until it succeeds or the sending Component is terminated
		while(!is_parent_terminating())
		{
			// Attempting to send the message, return if sent successfully
			if(send_to_message_queue(&value, MFLOW_MESSAGE_PUSH_ATTEMPT_TIMEOUT_MS))
			{
				// Indicate successful sending
				return MessageStatus::Okay;
			}
		}

		// Indicate unsuccessful sending due to the sending Component being terminated
		return MessageStatus::Terminated;
	}
};

/**
 * @brief   Sends a message to the target input port manually.
 * @details This function should not be used inside Component
 *          code, because it does not attempt to check for
 *          potential deadlocks. Use this function to send
 *          initial messages to Components.
 * @param   target  [in] The input port to send the message to.
 * @param   message [in] Message to send to the input port.
 * @retval  Status of the sending operation.
 */
template <class Type>
MessageStatus send_message(InputPort& target, const Type& message) {

	// Checking if the type of the message sent matches with the InputPort's type
	if(target.type_id() == ::type_id<Type>())
	{
		// Repeating the sending operation until it succeeds or the target input port closes
		while(!target.is_closed())
		{
			// Attempting to send the message, return if sent successfully
			if(target.send_to_message_queue(&message, MFLOW_MESSAGE_PUSH_ATTEMPT_TIMEOUT_MS))
			{
				// The message was sent successfully
				return MessageStatus::Okay;
			}
		}

		// The receiver Component has been already terminated
		return MessageStatus::Terminated;
	}

	// The type of the message sent does not match with the InputPort's type
	return MessageStatus::TypeMismatch;
}

/**
 * @brief Connects an output port to an input port.
 * @param source [in] Reference to the output port to connect.
 * @param target [in] Reference to the input port to connect.
 */
void connect(OutputPort& source, InputPort& target);

#endif // MFLOW_PORT_H_INCLUDED
