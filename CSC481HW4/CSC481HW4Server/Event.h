#pragma once
#include <string>
#include <vector>

/*
* Class representing an event of a particular type with a set of arguments. Events can be raised and handled by an event system
* to coordinate functionality.
*/
class Event
{
	public:
		/*
		* Enum representing the possible types for event arguments.
		*/
		enum ArgumentType
		{
			TYPE_INTEGER,
			TYPE_FLOAT
		};

		/*
		* A variant that represents a particular event argument by maintaining its type and value.
		*/
		struct ArgumentVariant
		{
			ArgumentType argType;

			union
			{
				int argAsInt;
				float argAsFloat;
			} argValue;
		};

	private:
		/* time at which the event should be handled */
		float timestamp;

		/* type of the event */
		std::string type;

		/* arguments for the event */
		std::vector<ArgumentVariant> arguments;

	public:
		/*
		* Constructs an event with the given values.
		* 
		* timestamp: time at which the event should be handled
		* type: type of the event
		* arguments: list of event arguments
		*/
		Event(float timestamp, std::string type, std::vector<ArgumentVariant> arguments);

		/*
		* Retrieves the argument at the given index.
		* 
		* index: index at which to retrieve argument
		* 
		* returns: argument at given index
		*/
		ArgumentVariant getArgument(int index);

		/*
		* Returns a string with the values of timestamp, type, and each argument in order separated by spaces. Generally used
		* for enabling sending events as strings across client and server.
		* 
		* returns: string representation of event
		*/
		std::string toString();

		/*
		* Returns the event's time to be handled.
		* 
		* returns: time at which event should be handled
		*/
		float getTimestamp();

		/*
		* Sets the event's timestamp to the given value.
		* 
		* timestamp: time at which event should be handled
		*/
		void setTimestamp(float timestamp);

		/*
		* Returns the event's type.
		* 
		* returns: event's type
		*/
		std::string getType();

		/*
		* Returns the list of event arguments.
		* 
		* returns: list of event arguments
		*/
		std::vector<ArgumentVariant> getArguments();
};

