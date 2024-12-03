#include "ObjectSystem.h"
#include "Class.h"

void MTObjectSystem::Destroy()
{
	MTObjectSystem::Get().ClearClass();
}

MTObjectSystem& MTObjectSystem::Get()
{
	static MTObjectSystem Instance;
	return Instance;
}

bool MTObjectSystem::HasClass(const MTName& ClassName) const
{
	return m_Classes.Contain(ClassName);
}

bool MTObjectSystem::AddClass(const MTName& ClassName, MTClass* Class)
{
	if (Class)
	{
		if (HasClass(ClassName) == false)
		{
			return m_Classes.Add(ClassName, Class);
		}
		else
		{
			MT_ASSERT_EXPR(0, "Duplicated Class.");
		}
	}
	else
	{
		MT_ASSERT_EXPR(0, "Class is invalid.");
	}

	return false;
}

void MTObjectSystem::ClearClass()
{
	for (MTMap<MTName, MTClass*>::Iterator ClassItr : m_Classes)
	{
		MTClass* Class = ClassItr.second;
		if (Class)
		{
			delete Class;
		}
	}
}

MTClass* MTObjectSystem::GetClass(const MTName& ClassName) const
{
	MTClass* const* Class = m_Classes.Get(ClassName);
	return Class ? *Class : nullptr;
}
