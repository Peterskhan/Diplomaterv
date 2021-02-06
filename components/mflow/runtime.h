#pragma once
#ifndef MFLOW_RUNTIME_H_INCLUDED
#define MFLOW_RUNTIME_H_INCLUDED

// Project includes
#include "component.h"

typedef Component* (*MFLOW_COMPONENT_FACTORY_FP)(void);

/**
 * @brief Registers a specific Component type factory for the runtime.
 * @param component_id [in] Textual identifier of the Component type.
 * @param p_factory    [in] Pointer to the factory method creating the Component.
 */
void register_component(const char* component_id, MFLOW_COMPONENT_FACTORY_FP p_factory);

/**
 * @brief Creates and adds a Component node to the runtime.
 * @param component_id [in] Textual identifier of the Component type.
 * @param name         [in] Name of the created Component instance.
 */
void add_node(const char* component_id, const char* name);

/**
 * @brief Removes a Component node from the runtime.
 * @param name [in] Name of the Component instance to remove.
 */
void remove_node(const char* name);

/**
 * @brief Creates a connection between the specified output and input ports.
 * @param source       [in] Name of the source Component.
 * @param output_index [in] Index of the output port on the source Component.
 * @param target       [in] Name of the target Component.
 * @parma input_index  [in] Index of the input port on the target Component.
 */
void add_edge(const char* source, unsigned output_index, const char* target, unsigned input_index);

/**
 * @brief Starts the execution of the currently specified dataflow network.
 */
void start_network(void);

/**
 * @brief Stops the execution of the currently specified dataflow network.
 */
void stop_network(void);

void add_initial(const char* name, unsigned input_index, const unsigned& message);

void add_initial(const char* name, unsigned input_index, const bool& message);

#endif // MFLOW_RUNTIME_H_INCLUDED
