#include <iostream>
#include <ctime>
#include <vector>
#include <fstream>
#include <d3d11.h>
#include <DirectXMath.h>
#include "Cube.h"
#include "Defines.h"
#include "XTime.h"
#include "brickwall.h"
#include "DDSTextureLoader.h"
#include "Trivial_PS.csh"
#include "Trivial_VS.csh"
#include "PixelShader.csh"
#include <thread>
#pragma comment(lib, "d3d11.lib")
#define BACKBUFFER_WIDTH	1280
#define BACKBUFFER_HEIGHT	720
#define NumSamples 4
#define SAFERELEASE(p) {if(p){p->Release(), p = nullptr;}}
using namespace std;
using namespace DirectX;
#define _CRT_SECURE_NO_WARNINGS

class DEMO_APP
{	
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;
	// TODO: PART 1 STEP 2

	
	IDXGISwapChain*					swapchain;
	ID3D11Device*					device;
	ID3D11DeviceContext*			devicecontext;


	ID3D11Buffer*					vertexbuffer;
	ID3D11Buffer*					indexbuffer;

	ID3D11Buffer*					planeVertex1;
	ID3D11Buffer*					planeVertex2;
	ID3D11Buffer*					planeVertex3;
	ID3D11Buffer*					planeVertex4;

	ID3D11Buffer*					planeIndex1;
	ID3D11Buffer*					planeIndex2;
	ID3D11Buffer*					planeIndex3;
	ID3D11Buffer*					planeIndex4;

	ID3D11Buffer*					quadvertexbuffer;
	ID3D11Buffer*					quadindexbuffer;
	ID3D11Buffer*					loadedvertexbuffer;
	ID3D11Buffer*					loadedvertexbuffer2;

	ID3D11Buffer*					treeindex;



	ID3D11Buffer*					Constbuffer;
	ID3D11Buffer*					Constbuffer2;
	ID3D11Buffer*					Constbuffer3;

	ID3D11Texture2D*				ZBuffer;
	ID3D11DepthStencilView*			StencilView;
	ID3D11DepthStencilState*		StencilState;
	ID3D11InputLayout*				inputlayout;
	ID3D11RenderTargetView *		pView;
	ID3D11Resource *				pBB;
	ID3D11ShaderResourceView*		shaderview;
	ID3D11ShaderResourceView*		shaderview2;
	ID3D11ShaderResourceView*		shaderview3;


	ID3D11Texture2D*				texture;
	ID3D11Texture2D*				texture2;
	ID3D11Texture2D*				texture3;


	ID3D11PixelShader*				pixelshader;

	ID3D11VertexShader*				vertshader;
	ID3D11RasterizerState*			Rasterstate;
	ID3D11RasterizerState*			transparentrender;

	ID3D11BlendState*				blendstate;

	ID3D11SamplerState*				samplerState;

	// TODO: PART 2 STEP 2

	D3D11_VIEWPORT					viewport[2];
	XTime							timer;
	unsigned int					numverticies;
	D3D11_MAPPED_SUBRESOURCE		mapsource;

	XMMATRIX						CubeWorld;
	XMMATRIX						View[2];
	XMMATRIX						Projection[2];
	XMMATRIX						SkyBoxMatrix;

public:
	float spotlighttravel = 0;
	float directionallighttravel = 0;
	bool pointlightreversed = false;
	struct Send_To_Vram
	{
		XMMATRIX worldmatrix;
	};
	struct Send_To_Vram2
	{
		XMMATRIX Viewmatrix;
		XMMATRIX Projmatrix;
	};
	struct Send_To_Vram3
	{
		float lightingbools[3];
		float padding;

		float directinoallightdir[3];
		float padding2;

		float directionallightcolor[4];

		float pointlightpos[3];
		float padding3;

		float spotlightpos[3];
		float padding4;

		float conedir[3];
		float coneratio;

	};
	XMMATRIX rotate;
	Send_To_Vram toShader;
	Send_To_Vram2 toShader2;
	Send_To_Vram3 toShader3;
	unsigned int activecamera;

	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown(); 

	SIMPLE_VERTEX plane2[4];
	SIMPLE_VERTEX plane3[4];
	SIMPLE_VERTEX plane4[4];
	SIMPLE_VERTEX plane5[4];


};

bool LoadOBJ(const char* path, 
	std::vector<XMFLOAT3>& out_vertices, 
	std::vector<XMFLOAT2>& out_uvs, 
	std::vector<XMFLOAT3>& out_normals);

DEMO_APP::DEMO_APP(HINSTANCE hinst, WNDPROC proc)
{
#pragma region Windows Code
	// ****************** BEGIN WARNING ***********************// 
	// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY! 
	application = hinst;
	appWndProc = proc;

	WNDCLASSEX  wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = appWndProc;
	wndClass.lpszClassName = L"DirectXApplication";
	wndClass.hInstance = application;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	//wndClass.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
	RegisterClassEx(&wndClass);

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(L"DirectXApplication", L"Lab 1a Line Land", WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);
	//********************* END WARNING ************************//
	// TODO: PART 1 STEP 3a
#pragma endregion

#pragma region View/Projection matricies
	toShader.worldmatrix = XMMatrixIdentity();


	View[0] = XMMatrixIdentity();
	View[1] = View[0];// XMMatrixSet(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 6, 1);
	
	Projection[0] = XMMatrixPerspectiveFovLH(XMConvertToRadians(90), ((float)(BACKBUFFER_WIDTH/2)/(float)BACKBUFFER_HEIGHT), NEAR_PLANE, FAR_PLANE);
	Projection[1] = XMMatrixPerspectiveFovLH(XMConvertToRadians(90), ((float)(BACKBUFFER_WIDTH/2) / (float)BACKBUFFER_HEIGHT), NEAR_PLANE, FAR_PLANE);

	toShader2.Viewmatrix = View[0];
	toShader2.Projmatrix = Projection[0];
	
#pragma endregion

#pragma region Create Device and SwapChain
	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BufferCount = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	desc.BufferDesc.RefreshRate = { 1, 60 };
	desc.BufferDesc.Height = BACKBUFFER_HEIGHT;
	desc.BufferDesc.Width = BACKBUFFER_WIDTH;
	desc.SampleDesc.Count = NumSamples;
	desc.OutputWindow = window;
	desc.Windowed = true;


	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, NULL , D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG, NULL, NULL , D3D11_SDK_VERSION, &desc, &swapchain, &device, NULL, &devicecontext);
#pragma endregion

#pragma region Creating Viewports
	swapchain->GetBuffer(0, __uuidof(pBB), reinterpret_cast<void**>(&pBB));
	device->CreateRenderTargetView(pBB, NULL, &pView);
	swapchain->GetDesc(&desc);
	viewport[0].Height = (FLOAT)desc.BufferDesc.Height;
	viewport[0].Width = (FLOAT)desc.BufferDesc.Width/2;
	viewport[0].TopLeftX = 0;
	viewport[0].TopLeftY = 0;
	viewport[0].MinDepth = 0;
	viewport[0].MaxDepth = 1;

	viewport[1].Height = (FLOAT)desc.BufferDesc.Height;
	viewport[1].Width = (FLOAT)desc.BufferDesc.Width/2;
	viewport[1].TopLeftX = (FLOAT)desc.BufferDesc.Width / 2;
	viewport[1].TopLeftY = 0;
	viewport[1].MinDepth = 0;
	viewport[1].MaxDepth = 1;
#pragma endregion

#pragma region Setting World Matricies
	CubeWorld = XMMatrixIdentity();
	CubeWorld = XMMatrixSet(1, 0, 0, 0,
							0, 1, 0, 0,
							0, 0, 1, 0,
							0, 0, 5, 1);
#pragma endregion

