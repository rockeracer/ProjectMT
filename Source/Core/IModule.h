#pragma once

class CORE_API IModule
{
public:
	IModule() {}
	virtual ~IModule() {}

	virtual void Init() {}
	virtual void Destroy() {}
};
