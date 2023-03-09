#pragma once

#include "renderer.h"
#include "view.h"


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dxgi1_2.h>
#include <d3d11_2.h>

#include <DirectXMath.h>

#include "ps_cube.csh"
#include "vs_cube.csh"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DXGI.lib")

using namespace DirectX;

// NOTE: This header file must *ONLY* be included by renderer.cpp
#define SAFE_RELEASE(ptr) { if(ptr) { ptr->Release(); ptr = nullptr; } }

namespace end
{
	struct renderer::impl
	{
		// platform/api specific members, functions, etc.
		// Device, swapchain, resource views, states, etc. can be members here
		ID3D11Device *device = nullptr;
		ID3D11DeviceContext *context = nullptr;
		IDXGISwapChain *swapchain = nullptr;

		ID3D11RenderTargetView*		render_target[VIEW_RENDER_TARGET::COUNT]{};

		ID3D11DepthStencilView*		depthStencilView[VIEW_DEPTH_STENCIL::COUNT]{};

		ID3D11DepthStencilState*	depthStencilState[STATE_DEPTH_STENCIL::COUNT]{};

		ID3D11Buffer*				vertex_buffer[VERTEX_BUFFER::COUNT]{};

		ID3D11Buffer*				index_buffer[INDEX_BUFFER::COUNT]{};
		
		ID3D11InputLayout*			input_layout[INPUT_LAYOUT::COUNT]{};

		ID3D11VertexShader*			vertex_shader[VERTEX_SHADER::COUNT]{};

		ID3D11Buffer*				constant_buffer[CONSTANT_BUFFER::COUNT]{};

		ID3D11PixelShader*			pixel_shader[PIXEL_SHADER::COUNT]{};

		ID3D11RasterizerState*		rasterState[STATE_RASTERIZER::COUNT]{};

		D3D11_VIEWPORT				view_port[VIEWPORT::COUNT]{};

		/* Add more as needed...
		ID3D11SamplerState*			sampler_state[STATE_SAMPLER::COUNT]{};

		ID3D11BlendState*			blend_state[STATE_BLEND::COUNT]{};
		*/

		HRESULT hr;
	
		struct ConstantBuffer
		{
			XMMATRIX mWorld;
			XMMATRIX mView;
			XMMATRIX mProjection;
		};

		XMMATRIX World;
		XMMATRIX WORLD2;
		XMVECTOR Look;
		XMVECTOR UP;
		XMMATRIX Camera;
		

		// Constructor for renderer implementation
		// 
		impl(native_handle_type window_handle, view_t& default_view)
		{
			// TODO:
			// Setup the default view 
			//	default_view should be setup with a perspective projection
			//	default_view should have a view matrix that positions the view at(0, 15, -15) and looks at(0, 0, 0)
			//		IMPORTANT: XMMatrixLookAtLH returns an inverted matrix

			World = XMMatrixTranslation(0, 15, -15);
			Look = XMVECTOR{ 0, 0, 0 };
			UP = XMVECTOR{ 0, 1, 0 };

			Camera = XMMatrixLookAtLH(World.r[3],Look,UP);

			default_view.Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, 1280.0f/720.0f, 0.1f, 1000.0f);
			default_view.View = XMMatrixInverse(nullptr, Camera);
 
			


			// TODO:
			// create device and swap chain

			DXGI_SWAP_CHAIN_DESC sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.BufferCount = 1;
			sd.BufferDesc.Width = 1280;
			sd.BufferDesc.Height = 720;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.OutputWindow = (HWND)window_handle;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.Windowed = TRUE;

			D3D_FEATURE_LEVEL  FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0;
			UINT               numLevelsRequested = 1;
			D3D_FEATURE_LEVEL  FeatureLevelsSupported;