#pragma region Creating Star
	SIMPLE_VERTEX star[12];
	ZeroMemory(&star, sizeof(star));
	for (unsigned int i = 0; i < 10; i++)
	{
		if (i%2)
		{
			star[i].vertex[0] = cos(((i / 180.0f) * 36) * 3.14f);
			star[i].vertex[1] = sin(((i / 180.0f) * 36) * 3.14f);
			star[i].vertex[3] = 1;

		}
		else
		{
			star[i].vertex[0] = cos(((i / 180.0f) * 36) * 3.14f) /2;
			star[i].vertex[1] = sin(((i / 180.0f) * 36) * 3.14f) /2;
			star[i].vertex[3] = 1;
			star[i].uv[0] = 1;
		}
	}
	star[10].vertex[2] = 0.5f;
	star[11].vertex[2] = -0.5f;
	star[10].vertex[3] = 1;
	star[11].vertex[3] = 1;
	star[10].uv[0] = 1;
	star[11].uv[0] = 1;
	
	unsigned int index[60];
	index[0] = 0;
	index[1] = 1;
	index[2] = 10;
	
	index[3] = 1;
	index[4] = 2;
	index[5] = 10;
	
	index[6] = 2;
	index[7] = 3;
	index[8] = 10;
	
	index[9] = 3;
	index[10] = 4;
	index[11] = 10;
	
	index[12] = 4;
	index[13] = 5;
	index[14] = 10;
	
	index[15] = 5;
	index[16] = 6;
	index[17] = 10;
	index[18] = 6;
	index[19] = 7;
	index[20] = 10;
	index[21] = 7;
	index[22] = 8;
	index[23] = 10;
	index[24] = 8;
	index[25] = 9;
	index[26] = 10;
	index[57] = 9;
	index[58] = 0;
	index[59] = 10;
	
	index[27] = 11;
	index[28] = 1;
	index[29] = 0;
	index[30] = 11;
	index[31] = 2;
	index[32] = 1;
	index[33] = 11;
	index[34] = 3;
	index[35] = 2;
	index[36] = 11;
	index[37] = 4;
	index[38] = 3;
	index[39] = 11;
	index[40] = 5;
	index[41] = 4;
	index[42] = 11;
	index[43] = 6;
	index[44] = 5;
	index[45] = 11;
	index[46] = 7;
	index[47] = 6;
	index[48] = 11;
	index[49] = 8;
	index[50] = 7;
	index[51] = 11;
	index[52] = 9;
	index[53] = 8;
	index[54] = 11;
	index[55] = 0;
	index[56] = 9;
#pragma endregion

#pragma region Creating Plane
	SIMPLE_VERTEX plane[4];
	plane[0].vertex[0] = -100;
	plane[0].vertex[1] = -5;
	plane[0].vertex[2] = 100;
	plane[1].vertex[0] = 100;
	plane[1].vertex[1] = -5;
	plane[1].vertex[2] = 100;
	plane[2].vertex[0] = -100;
	plane[2].vertex[1] = -5;
	plane[2].vertex[2] = -100;
	plane[3].vertex[0] = 100;
	plane[3].vertex[1] = -5;
	plane[3].vertex[2] = -100;
	plane[0].normals[0] = 0;
	plane[0].normals[1] = 1;
	plane[0].normals[2] = 0;
	plane[1].normals[0] = 0;
	plane[1].normals[1] = 1;
	plane[1].normals[2] = 0;
	plane[2].normals[0] = 0;
	plane[2].normals[1] = 1;
	plane[2].normals[2] = 0;
	plane[3].normals[0] = 0;
	plane[3].normals[1] = 1;
	plane[3].normals[2] = 0;
	unsigned int planeindex[6];
	planeindex[0] = 0;
	planeindex[1] = 1;
	planeindex[2] = 2;
	planeindex[3] = 3;
	planeindex[4] = 2;
	planeindex[5] = 1;
#pragma endregion

#pragma region Creating plane2
	plane2[0].vertex[0] = -1;
	plane2[0].vertex[1] = 1;
	plane2[0].vertex[2] = 1;
	plane2[1].vertex[0] = 1;
	plane2[1].vertex[1] = 1;
	plane2[1].vertex[2] = 1;
	plane2[2].vertex[0] = -1;
	plane2[2].vertex[1] = 2;
	plane2[2].vertex[2] = 1;
	plane2[3].vertex[0] = 1;
	plane2[3].vertex[1] = 2;
	plane2[3].vertex[2] = 1;
	plane2[0].normals[0] = 0;
	plane2[0].normals[1] = 0;
	plane2[0].normals[2] = -1;
	plane2[1].normals[0] = 0;
	plane2[1].normals[1] = 0;
	plane2[1].normals[2] = -1;
	plane2[2].normals[0] = 0;
	plane2[2].normals[1] = 0;
	plane2[2].normals[2] = -1;
	plane2[3].normals[0] = 0;
	plane2[3].normals[1] = 0;
	plane2[3].normals[2] = -1;
	plane2[0].uv[0] = 0;
	plane2[0].uv[1] = 0;
	plane2[1].uv[0] = 1;
	plane2[1].uv[1] = 0;
	plane2[2].uv[0] = 0;
	plane2[2].uv[1] = 1;
	plane2[3].uv[0] = 1;
	plane2[3].uv[1] = 1;
	unsigned int planeindex2[6];
	planeindex2[0] = 0;
	planeindex2[1] = 1;
	planeindex2[2] = 2;
	planeindex2[3] = 3;
	planeindex2[4] = 2;
	planeindex2[5] = 1;
#pragma endregion

#pragma region Creating plane3
	plane3[0].vertex[0] = -1;
	plane3[0].vertex[1] = 1;
	plane3[0].vertex[2] = 2;
	plane3[1].vertex[0] = 1;
	plane3[1].vertex[1] = 1;
	plane3[1].vertex[2] = 2;
	plane3[2].vertex[0] = -1;
	plane3[2].vertex[1] = 2;
	plane3[2].vertex[2] = 2;
	plane3[3].vertex[0] = 1;
	plane3[3].vertex[1] = 2;
	plane3[3].vertex[2] = 2;
	plane3[0].normals[0] = 0;
	plane3[0].normals[1] = 0;
	plane3[0].normals[2] = -1;
	plane3[1].normals[0] = 0;
	plane3[1].normals[1] = 0;
	plane3[1].normals[2] = -1;
	plane3[2].normals[0] = 0;
	plane3[2].normals[1] = 0;
	plane3[2].normals[2] = -1;
	plane3[3].normals[0] = 0;
	plane3[3].normals[1] = 0;
	plane3[3].normals[2] = -1;
	plane3[0].uv[0] = 0;
	plane3[0].uv[1] = 0;
	plane3[1].uv[0] = 1;
	plane3[1].uv[1] = 0;
	plane3[2].uv[0] = 0;
	plane3[2].uv[1] = 1;
	plane3[3].uv[0] = 1;
	plane3[3].uv[1] = 1;
	unsigned int planeindex3[6];
	planeindex3[0] = 0;
	planeindex3[1] = 1;
	planeindex3[2] = 2;
	planeindex3[3] = 3;
	planeindex3[4] = 2;
	planeindex3[5] = 1;
#pragma endregion

#pragma region Creating plane4
	plane4[0].vertex[0] = -1;
	plane4[0].vertex[1] = 1;
	plane4[0].vertex[2] = 3;
	plane4[1].vertex[0] = 1;
	plane4[1].vertex[1] = 1;
	plane4[1].vertex[2] = 3;
	plane4[2].vertex[0] = -1;
	plane4[2].vertex[1] = 2;
	plane4[2].vertex[2] = 3;
	plane4[3].vertex[0] = 1;
	plane4[3].vertex[1] = 2;
	plane4[3].vertex[2] = 3;
	plane4[0].normals[0] = 0;
	plane4[0].normals[1] = 0;
	plane4[0].normals[2] = -1;
	plane4[1].normals[0] = 0;
	plane4[1].normals[1] = 0;
	plane4[1].normals[2] = -1;
	plane4[2].normals[0] = 0;
	plane4[2].normals[1] = 0;
	plane4[2].normals[2] = -1;
	plane4[3].normals[0] = 0;
	plane4[3].normals[1] = 0;
	plane4[3].normals[2] = -1;
	plane4[0].uv[0] = 0;
	plane4[0].uv[1] = 0;
	plane4[1].uv[0] = 1;
	plane4[1].uv[1] = 0;
	plane4[2].uv[0] = 0;
	plane4[2].uv[1] = 1;
	plane4[3].uv[0] = 1;
	plane4[3].uv[1] = 1;
	unsigned int planeindex4[6];
	planeindex4[0] = 0;
	planeindex4[1] = 1;
	planeindex4[2] = 2;
	planeindex4[3] = 3;
	planeindex4[4] = 2;
	planeindex4[5] = 1;
