#include "Class.h"

MTClass::MTClass()
{
    
}

MTClass::~MTClass()
{
	for (auto Itr : m_Properties)
	{
		MTProperty* Property = Itr.second;
		delete Property;
	}
}

MTProperty* MTClass::GetProperty(const MTName& Name) const
{
    MTProperty* const* Property = m_Properties.Get(Name);
    return Property ? *Property : nullptr;
}
