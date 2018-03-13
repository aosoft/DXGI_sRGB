#include "stdafx.h"
#include "DeviceManager.h"

#include "HRException.h"

#include "VertexShader.csh"

DeviceManager::DeviceManager()
{
	HRESULT hr = S_OK;
	ComPtr<IDXGIFactory> factory;
	ComPtr<IDXGIAdapter> adapter;

	hr = CreateDXGIFactory(IID_PPV_ARGS(&factory));
	HRException::ThrowHR(hr);

	hr = factory->EnumAdapters(0, &adapter);
	HRException::ThrowHR(hr);

	if (adapter == nullptr)
	{
		throw std::exception("no adapter");
	}

	constexpr D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	hr = D3D11CreateDevice(
		adapter.GetInterfacePtr(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
#ifdef _DEBUG
		D3D11_CREATE_DEVICE_DEBUG,
#else
		0,
#endif
		featureLevels, _countof(featureLevels), D3D11_SDK_VERSION,
		&m_device, nullptr, &m_deviceContext);
	HRException::ThrowHR(hr);


	static constexpr D3D11_INPUT_ELEMENT_DESC inputdesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	static constexpr ::DirectX::XMFLOAT4 vertices[] =
	{
		{ -1.0f,  1.0f, 0.0f, 0.0f },
		{  1.0f,  1.0f, 1.0f, 0.0f },
		{  1.0f, -1.0f, 1.0f, 1.0f },
		{ -1.0f, -1.0f, 0.0f, 1.0f },
	};

	static constexpr UINT16 indices[] = { 0, 1, 3, 1, 2, 3 };

	hr = m_device->CreateVertexShader(g_csoVertexShader, sizeof(g_csoVertexShader), nullptr, &m_vertexShader);
	HRException::ThrowHR(hr);

	hr = m_device->CreateInputLayout(
		inputdesc, _countof(inputdesc), g_csoVertexShader, sizeof(g_csoVertexShader), &m_layout);
	HRException::ThrowHR(hr);

	auto initialData = D3D11_SUBRESOURCE_DATA();

	initialData.pSysMem = vertices;
	initialData.SysMemPitch = sizeof(::DirectX::XMFLOAT4);
	initialData.SysMemSlicePitch = 1;

	hr = m_device->CreateBuffer(
		&CD3D11_BUFFER_DESC(
			sizeof(vertices), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE, 0, 0,
			sizeof(::DirectX::XMFLOAT4)),
		&initialData,
		&m_vertexBuffer);
	HRException::ThrowHR(hr);

	initialData.pSysMem = indices;
	initialData.SysMemPitch = sizeof(UINT16);
	initialData.SysMemSlicePitch = 1;

	hr = m_device->CreateBuffer(
		&CD3D11_BUFFER_DESC(
			sizeof(indices), D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_IMMUTABLE, 0, 0,
			sizeof(UINT16)),
		&initialData,
		&m_indexBuffer);
	HRException::ThrowHR(hr);


	hr = m_device->CreateSamplerState(&CD3D11_SAMPLER_DESC(CD3D11_DEFAULT()), &m_samplerState);
	HRException::ThrowHR(hr);

	hr = m_device->CreateBlendState(&CD3D11_BLEND_DESC(CD3D11_DEFAULT()), &m_blendState);
	HRException::ThrowHR(hr);

	hr = m_device->CreateRasterizerState(&CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT()), &m_rasterizerState);
	HRException::ThrowHR(hr);
}

DeviceManager::~DeviceManager()
{
}


ComPtr<ID3D11PixelShader> DeviceManager::CreatePixelShader(const void *bytecode, SIZE_T bytecodeLength)
{
	ComPtr<ID3D11PixelShader> ret;

	HRESULT hr = m_device->CreatePixelShader(bytecode, bytecodeLength, nullptr, &ret);
	HRException::ThrowHR(hr);

	return ret;
}

ComPtr<ID3D11Texture2D> DeviceManager::CreateDefaultTexture2D(DXGI_FORMAT format, UINT width, UINT height)
{
	ComPtr<ID3D11Texture2D> ret;
	auto desc = CD3D11_TEXTURE2D_DESC(
		format,
		width, height, 1, 1,
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);

	HRESULT hr = m_device->CreateTexture2D(&desc, nullptr, &ret);
	HRException::ThrowHR(hr);

	return ret;
}