#pragma endregion

#pragma region Creating plane5
	plane5[0].vertex[0] = -1;
	plane5[0].vertex[1] = 1;
	plane5[0].vertex[2] = 4;
	plane5[1].vertex[0] = 1;
	plane5[1].vertex[1] = 1;
	plane5[1].vertex[2] = 4;
	plane5[2].vertex[0] = -1;
	plane5[2].vertex[1] = 2;
	plane5[2].vertex[2] = 4;
	plane5[3].vertex[0] = 1;
	plane5[3].vertex[1] = 2;
	plane5[3].vertex[2] = 4;
	plane5[0].normals[0] = 0;
	plane5[0].normals[1] = 0;
	plane5[0].normals[2] = -1;
	plane5[1].normals[0] = 0;
	plane5[1].normals[1] = 0;
	plane5[1].normals[2] = -1;
	plane5[2].normals[0] = 0;
	plane5[2].normals[1] = 0;
	plane5[2].normals[2] = -1;
	plane5[3].normals[0] = 0;
	plane5[3].normals[1] = 0;
	plane5[3].normals[2] = -1;
	plane5[0].uv[0] = 0;
	plane5[0].uv[1] = 0;
	plane5[1].uv[0] = 1;
	plane5[1].uv[1] = 0;
	plane5[2].uv[0] = 0;
	plane5[2].uv[1] = 1;
	plane5[3].uv[0] = 1;
	plane5[3].uv[1] = 1;
	unsigned int planeindex5[6];
	planeindex5[0] = 0;
	planeindex5[1] = 1;
	planeindex5[2] = 2;
	planeindex5[3] = 3;
	planeindex5[4] = 2;
	planeindex5[5] = 1;
#pragma endregion

#pragma region Setting Pixel Shader Buffer
	ZeroMemory(&toShader3, sizeof(toShader3));
	toShader3.lightingbools[0] = 0;
	toShader3.lightingbools[1] = 1;
	toShader3.lightingbools[2] = 0;

	toShader3.directinoallightdir[0] = 1;
	toShader3.directinoallightdir[1] = 0;
	toShader3.directinoallightdir[2] = 0;


	toShader3.directionallightcolor[0] = .25;
	toShader3.directionallightcolor[1] = .75;
	toShader3.directionallightcolor[2] = .25;
	toShader3.directionallightcolor[3] = 0;
	
	toShader3.pointlightpos[0] = 0;
	toShader3.pointlightpos[1] = -.9;
	toShader3.pointlightpos[2] = 4.7;

	toShader3.spotlightpos[0] = 0;
	toShader3.spotlightpos[1] = 3;
	toShader3.spotlightpos[2] = 6;

	toShader3.conedir[0] = 0;
	toShader3.conedir[1] = 0;
	toShader3.conedir[2] = -1;
	toShader3.coneratio = 0.5f;
#pragma endregion

#pragma region Object / Model Loading
	std::vector<XMFLOAT3> verticies;
	std::vector<XMFLOAT2> uvs;
	std::vector<XMFLOAT3> normals;

	LoadOBJ("test pyramid.obj", verticies, uvs, normals);
	SIMPLE_VERTEX loadedvertricies[15];
	for (unsigned int i = 0; i < 15; i++)
	{
		loadedvertricies[i].vertex[0] = verticies[i].x;
		loadedvertricies[i].vertex[1] = verticies[i].y;
		loadedvertricies[i].vertex[2] = verticies[i].z;
		loadedvertricies[i].uv[0] = uvs[i].x;
		loadedvertricies[i].uv[1] = (uvs[i].y);
		loadedvertricies[i].normals[0] = normals[i].x;
		loadedvertricies[i].normals[1] = normals[i].y;
		loadedvertricies[i].normals[2] = normals[i].z;
	}
																																																																						
	std::vector<XMFLOAT3> verticies2;
	std::vector<XMFLOAT2> uvs2;
	std::vector<XMFLOAT3> normals2;

	LoadOBJ("treething2.obj", verticies2, uvs2, normals2);
	SIMPLE_VERTEX loadedvertricies2[1092];
	unsigned int loadedindicies2[1092] = { 0 };
	for (unsigned int i = 0; i < 1092; i++)
	{
		loadedvertricies2[i].vertex[0] = verticies2[i].x;
		loadedvertricies2[i].vertex[1] = verticies2[i].y;
		loadedvertricies2[i].vertex[2] = verticies2[i].z;
		loadedvertricies2[i].uv[0] = (uvs2[i].x);
		loadedvertricies2[i].uv[1] = (1.0f-uvs2[i].y);
		loadedvertricies2[i].normals[0] = normals2[i].x;
		loadedvertricies2[i].normals[1] = normals2[i].y;
		loadedvertricies2[i].normals[2] = normals2[i].z;

		loadedindicies2[i] = i;
	}
#pragma endregion

#pragma region Creating Skybox
	SIMPLE_VERTEX cube[8];
	cube[0].vertex[0] = 0.25f;
	cube[0].vertex[1] = 0.25f;
	cube[0].vertex[2] = 0.25f;
	cube[0].vertex[3] = 1;
		
	cube[1].vertex[0] = -0.25f;
	cube[1].vertex[1] = 0.25f;
	cube[1].vertex[2] = 0.25f;
	cube[1].vertex[3] = 1;
		
	cube[2].vertex[0] = -0.25f;
	cube[2].vertex[1] = 0.25f;
	cube[2].vertex[2] = -0.25f;
	cube[2].vertex[3] = 1;
		
	cube[3].vertex[0] = 0.25f;
	cube[3].vertex[1] = 0.25f;
	cube[3].vertex[2] = -0.25f;
	cube[3].vertex[3] = 1;
		
	cube[4].vertex[0] = 0.25f;
	cube[4].vertex[1] = -0.25f;
	cube[4].vertex[2] = 0.25f;
	cube[4].vertex[3] = 1;
		
	cube[5].vertex[0] = -0.25f;
	cube[5].vertex[1] = -0.25f;
	cube[5].vertex[2] = 0.25f;
	cube[5].vertex[3] = 1;
		
	cube[6].vertex[0] = -0.25f;
	cube[6].vertex[1] = -0.25f;
	cube[6].vertex[2] = -0.25f;
	cube[6].vertex[3] = 1;
		
	cube[7].vertex[0] = 0.25f;
	cube[7].vertex[1] = -0.25f;
	cube[7].vertex[2] = -0.25f;
	cube[7].vertex[3] = 1;

	cube[0].uv[0] = 1;
	cube[0].uv[1] = 0;
			
	cube[1].uv[0] = 0;
	cube[1].uv[1] = 0;
			
	cube[2].uv[0] = 0;
	cube[2].uv[1] = 1;
			
	cube[3].uv[0] = 1;
	cube[3].uv[1] = 1;

			
	cube[4].uv[0] = 0;
	cube[4].uv[1] = 0;

			
	cube[5].uv[0] = 1;
	cube[5].uv[1] = 0;

			
	cube[6].uv[0] = 1;
	cube[6].uv[1] = 1;

			
	cube[7].uv[0] = 1;
	cube[7].uv[1] = 0;

	unsigned int cubeindex[36];
	cubeindex[0] = 0;
	cubeindex[1] = 1;
	cubeindex[2] = 2;
	cubeindex[3] = 3;
	cubeindex[4] = 0;
	cubeindex[5] = 2;
	cubeindex[6] = 4;
	cubeindex[7] = 5;
	cubeindex[8] = 6;
	cubeindex[9] = 7;
	cubeindex[10] = 4;
	cubeindex[11] = 6;
	cubeindex[12] = 0;
	cubeindex[13] = 4;
	cubeindex[14] = 1;
	cubeindex[15] = 1;
	cubeindex[16] = 5;
	cubeindex[17] = 4;
	cubeindex[18] = 0;
	cubeindex[19] = 4;
	cubeindex[20] = 3;
	cubeindex[21] = 3;
	cubeindex[22] = 4;
	cubeindex[23] = 7;
	cubeindex[24] = 1;
	cubeindex[25] = 5;
	cubeindex[26] = 6;
	cubeindex[27] = 2;
	cubeindex[28] = 1;
	cubeindex[29] = 6;
	cubeindex[30] = 3;
	cubeindex[31] = 6;
	cubeindex[32] = 2;
	cubeindex[33] = 3;
	cubeindex[34] = 6;
	cubeindex[45] = 7;









