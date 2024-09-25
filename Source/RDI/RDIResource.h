#pragma once

enum class MTPrimitiveType
{
    Point,
    Line,
    LineStrip,
    Triangle,
    TriangleStrip,
};

enum class MTIndexType
{
    UInt16,
    UInt32,
};

enum class MTCompareFunction
{
    Never,
    Always,
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
};

enum class MTTriangleFacingWinding
{
    Clockwise,
    CounterClockwise
};

enum class MTCullMode
{
    None,
    Front,
    Back,
};

struct MTRenderVar
{
	const static MTInt32 BACK_BUFFER_COUNT = 3;
	const static MTInt32 RENDER_FRAME_COUNT = 3;
};

class MTRDIResource
{
public:
    virtual ~MTRDIResource() {}
};

class MTRDIWindowView : public MTRDIResource
{
public:
	MTRDIWindowView(MTUInt32 Width, MTUInt32 Height) : m_Width(Width), m_Height(Height)
	{}

	void Resize(MTUInt32 Width, MTUInt32 Height)
	{
		m_Width = Width;
		m_Height = Height;

		OnResize(Width, Height);
	}

	virtual void OnResize(MTUInt32 Width, MTUInt32 Height) = 0;

	float GetAspectRatio() const { return float(m_Width) / float(m_Height); }

	MTUInt32 GetWidth() const { return m_Width; }

	MTUInt32 GetHeight() const { return m_Height; }

private:
	MTUInt32 m_Width = 0;
	MTUInt32 m_Height = 0;
};

class MTRDIShader : public MTRDIResource
{
};

class MTRDITexture : public MTRDIResource
{
public:
};

class MTRDIMaterial : public MTRDIResource
{
public:
	virtual ~MTRDIMaterial()
	{
		for (MTRDITexture* Texture : Textures)
		{
			if (Texture)
			{
				delete Texture;
			}
		}
	}

    MTArray<MTRDITexture*> Textures;
};

class MTRDIBuffer : public MTRDIResource
{
public:
	MTRDIBuffer(MTUInt32 Size, MTUInt32 Stride) : m_Size(Size), m_Stride(Stride)
	{}

	MTUInt32 GetSize() const { return m_Size; }

	MTUInt32 GetStride() const { return m_Stride; }

	MTUInt32 GetElementCount() const { return m_Size / m_Stride;  }

private:
	MTUInt32 m_Size;
	MTUInt32 m_Stride;
};

class MTRDIDepthStencilState : public MTRDIResource
{
    
};
