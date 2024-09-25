#include "MetalAPI.pch"
#include "MetalInterface.h"
#include "MetalResource.h"
#include "MetalGUI.h"
#include "MetalCommandBuffer.h"

#include "GUIRender.h"

MTMetalRDIModule::MTMetalRDIModule()
{
    m_MetalDevice = MTLCreateSystemDefaultDevice();
    m_MetalCommandQueue = [m_MetalDevice newCommandQueue];
}

MTMetalRDIModule::~MTMetalRDIModule()
{
    m_MetalCommandQueue = nullptr;
    m_MetalDevice = nullptr;
}

void MTMetalRDIModule::Init()
{
    
}

void MTMetalRDIModule::Destroy()
{
    
}

MTRDIWindowView* MTMetalRDIModule::CreateWindowView(void* Handle, const MTGUIWindowSize& Size, MTInt32 BackbufferCount)
{
    NSWindow* Window = (__bridge NSWindow*)(Handle);
    
    MTMetalWindowView* WindowView = new MTMetalWindowView;
    WindowView->MetalLayer = [CAMetalLayer layer];
    [WindowView->MetalLayer setDevice:m_MetalDevice];
	WindowView->MetalLayer.maximumDrawableCount = BackbufferCount;
    ResizeView(WindowView, Size);

    [[Window contentView] setLayer:WindowView->MetalLayer];
    [[Window contentView] setWantsLayer:YES];
    
    return WindowView;
}

void MTMetalRDIModule::ResizeWindowView(MTRDIWindowView* View, const MTGUIWindowSize& Size)
{
    ResizeView(View, Size);
}
void MTMetalRDIModule::ResizeView(MTRDIWindowView* View, const MTGUIWindowSize& Size)
{
    MTMetalWindowView* MetalView = dynamic_cast<MTMetalWindowView*>(View);
    if (MetalView)
    {
		MetalView->Width = Size.Width;
		MetalView->Height = Size.Height;

        CGRect bounds = CGRectMake(0, 0, Size.Width, Size.Height);
        CGRect nativeBounds = CGRectMake(0, 0, Size.DisplayWidth, Size.DisplayHeight);
        [MetalView->MetalLayer setFrame:bounds];
        [MetalView->MetalLayer setContentsScale:nativeBounds.size.width / bounds.size.width];
        [MetalView->MetalLayer setDrawableSize:nativeBounds.size];
        
        MTLTextureDescriptor* DepthTextureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: MTLPixelFormatDepth32Float width: nativeBounds.size.width height: nativeBounds.size.height mipmapped: NO];
        DepthTextureDesc.storageMode = MTLStorageModePrivate;
        MetalView->DepthBuffer = [m_MetalDevice newTextureWithDescriptor: DepthTextureDesc];
    }
}

void* MTMetalRDIModule::InitGUIRender(const MTGUIFontTextureDesc& FontTextureDesc, const MTGUIVertexDesc& VertexDesc, MTInt32 BackbufferCount)
{
    m_MetalGUI = new MTMetalGUI(m_MetalDevice, m_MetalCommandQueue);
    return m_MetalGUI->Init(FontTextureDesc, VertexDesc);
}

void MTMetalRDIModule::ShutdownGUIRender()
{
    if (m_MetalGUI)
    {
        m_MetalGUI->Destroy();
        
        delete m_MetalGUI;
        m_MetalGUI = nullptr;
    }
}

void MTMetalRDIModule::BeginRender(const MTRDICommandBuffer* CommandBuffer)
{
	MTMetalWindowView* MetalView = dynamic_cast<MTMetalWindowView*>(CommandBuffer->View);
    if (MetalView)
    {
        MetalView->BeginFrame();
    }

    m_MetalGUI->BeginRender(CommandBuffer->View);
}

void MTMetalRDIModule::RenderGUI(MTRDIWindowView* View, const MTGUIViewportDesc& Viewport, ImDrawData* DrawData)
{
    m_MetalGUI->RenderGUI(View, Viewport, DrawData);
}

void MTMetalRDIModule::EndRender(MTRDIWindowView* View)
{
    m_MetalGUI->EndRender(View);
}

void MTMetalRDIModule::WaitForRenderCompleted()
{
    m_MetalGUI->WaitForRenderCompleted();
}