#pragma endregion

#pragma region Setting Vertex Buffer Descriptions

	D3D11_BUFFER_DESC vertbuffdesc;
	ZeroMemory(&vertbuffdesc, sizeof(vertbuffdesc));
	vertbuffdesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertbuffdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertbuffdesc.CPUAccessFlags = NULL;
	vertbuffdesc.ByteWidth = sizeof(OBJ_VERT) * 12;
	vertbuffdesc.StructureByteStride = sizeof(OBJ_VERT);

	D3D11_BUFFER_DESC vertbuffdesc2;
	ZeroMemory(&vertbuffdesc2, sizeof(vertbuffdesc2));
	vertbuffdesc2.Usage = D3D11_USAGE_IMMUTABLE;
	vertbuffdesc2.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertbuffdesc2.CPUAccessFlags = NULL;
	vertbuffdesc2.ByteWidth = sizeof(OBJ_VERT) * 4;
	vertbuffdesc2.StructureByteStride = sizeof(OBJ_VERT);

	D3D11_BUFFER_DESC planebuff;
	ZeroMemory(&planebuff, sizeof(planebuff));
	planebuff.Usage = D3D11_USAGE_IMMUTABLE;
	planebuff.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	planebuff.CPUAccessFlags = NULL;
	planebuff.ByteWidth = sizeof(OBJ_VERT) * 4;
	planebuff.StructureByteStride = sizeof(OBJ_VERT);

	D3D11_BUFFER_DESC planebuff2;
	ZeroMemory(&planebuff2, sizeof(planebuff2));
	planebuff2.Usage = D3D11_USAGE_IMMUTABLE;
	planebuff2.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	planebuff2.CPUAccessFlags = NULL;
	planebuff2.ByteWidth = sizeof(OBJ_VERT) * 4;
	planebuff2.StructureByteStride = sizeof(OBJ_VERT);

	D3D11_BUFFER_DESC planebuff3;
	ZeroMemory(&planebuff3, sizeof(planebuff3));
	planebuff3.Usage = D3D11_USAGE_IMMUTABLE;
	planebuff3.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	planebuff3.CPUAccessFlags = NULL;
	planebuff3.ByteWidth = sizeof(OBJ_VERT) * 4;
	planebuff3.StructureByteStride = sizeof(OBJ_VERT);

	D3D11_BUFFER_DESC planebuff4;
	ZeroMemory(&planebuff4, sizeof(planebuff4));
	planebuff4.Usage = D3D11_USAGE_IMMUTABLE;
	planebuff4.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	planebuff4.CPUAccessFlags = NULL;
	planebuff4.ByteWidth = sizeof(OBJ_VERT) * 4;
	planebuff4.StructureByteStride = sizeof(OBJ_VERT);


	D3D11_BUFFER_DESC vertbuffdesc3;
	ZeroMemory(&vertbuffdesc3, sizeof(vertbuffdesc3));
	vertbuffdesc3.Usage = D3D11_USAGE_IMMUTABLE;
	vertbuffdesc3.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertbuffdesc3.CPUAccessFlags = NULL;
	vertbuffdesc3.ByteWidth = sizeof(SIMPLE_VERTEX) * verticies.size();
	vertbuffdesc3.StructureByteStride = sizeof(SIMPLE_VERTEX);

	D3D11_BUFFER_DESC vertbuffdesc4;
	ZeroMemory(&vertbuffdesc4, sizeof(vertbuffdesc4));
	vertbuffdesc4.Usage = D3D11_USAGE_IMMUTABLE;
	vertbuffdesc4.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertbuffdesc4.CPUAccessFlags = NULL;
	vertbuffdesc4.ByteWidth = sizeof(SIMPLE_VERTEX) * verticies2.size();
	vertbuffdesc4.StructureByteStride = sizeof(SIMPLE_VERTEX);

	D3D11_BUFFER_DESC vertbuffdesc5;
	ZeroMemory(&vertbuffdesc5, sizeof(vertbuffdesc5));
	vertbuffdesc5.Usage = D3D11_USAGE_IMMUTABLE;
	vertbuffdesc5.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertbuffdesc5.CPUAccessFlags = NULL;
	vertbuffdesc5.ByteWidth = sizeof(SIMPLE_VERTEX) * 8;
	vertbuffdesc5.StructureByteStride = sizeof(SIMPLE_VERTEX);
#pragma endregion

#pragma region Setting Index Buffer Descriptions
	D3D11_BUFFER_DESC indexbuffdesc;
	ZeroMemory(&indexbuffdesc, sizeof(indexbuffdesc));
	indexbuffdesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexbuffdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexbuffdesc.CPUAccessFlags = NULL;
	indexbuffdesc.ByteWidth = sizeof(unsigned int) * 60;
	indexbuffdesc.StructureByteStride = sizeof(unsigned int);


	D3D11_BUFFER_DESC indexbuffdesc2;
	ZeroMemory(&indexbuffdesc2, sizeof(indexbuffdesc2));
	indexbuffdesc2.Usage = D3D11_USAGE_IMMUTABLE;
	indexbuffdesc2.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexbuffdesc2.CPUAccessFlags = NULL;
	indexbuffdesc2.ByteWidth = sizeof(unsigned int) * 6;
	indexbuffdesc2.StructureByteStride = sizeof(unsigned int);

	D3D11_BUFFER_DESC indexbuffdesc3;
	ZeroMemory(&indexbuffdesc3, sizeof(indexbuffdesc3));
	indexbuffdesc3.Usage = D3D11_USAGE_IMMUTABLE;
	indexbuffdesc3.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexbuffdesc3.CPUAccessFlags = NULL;
	indexbuffdesc3.ByteWidth = sizeof(unsigned int) * 36;
	indexbuffdesc3.StructureByteStride = sizeof(unsigned int);


	D3D11_BUFFER_DESC PlaneIndex;
	ZeroMemory(&PlaneIndex, sizeof(PlaneIndex));
	PlaneIndex.Usage = D3D11_USAGE_IMMUTABLE;
	PlaneIndex.BindFlags = D3D11_BIND_INDEX_BUFFER;
	PlaneIndex.CPUAccessFlags = NULL;
	PlaneIndex.ByteWidth = sizeof(unsigned int) * 6;
	PlaneIndex.StructureByteStride = sizeof(unsigned int);

	D3D11_BUFFER_DESC PlaneIndex2;
	ZeroMemory(&PlaneIndex2, sizeof(PlaneIndex2));
	PlaneIndex2.Usage = D3D11_USAGE_IMMUTABLE;
	PlaneIndex2.BindFlags = D3D11_BIND_INDEX_BUFFER;
	PlaneIndex2.CPUAccessFlags = NULL;
	PlaneIndex2.ByteWidth = sizeof(unsigned int) * 6;
	PlaneIndex2.StructureByteStride = sizeof(unsigned int);

	D3D11_BUFFER_DESC PlaneIndex3;
	ZeroMemory(&PlaneIndex3, sizeof(PlaneIndex3));
	PlaneIndex3.Usage = D3D11_USAGE_IMMUTABLE;
	PlaneIndex3.BindFlags = D3D11_BIND_INDEX_BUFFER;
	PlaneIndex3.CPUAccessFlags = NULL;
	PlaneIndex3.ByteWidth = sizeof(unsigned int) * 6;
	PlaneIndex3.StructureByteStride = sizeof(unsigned int);

	D3D11_BUFFER_DESC PlaneIndex4;
	ZeroMemory(&PlaneIndex4, sizeof(PlaneIndex4));
	PlaneIndex4.Usage = D3D11_USAGE_IMMUTABLE;
	PlaneIndex4.BindFlags = D3D11_BIND_INDEX_BUFFER;
	PlaneIndex4.CPUAccessFlags = NULL;
	PlaneIndex4.ByteWidth = sizeof(unsigned int) * 6;
	PlaneIndex4.StructureByteStride = sizeof(unsigned int);

	D3D11_BUFFER_DESC TreeIndex;
	ZeroMemory(&TreeIndex, sizeof(TreeIndex));
	TreeIndex.Usage = D3D11_USAGE_IMMUTABLE;
	TreeIndex.BindFlags = D3D11_BIND_INDEX_BUFFER;
	TreeIndex.CPUAccessFlags = NULL;
	TreeIndex.ByteWidth = sizeof(unsigned int) * 1092;
	TreeIndex.StructureByteStride = sizeof(unsigned int);
