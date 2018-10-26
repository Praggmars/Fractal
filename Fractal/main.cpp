#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

constexpr auto SCREEN_WIDTH = 900;
constexpr auto SCREEN_HEIGHT = 720;

#pragma region Shader codes

LPCSTR g_vsCode = "struct VIT{float4 p : POSITION;float2 t : TEXCOORD;};struct PIT{float4 p:SV_POSITION;float2 t:TEXCOORD;};PIT main(VIT v){PIT p;p.p=v.p;p.t=v.t;return v;}";
LPCSTR g_psCodeFloatMandelbrot = "\
cbuffer Data\
{\
	float2 center;\
	float2 aspectRatio;\
	float2 offset;\
	float zoom;\
	float iterCount;\
};\
struct PIT\
{\
	float4 p:SV_POSITION;\
	float2 t:TEXCOORD;\
};\
float4 IterationsToColor(float r)\
{\
	float R = abs(r * 6 - 3) - 1;\
	float G = 2 - abs(r * 6 - 2);\
	float B = 2 - abs(r * 6 - 4);\
	return saturate(float4(B, G, R, 1.0))*(1.0 - R * 0.49);\
}\
float4 main(PIT input):SV_TARGET{\
float2 z, tmp, coord;\
float i;\
z = float2(0.0f, 0.0f);\
coord = (input.t*aspectRatio)/zoom+center;\
for (i = 0.0; i < iterCount; i++)\
{\
	tmp.x = z.x*z.x - z.y*z.y;\
	tmp.y = 2 * z.x*z.y;\
	z = tmp + coord;\
	if (z.x*z.x + z.y*z.y > 4.0)\
		break;\
}\
return IterationsToColor(i / iterCount);\
}";
LPCSTR g_psCodeDoubleMandelbrot = "\
cbuffer Data\
{\
	double2 center;\
	double2 aspectRatio;\
	double2 offset;\
	double zoom;\
	double iterCount;\
};\
struct PIT\
{\
	float4 p:SV_POSITION;\
	float2 t:TEXCOORD;\
};\
float4 IterationsToColor(float r)\
{\
	float R = abs(r * 6 - 3) - 1;\
	float G = 2 - abs(r * 6 - 2);\
	float B = 2 - abs(r * 6 - 4);\
	return saturate(float4(B, G, R, 1.0))*(1.0 - R * 0.49);\
}\
float4 main(PIT input):SV_TARGET{\
double2 z, tmp, coord;\
double i;\
z = float2(0.0f, 0.0f);\
coord = (input.t*aspectRatio)/zoom+center;\
for (i = 0.0; i < iterCount; i++)\
{\
	tmp.x = z.x*z.x - z.y*z.y;\
	tmp.y = 2 * z.x*z.y;\
	z = tmp + coord;\
	if (z.x*z.x + z.y*z.y > 4.0)\
		break;\
}\
return IterationsToColor(i / iterCount);\
}";
LPCSTR g_psCodeFloatJulia = "\
cbuffer Data\
{\
	float2 center;\
	float2 aspectRatio;\
	float2 offset;\
	float zoom;\
	float iterCount;\
};\
struct PIT\
{\
	float4 p:SV_POSITION;\
	float2 t:TEXCOORD;\
};\
float4 IterationsToColor(float r)\
{\
	float R = abs(r * 6 - 3) - 1;\
	float G = 2 - abs(r * 6 - 2);\
	float B = 2 - abs(r * 6 - 4);\
	return saturate(float4(B, G, R, 1.0))*(1.0 - R * 0.49);\
}\
float4 main(PIT input):SV_TARGET{\
float2 z, tmp, coord;\
float i;\
z = (input.t*aspectRatio)/zoom+center;\
coord = offset;\
for (i = 0.0; i < iterCount; i++)\
{\
	tmp.x = z.x*z.x - z.y*z.y;\
	tmp.y = 2 * z.x*z.y;\
	z = tmp + coord;\
	if (z.x*z.x + z.y*z.y > 4.0)\
		break;\
}\
return IterationsToColor(i / iterCount);\
}";
LPCSTR g_psCodeDoubleJulia = "\
cbuffer Data\
{\
	double2 center;\
	double2 aspectRatio;\
	double2 offset;\
	double zoom;\
	double iterCount;\
};\
struct PIT\
{\
	float4 p:SV_POSITION;\
	float2 t:TEXCOORD;\
};\
float4 IterationsToColor(float r)\
{\
	float R = abs(r * 6 - 3) - 1;\
	float G = 2 - abs(r * 6 - 2);\
	float B = 2 - abs(r * 6 - 4);\
	return saturate(float4(B, G, R, 1.0))*(1.0 - R * 0.49);\
}\
float4 main(PIT input):SV_TARGET{\
double2 z, tmp, coord;\
double i;\
z = (input.t*aspectRatio)/zoom+center;\
coord = offset;\
for (i = 0.0; i < iterCount; i++)\
{\
	tmp.x = z.x*z.x - z.y*z.y;\
	tmp.y = 2 * z.x*z.y;\
	z = tmp + coord;\
	if (z.x*z.x + z.y*z.y > 4.0)\
		break;\
}\
return IterationsToColor(i / iterCount);\
}";