MTRDIShader* MTMetalRDIModule::CreateShaderWithSource(const MTString& Source, const MTString& VertexShaderFunction, const MTString& PixelShaderFunction)
{
    MTMetalShader* MetalShader = new MTMetalShader;
    NSError* Error = nil;
	MTLCompileOptions* CompileOptions = [MTLCompileOptions new];
	CompileOptions.preprocessorMacros = @{
		@"METAL" : @1
	};

    MetalShader->Library = [m_MetalDevice newLibraryWithSource:[NSString stringWithUTF8String:Source.c_str()] options:CompileOptions error:&Error];
    MetalShader->VertexShaderFunction = [MetalShader->Library newFunctionWithName:[NSString stringWithUTF8String:VertexShaderFunction.c_str()]];
    MetalShader->PixelShaderFunction = [MetalShader->Library newFunctionWithName:[NSString stringWithUTF8String:PixelShaderFunction.c_str()]];
    
    MTLRenderPipelineDescriptor* pipelineDescriptor = [MTLRenderPipelineDescriptor new];
    pipelineDescriptor.vertexFunction = MetalShader->VertexShaderFunction;
    pipelineDescriptor.fragmentFunction = MetalShader->PixelShaderFunction;
    pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
    MetalShader->RenderPipelineState = [m_MetalDevice newRenderPipelineStateWithDescriptor:pipelineDescriptor error:nullptr];

    return MetalShader;
}

IRDICommandBuffer* MTMetalRDIModule::CreateCommandBuffer()
{
    return new MTMetalCommandBuffer(m_MetalCommandQueue);
}

MTRenderCommandBuffer* CreateCommandBufferResource()
{
	return new MTRenderCommandBuffer;
}

MTRDIBuffer* CreateVertexBuffer(const void* Data, MTInt32 BufferSize, MTInt32 VertexSize)
{
	return CreateBuffer(Data, BufferSize);
}

MTRDIBuffer* CreateIndexBuffer(const void* Data, MTInt32 BufferSize, MTInt32 IndexSize)
{
	return CreateBuffer(Data, BufferSize);
}

MTRDIBuffer* CreateConstantBuffer(MTInt32 BufferSize, MTInt32 BackbufferCount)
{
	return CreateBuffer(Data, BufferSize);
}

MTRDIBuffer* MTMetalRDIModule::CreateBuffer(const void* Data, MTInt32 Size)
{
    MTMetalBuffer* Buffer = new MTMetalBuffer;
    Buffer->Buffer = [m_MetalDevice newBufferWithBytes:Data length:Size options:0];
    return Buffer;
}

MTRDIDepthStencilState* MTMetalRDIModule::CreateDepthStencilState(MTCompareFunction CompareFunction, bool bDepthWrite)
{
    MTMetalDepthStencilState* DepthStencilState = new MTMetalDepthStencilState;
    
    MTLDepthStencilDescriptor* DepthStencilDesc = [MTLDepthStencilDescriptor new];
    DepthStencilDesc.depthCompareFunction = MTMetalTypeConverter::CompareFunction(CompareFunction);
    DepthStencilDesc.depthWriteEnabled = bDepthWrite ? YES : NO;
    DepthStencilState->DepthStencilState = [m_MetalDevice newDepthStencilStateWithDescriptor:DepthStencilDesc];
    
    return DepthStencilState;
}

MTRDITexture* MTMetalRDIModule::CreateTextureFromFilePath(const MTString& FilePath)
{
    MTMetalTexture* Texture = new MTMetalTexture;
    
    MTKTextureLoader* textureLoader = [[MTKTextureLoader alloc] initWithDevice:m_MetalDevice];
    NSURL* FileURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:FilePath.c_str()]];
    Texture->Texture = [textureLoader newTextureWithContentsOfURL:FileURL options:nullptr error:nullptr];
    
    MTLSamplerDescriptor* samplerDescriptor = [MTLSamplerDescriptor new];
    samplerDescriptor.sAddressMode = MTLSamplerAddressModeRepeat;
    samplerDescriptor.tAddressMode = MTLSamplerAddressModeRepeat;
    samplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
    samplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
    Texture->Sampler = [m_MetalDevice newSamplerStateWithDescriptor:samplerDescriptor];
    
    return Texture;
}