#pragma endregion

#pragma region Setting Constant Buffer Descriptions
	D3D11_BUFFER_DESC constbuffdesc;
	ZeroMemory(&constbuffdesc, sizeof(constbuffdesc));
	constbuffdesc.Usage = D3D11_USAGE_DYNAMIC;
	constbuffdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constbuffdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constbuffdesc.ByteWidth = sizeof(Send_To_Vram);
	constbuffdesc.StructureByteStride = 0;
#pragma endregion

#pragma region Creating Input Layout Description
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
#pragma endregion

#pragma region Creating SubResource Data
	D3D11_SUBRESOURCE_DATA data6;
	ZeroMemory(&data6, sizeof(data6));
	data6.pSysMem = &loadedvertricies;

	D3D11_SUBRESOURCE_DATA data8;
	ZeroMemory(&data8, sizeof(data8));
	data8.pSysMem = loadedvertricies2;

	D3D11_SUBRESOURCE_DATA data9;
	ZeroMemory(&data9, sizeof(data9));
	data9.pSysMem = &cube;

	D3D11_SUBRESOURCE_DATA data10;
	ZeroMemory(&data10, sizeof(data10));
	data10.pSysMem = &cubeindex;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = &star; 

	D3D11_SUBRESOURCE_DATA indexdata;
	ZeroMemory(&indexdata, sizeof(indexdata));
	indexdata.pSysMem = &index;
	
	D3D11_SUBRESOURCE_DATA data2;
	ZeroMemory(&data2, sizeof(data2));
	data2.pSysMem = &plane;
	 
	D3D11_SUBRESOURCE_DATA indexdata2;
	ZeroMemory(&indexdata2, sizeof(indexdata2));
	indexdata2.pSysMem = &planeindex;

	D3D11_SUBRESOURCE_DATA data3;
	ZeroMemory(&data3, sizeof(data3));
	data3.pSysMem = &plane2;

	D3D11_SUBRESOURCE_DATA indexdata3;
	ZeroMemory(&indexdata3, sizeof(indexdata3));
	indexdata3.pSysMem = &planeindex2;

	D3D11_SUBRESOURCE_DATA data4;
	ZeroMemory(&data4, sizeof(data4));
	data4.pSysMem = &plane3;

	D3D11_SUBRESOURCE_DATA indexdata4;
	ZeroMemory(&indexdata4, sizeof(indexdata4));
	indexdata4.pSysMem = &planeindex3;

	D3D11_SUBRESOURCE_DATA data5;
	ZeroMemory(&data5, sizeof(data5));
	data5.pSysMem = &plane4;

	D3D11_SUBRESOURCE_DATA indexdata5;
	ZeroMemory(&indexdata5, sizeof(indexdata5));
	indexdata5.pSysMem = &planeindex4;

	D3D11_SUBRESOURCE_DATA data7;
	ZeroMemory(&data7, sizeof(data7));
	data7.pSysMem = plane5;

	D3D11_SUBRESOURCE_DATA indexdata7;
	ZeroMemory(&indexdata7, sizeof(indexdata7));
	indexdata7.pSysMem = planeindex5;

	D3D11_SUBRESOURCE_DATA treeindexdata;
	ZeroMemory(&treeindexdata, sizeof(treeindexdata));
	treeindexdata.pSysMem = loadedindicies2;

	D3D11_SUBRESOURCE_DATA constdata;
	constdata.pSysMem = &toShader;
#pragma endregion

#pragma region Creating Buffers
	device->CreateBuffer(&vertbuffdesc, &data, &vertexbuffer);
	device->CreateBuffer(&vertbuffdesc2, &data2, &quadvertexbuffer);
	device->CreateBuffer(&vertbuffdesc3, &data6, &loadedvertexbuffer);
	device->CreateBuffer(&vertbuffdesc4, &data8, &loadedvertexbuffer2);


	device->CreateBuffer(&planebuff, &data3, &planeVertex1);
	device->CreateBuffer(&planebuff2, &data4, &planeVertex2);
	device->CreateBuffer(&planebuff3, &data5, &planeVertex3);
	device->CreateBuffer(&planebuff4, &data7, &planeVertex4);

	device->CreateBuffer(&indexbuffdesc, &indexdata, &indexbuffer);
	device->CreateBuffer(&indexbuffdesc2, &indexdata2, &quadindexbuffer);

	device->CreateBuffer(&PlaneIndex, &indexdata3, &planeIndex1);
	device->CreateBuffer(&PlaneIndex2, &indexdata4, &planeIndex2);
	device->CreateBuffer(&PlaneIndex3, &indexdata5, &planeIndex3);
	device->CreateBuffer(&PlaneIndex4, &indexdata7, &planeIndex4);

	device->CreateBuffer(&TreeIndex, &treeindexdata, &treeindex);


	device->CreateBuffer(&constbuffdesc, &constdata, &Constbuffer);
	constbuffdesc.ByteWidth = sizeof(Send_To_Vram2);
	constdata.pSysMem = &toShader2;
	device->CreateBuffer(&constbuffdesc, &constdata, &Constbuffer2);
	constbuffdesc.ByteWidth = sizeof(Send_To_Vram3);
	constdata.pSysMem = &toShader3;
	device->CreateBuffer(&constbuffdesc, &constdata, &Constbuffer3);
#pragma endregion

#pragma region Creating Pixel Shaders
	device->CreatePixelShader(&Trivial_PS, sizeof(Trivial_PS), NULL, &pixelshader);
#pragma endregion

#pragma region Creating Vertex Shaders
	device->CreateVertexShader(&Trivial_VS, sizeof(Trivial_VS), NULL, &vertshader);
#pragma endregion

#pragma region Creating Input Layouts
	device->CreateInputLayout(layout, 3, Trivial_VS, sizeof(Trivial_VS), &inputlayout);
#pragma endregion
	
#pragma region Creating ZBuffer
	D3D11_TEXTURE2D_DESC textdesc;
	ZeroMemory(&textdesc, sizeof(textdesc));
	textdesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textdesc.Format = DXGI_FORMAT_D32_FLOAT;
	textdesc.Width = BACKBUFFER_WIDTH;
	textdesc.Height = BACKBUFFER_HEIGHT;
	textdesc.ArraySize = 1;
	textdesc.SampleDesc = { NumSamples, 0 };
	textdesc.MipLevels = 1;
	
	device->CreateTexture2D(&textdesc, NULL, &ZBuffer);
#pragma endregion

#pragma region Creating Depth Stencil State/View
	D3D11_DEPTH_STENCIL_VIEW_DESC stenview;
	ZeroMemory(&stenview, sizeof(stenview));
	stenview.Format = DXGI_FORMAT_D32_FLOAT;
	stenview.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	D3D11_DEPTH_STENCIL_DESC stencildesc;
	ZeroMemory(&stencildesc, sizeof(stencildesc));
	stencildesc.DepthEnable = true;
	stencildesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	stencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	stencildesc.StencilEnable = true;
	stencildesc.StencilReadMask = 0xFF;
	stencildesc.StencilWriteMask = 0xFF;
	D3D11_DEPTH_STENCILOP_DESC desc1;
	desc1.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	desc1.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc1.StencilFunc = D3D11_COMPARISON_ALWAYS;
	desc1.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	stencildesc.BackFace = desc1;
	desc1.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	stencildesc.FrontFace = desc1;

	device->CreateDepthStencilView(ZBuffer, nullptr, &StencilView);
	device->CreateDepthStencilState(&stencildesc, &StencilState);
#pragma endregion

#pragma region Setting Random Variables
	rotate = XMMatrixIdentity();
	activecamera = 0;
#pragma endregion

