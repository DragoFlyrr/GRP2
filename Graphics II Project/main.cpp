// D3D LAB 1a "Line Land".
// Author: L.Norri CD DRX, FullSail University

// Introduction:
// Welcome to the first lab of the Direct3D Graphics Programming class.
// This is the ONLY guided lab in this course! 
// Future labs will demand the habits & foundation you develop right now!  
// It is CRITICAL that you follow each and every step. ESPECIALLY THE READING!!!

// TO BEGIN: Open the word document that acompanies this lab and start from the top.

//************************************************************
//************ INCLUDES & DEFINES ****************************
//************************************************************

#include <iostream>
#include <ctime>
#include "XTime.h"
#include "Defines.h"
#include "Cube.h"
#include "brickwall.h"
#include "DDSTextureLoader.h"


using namespace std;

// BEGIN PART 1
// TODO: PART 1 STEP 1a
#include <d3d11.h>
// TODO: PART 1 STEP 1b
#pragma comment(lib, "d3d11.lib")
// TODO: PART 2 STEP 6
#include <DirectXMath.h>
#include "Trivial_PS.csh"
#include "Trivial_VS.csh"
#include "Skybox_PS.csh"
#include "Skybox_VS.csh"
//using namespace DirectX;

#define BACKBUFFER_WIDTH	1280
#define BACKBUFFER_HEIGHT	720
#define SAFERELEASE(p) {if(p){p->Release(), p = nullptr;}}
using namespace DirectX;
//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

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
	ID3D11Buffer*					SkyVertexbuffer;
	ID3D11Buffer*					SkyIndexBuffer;

	ID3D11Buffer*					quadvertexbuffer;
	ID3D11Buffer*					quadindexbuffer;


	ID3D11Buffer*					Constbuffer;
	ID3D11Buffer*					Constbuffer2;
	ID3D11Texture2D*				ZBuffer;
	ID3D11DepthStencilView*			StencilView;
	ID3D11InputLayout*				inputlayout;
	ID3D11RenderTargetView *		pView;
	ID3D11Resource *				pBB;
	ID3D11ShaderResourceView*		shaderview;
	ID3D11ShaderResourceView*		Same;
	ID3D11Texture2D*				texture;
	ID3D11Texture2D*				same;
	ID3D11PixelShader*				pixelshader;
	ID3D11VertexShader*				vertshader;

	ID3D11PixelShader*				lsdfasdfosdalfasdsdfasdfasdfkljadfklasdklsdfpixelshader;
	ID3D11VertexShader*				wawlidillsdflaslisdiljldjkl3vertexshader;
	// TODO: PART 2 STEP 2

	D3D11_VIEWPORT					viewport;
	XTime							timer;
	unsigned int					numverticies;
	D3D11_MAPPED_SUBRESOURCE		mapsource;

	XMMATRIX						CubeWorld;
	XMMATRIX						View;
	XMMATRIX						Projection;



	// BEGIN PART 5
	// TODO: PART 5 STEP 1
	// TODO: PART 2 STEP 4
	
	// BEGIN PART 3
	// TODO: PART 3 STEP 1

	// TODO: PART 3 STEP 2b
	
	// TODO: PART 3 STEP 4a

public:
	// BEGIN PART 2
	// TODO: PART 2 STEP 1


	struct Send_To_Vram
	{
		XMMATRIX worldmatrix;
	};
	struct Send_To_Vram2
	{
		XMMATRIX Viewmatrix;
		XMMATRIX Projmatrix;
	};
	XMMATRIX rotate;
	Send_To_Vram toShader;
	Send_To_Vram2 toShader2;

	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
};

//************************************************************
//************ CREATION OF OBJECTS & RESOURCES ***************
//************************************************************

