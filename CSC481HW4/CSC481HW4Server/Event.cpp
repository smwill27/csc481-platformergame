#include "Event.h"

Event::Event(float timestamp, std::string type, std::vector<ArgumentVariant> arguments)
{
	this->timestamp = timestamp;
	this->type = type;
	this->arguments = arguments;
}

Event::ArgumentVariant Event::getArgument(int index)
{
	return arguments[index];
}

std::string Event::toString()
{
	std::string eventStr = std::to_string(timestamp) + " " + type;
	int argNum = arguments.size();
	for (int i = 0; i < argNum; i++)
	{
		if (arguments[i].argType == ArgumentType::TYPE_INTEGER)
		{
			eventStr += " " + std::to_string(arguments[i].argValue.argAsInt);
		}
		else if (arguments[i].argType == ArgumentType::TYPE_FLOAT)
		{
			eventStr += " " + std::to_string(arguments[i].argValue.argAsInt);
		}
	}

	return eventStr;
}

float Event::getTimestamp()
{
	return timestamp;
}

void Event::setTimestamp(float timestamp)
{
	this->timestamp = timestamp;
}

std::string Event::getType()
{
	return type;
}

std::vector<Event::ArgumentVariant> Event::getArguments()
{
	return arguments;
}