#pragma region Creating Textures
	D3D11_TEXTURE2D_DESC texturedesc;
	ZeroMemory(&texturedesc, sizeof(texturedesc));
	texturedesc.ArraySize = 1;
	texturedesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texturedesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texturedesc.Height = brickwall_height;
	texturedesc.Width = brickwall_width;
	texturedesc.MipLevels = brickwall_numlevels;
	texturedesc.SampleDesc = { 1, 0 };
	texturedesc.SampleDesc.Count = 2;
	
	std::thread thread1;
	std::thread thread2;
	std::thread thread3;
	//multithreading! :D

	thread1 = std::thread(CreateDDSTextureFromFile, device, L"brickwall.dds", (ID3D11Resource**)&texture, &shaderview, 0);
	thread2 = std::thread(CreateDDSTextureFromFile, device, L"numbers_test.dds", (ID3D11Resource**)&texture2, &shaderview2, 0);
	thread3 = std::thread(CreateDDSTextureFromFile, device, L"Treething.dds", (ID3D11Resource**)&texture3, &shaderview3, 0);
	thread1.join();
	thread2.join();
	thread3.join();


#pragma endregion

#pragma region Creating Rasterizer State
	//D3D11_RASTERIZER_DESC rasterdesc;
	//ZeroMemory(&rasterdesc, sizeof(rasterdesc));
	//rasterdesc.FrontCounterClockwise = false;
	//rasterdesc.FillMode = D3D11_FILL_SOLID;
	//rasterdesc.CullMode = D3D11_CULL_NONE;
	//rasterdesc.AntialiasedLineEnable = true;
	//device->CreateRasterizerState(&rasterdesc, &Rasterstate);

	D3D11_RASTERIZER_DESC rasterdesc2;
	ZeroMemory(&rasterdesc2, sizeof(rasterdesc2));
	rasterdesc2.FrontCounterClockwise = false;
	rasterdesc2.FillMode = D3D11_FILL_SOLID;
	rasterdesc2.CullMode = D3D11_CULL_NONE;
	rasterdesc2.DepthClipEnable = true;
	rasterdesc2.AntialiasedLineEnable = true;
	rasterdesc2.MultisampleEnable = true;
	device->CreateRasterizerState(&rasterdesc2, &transparentrender);

	D3D11_RASTERIZER_DESC rasterdesc3;
	ZeroMemory(&rasterdesc3, sizeof(rasterdesc3));
	rasterdesc3.FrontCounterClockwise = false;
	rasterdesc3.FillMode = D3D11_FILL_SOLID;
	rasterdesc3.CullMode = D3D11_CULL_FRONT;
	rasterdesc3.DepthClipEnable = true;
	rasterdesc3.AntialiasedLineEnable = true;
	rasterdesc3.MultisampleEnable = true;
 #pragma endregion

#pragma region Creating Blend State

	D3D11_BLEND_DESC blenddesc;
	ZeroMemory(&blenddesc, sizeof(blenddesc));
	blenddesc.RenderTarget[0].BlendEnable = TRUE;
	blenddesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blenddesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blenddesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blenddesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blenddesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blenddesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blenddesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blenddesc, &blendstate);

#pragma endregion
	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(sampler_desc));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.MinLOD = -FLT_MAX;
	sampler_desc.MaxLOD = FLT_MAX;
	sampler_desc.MipLODBias = 0.0f;
	sampler_desc.MaxAnisotropy = 1;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	device->CreateSamplerState(&sampler_desc, &samplerState);
}