#pragma endregion

template <typename T>
class AutoReleasePtr
{
	T* m_ptr;

public:
	AutoReleasePtr() :m_ptr(nullptr) {}
	AutoReleasePtr(T& ptr) :m_ptr(&ptr) {}
	AutoReleasePtr(T* ptr) :m_ptr(ptr) {}
	AutoReleasePtr(AutoReleasePtr& ptr) :m_ptr(ptr.m_ptr) { ptr.m_ptr = nullptr; }
	~AutoReleasePtr() { Release(); }
	void Release()
	{
		if (m_ptr)
		{
			m_ptr->Release();
			m_ptr = nullptr;
		}
	}
	T* get() const { return m_ptr; }
	T* getAddress() const { return &m_ptr; }
	operator T*() const { return m_ptr; }
	operator bool() const { return m_ptr != nullptr; }
	T* operator->() const { return m_ptr; }
	T** operator&() { return &m_ptr; }
	T* operator=(T* ptr) { return m_ptr = ptr; }
	T* operator=(AutoReleasePtr& ptr)
	{
		m_ptr = ptr.m_ptr;
		ptr.m_ptr = nullptr;
		return m_ptr;
	}
	bool operator==(T* ptr) const { return m_ptr == ptr; }
	bool operator!=(T* ptr) const { return m_ptr != ptr; }
};

struct Graphics
{
	AutoReleasePtr<ID3D11Device> device;
	AutoReleasePtr<ID3D11DeviceContext> deviceContext;
	AutoReleasePtr<IDXGISwapChain> swapChain;
	AutoReleasePtr<ID3D11RenderTargetView> renderTargetView;
	AutoReleasePtr<ID3D11Texture2D> depthStencilBuffer;
	AutoReleasePtr<ID3D11DepthStencilView> depthStencilView;
	AutoReleasePtr<ID3D11DepthStencilState> depthStencilState;
	AutoReleasePtr<ID3D11RasterizerState> rasterizerState;

	AutoReleasePtr<ID3D11VertexShader> vertexShader;
	AutoReleasePtr<ID3D11PixelShader> psFloat;
	AutoReleasePtr<ID3D11PixelShader> psDouble;
	AutoReleasePtr<ID3D11Buffer> vertexBuffer;
	AutoReleasePtr<ID3D11InputLayout> inputLayout;
	AutoReleasePtr<ID3D11Buffer> cbFloat;
	AutoReleasePtr<ID3D11Buffer> cbDouble;
};

template <typename T>
struct FractalData
{
	T center[2];
	T aspectRatio[2];
	T offset[2];
	T zoom;
	T iterCount;

public:
	inline FractalData()
	{
		SetToDefault();
	}
	inline void SetToDefault()
	{
		center[0] = 0;
		center[1] = 0;
		aspectRatio[0] = (T)SCREEN_WIDTH / (T)SCREEN_HEIGHT;
		aspectRatio[1] = 1;
		offset[0] = 0;
		offset[1] = 0;
		zoom = 1;
		iterCount = 256;
	}
	inline void Zoom(T z)
	{
		zoom *= z;
	}
	inline void MulIterCount(T i)
	{
		iterCount *= i;
	}
	inline void Move(short dx, short dy)
	{
		center[0] -= dx / zoom * aspectRatio[0] / SCREEN_WIDTH * 2;
		center[1] += dy / zoom * aspectRatio[1] / SCREEN_HEIGHT * 2;
	}
	inline void setCenter(short x, short y)
	{
		center[0] = (x / (T)SCREEN_WIDTH * 2 - 1) / zoom * aspectRatio[0] + center[0];
		center[1] = (-y / (T)SCREEN_HEIGHT * 2 + 1) / zoom * aspectRatio[1] + center[1];
	}
	inline void setOffset(short x, short y)
	{
		offset[0] = (x / (T)SCREEN_WIDTH * 2 - 1) / zoom * aspectRatio[0] + center[0];
		offset[1] = (-y / (T)SCREEN_HEIGHT * 2 + 1) / zoom * aspectRatio[1] + center[1];
	}
};