			hr = D3D11CreateDeviceAndSwapChain(NULL,
				D3D_DRIVER_TYPE_HARDWARE,
				NULL,
				D3D11_CREATE_DEVICE_DEBUG,
				&FeatureLevelsRequested,
				numLevelsRequested,
				D3D11_SDK_VERSION,
				&sd,
				&swapchain,
				&device,
				&FeatureLevelsSupported,
				&context);
			if (FAILED(hr)) 
			{ MessageBox(nullptr, L"constant buffer creation failed", L"Error", MB_OK);
			exit(0); 
			}

			ID3D11Texture2D* pBackBuffer = nullptr;
			swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));


			hr = device->CreateRenderTargetView(pBackBuffer, nullptr, &render_target[VIEW_RENDER_TARGET::DEFAULT]);
			if (FAILED(hr))
			{
				MessageBox(nullptr, L"RTV", L"Error", MB_OK);
				exit(0);
			}
			pBackBuffer->Release();
			
			// create depth-stencil buffer/state/view
			D3D11_BUFFER_DESC BD = {};

			BD.Usage = D3D11_USAGE_DEFAULT;
			BD.ByteWidth = sizeof(ConstantBuffer);
			BD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			BD.CPUAccessFlags = 0;

			hr = device->CreateBuffer(&BD, nullptr, constant_buffer);
			if (FAILED(hr))
			{
				MessageBox(nullptr, L"constant buffer creation failed", L"Error", MB_OK);
				exit(0);
			}

			ID3D11Texture2D* pDepthStencil = NULL;
			D3D11_TEXTURE2D_DESC descDepth;
			descDepth.Width = 1280;
			descDepth.Height = 720;
			descDepth.MipLevels = 1;
			descDepth.ArraySize = 1;
			descDepth.Format = DXGI_FORMAT_D32_FLOAT; 
			descDepth.SampleDesc.Count = 1;
			descDepth.SampleDesc.Quality = 0;
			descDepth.Usage = D3D11_USAGE_DEFAULT;
			descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			descDepth.CPUAccessFlags = 0;
			descDepth.MiscFlags = 0;
			hr = device->CreateTexture2D(&descDepth, NULL, &pDepthStencil);
			if (FAILED(hr))
			{
				MessageBox(nullptr, L"constant buffer creation failed", L"Error", MB_OK);
				exit(0);
			}

			D3D11_DEPTH_STENCIL_DESC dsDesc;

			// Depth test parameters
			dsDesc.DepthEnable = true;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

			// Stencil test parameters
			dsDesc.StencilEnable = false;
			dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
			dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

			// Stencil operations if pixel is front-facing
			dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			// Stencil operations if pixel is back-facing
			dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			// Create depth stencil state
			

			hr = device->CreateDepthStencilState(&dsDesc, depthStencilState);
			if (FAILED(hr))
			{
				MessageBox(nullptr, L"this one", L"Error", MB_OK);
				exit(0);
			}

			// Bind depth stencil state
			context->OMSetDepthStencilState(depthStencilState[0], 1);
			

			// Create the depth stencil view
			
			hr = device->CreateDepthStencilView(pDepthStencil, // Depth stencil texture
				nullptr, // Depth stencil desc
				&depthStencilView[0]);  // [out] Depth stencil view
			if (FAILED(hr))
			{
				MessageBox(nullptr, L"thisone", L"Error", MB_OK);
				exit(0);
			}

