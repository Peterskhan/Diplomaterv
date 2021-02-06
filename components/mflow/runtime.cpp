#include "runtime.h"


static std::map<const char*, Component*> s_nodes;
static std::map<const char*, MFLOW_COMPONENT_FACTORY_FP> s_factories;

void register_component(const char* component_id, MFLOW_COMPONENT_FACTORY_FP p_factory)
{
	s_factories[component_id] = p_factory;
}

void add_node(const char* component_id, const char* name)
{
	s_nodes[name] = s_factories[component_id]();
}

void remove_node(const char* name)
{
	s_nodes.erase(name);
}

void add_edge(const char* source, unsigned output_index, const char* target, unsigned input_index)
{
	Component* source_component = s_nodes.find(source) == s_nodes.end() ? nullptr : s_nodes[source];
	Component* target_component = s_nodes.find(target) == s_nodes.end() ? nullptr : s_nodes[target];

	if(source_component != nullptr && target_component != nullptr)
	{
		connect(*source_component, output_index, *target_component, input_index);
	}
}

void start_network(void)
{
	for(auto component : s_nodes)
	{
		component.second->start_process();
	}
}

void stop_network(void)
{
	for(auto component : s_nodes)
	{
		component.second->stop_process();
	}
}

void add_initial(const char* name, unsigned input_index, const unsigned& message)
{
	send_message<unsigned>(s_nodes[name]->inputs[input_index], message);
}

void add_initial(const char* name, unsigned input_index, const bool& message)
{
	send_message<bool>(s_nodes[name]->inputs[input_index], message);
}
