#include "RDIModule.h"
#include "IRDI.h"

MTRDIModule::MTRDIModule()
{
#ifdef _WIN32
	m_RDI = dynamic_cast<IRDI*>(Core::LoadModule("DX12RDI"));
	//m_RDI = dynamic_cast<IRDI*>(Core::LoadModule("VulkanRDI"));
#elif __APPLE__
	m_RDI = dynamic_cast<IRDI*>(Core::LoadModule("MetalRDI"));
#endif
}

MTRDIModule::~MTRDIModule()
{

}
