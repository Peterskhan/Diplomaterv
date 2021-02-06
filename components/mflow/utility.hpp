#pragma once
#ifndef MFLOW_UTILITY_HPP_INCLUDED
#define MFLOW_UTILITY_HPP_INCLUDED

// Standard includes
#include <cstdint>


/**
 * @brief   Simple type index class that can be used for non-polymorphic types.
 * @details This class provides type checking utilities for sending messages and
 *          connecting ports. The implementation is valid without RTTI enabled,
 *          and operates with static template instantiation, resulting in a slight
 *          overhead in code memory and ROM.
 */
class type_index {
public:

	/**
	 * @brief Constructs a type index object
	 */
	type_index(const uint8_t* id) : m_id(id) { }

	// Overloads for standard operators for comparing type index objects
	inline bool operator==(type_index const& t) const { return m_id == t.m_id; }
	inline bool operator!=(type_index const& t) const { return m_id != t.m_id; }
	inline bool operator<(type_index const& t)  const { return m_id <  t.m_id; }
	inline bool operator<=(type_index const& t) const { return m_id <= t.m_id; }
	inline bool operator>(type_index const& t)  const { return m_id >  t.m_id; }
	inline bool operator>=(type_index const& t) const { return m_id >= t.m_id; }

	// Function that constructs type index objects
	template <class Type>
	friend type_index type_id(void);

public:
	const uint8_t* m_id; /**< Pointer to a static variable unique for every type. */
};

/**
 * @brief Constructs a type index object for the specified type.
 */
template <class Type>
type_index type_id(void) {

	// Because of static nature of this variable, there will be only one static instance for
	// every function instantiation for a specific type.
	static uint8_t unique_for_every_type = 0U;

	// The C++ standard guarantees that every existing variable has a unique address in memory,
	// we can use the address of the static variable to identify the type.
	return type_index(&unique_for_every_type);
}

/**
 * @brief Enumeration describing the results of a message sending/receiving operation.
 */
enum class MessageStatus {
	Okay,         /**< The message was sent/received successfully.                            */
	TypeMismatch, /**< The message send/receive failed due to type-mismatch.                  */
	Terminated,   /**< The message send/receive failed due to the Component being terminated. */
	Error         /**< The message send/receive failed due to an internal error.              */
};

/**
 * @brief This class is used to represent the result of a message receive operation,
 *        that might have failed due to various reasons.
 */
template <class Type>
class optional {
public:

	/**
	 * @brief Constructs an optional object containing a value and a message status.
	 * @param message [in] The message to store in the optional object.
	 * @param status  [in] The status of the message operation.
	 */
	optional(const Type& message, MessageStatus status)
		: m_status(status)
	{
		// Constructing the message value in-place by copying the message
		new (m_message) Type(message);
	}

	/**
	 * @brief Constructs an optional object without a contained value.
	 * @param status [in] The status of the message operation.
	 */
	optional(MessageStatus status)
		: m_status(status)
	{
		// No value supplied, nothing to construct...
	}

	/**
	 * @brief   Converts an optional value containing a message to the message's type.
	 * @details Do not use the result of this conversion if the message status is not
	 *          equal to MessageStatus::Okay. This operator does not exist when the
	 *          type of the contained message is boolean, use the value() member.
	 * @retval  The value of the message contained in the optional object.
	 */
	template <class T = Type, typename std::enable_if<!std::is_same<T, bool>::value, int>::type = 0>
	operator Type(void) const
	{
		return value();
	}

	/**
	 * @brief   Converts the optional object to a boolean value representing the success
	 *          of the message operation.
	 * @details There is one case of the conversion result being ambigous, when the Type
	 *          contained as a message is also a boolean. In this case, use the value()
	 *          member function to query the message itself, and use this operator in
	 *          contextual conversions only.
	 * @retval  True when the operation was successful (MessageStatus::Okay), false otherwise.
	 */
	explicit operator bool(void) const
	{
		return m_status == MessageStatus::Okay;
	}

	/**
	 * @brief  Queries the message contained in the optional object (see operator Type).
	 * @retval The message contained in the optional object.
	 */
	Type& value(void)
	{
		return *reinterpret_cast<Type*>(m_message);
	}

	/**
	 * @brief  Queries the message contained in the optional object (see operator Type).
	 * @retval The message contained in the optional object.
	 */
	const Type& value(void) const
	{
		return *reinterpret_cast<const Type*>(m_message);
	}

	/**
	 * @brief  Queries the status of the message operation.
	 * @retval The status of the message operation.
	 */
	MessageStatus status(void) const
	{
		return m_status;
	}

private:
	uint8_t       m_message[sizeof(Type)]; /**< Reserved memory for the message (lazy-initialization optimalization) */
	MessageStatus m_status;                /**< The status of the message operation.                                 */
};

#endif // MFLOW_UTILITY_HPP_INCLUDED