class FractalWindow
{
	Graphics m_gfx;
	HWND m_hwnd;
	bool m_highPrecision;
	FractalData<float> m_dataFloat;
	FractalData<double> m_dataDouble;

private:

	bool LoadResources(LPCSTR psCodeFloat, LPCSTR psCodeDouble)
	{
		D3D11_BUFFER_DESC bufferDesc{};
		float vertices[] = {
			-1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
			-1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
			1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f, 1.0f
		};
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.ByteWidth = sizeof(vertices);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = vertices;
		if (FAILED(m_gfx.device->CreateBuffer(&bufferDesc, &initData, &m_gfx.vertexBuffer)))
			return false;
		AutoReleasePtr<ID3DBlob> shaderByteCode;
		if (FAILED(D3DCompile(g_vsCode, strlen(g_vsCode), NULL, NULL, NULL, "main", "vs_5_0", 0, 0, &shaderByteCode, NULL)))
			return false;
		if (FAILED(m_gfx.device->CreateVertexShader(shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(), NULL, &m_gfx.vertexShader)))
			return false;
		D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[2];
		inputLayoutDesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		inputLayoutDesc[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		if (FAILED(m_gfx.device->CreateInputLayout(inputLayoutDesc, 2, shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(), &m_gfx.inputLayout)))
			return false;

		shaderByteCode.Release();
		if (FAILED(D3DCompile(psCodeFloat, strlen(psCodeFloat), NULL, NULL, NULL, "main", "ps_5_0", 0, 0, &shaderByteCode, NULL)))
			return false;
		if (FAILED(m_gfx.device->CreatePixelShader(shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(), NULL, &m_gfx.psFloat)))
			return false;
		shaderByteCode.Release();
		if (FAILED(D3DCompile(psCodeDouble, strlen(psCodeDouble), NULL, NULL, NULL, "main", "ps_5_0", 0, 0, &shaderByteCode, NULL)))
			return false;
		if (FAILED(m_gfx.device->CreatePixelShader(shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(), NULL, &m_gfx.psDouble)))
			return false;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = sizeof(m_dataFloat);
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		if (FAILED(m_gfx.device->CreateBuffer(&bufferDesc, NULL, &m_gfx.cbFloat)))
			return false;
		bufferDesc.ByteWidth = sizeof(m_dataDouble);
		if (FAILED(m_gfx.device->CreateBuffer(&bufferDesc, NULL, &m_gfx.cbDouble)))
			return false;

		UINT stride = sizeof(float) * 5;
		UINT offset = 0;
		UINT bufferNumber = 0;
		m_gfx.deviceContext->IASetVertexBuffers(0, 1, &m_gfx.vertexBuffer, &stride, &offset);
		m_gfx.deviceContext->IASetInputLayout(m_gfx.inputLayout);
		m_gfx.deviceContext->VSSetShader(m_gfx.vertexShader, NULL, 0);
		ChangePrecision(false);
		return true;
	}

	bool InitDirect3D()
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		D3D_FEATURE_LEVEL featureLevel[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};
		swapChainDesc.BufferCount = 2;
		swapChainDesc.BufferDesc.Width = SCREEN_WIDTH;
		swapChainDesc.BufferDesc.Height = SCREEN_HEIGHT;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = m_hwnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;
		if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0,
			featureLevel, sizeof(featureLevel) / sizeof(featureLevel[0]), D3D11_SDK_VERSION,
			&swapChainDesc, &m_gfx.swapChain, &m_gfx.device, NULL, &m_gfx.deviceContext)))
			return false;
		AutoReleasePtr<ID3D11Texture2D> backBufferPtr;
		if (FAILED(m_gfx.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBufferPtr)))
			return false;
		if (FAILED(m_gfx.device->CreateRenderTargetView(backBufferPtr, NULL, &m_gfx.renderTargetView)))
			return false;
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.StencilEnable = TRUE;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;
		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.DepthEnable = TRUE;
		if (FAILED(m_gfx.device->CreateDepthStencilState(&depthStencilDesc, &m_gfx.depthStencilState)))
			return false;
		D3D11_TEXTURE2D_DESC depthBufferDesc{};
		depthBufferDesc.Width = SCREEN_WIDTH;
		depthBufferDesc.Height = SCREEN_HEIGHT;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;
		if (FAILED(m_gfx.device->CreateTexture2D(&depthBufferDesc, NULL, &m_gfx.depthStencilBuffer)))
			return false;
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		if (FAILED(m_gfx.device->CreateDepthStencilView(m_gfx.depthStencilBuffer, &depthStencilViewDesc, &m_gfx.depthStencilView)))
			return false;
		D3D11_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.AntialiasedLineEnable = FALSE;
		rasterizerDesc.CullMode = D3D11_CULL_BACK;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = FALSE;
		rasterizerDesc.MultisampleEnable = FALSE;
		rasterizerDesc.ScissorEnable = FALSE;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		if (FAILED(m_gfx.device->CreateRasterizerState(&rasterizerDesc, &m_gfx.rasterizerState)))
			return false;
		m_gfx.deviceContext->RSSetState(m_gfx.rasterizerState);
		m_gfx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_gfx.deviceContext->OMSetRenderTargets(1, &m_gfx.renderTargetView, m_gfx.depthStencilView);
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = SCREEN_WIDTH;
		viewport.Height = SCREEN_HEIGHT;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		m_gfx.deviceContext->RSSetViewports(1, &viewport);
		m_gfx.deviceContext->OMSetDepthStencilState(m_gfx.depthStencilState, 0);
		return true;
	}