ComPtr<ID3D11Texture2D> DeviceManager::CreateStagingTexture2D(DXGI_FORMAT format, UINT width, UINT height)
{
	ComPtr<ID3D11Texture2D> ret;
	auto desc = CD3D11_TEXTURE2D_DESC(
		format,
		width, height, 1, 1,
		0, D3D11_USAGE_STAGING,
		D3D11_CPU_ACCESS_READ);

	HRESULT hr = m_device->CreateTexture2D(&desc, nullptr, &ret);
	HRException::ThrowHR(hr);

	return ret;
}

ComPtr<ID3D11Texture2D> DeviceManager::CreateImmutableTexture2D(DXGI_FORMAT format, const void *p, UINT stride, UINT width, UINT height)
{
	ComPtr<ID3D11Texture2D> ret;
	auto desc = CD3D11_TEXTURE2D_DESC(
		format,
		width, height, 1, 1,
		D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_IMMUTABLE);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = p;
	data.SysMemPitch = stride;
	data.SysMemSlicePitch = 1;

	HRESULT hr = m_device->CreateTexture2D(&desc, &data, &ret);
	HRException::ThrowHR(hr);

	return ret;
}


void DeviceManager::Draw(
	ComPtr<ID3D11Texture2D> renderTarget,
	ComPtr<ID3D11Texture2D> source,
	ComPtr<ID3D11PixelShader> pixelShader)
{
	HRESULT hr = S_OK;
	ComPtr<ID3D11RenderTargetView> rtv;
	ComPtr<ID3D11ShaderResourceView> srv;

	D3D11_TEXTURE2D_DESC rtDesc;

	renderTarget->GetDesc(&rtDesc);

	hr = m_device->CreateRenderTargetView(
		renderTarget, &CD3D11_RENDER_TARGET_VIEW_DESC(renderTarget, D3D11_RTV_DIMENSION_TEXTURE2D),
		&rtv);
	HRException::ThrowHR(hr);

	if (source != nullptr)
	{
		hr = m_device->CreateShaderResourceView(
			source, &CD3D11_SHADER_RESOURCE_VIEW_DESC(D3D11_SRV_DIMENSION_TEXTURE2D),
			&srv);
		HRException::ThrowHR(hr);
	}

	m_deviceContext->ClearState();

	ID3D11Buffer *vbufs[] = { m_vertexBuffer.GetInterfacePtr() };
	UINT strides[] = { sizeof(::DirectX::XMFLOAT4) };
	UINT offsets[] = { 0 };

	m_deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceContext->IASetInputLayout(m_layout);
	m_deviceContext->IASetVertexBuffers(0, _countof(vbufs), vbufs, strides, offsets);
	m_deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	m_deviceContext->VSSetShader(m_vertexShader, nullptr, 0);

	if (srv != nullptr)
	{
		m_deviceContext->PSSetSamplers(0, 1, &m_samplerState.GetInterfacePtr());
		m_deviceContext->PSSetShaderResources(0, 1, &srv.GetInterfacePtr());
	}

	m_deviceContext->PSSetShader(pixelShader, nullptr, 0);

	m_deviceContext->RSSetState(m_rasterizerState);

	auto viewport = D3D11_VIEWPORT();
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = rtDesc.Width;
	viewport.Height = rtDesc.Height;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	m_deviceContext->RSSetViewports(1, &viewport);

	m_deviceContext->OMSetBlendState(m_blendState, nullptr, 0xffffffff);
	m_deviceContext->OMSetRenderTargets(1, &rtv.GetInterfacePtr(), nullptr);
	m_deviceContext->DrawIndexed(6, 0, 0);
}

std::vector<uint8_t> DeviceManager::ReadTextureData(ID3D11Texture2D *src)
{
	std::vector<uint8_t> ret;
	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE mapped;
	D3D11_TEXTURE2D_DESC desc;

	src->GetDesc(&desc);

	hr = m_deviceContext->Map(src, 0, D3D11_MAP_READ, 0, &mapped);
	HRException::ThrowHR(hr);

	try
	{
		ret.resize(mapped.RowPitch * desc.Height);
		for (uint32_t i = 0; i < desc.Height; i++)
		{
			memcpy(
				&ret[i * mapped.RowPitch],
				static_cast<uint8_t *>(mapped.pData) + i * mapped.RowPitch,
				mapped.RowPitch);
		}
		m_deviceContext->Unmap(src, 0);
	}
	catch (...)
	{
		m_deviceContext->Unmap(src, 0);
		throw;
	}

	return ret;
}