// Bind the depth stencil view
			context->OMSetRenderTargets(1,          // One rendertarget view
				render_target,      // Render target view, created earlier
				depthStencilView[0]);     // Depth stencil view for the render target


			// create rasterizer state
			D3D11_RASTERIZER_DESC RD;
			RD.FillMode = D3D11_FILL_SOLID;
			RD.CullMode = D3D11_CULL_NONE;
			RD.FrontCounterClockwise = false;
			RD.DepthBias = 0;
			RD.DepthBiasClamp = 0.0f;
			RD.SlopeScaledDepthBias = 0.0f;
			RD.DepthClipEnable = false;
			RD.ScissorEnable = false;
			RD.MultisampleEnable = false;
			RD.AntialiasedLineEnable = false;

			hr = device->CreateRasterizerState(&RD, rasterState);
			if (FAILED(hr))
			{
				MessageBox(nullptr, L"1", L"Error", MB_OK);
				exit(0);
			}

			// create input layout
			D3D11_INPUT_ELEMENT_DESC IputLayout[] =
			{
				{"SV_VertexID", 0, DXGI_FORMAT_R32_UINT, 0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
			};

			hr = device->CreateInputLayout(IputLayout, 1, vs_cube, sizeof(vs_cube), input_layout);
			if (FAILED(hr))
			{
				MessageBox(nullptr, L"Layout", L"Error", MB_OK);
				exit(0);
			}

			context->IASetInputLayout(input_layout[0]);

			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// create vertex and pixel shaders
			hr = device->CreateVertexShader(vs_cube, sizeof(vs_cube), nullptr, vertex_shader);
			if (FAILED(hr))
			{
				MessageBox(nullptr, L"VS", L"Error", MB_OK);
				exit(0);
			}

			hr = device->CreatePixelShader(ps_cube, sizeof(ps_cube), nullptr, pixel_shader);
			if (FAILED(hr))
			{
				MessageBox(nullptr, L"PS", L"Error", MB_OK);
				exit(0);
			}
		}

		void draw_view(view_t& v)
		{
			// TODO:
			// Apply view properties

			

			

			//	Set and clear render targets
			

			context->OMSetRenderTargets(1, &render_target[0], depthStencilView[0]);

			const float d_green[] = { 0, 1, 0, 1 };
			context->ClearRenderTargetView(render_target[0], d_green);
			context->ClearDepthStencilView(depthStencilView[0], D3D11_CLEAR_DEPTH, 1, 0);

			//	Set viewport(s), etc
		
			view_port[0].Width = (FLOAT)1280;
			view_port[0].Height = (FLOAT)720;
			view_port[0].MinDepth = 0.01f;
			view_port[0].MaxDepth = 1.0f;
			view_port[0].TopLeftX = 0;
			view_port[0].TopLeftY = 0;
			context->RSSetViewports(1, &view_port[0]);


			// **SKIP**:
			// Draw batches in visible set (Implemented In a future assignment)

			
			// TODO:
			// Using the vs_cube/ps_cube shaders, Draw 36 vertices to render a cube.
			// The vs_cube shader contains all the vertex data, so no vertex buffers are required

			ConstantBuffer CB;

			WORLD2 = XMMatrixIdentity();

			CB.mWorld = XMMatrixTranspose(WORLD2);
			CB.mView = XMMatrixInverse(nullptr, XMMatrixTranspose(v.View));
			CB.mProjection = XMMatrixTranspose(v.Projection);

			context->UpdateSubresource(constant_buffer[0], 0, nullptr, &CB, 0, 0);

			context->VSSetConstantBuffers(0, 1, constant_buffer);

			context->VSSetShader(vertex_shader[0], nullptr, 0);
			context->PSSetShader(pixel_shader[0], nullptr, 0);

			context->Draw(36, 0);
		
			swapchain->Present(0, 0);
		}

		~impl()
		{
			// TODO:
			//Clean-up
			
			//render_target[0]->Release();
			SAFE_RELEASE(render_target[0]);

			depthStencilView[0]->Release();

			depthStencilState[0]->Release();

			//vertex_buffer[0]->Release();
			SAFE_RELEASE(vertex_buffer[0]);

			index_buffer[0]->Release();

			input_layout[0]->Release();

			vertex_shader[0]->Release();

			constant_buffer[0]->Release();

			pixel_shader[0]->Release();

			rasterState[0]->Release();

			device->Release();

			context->Release();

			swapchain->Release();



			// In general, release objects in reverse order of creation
		}
	};
}