#pragma once

class MTProperty
{
public:
	MTProperty(MTName Type, MTName Name)
		: m_Type(Type), m_Name(Name)
	{}

	virtual ~MTProperty() {}

private:
	MTName m_Type;
	MTName m_Name;
};

template<typename T, typename Class>
class MTValueTypeProperty : public MTProperty
{
public:
	MTValueTypeProperty(MTName Type, MTName Name, T Class::* Property)
		: MTProperty(Type, Name), m_Property(Property)
	{

	}

private:
	T Class::* m_Property;
};
