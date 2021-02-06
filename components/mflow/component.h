#pragma once
#ifndef MFLOW_COMPONENT_H_INCLUDED
#define MFLOW_COMPONENT_H_INCLUDED

// Standard includes
#include <map>

// FreeRTOS includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Project includes
#include "mflow_config.h"
#include "port.h"


/**
 * @brief   The Component class implements the basic interface for
 *          flow based programming components.
 * @details Derived classes can implement the #process() method to
 *          create custom behaviours. Components can have input and
 *          output ports that can be used to communicate with other
 *          components. Input and output ports can be connected with
 *          the global #connect() function. Each component executes
 *          in a separate thread until signalled to stop.
 */
class Component {
public:

	// Making the input port class a friend, so it can
	// have access to the thread handle that is executing
	// this component. This is necessary for passing the
	// handle to the MessageQueue constructor.
	friend class InputPort;

	/**
	 * @brief   Initializes the component.
	 * @details Use the constructor to initialize member variables to
	 *          default values. You should create the input and output
	 *          ports in the constructor, but proper initialization
	 *          should be performed in the #initialize() method, because
	 *          initial messages will only be available after the call
	 *          to the constructor finishes.
	 */
	Component();

	/**
	 * @brief   Destroys the component.
	 * @details Use the destructor to release any dynamic resources
	 *          allocated by the component.
	 */
	virtual ~Component() = default;

	/**
	 * @brief   Initializes the component.
	 * @details This initialization is called exactly once after the
	 *          component constructor and before the first call to
	 *          the process method.
	 */
	virtual void initialize(void) = 0;

	/**
	 * @brief   Implements the main functionality of the component.
	 * @details You can use this method to read input ports, perform
	 *          component logic and write output ports. This method
	 *          is invoked in a loop from the executing thread, while
	 *          the process terminates. If the component has option
	 *          ports for configuration, you should check them at the
	 *          beginning of this method and react to value changes
	 *          accordingly.
	 */
	virtual void process(void) = 0;

	/**
	 * @brief Signals the process that it can start execution.
	 */
	void start_process(void);

	/**
	 * @brief Signals the process to stop execution.
	 */
	void stop_process(void);

	/**
	 * @brief Returns whether the task should execute.
	 */
	bool should_run(void) const;

	/**
	 * @brief Returns whether the task is actually executing.
	 */
	bool is_running(void) const;

	/**
	 * @brief Internal storage class to store and query input ports.
	 */
	class InputArray {
	public:

		/**
		 * @brief Constructs an empty input array referencing the parent Component.
		 * @param parent [in] Pointer to the parent component.
		 */
		InputArray(Component* parent);

		/**
		 * @brief Creates and registers a new input port with the specified type.
		 * @param index    [in] The numeric identifier for the input port to create.
		 * @param capacity [in] The capacity of the input port's message queue.
		 */
		template <class Type>
		void addPort(unsigned index, unsigned capacity)
		{
			// Creating the new input port in-place in the container
			m_ports.emplace(std::piecewise_construct,
					        std::forward_as_tuple(index),
							std::forward_as_tuple(m_parent, sizeof(Type), capacity, type_id<Type>()));
		}

		/**
		 * @brief  Queries the input port at the specified index.
		 * @param  index [in] Index of the input port in the container.
		 * @retval Reference to the input port at the specified index.
		 */
		InputPort& operator[](unsigned index);

	private:
		std::map<unsigned, InputPort> m_ports;  /**< Associative storage for input ports. */
		Component*                    m_parent; /**< Pointer to the parent Component.     */
	};

	/**
	 * @brief Internal storage class to store and query output ports.
	 */
	class OutputArray {
	public:

		/**
		 * @brief Constructs an empty output array referencing the parent Component.
		 * @param parent [in] Pointer to the parent component.
		 */
		OutputArray(Component* parent);

		/**
		 * @brief Creates and registers a new output port with the specified type.
		 * @param index [in] The numeric identifier for the output port to create.
		 */
		template <class Type>
		void addPort(unsigned index)
		{
			// Creating the new output port in-place in the container
			m_ports.emplace(std::piecewise_construct,
					        std::forward_as_tuple(index),
							std::forward_as_tuple(m_parent, type_id<Type>()));
		}

		/**
		 * @brief  Queries the output port at the specified index.
		 * @param  index [in] Index of the output port in the container.
		 * @retval Reference to the output port at the specified index.
		 */
		OutputPort& operator[](unsigned index);

	private:
		std::map<unsigned, OutputPort> m_ports;  /**< Associative storage for output ports. */
		Component*                     m_parent; /**< Pointer to the parent Component.      */
	};

	InputArray  inputs;  /**< Container of input ports.  */
	OutputArray outputs; /**< Container of output ports. */

protected:

	/**
	 * @brief  Blocks execution of the component until an input port receives a message.
	 * @param  input_indices [in] Braced initialized list of input port indices to wait for.
	 * @retval Optional value containing the input index that has a message or error status.
	 */
	optional<unsigned> await(std::initializer_list<unsigned> input_indices);

private:
	TaskHandle_t  m_thread;     /**< Handle to the task executing this Component.           */
	volatile bool m_should_run; /**< Flag to indicate whether the Component should execute. */
	volatile bool m_is_running; /**< Flag to indicate whether the Component is executing.   */

	/**
	 * @brief Executes the process in a separate thread.
	 */
	static void run_process(void* p_process);
};

/**
 * @brief Connects an output port of one component to an input port of another.
 * @param source       [in] Reference to the data source component.
 * @param source_index [in] Index of the source output port.
 * @param target       [in] Reference to the data target component.
 * @param target_index [in] Index of the target input port.
 */
void connect(Component& source, unsigned source_index, Component& target, unsigned target_index);

#endif // MFLOW_COMPONENT_H_INCLUDED
