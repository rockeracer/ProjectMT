#pragma once

#include "IModule.h"

class IRDI;

class MTRDIModule : public IModule
{
public:
	MTRDIModule();
	virtual ~MTRDIModule();

	IRDI* GetRDI() const
	{
		return m_RDI;
	}

private:
	IRDI* m_RDI = nullptr;
};