bool DEMO_APP::Run()
{
	// TODO: PART 4 STEP 2	
	timer.Signal();
#pragma region Camera Movement
	if (GetAsyncKeyState(VK_UP) && GetAsyncKeyState(VK_SHIFT))
		View[0] = XMMatrixMultiply(XMMatrixTranslation(0.0f, (float)(15.0f * (float)timer.SmoothDelta()), 0.0f),View[0]);
	else if (GetAsyncKeyState(VK_UP) && GetAsyncKeyState(VK_SPACE))
	{
		XMVECTOR temp = View[0].r[3];
		View[0].r[3] = XMVectorZero();
		View[0] = XMMatrixMultiply(XMMatrixRotationX(XMConvertToRadians((float)(45.0f * (float)timer.SmoothDelta()))),View[0]);
		View[0].r[3] = temp;
	}
	else if (GetAsyncKeyState(VK_UP))
		View[0] = XMMatrixMultiply(XMMatrixTranslation(0, 0, (float)(15.0f * (float)timer.SmoothDelta())),View[0]);
	if (GetAsyncKeyState(VK_DOWN) && GetAsyncKeyState(VK_SHIFT))
		View[0] = XMMatrixMultiply(XMMatrixTranslation(0, (float)(-15.0f * (float)timer.SmoothDelta()), 0),View[0]);
	else if (GetAsyncKeyState(VK_DOWN) && GetAsyncKeyState(VK_SPACE))
	{
		XMVECTOR temp =View[0].r[3];
		View[0].r[3] = XMVectorZero();
		View[0] = XMMatrixMultiply(XMMatrixRotationX(XMConvertToRadians((float)(-45.0f * (float)timer.SmoothDelta()))),View[0]);
		View[0].r[3] = temp;
	}
	else if (GetAsyncKeyState(VK_DOWN))
		View[0] = XMMatrixMultiply(XMMatrixTranslation(0, 0, (float)(-15.0f * (float)timer.SmoothDelta())),View[0]);
	if (GetAsyncKeyState(VK_RIGHT) && GetAsyncKeyState(VK_SHIFT))
	{
		XMVECTOR temp =View[0].r[3];
		View[0].r[3] = XMVectorZero();
		View[0] = XMMatrixMultiply(XMMatrixRotationY(XMConvertToRadians((float)(45.0f * (float)timer.SmoothDelta()))),View[0]);
		View[0].r[3] = temp;
	}
	else if (GetAsyncKeyState(VK_RIGHT))
		View[0] = XMMatrixMultiply(XMMatrixTranslation((float)(15.0f * (float)timer.SmoothDelta()), 0, 0),View[0]);
	if (GetAsyncKeyState(VK_LEFT) && GetAsyncKeyState(VK_SHIFT))
	{
		XMVECTOR temp =View[0].r[3];
		View[0].r[3] = XMVectorZero();
		View[0] = XMMatrixMultiply(XMMatrixRotationY(XMConvertToRadians((float)(-45.0f * (float)timer.SmoothDelta()))),View[0]);
		View[0].r[3] = temp;
	}
	else if (GetAsyncKeyState(VK_LEFT))
		View[0] = XMMatrixMultiply(XMMatrixTranslation((float)(-15.0f * (float)timer.SmoothDelta()), 0, 0),View[0]);
#pragma endregion

#pragma region Lighting Toggles
	if (GetAsyncKeyState(VK_NUMPAD0) & 0x1)
	{
		if (toShader3.lightingbools[0] == 1)
			toShader3.lightingbools[0] = 0;
		else
			toShader3.lightingbools[0] = 1;
	}
	if (GetAsyncKeyState(VK_NUMPAD1) & 0x1)
	{
		if (toShader3.lightingbools[1] == 1)
			toShader3.lightingbools[1] = 0;
		else
			toShader3.lightingbools[1] = 1;
	}
	if (GetAsyncKeyState(VK_NUMPAD2) & 0x1)
	{
		if (toShader3.lightingbools[2] == 1)
			toShader3.lightingbools[2] = 0;
		else
			toShader3.lightingbools[2] = 1;
	}  
#pragma endregion

#pragma region Light Movement
	if (spotlighttravel == 0)
	{
		toShader3.conedir[2] += 1 * (float)timer.SmoothDelta();
		toShader3.conedir[1] -= 1 * (float)timer.SmoothDelta();
		toShader3.spotlightpos[2] -= 1 * (float)timer.SmoothDelta();
	}
	else if (spotlighttravel == 1)
	{
		toShader3.conedir[2] += 1 * (float)timer.SmoothDelta();
		toShader3.conedir[1] += 1 * (float)timer.SmoothDelta();
		toShader3.spotlightpos[2] -= 1 * (float)timer.SmoothDelta();
	}
	else if (spotlighttravel == 2)
	{
		toShader3.conedir[2] -= 1 * (float)timer.SmoothDelta();
		toShader3.conedir[1] -= 1 * (float)timer.SmoothDelta();
		toShader3.spotlightpos[2] += 1 * (float)timer.SmoothDelta();
	}
	else if (spotlighttravel == 3)
	{
		toShader3.conedir[2] -= 1 * (float)timer.SmoothDelta();
		toShader3.conedir[1] += 1 * (float)timer.SmoothDelta();
		toShader3.spotlightpos[2] += 1 * (float)timer.SmoothDelta();
	}
	if (toShader3.conedir[2] <= 0.01 && toShader3.conedir[2] >= -0.01 && spotlighttravel == 0)
		spotlighttravel = 1;
	if (toShader3.conedir[2] >= 1 && spotlighttravel == 1)
		spotlighttravel = 2;
	if (toShader3.conedir[2] <= 0.1 && toShader3.conedir[2] >= -0.1 && spotlighttravel == 2)
		spotlighttravel = 3;
	if (toShader3.conedir[2] <= -1 && spotlighttravel == 3)
		spotlighttravel = 0;

	if (directionallighttravel == 0)
	{
		toShader3.directinoallightdir[0] -= 1 * (float)timer.SmoothDelta();
		toShader3.directinoallightdir[1] -= 1 * (float)timer.SmoothDelta();
	}
	else if (directionallighttravel == 1)
	{
		toShader3.directinoallightdir[0] -= 1 * (float)timer.SmoothDelta();
		toShader3.directinoallightdir[1] += 1 * (float)timer.SmoothDelta();
	}
	else if (directionallighttravel == 2)
	{
		toShader3.directinoallightdir[0] += 1 * (float)timer.SmoothDelta();
		toShader3.directinoallightdir[1] -= 1 * (float)timer.SmoothDelta();
	}
	else if (directionallighttravel == 3)
	{
		toShader3.directinoallightdir[0] -= 1 * (float)timer.SmoothDelta();
		toShader3.directinoallightdir[1] += 1 * (float)timer.SmoothDelta();
	}
	if (toShader3.directinoallightdir[0] <= 0.01 && toShader3.directinoallightdir[0] >= -0.01 && directionallighttravel == 0)
		directionallighttravel = 1;
	if (toShader3.directinoallightdir[0] <= -1 && directionallighttravel == 1)
		directionallighttravel = 2;
	if (toShader3.directinoallightdir[0] <= 0.01 && toShader3.directinoallightdir[0] >= -0.01 && directionallighttravel == 2)
		directionallighttravel = 3;
	if (toShader3.directinoallightdir[0] >= 1 && directionallighttravel == 3)
		directionallighttravel = 0;

	if (!pointlightreversed)
		toShader3.pointlightpos[0] += (float)timer.SmoothDelta();
	else
		toShader3.pointlightpos[0] -= (float)timer.SmoothDelta();
	if (toShader3.pointlightpos[0] >= 10)
		pointlightreversed = true;
	if (toShader3.pointlightpos[0] <= -10)
		pointlightreversed = false;
#pragma endregion

#pragma region Setting Render Targets
		devicecontext->OMSetRenderTargets(1, &pView, StencilView);
#pragma endregion

#pragma region Setting Depth Stencil State
		//devicecontext->OMSetDepthStencilState(StencilState, 0);
		devicecontext->ClearDepthStencilView(StencilView, D3D11_CLEAR_DEPTH, 1, 0);
#pragma endregion

#pragma region Clearing Buffers
		float color[4] = { 0, 0, 1, 0 };
		devicecontext->ClearRenderTargetView(pView, color);
#pragma endregion

	for (unsigned int i = 0; i < 2; i++)
	{
#pragma region Setting Projection/View Matricies
		XMVECTOR vec = View[i].r[3];
		toShader2.Viewmatrix = XMMatrixInverse(&vec, View[i]);
		toShader2.Projmatrix = Projection[i];
#pragma endregion
		devicecontext->RSSetViewports(1, &viewport[i]);

		UINT offset = { 0 };
		UINT stride = sizeof(SIMPLE_VERTEX);

#pragma region Setting Input Layouts
		devicecontext->IASetInputLayout(inputlayout);
#pragma endregion

#pragma region Setting Vertex Shaders
		devicecontext->VSSetShader(vertshader, NULL, NULL);
#pragma endregion

#pragma region Setting Pixel Shaders
#pragma endregion

#pragma region Setting Primitive Topology
		devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
#pragma endregion

#pragma region Setting Blend State
		devicecontext->OMSetBlendState(blendstate, NULL, 0xFFFFFFFF);
#pragma endregion

#pragma region Setting Rasterizer State
#pragma endregion

#pragma region Mapping
		devicecontext->Map(Constbuffer3, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &mapsource);
		memcpy(mapsource.pData, &toShader3, sizeof(toShader3));
		devicecontext->Unmap(Constbuffer3, NULL);
		devicecontext->Map(Constbuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &mapsource);
		memcpy(mapsource.pData, &toShader, sizeof(toShader));
		devicecontext->Unmap(Constbuffer, NULL);
		devicecontext->Map(Constbuffer2, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &mapsource);
		memcpy(mapsource.pData, &toShader2, sizeof(toShader2));
		devicecontext->Unmap(Constbuffer2, NULL);
#pragma endregion

#pragma region Setting World Matrix
		toShader.worldmatrix = XMMatrixScaling(100, 100, 100);
		toShader.worldmatrix.r[3] = View[0].r[3];
#pragma endregion

#pragma region Mapping
		devicecontext->Map(Constbuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &mapsource);
		memcpy(mapsource.pData, &toShader, sizeof(toShader));
		devicecontext->Unmap(Constbuffer, NULL);
#pragma endregion

#pragma region Setting Constant Buffers
		devicecontext->VSSetConstantBuffers(1, 1, &Constbuffer);
		devicecontext->VSSetConstantBuffers(2, 1, &Constbuffer2);
		devicecontext->PSSetConstantBuffers(1, 1, &Constbuffer3);
#pragma endregion


#pragma region Setting Shader Resources
		devicecontext->PSSetShaderResources(0, 1, &shaderview);
#pragma endregion



#pragma region Drawing Skybox
		//devicecontext->DrawIndexed(36, 0, 0);
#pragma  
#pragma region Setting Pixel Shaders
		devicecontext->PSSetShader(pixelshader, NULL, NULL);
#pragma endregion
#pragma region Setting Rasterizer State
		devicecontext->RSSetState(NULL);
#pragma endregion
#pragma region Setting World Matrix
		toShader.worldmatrix = XMMatrixSet(1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 5, 1);
#pragma endregion

#pragma region Mapping
		devicecontext->Map(Constbuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &mapsource);
		memcpy(mapsource.pData, &toShader, sizeof(toShader));
		devicecontext->Unmap(Constbuffer, NULL);
		devicecontext->VSSetConstantBuffers(1, 1, &Constbuffer);

#pragma endregion

#pragma region Setting Vertex Buffers
		devicecontext->IASetVertexBuffers(NULL, 1, &quadvertexbuffer, &stride, &offset);
#pragma endregion

#pragma region Setting Index Buffers
		devicecontext->IASetIndexBuffer(quadindexbuffer, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
#pragma endregion
#pragma region Setting Sampler State
		devicecontext->PSSetSamplers(0, 1, &samplerState);
#pragma endregion
#pragma region Drawing Plane
		devicecontext->DrawIndexed(6, 0, 0);
#pragma endregion

#pragma region Setting Vertex Buffers
		devicecontext->IASetVertexBuffers(NULL, 1, &loadedvertexbuffer, &stride, &offset);
#pragma endregion
#pragma region Setting Sampler State
		devicecontext->PSSetSamplers(0, 1, &samplerState);
#pragma endregion
#pragma region Draw Pyramid
		devicecontext->Draw(15, 0);
#pragma endregion

#pragma region Setting Rasterizer State
		devicecontext->RSSetState(transparentrender);
#pragma endregion

#pragma region Setting Vertex Buffers
		devicecontext->IASetVertexBuffers(NULL, 1, &loadedvertexbuffer2, &stride, &offset);
#pragma endregion

#pragma region Setting Shader Resources
		devicecontext->PSSetShaderResources(0, 1, &shaderview3);
#pragma endregion

#pragma region Setting Sampler State
		devicecontext->PSSetSamplers(0, 1, &samplerState);
#pragma endregion

#pragma region Setting index Buffer
		devicecontext->IASetIndexBuffer(treeindex, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
#pragma endregion

#pragma region Draw Death
		devicecontext->DrawIndexed(1092, 0,0);
#pragma endregion


#pragma region Setting Shader Resources
		devicecontext->PSSetShaderResources(0, 1, &shaderview2);
#pragma endregion




		XMMATRIX farplane = XMMatrixTranslation(plane5[0].vertex[0], plane5[0].vertex[1], plane5[0].vertex[2]);
		XMMATRIX nearplane2 = XMMatrixTranslation(plane2[0].vertex[0], plane2[0].vertex[1], plane2[0].vertex[2]);


		farplane = XMMatrixMultiply(farplane, View[0]);
		nearplane2 = XMMatrixMultiply(nearplane2, View[0]);



		if (XMVectorGetZ(nearplane2.r[3]) > XMVectorGetZ(farplane.r[3]))
		{


#pragma region Setting Vertex Buffers
		devicecontext->IASetVertexBuffers(NULL, 1, &planeVertex1, &stride, &offset);
#pragma endregion

#pragma region Setting index Buffers
		devicecontext->IASetIndexBuffer(planeIndex1, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
#pragma endregion


#pragma region Drawing Plane1
		//devicecontext->DrawIndexed(6, 0, 0);
#pragma endregion

#pragma region Setting Vertex Buffers
		devicecontext->IASetVertexBuffers(NULL, 1, &planeVertex2, &stride, &offset);
#pragma endregion

#pragma region Setting index Buffers
		devicecontext->IASetIndexBuffer(planeIndex2, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
#pragma endregion

#pragma region Drawing Plane2
		//devicecontext->DrawIndexed(6, 0, 0);
#pragma endregion

#pragma region Setting Vertex Buffers
		devicecontext->IASetVertexBuffers(NULL, 1, &planeVertex3, &stride, &offset);
#pragma endregion

#pragma region Setting index Buffers
		devicecontext->IASetIndexBuffer(planeIndex3, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
#pragma endregion

#pragma region Drawing Plane3
		//devicecontext->DrawIndexed(6, 0, 0);
#pragma endregion

#pragma region Setting Vertex Buffers
		devicecontext->IASetVertexBuffers(NULL, 1, &planeVertex4, &stride, &offset);
#pragma endregion

#pragma region Setting index Buffers
		devicecontext->IASetIndexBuffer(planeIndex4, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
#pragma endregion

#pragma region Drawing Plane4
		//devicecontext->DrawIndexed(6, 0, 0);
#pragma endregion
		}
		else
		{

#pragma region Setting Vertex Buffers
			devicecontext->IASetVertexBuffers(NULL, 1, &planeVertex4, &stride, &offset);
#pragma endregion

#pragma region Setting index Buffers
			devicecontext->IASetIndexBuffer(planeIndex4, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
#pragma endregion

#pragma region Drawing Plane4
		//	devicecontext->DrawIndexed(6, 0, 0);	
#pragma endregion


#pragma region Setting Vertex Buffers
			devicecontext->IASetVertexBuffers(NULL, 1, &planeVertex3, &stride, &offset);
#pragma endregion

#pragma region Setting index Buffers
			devicecontext->IASetIndexBuffer(planeIndex3, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
#pragma endregion

#pragma region Drawing Plane3
		//	devicecontext->DrawIndexed(6, 0, 0);
#pragma endregion

#pragma region Setting Vertex Buffers
			devicecontext->IASetVertexBuffers(NULL, 1, &planeVertex2, &stride, &offset);
#pragma endregion

#pragma region Setting index Buffers
			devicecontext->IASetIndexBuffer(planeIndex2, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
#pragma endregion

#pragma region Drawing Plane2
		//	devicecontext->DrawIndexed(6, 0, 0);
#pragma endregion

#pragma region Setting Vertex Buffers
			devicecontext->IASetVertexBuffers(NULL, 1, &planeVertex1, &stride, &offset);
#pragma endregion

#pragma region Setting index Buffers
			devicecontext->IASetIndexBuffer(planeIndex1, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
#pragma endregion


#pragma region Drawing Plane1
		//	devicecontext->DrawIndexed(6, 0, 0);
#pragma endregion
		}


	}
#pragma region Presenting
	swapchain->Present(0, 0);
#pragma endregion



	return true; 
}

bool DEMO_APP::ShutDown()
{

	devicecontext->ClearState();
#pragma region Safe Release
	SAFERELEASE(swapchain);
	SAFERELEASE(device);
	SAFERELEASE(devicecontext);
	SAFERELEASE(vertexbuffer);
	SAFERELEASE(indexbuffer);
	SAFERELEASE(Constbuffer);
	SAFERELEASE(Constbuffer2);
	SAFERELEASE(Constbuffer3);
	SAFERELEASE(ZBuffer);
	SAFERELEASE(StencilView);
	SAFERELEASE(StencilState);
	SAFERELEASE(inputlayout);
	SAFERELEASE(pView);
	SAFERELEASE(pBB);
	SAFERELEASE(shaderview);
	SAFERELEASE(texture);
	SAFERELEASE(pixelshader);
	SAFERELEASE(vertshader);
	SAFERELEASE(quadindexbuffer);
	SAFERELEASE(quadvertexbuffer);
	SAFERELEASE(loadedvertexbuffer);
	SAFERELEASE(loadedvertexbuffer2);

	SAFERELEASE(Rasterstate);
	SAFERELEASE(blendstate);
	SAFERELEASE(planeVertex1);
	SAFERELEASE(planeVertex2);
	SAFERELEASE(planeVertex3);
	SAFERELEASE(planeVertex4);
	SAFERELEASE(planeIndex1);
	SAFERELEASE(planeIndex2);
	SAFERELEASE(planeIndex3);
	SAFERELEASE(planeIndex4);

	SAFERELEASE(treeindex);
	SAFERELEASE(samplerState);
	

	SAFERELEASE(Rasterstate);
	SAFERELEASE(transparentrender);
	SAFERELEASE(blendstate);
	SAFERELEASE(shaderview2);
	SAFERELEASE(shaderview3);

	SAFERELEASE(texture2);
	SAFERELEASE(texture3);

	
	
	
	
	
	
	

	

#pragma endregion


	UnregisterClass(L"DirectXApplication", application);
	return true;
}

#pragma region Windows Code
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,	int nCmdShow );						   
LRESULT CALLBACK WndProc(HWND hWnd,	UINT message, WPARAM wparam, LPARAM lparam );		
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE, LPTSTR, int )
{
	srand(unsigned int(time(0)));
	DEMO_APP myApp(hInstance,(WNDPROC)WndProc);	
    MSG msg; ZeroMemory( &msg, sizeof( msg ) );
    while ( msg.message != WM_QUIT && myApp.Run() )
    {	
	    if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        { 
            TranslateMessage( &msg );
            DispatchMessage( &msg ); 
        }
    }
	myApp.ShutDown(); 
	return 0; 
}
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    if(GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
    switch ( message )
    {
        case ( WM_DESTROY ): { PostQuitMessage( 0 ); }
        break;
    }
    return DefWindowProc( hWnd, message, wParam, lParam );
}
//********************* END WARNING ************************//
#pragma endregion

bool LoadOBJ(const char* path, std::vector<XMFLOAT3>& out_vertices, std::vector<XMFLOAT2>& out_uvs, std::vector<XMFLOAT3>& out_normals)
{
	std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	std::vector< XMFLOAT3 > temp_vertices;
	std::vector< XMFLOAT2 > temp_uvs;
	std::vector< XMFLOAT3 > temp_normals;

	FILE * file = fopen(path, "r");
	if (file == NULL){
		printf("Impossible to open the file !\n");
		return false;
	}

	while (1){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		if (strcmp(lineHeader, "v") == 0){
			XMFLOAT3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0){
			XMFLOAT2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0){
			XMFLOAT3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0){
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9){
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}
	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++){
		unsigned int vertexIndex = vertexIndices[i];
		XMFLOAT3 vertex = temp_vertices[vertexIndex - 1];
		out_vertices.push_back(vertex);
	}
	for (unsigned int i = 0; i < uvIndices.size(); i++){
		unsigned int UVIndex = uvIndices[i];
		XMFLOAT2 uvvertex = temp_uvs[UVIndex - 1];
		out_uvs.push_back(uvvertex);
	}
	for (unsigned int i = 0; i < normalIndices.size(); i++){
		unsigned int nmvertexIndex = normalIndices[i];
		XMFLOAT3 nrmvertex = temp_normals[nmvertexIndex - 1];
		out_normals.push_back(nrmvertex);
	}
	return true;
}