DEMO_APP::DEMO_APP(HINSTANCE hinst, WNDPROC proc)
{
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

	toShader.worldmatrix = XMMatrixIdentity();


	View = XMMatrixIdentity();

	
	Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(90), ((float)BACKBUFFER_WIDTH/(float)BACKBUFFER_HEIGHT), NEAR_PLANE, FAR_PLANE);

	toShader2.Viewmatrix = View;
	toShader2.Projmatrix = Projection;

	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BufferCount = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	desc.BufferDesc.RefreshRate = { 1, 60 };
	desc.BufferDesc.Height = BACKBUFFER_HEIGHT;
	desc.BufferDesc.Width = BACKBUFFER_WIDTH;
	desc.SampleDesc.Count = 1;
	desc.OutputWindow = window;
	desc.Windowed = true;


	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, NULL , D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG, NULL, NULL , D3D11_SDK_VERSION, &desc, &swapchain, &device, NULL, &devicecontext);
	// TODO: PART 1 STEP 3b

	swapchain->GetBuffer(0, __uuidof(pBB), reinterpret_cast<void**>(&pBB));
	device->CreateRenderTargetView(pBB, NULL, &pView);
	swapchain->GetDesc(&desc);
	viewport.Height = (FLOAT)desc.BufferDesc.Height;
	viewport.Width = (FLOAT)desc.BufferDesc.Width;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	CubeWorld = XMMatrixIdentity();
	CubeWorld = XMMatrixSet(1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 5, 1);

	
	// TODO: PART 1 STEP 4
	//SIMPLE_VERTEX star[12];
	//ZeroMemory(&star, sizeof(star));
	//for (unsigned int i = 0; i < 10; i++)
	//{
	//	if (i%2)
	//	{
	//		star[i].vertex[0] = cos(((i / 180.0f) * 36) * 3.14f);
	//		star[i].vertex[1] = sin(((i / 180.0f) * 36) * 3.14f);
	//		star[i].vertex[3] = 1;
	//	}
	//	else
	//	{
	//		star[i].vertex[0] = cos(((i / 180.0f) * 36) * 3.14f) /2;
	//		star[i].vertex[1] = sin(((i / 180.0f) * 36) * 3.14f) /2;
	//		star[i].vertex[3] = 1;
	//	}
	//}
	//star[10].vertex[2] = 0.5f;
	//star[11].vertex[2] = -0.5f;
	//star[10].vertex[3] = 1;
	//star[11].vertex[3] = 1;
	//
	//star[10].color[0] = 1;
	//star[10].color[1] = 1;
	//star[10].color[2] = 1;
	//star[10].color[3] = 1;
	//star[11].color[0] = 1;
	//star[11].color[1] = 1;
	//star[11].color[2] = 1;
	//star[11].color[3] = 1;	
	////star[10].color = { 1, 1, 1, 1 };
	//unsigned int index[60];
	//index[0] = 0;
	//index[1] = 1;
	//index[2] = 10;
	//
	//index[3] = 1;
	//index[4] = 2;
	//index[5] = 10;
	//
	//index[6] = 2;
	//index[7] = 3;
	//index[8] = 10;
	//
	//index[9] = 3;
	//index[10] = 4;
	//index[11] = 10;
	//
	//index[12] = 4;
	//index[13] = 5;
	//index[14] = 10;
	//
	//index[15] = 5;
	//index[16] = 6;
	//index[17] = 10;
	//index[18] = 6;
	//index[19] = 7;
	//index[20] = 10;
	//index[21] = 7;
	//index[22] = 8;
	//index[23] = 10;
	//index[24] = 8;
	//index[25] = 9;
	//index[26] = 10;
	//index[57] = 9;
	//index[58] = 0;
	//index[59] = 10;
	//
	//index[27] = 11;
	//index[28] = 1;
	//index[29] = 0;
	//index[30] = 11;
	//index[31] = 2;
	//index[32] = 1;
	//index[33] = 11;
	//index[34] = 3;
	//index[35] = 2;
	//index[36] = 11;
	//index[37] = 4;
	//index[38] = 3;
	//index[39] = 11;
	//index[40] = 5;
	//index[41] = 4;
	//index[42] = 11;
	//index[43] = 6;
	//index[44] = 5;
	//index[45] = 11;
	//index[46] = 7;
	//index[47] = 6;
	//index[48] = 11;
	//index[49] = 8;
	//index[50] = 7;
	//index[51] = 11;
	//index[52] = 9;
	//index[53] = 8;
	//index[54] = 11;
	//index[55] = 0;
	//index[56] = 9;
	
	SIMPLE_VERTEX plane[4];
	plane[0].vertex[0] = -1; //x 
	plane[0].vertex[1] = -.5; //y
	plane[0].vertex[2] = 1; //z

	plane[1].vertex[0] = 1;
	plane[1].vertex[1] = -.5;
	plane[1].vertex[2] = 1;

	plane[2].vertex[0] = -1;
	plane[2].vertex[1] = -.5;
	plane[2].vertex[2] = -1;

	plane[3].vertex[0] = 1;
	plane[3].vertex[1] = -.5;
	plane[3].vertex[2] = -1;

	unsigned int planeindex[6];
	planeindex[0] = 0;
	planeindex[1] = 1;
	planeindex[2] = 2;
	planeindex[3] = 3;
	planeindex[4] = 2;
	planeindex[5] = 1;

	SIMPLE_VERTEX box[8];
	box[0].vertex[0] = -1; box[1].vertex[0] = 1;
	box[0].vertex[1] = 1;  box[1].vertex[1] = 1;
	box[0].vertex[2] = -1; box[1].vertex[2] = -1;
	
	box[2].vertex[0] = -1; box[3].vertex[0] = 1;
	box[2].vertex[1] = -1; box[3].vertex[1] = -1;
	box[2].vertex[2] = -1; box[3].vertex[2] = -1;

											     box[4].vertex[0] = -1; box[5].vertex[0] = 1;
											     box[4].vertex[1] = 1;  box[5].vertex[1] = 1;
											     box[4].vertex[2] = 1; box[5].vertex[2] = 1;
											     
											     box[6].vertex[0] = -1; box[7].vertex[0] = 1;
											     box[6].vertex[1] = -1; box[7].vertex[1] = -1;
											     box[6].vertex[2] = 1; box[7].vertex[2] = 1;


	unsigned int boxindex[36];
	
	//front face
	boxindex[0] = 0;
	boxindex[1] = 1;
	boxindex[2] = 2;

	boxindex[3] = 1;
	boxindex[4] = 3;
	boxindex[5] = 2;

	//right face
	boxindex[6] = 1;
	boxindex[7] = 7;
	boxindex[8] = 3;

	boxindex[9] = 7;
	boxindex[10] = 5;
	boxindex[11] = 1;

	//top face
	boxindex[12] = 1;
	boxindex[13] = 4;
	boxindex[14] = 5;

	boxindex[15] = 4;
	boxindex[16] = 1;
	boxindex[17] = 0;

	//left face
	boxindex[18] = 0;
	boxindex[19] = 6;
	boxindex[20] = 2;

	boxindex[21] = 4;
	boxindex[22] = 6;
	boxindex[23] = 0;

	//bottom face
	boxindex[24] = 6;
	boxindex[25] = 3;
	boxindex[26] = 7;

	boxindex[27] = 2;
	boxindex[28] = 3;
	boxindex[29] = 6;



	// TODO: PART 1 STEP 5


	// TODO: PART 2 STEP 3a

	// cube buffers
	D3D11_BUFFER_DESC vertbuffdesc;
	ZeroMemory(&vertbuffdesc, sizeof(vertbuffdesc));
	vertbuffdesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertbuffdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertbuffdesc.CPUAccessFlags = NULL;
	vertbuffdesc.ByteWidth = sizeof(OBJ_VERT) * 776;
	vertbuffdesc.StructureByteStride = sizeof(OBJ_VERT);

	D3D11_BUFFER_DESC indexbuffdesc;
	ZeroMemory(&indexbuffdesc, sizeof(indexbuffdesc));
	indexbuffdesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexbuffdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexbuffdesc.CPUAccessFlags = NULL;
	indexbuffdesc.ByteWidth = sizeof(unsigned int) * 1692;
	indexbuffdesc.StructureByteStride = sizeof(unsigned int);

	//ground plain buffers
	D3D11_BUFFER_DESC vertbuffdesc2;
	ZeroMemory(&vertbuffdesc2, sizeof(vertbuffdesc2));
	vertbuffdesc2.Usage = D3D11_USAGE_IMMUTABLE;
	vertbuffdesc2.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertbuffdesc2.CPUAccessFlags = NULL;
	vertbuffdesc2.ByteWidth = sizeof(OBJ_VERT) * 4;
	vertbuffdesc2.StructureByteStride = sizeof(OBJ_VERT);

	D3D11_BUFFER_DESC indexbuffdesc2;
	ZeroMemory(&indexbuffdesc2, sizeof(indexbuffdesc2));
	indexbuffdesc2.Usage = D3D11_USAGE_IMMUTABLE;
	indexbuffdesc2.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexbuffdesc2.CPUAccessFlags = NULL;
	indexbuffdesc2.ByteWidth = sizeof(unsigned int) * 6;
	indexbuffdesc2.StructureByteStride = sizeof(unsigned int);
	
	//Skybox buffers

	D3D11_BUFFER_DESC skyvertbuffdesc;
	ZeroMemory(&skyvertbuffdesc, sizeof(skyvertbuffdesc));
	skyvertbuffdesc.Usage = D3D11_USAGE_IMMUTABLE;
	skyvertbuffdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	skyvertbuffdesc.CPUAccessFlags = NULL;
	skyvertbuffdesc.ByteWidth = sizeof(OBJ_VERT) * 8;
	skyvertbuffdesc.StructureByteStride = sizeof(OBJ_VERT);

	D3D11_BUFFER_DESC skyindexbuffdesc;
	ZeroMemory(&skyindexbuffdesc, sizeof(skyindexbuffdesc));
	skyindexbuffdesc.Usage = D3D11_USAGE_IMMUTABLE;
	skyindexbuffdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	skyindexbuffdesc.CPUAccessFlags = NULL;
	skyindexbuffdesc.ByteWidth = sizeof(unsigned int) * 36;
	skyindexbuffdesc.StructureByteStride = sizeof(unsigned int);

	//constant buffer init

	D3D11_BUFFER_DESC constbuffdesc;
	ZeroMemory(&constbuffdesc, sizeof(constbuffdesc));
	constbuffdesc.Usage = D3D11_USAGE_DYNAMIC;
	constbuffdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constbuffdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constbuffdesc.ByteWidth = sizeof(Send_To_Vram);
	constbuffdesc.StructureByteStride = 0;

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = &Cube_data; 

	D3D11_SUBRESOURCE_DATA indexdata;
	ZeroMemory(&indexdata, sizeof(indexdata));
	indexdata.pSysMem = &Cube_indicies;
	
	D3D11_SUBRESOURCE_DATA data2;
	ZeroMemory(&data2, sizeof(data2));
	data2.pSysMem = &plane;

	D3D11_SUBRESOURCE_DATA indexdata2;
	ZeroMemory(&indexdata2, sizeof(indexdata2));
	indexdata2.pSysMem = &planeindex;

	D3D11_SUBRESOURCE_DATA constdata;
	constdata.pSysMem = &toShader;

	D3D11_SUBRESOURCE_DATA skyboxvertdata;
	skyboxvertdata.pSysMem = &box;

	D3D11_SUBRESOURCE_DATA skyboxindexdata;
	skyboxindexdata.pSysMem = &boxindex;

	device->CreateBuffer(&vertbuffdesc, &data, &vertexbuffer);
	device->CreateBuffer(&vertbuffdesc2, &data2, &quadvertexbuffer);
	device->CreateBuffer(&indexbuffdesc, &indexdata, &indexbuffer);
	device->CreateBuffer(&indexbuffdesc2, &indexdata2, &quadindexbuffer);
	device->CreateBuffer(&constbuffdesc, &constdata, &Constbuffer);
	device->CreateBuffer(&skyvertbuffdesc, &skyboxvertdata, &SkyVertexbuffer);
	device->CreateBuffer(&skyindexbuffdesc, &skyboxindexdata, &SkyIndexBuffer);

	constbuffdesc.ByteWidth = sizeof(Send_To_Vram2);
	constdata.pSysMem = &toShader2;
	device->CreateBuffer(&constbuffdesc, &constdata, &Constbuffer2);

	device->CreatePixelShader(&Trivial_PS, sizeof(Trivial_PS), NULL, &pixelshader);
	device->CreateVertexShader(&Trivial_VS, sizeof(Trivial_VS), NULL, &vertshader);
	device->CreatePixelShader(&Skybox_PS, sizeof(Skybox_PS), NULL, &lsdfasdfosdalfasdsdfasdfasdfkljadfklasdklsdfpixelshader);
	device->CreateVertexShader(&Skybox_VS, sizeof(Skybox_VS), NULL, &wawlidillsdflaslisdiljldjkl3vertexshader);

	device->CreateInputLayout(layout, 3, Trivial_VS, sizeof(Trivial_VS), &inputlayout);
	
	D3D11_TEXTURE2D_DESC textdesc;
	ZeroMemory(&textdesc, sizeof(textdesc));
	textdesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textdesc.Format = DXGI_FORMAT_D32_FLOAT;
	textdesc.Width = BACKBUFFER_WIDTH;
	textdesc.Height = BACKBUFFER_HEIGHT;
	textdesc.ArraySize = 1;
	textdesc.SampleDesc = { 1, 0 };
	textdesc.MipLevels = 1;
	device->CreateTexture2D(&textdesc, NULL, &ZBuffer);

	D3D11_DEPTH_STENCIL_VIEW_DESC stenview;
	ZeroMemory(&stenview, sizeof(stenview));
	stenview.Format = DXGI_FORMAT_D32_FLOAT;
	stenview.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	//stenview.Texture2D.MipSlice = 0;


	device->CreateDepthStencilView(ZBuffer, &stenview, &StencilView);



	// BEGIN PART 4
	// TODO: PART 4 STEP 1

	// TODO: PART 2 STEP 3b
	
    // TODO: PART 2 STEP 3c
	
	// TODO: PART 2 STEP 3d
	
	// TODO: PART 5 STEP 2a
	
	// TODO: PART 5 STEP 2b
	rotate = XMMatrixIdentity();

	D3D11_TEXTURE2D_DESC texturedesc;
	ZeroMemory(&texturedesc, sizeof(texturedesc));
	texturedesc.ArraySize = 1;
	texturedesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texturedesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texturedesc.Height = brickwall_height;
	texturedesc.Width = brickwall_width;
	texturedesc.MipLevels = brickwall_numlevels;
	texturedesc.SampleDesc = { 1, 0 };

	D3D11_SUBRESOURCE_DATA texturedata[brickwall_numlevels];
	ZeroMemory(&texturedata, sizeof(texturedata));
	for (unsigned int i = 0; i < brickwall_numlevels; i++)
	{
		texturedata[i].pSysMem = &brickwall_pixels[brickwall_leveloffsets[i]];
		//texturedata[i].
		texturedata[i].SysMemPitch = (brickwall_width >> i) * sizeof(unsigned int);
	}

	
	device->CreateTexture2D(&texturedesc,texturedata,&texture);
	CreateDDSTextureFromFile(device, L"OutputCube.dds",(ID3D11Resource**)&same,&Same);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderdesc;
	ZeroMemory(&shaderdesc, sizeof(shaderdesc));
	shaderdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	shaderdesc.Texture2D.MipLevels = brickwall_numlevels;
	shaderdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(texture, &shaderdesc, &shaderview);

	// TODO: PART 5 STEP 3
		
	// TODO: PART 2 STEP 5
	// ADD SHADERS TO PROJECT, SET BUILD OPTIONS & COMPILE

	// TODO: PART 2 STEP 7
	
	// TODO: PART 2 STEP 8a
	
	// TODO: PART 2 STEP 8b
    
	// TODO: PART 3 STEP 3

	// TODO: PART 3 STEP 4b

}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	// TODO: PART 4 STEP 2	
	timer.Signal();

	if (GetAsyncKeyState(VK_UP) && GetAsyncKeyState(VK_SHIFT))
		View = XMMatrixMultiply(XMMatrixTranslation(0.0f, (float)(15.0f * timer.SmoothDelta()), 0.0f), View);
	else if (GetAsyncKeyState(VK_UP) && GetAsyncKeyState(VK_SPACE))
	{
		XMVECTOR temp = View.r[3];
		View.r[3] = XMVectorZero();
		View = XMMatrixMultiply(XMMatrixRotationX(XMConvertToRadians((float)(15.0f * timer.SmoothDelta()))), View);
		View.r[3] = temp;
	}
	else if (GetAsyncKeyState(VK_UP))
		View = XMMatrixMultiply(XMMatrixTranslation(0, 0, (float)(15.0f * timer.SmoothDelta())), View);
	if (GetAsyncKeyState(VK_DOWN) && GetAsyncKeyState(VK_SHIFT))
		View = XMMatrixMultiply(XMMatrixTranslation(0, (float)(-15.0f * timer.SmoothDelta()), 0), View);
	else if (GetAsyncKeyState(VK_DOWN) && GetAsyncKeyState(VK_SPACE))
	{
		XMVECTOR temp = View.r[3];
		View.r[3] = XMVectorZero();
		View = XMMatrixMultiply(XMMatrixRotationX(XMConvertToRadians((float)(-15.0f * timer.SmoothDelta()))), View);
		View.r[3] = temp;
	}
	else if (GetAsyncKeyState(VK_DOWN))
		View = XMMatrixMultiply(XMMatrixTranslation(0, 0, (float)(-15.0f * timer.SmoothDelta())), View);
	if (GetAsyncKeyState(VK_RIGHT) && GetAsyncKeyState(VK_SHIFT))
	{
		XMVECTOR temp = View.r[3];
		View.r[3] = XMVectorZero();
		View = XMMatrixMultiply(XMMatrixRotationY(XMConvertToRadians((float)(15.0f * timer.SmoothDelta()))), View);
		View.r[3] = temp;
	}
	else if (GetAsyncKeyState(VK_RIGHT))
		View = XMMatrixMultiply(XMMatrixTranslation((float)(15.0f * timer.SmoothDelta()), 0, 0), View);
	if (GetAsyncKeyState(VK_LEFT) && GetAsyncKeyState(VK_SHIFT))
	{
		XMVECTOR temp = View.r[3];
		View.r[3] = XMVectorZero();
		View = XMMatrixMultiply(XMMatrixRotationY(XMConvertToRadians((float)(-15.0f * timer.SmoothDelta()))), View);
		View.r[3] = temp;
	}
	else if (GetAsyncKeyState(VK_LEFT))
		View = XMMatrixMultiply(XMMatrixTranslation((float)(-15.0f * timer.SmoothDelta()), 0, 0), View);

	XMVECTOR vec = View.r[3];
	toShader2.Viewmatrix = XMMatrixInverse(&vec, View);




	devicecontext->OMSetRenderTargets(1, &pView, StencilView);
	devicecontext->PSSetShaderResources(0, 0, &Same);
	
	float color[4] = { 0, 0, 1, 0 };
	devicecontext->ClearDepthStencilView(StencilView, D3D11_CLEAR_DEPTH , 1, 0);
	devicecontext->ClearRenderTargetView(pView, color);
	
	
	UINT offset = { 0 };
	UINT stride = sizeof(SIMPLE_VERTEX);
	devicecontext->RSSetViewports(1, &viewport);
	devicecontext->IASetVertexBuffers(NULL, 1, &SkyVertexbuffer, &stride, &offset);
	devicecontext->IASetIndexBuffer(SkyIndexBuffer, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	devicecontext->IASetInputLayout(inputlayout);
	devicecontext->VSSetShader(wawlidillsdflaslisdiljldjkl3vertexshader, NULL, NULL);
	devicecontext->PSSetShader(lsdfasdfosdalfasdsdfasdfasdfkljadfklasdklsdfpixelshader, NULL, NULL);
	devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//devicecontext->set


	//CubeWorld = XMMatrixMultiply(XMMatrixRotationY(XMConvertToRadians((float)(15.0f * timer.SmoothDelta()))), CubeWorld);
	toShader.worldmatrix = CubeWorld;

	// TODO: PART 4 STEP 3
	devicecontext->Map(Constbuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &mapsource);
	memcpy(mapsource.pData, &toShader, sizeof(toShader));
	devicecontext->Unmap(Constbuffer, NULL);


	devicecontext->Map(Constbuffer2, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &mapsource);
	memcpy(mapsource.pData, &toShader2, sizeof(toShader2));
	devicecontext->Unmap(Constbuffer2, NULL);

	devicecontext->VSSetConstantBuffers(1, 1, &Constbuffer);
	devicecontext->VSSetConstantBuffers(2, 1, &Constbuffer2);
	

	
	
	devicecontext->DrawIndexed(36, 0, 0);
	//devicecontext->ClearDepthStencilView(StencilView, D3D11_CLEAR_DEPTH, 1, 0);
	devicecontext->VSSetShader(vertshader, NULL, NULL);
	devicecontext->PSSetShader(pixelshader, NULL, NULL);
	toShader.worldmatrix = XMMatrixSet(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 5, 1);
	// TODO: PART 4 STEP 3
	devicecontext->Map(Constbuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &mapsource);
	memcpy(mapsource.pData, &toShader, sizeof(toShader));
	devicecontext->Unmap(Constbuffer, NULL);

	devicecontext->IASetVertexBuffers(NULL, 1, &quadvertexbuffer, &stride, &offset);
	devicecontext->IASetIndexBuffer(quadindexbuffer, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

	devicecontext->DrawIndexed(6, 0, 0);



	// TODO: PART 4 STEP 5
	
	// END PART 4

	// TODO: PART 1 STEP 7a
	
	// TODO: PART 1 STEP 7b
	
	// TODO: PART 1 STEP 7c
	
	// TODO: PART 5 STEP 4
	
	// TODO: PART 5 STEP 5
	
	// TODO: PART 5 STEP 6
	
	// TODO: PART 5 STEP 7
	
	// END PART 5
	
	// TODO: PART 3 STEP 5
	
	// TODO: PART 3 STEP 6
	
	// TODO: PART 2 STEP 9a
	
	// TODO: PART 2 STEP 9b
	
	// TODO: PART 2 STEP 9c
	

	// TODO: PART 2 STEP 9d
	
	// TODO: PART 2 STEP 10
	
	// END PART 2

	// TODO: PART 1 STEP 8
	
	
	// END OF PART 1
	swapchain->Present(0, 0);
	return true; 
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	// TODO: PART 1 STEP 6

	devicecontext->ClearState();

	SAFERELEASE(swapchain);
	SAFERELEASE(device); 
	SAFERELEASE(devicecontext);
	SAFERELEASE(vertexbuffer);
	SAFERELEASE(indexbuffer);
	SAFERELEASE(Constbuffer);
	SAFERELEASE(Constbuffer2); 
	SAFERELEASE(ZBuffer);
	SAFERELEASE(StencilView);
	SAFERELEASE(inputlayout); 
	SAFERELEASE(pView);
	SAFERELEASE(pBB); 
	SAFERELEASE(shaderview);
	SAFERELEASE(texture);
	SAFERELEASE(pixelshader);
	SAFERELEASE(vertshader);
	SAFERELEASE(quadindexbuffer);
	SAFERELEASE(quadvertexbuffer);


	UnregisterClass(L"DirectXApplication", application);
	return true;
}

//************************************************************
//************ WINDOWS RELATED *******************************
//************************************************************

// ****************** BEGIN WARNING ***********************// 
// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY!
	
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