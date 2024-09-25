#pragma once

class MTObject;
class MTProperty;

class COREOBJECT_API MTClass
{
public:
    MTClass();
	virtual ~MTClass();
    
    MTObject* NewObject()
    {
        return CreateObject();
    }
    
	template<typename T, typename Class>
    void AddProperty(const MTName& Type, const MTName& Name, T Class::* Property)
	{
		if (Property)
		{
			m_Properties.Add(Name, new MTValueTypeProperty<T, Class>(Type, Name, Property));
		}
	}
    
    MTProperty* GetProperty(const MTName& Name) const;
    
    MTFunction<MTObject*()> CreateObject;

private:
    MTMap<MTName, MTProperty*> m_Properties;
};