public:
	bool Init(bool isMandelbrot)
	{
		RECT rect{};
		rect.right = SCREEN_WIDTH;
		rect.bottom = SCREEN_HEIGHT;
		AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
		rect.left = (GetSystemMetrics(SM_CXSCREEN) - rect.right) / 2;
		rect.top = (GetSystemMetrics(SM_CYSCREEN) - rect.bottom) / 2;
		m_hwnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"Fractal", isMandelbrot ? L"Mandelbrot" : L"Julia", WS_OVERLAPPEDWINDOW,
			(GetSystemMetrics(SM_CXSCREEN) / 2 - rect.right) / 2 + (isMandelbrot ? (GetSystemMetrics(SM_CXSCREEN) / 2) : 0),
			(GetSystemMetrics(SM_CYSCREEN) - rect.bottom) / 2,
			rect.right, rect.bottom, NULL, NULL, GetModuleHandle(NULL), NULL);
		m_highPrecision = false;
		if (!InitDirect3D())
			return false;
		if (isMandelbrot)
		{
			if (!LoadResources(g_psCodeFloatMandelbrot, g_psCodeDoubleMandelbrot))
				return false;
			m_dataFloat.center[0] = -0.5f;
			m_dataDouble.center[0] = -0.5;
		}
		else
		{
			if (!LoadResources(g_psCodeFloatJulia, g_psCodeDoubleJulia))
				return false;
		}
		ShowWindow(m_hwnd, SW_SHOW);
		UpdateWindow(m_hwnd);
		return true;
	}

	void SwitchPrecision()
	{
		ChangePrecision(!m_highPrecision);
	}
	void ChangePrecision(bool changeToHigh)
	{
		if (changeToHigh)
		{
			m_highPrecision = true;
			m_gfx.deviceContext->PSSetShader(m_gfx.psDouble, NULL, 0);
			m_gfx.deviceContext->PSSetConstantBuffers(0, 1, &m_gfx.cbDouble);
		}
		else
		{
			m_highPrecision = false;
			m_gfx.deviceContext->PSSetShader(m_gfx.psFloat, NULL, 0);
			m_gfx.deviceContext->PSSetConstantBuffers(0, 1, &m_gfx.cbFloat);
		}
	}

	void Paint()
	{
		HDC hdc;
		PAINTSTRUCT ps;
		hdc = BeginPaint(m_hwnd, &ps);
		EndPaint(m_hwnd, &ps);
		EndPaint(m_hwnd, &ps);

		D3D11_MAPPED_SUBRESOURCE resource;
		if (SUCCEEDED(m_gfx.deviceContext->Map(m_highPrecision ? m_gfx.cbDouble : m_gfx.cbFloat, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource)))
		{
			if (m_highPrecision)
			{
				memcpy(resource.pData, &m_dataDouble, sizeof(m_dataDouble));
				m_gfx.deviceContext->Unmap(m_gfx.cbDouble, 0);
			}
			else
			{
				memcpy(resource.pData, &m_dataFloat, sizeof(m_dataFloat));
				m_gfx.deviceContext->Unmap(m_gfx.cbFloat, 0);
			}
			m_gfx.deviceContext->Draw(6, 0);
			m_gfx.swapChain->Present(0, 0);
		}
	}

	HWND getHWND()
	{
		return m_hwnd;
	}

	FractalData<float>& getFractalDataFloat()
	{
		return m_dataFloat;
	}
	FractalData<double>& getFractalDataDouble()
	{
		return m_dataDouble;
	}
};

