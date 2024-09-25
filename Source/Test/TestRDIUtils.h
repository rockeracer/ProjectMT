#pragma once

#include "RDI.h"

class TestRDIUtils
{
public:
	static IShaderCodeFormat* GetDX12ShaderFormat()
	{
		return Core::LoadModule<IRDI>("DX12RDI")->GetShaderCodeFormat();
	}
};