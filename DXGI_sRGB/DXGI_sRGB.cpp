// DXGI_sRGB.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include "HRException.h"
#include "DeviceManager.h"

#include "PixelShader.csh"
#include "PixelShader2.csh"


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

static void WriteCSV(const char *filename, std::vector<uint8_t> const& v)
{
	FILE *fp = fopen(filename, "w");
	if (fp != nullptr)
	{
		int i = 0;
		for (auto itr = v.begin(); itr != v.end(); itr++)
		{
			fprintf(fp, "%d,", *itr);
			if (++i % 4 == 0)
			{
				fprintf(fp, "\n");
			}
		}
		fclose(fp);
	}
}

void main2()
{
	DeviceManager deviceManager;

	auto pixelShader = deviceManager.CreatePixelShader(g_csoPixelShader, sizeof(g_csoPixelShader));
	auto pixelShader2 = deviceManager.CreatePixelShader(g_csoPixelShader2, sizeof(g_csoPixelShader2));


	auto tex = deviceManager.CreateDefaultTexture2D(DXGI_FORMAT_R8G8B8A8_UNORM);
	auto texSysmem = deviceManager.CreateStagingTexture2D(DXGI_FORMAT_R8G8B8A8_UNORM);

	deviceManager.Draw(tex, nullptr, pixelShader);
	deviceManager.CopyResource(texSysmem, tex);
	auto data1 = deviceManager.ReadTextureData(texSysmem);

	auto texSRGB = deviceManager.CreateDefaultTexture2D(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	auto texSysmemSRGB = deviceManager.CreateStagingTexture2D(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

	deviceManager.Draw(texSRGB, nullptr, pixelShader);
	deviceManager.CopyResource(texSysmemSRGB, texSRGB);
	auto data2 = deviceManager.ReadTextureData(texSysmemSRGB);


	auto texSrc = deviceManager.CreateImmutableTexture2D(
		DXGI_FORMAT_R8G8B8A8_UNORM, &data2[0], data2.size());
	auto texSrcSRGB = deviceManager.CreateImmutableTexture2D(
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, &data2[0], data2.size());


	deviceManager.Draw(tex, texSrc, pixelShader2);
	deviceManager.CopyResource(texSysmem, tex);
	auto data3 = deviceManager.ReadTextureData(texSysmem);

	deviceManager.Draw(tex, texSrcSRGB, pixelShader2);
	deviceManager.CopyResource(texSysmem, tex);
	auto data4 = deviceManager.ReadTextureData(texSysmem);

	deviceManager.Draw(texSRGB, texSrc, pixelShader2);
	deviceManager.CopyResource(texSysmemSRGB, texSRGB);
	auto data5 = deviceManager.ReadTextureData(texSysmemSRGB);

	deviceManager.Draw(texSRGB, texSrcSRGB, pixelShader2);
	deviceManager.CopyResource(texSysmemSRGB, texSRGB);
	auto data6 = deviceManager.ReadTextureData(texSysmemSRGB);

	WriteCSV("data1.csv", data1);
	WriteCSV("data2.csv", data2);
	WriteCSV("data3.csv", data3);
	WriteCSV("data4.csv", data4);
	WriteCSV("data5.csv", data5);
	WriteCSV("data6.csv", data6);
}


int main()
{
	try
	{
		main2();
	} 
	catch (const HRException& e)
	{
		printf("HRESULT = %08x\n", e.GetResult());
	}
	catch (const std::exception& e)
	{
		printf("error: %s\n", e.what());
	}

	printf("end.\n");
    return 0;
}