FractalWindow g_mandelbrot;
FractalWindow g_julia;

void RedrawRequest(HWND hwnd)
{
	InvalidateRect(hwnd, NULL, FALSE);
}
void RedrawRequest()
{
	InvalidateRect(g_mandelbrot.getHWND(), NULL, FALSE);
	InvalidateRect(g_julia.getHWND(), NULL, FALSE);
}

void MouseMove(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static short prevMX = 0;
	static short prevMY = 0;
	short mx = LOWORD(lparam);
	short my = HIWORD(lparam);
	if (wparam&MK_LBUTTON)
	{
		if (wparam&MK_CONTROL)
		{
			if (hwnd == g_mandelbrot.getHWND())
			{
				g_mandelbrot.getFractalDataFloat().setOffset(mx, my);
				g_mandelbrot.getFractalDataDouble().setOffset(mx, my);
				g_julia.getFractalDataFloat().offset[0] = g_mandelbrot.getFractalDataFloat().offset[0];
				g_julia.getFractalDataFloat().offset[1] = g_mandelbrot.getFractalDataFloat().offset[1];
				g_julia.getFractalDataDouble().offset[0] = g_mandelbrot.getFractalDataDouble().offset[0];
				g_julia.getFractalDataDouble().offset[1] = g_mandelbrot.getFractalDataDouble().offset[1];
			}
			if (hwnd == g_julia.getHWND())
			{
				g_julia.getFractalDataFloat().setOffset(mx, my);
				g_julia.getFractalDataDouble().setOffset(mx, my);
				g_mandelbrot.getFractalDataFloat().offset[0] = g_julia.getFractalDataFloat().offset[0];
				g_mandelbrot.getFractalDataFloat().offset[1] = g_julia.getFractalDataFloat().offset[1];
				g_mandelbrot.getFractalDataDouble().offset[0] = g_julia.getFractalDataDouble().offset[0];
				g_mandelbrot.getFractalDataDouble().offset[1] = g_julia.getFractalDataDouble().offset[1];
			}
			RedrawRequest(g_julia.getHWND());
		}
		else
		{
			if (hwnd == g_mandelbrot.getHWND())
			{
				g_mandelbrot.getFractalDataFloat().Move(mx - prevMX, my - prevMY);
				g_mandelbrot.getFractalDataDouble().Move(mx - prevMX, my - prevMY);
			}
			if (hwnd == g_julia.getHWND())
			{
				g_julia.getFractalDataFloat().Move(mx - prevMX, my - prevMY);
				g_julia.getFractalDataDouble().Move(mx - prevMX, my - prevMY);
			}
			RedrawRequest(hwnd);
		}
	}
	prevMX = mx;
	prevMY = my;
}
void MouseWheel(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	double change = (wparam & MK_SHIFT) ? 2.0 : 1.1;
	if (GET_WHEEL_DELTA_WPARAM(wparam) < 0)
		change = 1 / change;
	if (wparam & MK_CONTROL)
	{
		if (hwnd == g_mandelbrot.getHWND())
		{
			g_mandelbrot.getFractalDataFloat().MulIterCount((float)change);
			g_mandelbrot.getFractalDataDouble().MulIterCount((double)change);
		}
		if (hwnd == g_julia.getHWND())
		{
			g_julia.getFractalDataFloat().MulIterCount((float)change);
			g_julia.getFractalDataDouble().MulIterCount((double)change);
		}
	}
	else
	{
		if (hwnd == g_mandelbrot.getHWND())
		{
			g_mandelbrot.getFractalDataFloat().Zoom((float)change);
			g_mandelbrot.getFractalDataDouble().Zoom((double)change);
		}
		if (hwnd == g_julia.getHWND())
		{
			g_julia.getFractalDataFloat().Zoom((float)change);
			g_julia.getFractalDataDouble().Zoom((double)change);
		}
	}
	RedrawRequest(hwnd);
}
void RButtonDown(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	short mx = LOWORD(lparam);
	short my = HIWORD(lparam);
	if (wparam&MK_CONTROL)
	{
		if (hwnd == g_mandelbrot.getHWND())
		{
			g_mandelbrot.getFractalDataFloat().setOffset(mx, my);
			g_mandelbrot.getFractalDataDouble().setOffset(mx, my);
			g_julia.getFractalDataFloat().offset[0] = g_mandelbrot.getFractalDataFloat().offset[0];
			g_julia.getFractalDataFloat().offset[1] = g_mandelbrot.getFractalDataFloat().offset[1];
			g_julia.getFractalDataDouble().offset[0] = g_mandelbrot.getFractalDataDouble().offset[0];
			g_julia.getFractalDataDouble().offset[1] = g_mandelbrot.getFractalDataDouble().offset[1];
		}
		if (hwnd == g_julia.getHWND())
		{
			g_julia.getFractalDataFloat().setOffset(mx, my);
			g_julia.getFractalDataDouble().setOffset(mx, my);
			g_mandelbrot.getFractalDataFloat().offset[0] = g_julia.getFractalDataFloat().offset[0];
			g_mandelbrot.getFractalDataFloat().offset[1] = g_julia.getFractalDataFloat().offset[1];
			g_mandelbrot.getFractalDataDouble().offset[0] = g_julia.getFractalDataDouble().offset[0];
			g_mandelbrot.getFractalDataDouble().offset[1] = g_julia.getFractalDataDouble().offset[1];
		}
		RedrawRequest(g_julia.getHWND());
	}
	else
	{
		if (hwnd == g_mandelbrot.getHWND())
		{
			g_mandelbrot.getFractalDataFloat().setCenter(mx, my);
			g_mandelbrot.getFractalDataDouble().setCenter(mx, my);
		}
		if (hwnd == g_julia.getHWND())
		{
			g_julia.getFractalDataFloat().setCenter(mx, my);
			g_julia.getFractalDataDouble().setCenter(mx, my);
		}
		RedrawRequest(hwnd);
	}
}
void ResetSettings()
{
	g_mandelbrot.ChangePrecision(false);
	g_mandelbrot.getFractalDataFloat().SetToDefault();
	g_mandelbrot.getFractalDataDouble().SetToDefault();
	g_mandelbrot.getFractalDataFloat().center[0] = -0.5f;
	g_mandelbrot.getFractalDataDouble().center[0] = -0.5;
	g_julia.ChangePrecision(false);
	g_julia.getFractalDataFloat().SetToDefault();
	g_julia.getFractalDataDouble().SetToDefault();
	RedrawRequest();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_MOUSEMOVE:
		MouseMove(hwnd, msg, wparam, lparam);
		return 0;
	case WM_MOUSEWHEEL:
		MouseWheel(hwnd, msg, wparam, lparam);
		return 0;
	case WM_RBUTTONDOWN:
		RButtonDown(hwnd, msg, wparam, lparam);
		return 0;
	case WM_KEYDOWN:
		switch (wparam)
		{
		case VK_SPACE:
			g_mandelbrot.SwitchPrecision();
			g_julia.SwitchPrecision();
			RedrawRequest();
			break;
		case 'R':
			ResetSettings();
			break;
		}
		return 0;
	case WM_PAINT:
		if (hwnd == g_mandelbrot.getHWND())
			g_mandelbrot.Paint();
		if (hwnd == g_julia.getHWND())
			g_julia.Paint();
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrecInstance, LPWSTR szCmdLine, INT iCmdShow)
{
	WNDCLASSEX wc{};
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszClassName = L"Fractal";
	wc.cbSize = sizeof(WNDCLASSEX);
	RegisterClassEx(&wc);

	g_mandelbrot.Init(true);
	g_julia.Init(false);

	MSG msg{};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (INT)msg.wParam;
}