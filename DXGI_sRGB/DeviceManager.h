#pragma once

#include <comdef.h>
#include <vector>

template<class Intf>
using ComPtr = _com_ptr_t<_com_IIID<Intf, &__uuidof(Intf)>>;

class DeviceManager
{
private:
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_deviceContext;

	ComPtr<ID3D11InputLayout> m_layout;
	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;

	ComPtr<ID3D11SamplerState> m_samplerState;
	ComPtr<ID3D11BlendState> m_blendState;
	ComPtr<ID3D11RasterizerState> m_rasterizerState;

public:
	DeviceManager();
	~DeviceManager();

	ComPtr<ID3D11Device> const& GetDevice()
	{
		return m_device;
	}

	ComPtr<ID3D11DeviceContext> const& GetDeviceContext()
	{
		return m_deviceContext;
	}

	ComPtr<ID3D11SamplerState> const& GetSamplerState()
	{
		return m_samplerState;
	}

	ComPtr<ID3D11BlendState> const& GetBlendState()
	{
		return m_blendState;
	}

	ComPtr<ID3D11RasterizerState> const& GetRasterizerState()
	{
		return m_rasterizerState;
	}

	void CopyResource(ID3D11Resource *dst, ID3D11Resource *src)
	{
		m_deviceContext->CopyResource(dst, src);
	}

	ComPtr<ID3D11PixelShader> CreatePixelShader(const void *bytecode, SIZE_T bytecodeLength);

	ComPtr<ID3D11Texture2D> CreateDefaultTexture2D(DXGI_FORMAT format, UINT width = 256, UINT height = 1);
	ComPtr<ID3D11Texture2D> CreateStagingTexture2D(DXGI_FORMAT format, UINT width = 256, UINT height = 1);
	ComPtr<ID3D11Texture2D> CreateImmutableTexture2D(DXGI_FORMAT format, const void *p, UINT stride, UINT width = 256, UINT height = 1);

	void Draw(
		ComPtr<ID3D11Texture2D> renderTarget,
		ComPtr<ID3D11Texture2D> source,
		ComPtr<ID3D11PixelShader> pixelShader);

	std::vector<uint8_t> ReadTextureData(ID3D11Texture2D *src);
};

