#include "stdafx.h"
#include "tlCGuiRenderer.h"
#include "imgui/imgui_internal.h"
#include "tlC3DGfx.h"

IDirect3DDevice9*		CGuiRenderer::s_pDevice			= NULL;
IDirect3DTexture9*		CGuiRenderer::s_pFontTexture	= NULL;
IDirect3DTexture9*		CGuiRenderer::s_pWhiteTexture	= NULL;
IDirect3DVertexBuffer9* CGuiRenderer::s_pVB				= NULL;
IDirect3DIndexBuffer9*	CGuiRenderer::s_pIB				= NULL;
ID3DXEffect*			CGuiRenderer::s_pShader			= NULL;

void CGuiRenderer::RenderCallback(ImDrawData* draw_data)
{
	Vertex*		vtx_dst;
	ImDrawIdx*	idx_dst;
	if (s_pVB->Lock(0, (UINT)(draw_data->TotalVtxCount * sizeof(Vertex)), (void**)&vtx_dst, D3DLOCK_DISCARD) < 0)
		return;
	if (s_pIB->Lock(0, (UINT)(draw_data->TotalIdxCount * sizeof(ImDrawIdx)), (void**)&idx_dst, D3DLOCK_DISCARD) < 0)
		return;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const ImDrawVert* vtx_src = &cmd_list->VtxBuffer[0];
		for (int i = 0; i < cmd_list->VtxBuffer.size(); i++)
		{
			vtx_dst->pos.x = vtx_src->pos.x;
			vtx_dst->pos.y = vtx_src->pos.y;
			vtx_dst->pos.z = 0.0f;
			vtx_dst->col = (vtx_src->col & 0xFF00FF00) | ((vtx_src->col & 0xFF0000) >> 16) | ((vtx_src->col & 0xFF) << 16);     // RGBA --> ARGB for DirectX9
			vtx_dst->uv.x = vtx_src->uv.x;
			vtx_dst->uv.y = vtx_src->uv.y;
			vtx_dst++;
			vtx_src++;
		}
		memcpy(idx_dst, &cmd_list->IdxBuffer[0], cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx));
		idx_dst += cmd_list->IdxBuffer.size();
	}
	s_pVB->Unlock();
	s_pIB->Unlock();
	s_pDevice->SetStreamSource(0, s_pVB, 0, sizeof(Vertex));
	s_pDevice->SetIndices(s_pIB);
	s_pDevice->SetFVF(FVF);

	// Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing
	/*
	s_pDevice->SetPixelShader(NULL);
	s_pDevice->SetVertexShader(NULL);
	s_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	s_pDevice->SetRenderState(D3DRS_LIGHTING, false);
	s_pDevice->SetRenderState(D3DRS_ZENABLE, false);
	s_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	s_pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	s_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	s_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	s_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	s_pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
	s_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	s_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	s_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	s_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	s_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	s_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	s_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	*/

	// Setup orthographic projection matrix
	D3DXMATRIXA16 mat;
	D3DXMatrixIdentity(&mat);
	//s_pDevice->SetTransform(D3DTS_WORLD, &mat);
	//s_pDevice->SetTransform(D3DTS_VIEW, &mat);
	D3DXMatrixOrthoOffCenterLH(&mat, 0.5f, ImGui::GetIO().DisplaySize.x + 0.5f, ImGui::GetIO().DisplaySize.y + 0.5f, 0.5f, -1.0f, +1.0f);
	//s_pDevice->SetTransform(D3DTS_PROJECTION, &mat);

	s_pShader->SetMatrix ( "g_mProj", &mat ) ;

	// Render command lists
	int vtx_offset = 0;
	int idx_offset = 0;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				const RECT r = { (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w };
				//s_pDevice->SetTexture(0, (LPDIRECT3DTEXTURE9)pcmd->TextureId);
				if ( pcmd->TextureId )
					s_pShader->SetTexture ( "g_txDiffuse", (LPDIRECT3DTEXTURE9)pcmd->TextureId ) ;
				else
					s_pShader->SetTexture ( "g_txDiffuse", s_pWhiteTexture ) ;

				s_pDevice->SetScissorRect(&r);

				UINT uiPassCount ;
				s_pShader->Begin ( &uiPassCount, 0 ) ;
				for ( UINT uiPass = 0 ; uiPass < uiPassCount ; uiPass++ ) {
					s_pShader->BeginPass ( uiPass ) ;
					s_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vtx_offset, 0, (UINT)cmd_list->VtxBuffer.size(), idx_offset, pcmd->ElemCount / 3);
					s_pShader->EndPass() ;
				}
				s_pShader->End () ;

			}
			idx_offset += pcmd->ElemCount;
		}
		vtx_offset += cmd_list->VtxBuffer.size();
	}

	s_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	s_pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
}

bool CGuiRenderer::Initialize(IDirect3DDevice9* device, int width, int height)
{
	s_pDevice = device;
	device->AddRef();

	ImGuiIO& io = ImGui::GetIO();
	io.MouseDoubleClickTime = 0.1f;
	io.DisplaySize = ImVec2((float)width, (float)height); // By Rahimi
	io.KeyMap[ImGuiKey_Tab] = VK_TAB; // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
	io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
	io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
	io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
	io.KeyMap[ImGuiKey_Home] = VK_HOME;
	io.KeyMap[ImGuiKey_End] = VK_END;
	io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
	io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
	io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
	io.KeyMap[ImGuiKey_A] = 'A';
	io.KeyMap[ImGuiKey_C] = 'C';
	io.KeyMap[ImGuiKey_V] = 'V';
	io.KeyMap[ImGuiKey_X] = 'X';
	io.KeyMap[ImGuiKey_Y] = 'Y';
	io.KeyMap[ImGuiKey_Z] = 'Z';

	io.RenderDrawListsFn = RenderCallback;  // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
	io.MouseDrawCursor = false;
	ShowCursor(io.MouseDrawCursor ? FALSE : TRUE );

	D3DXCreateEffectFromFileA ( device,
		"Gui.fx",
		NULL,
		NULL,
		0,
		C3DGfx::GetInstance ()->GetEffectPool (),
		&s_pShader,
		NULL );
	//s_pShader = CResourceManager::GetInstance()->LoadEffectFromFile( "Data/Shader/Gui.fx", device, 0, "Data/Shader/" ) ;
	if ( ! s_pShader )
	{
		return false;
	}

	if (FAILED(device->CreateVertexBuffer(VB_SIZE * sizeof(Vertex), /*D3DUSAGE_DYNAMIC |*/ D3DUSAGE_WRITEONLY, FVF,
		D3DPOOL_MANAGED, &s_pVB, NULL)))
	{
		return false;
	}

	if (FAILED(device->CreateIndexBuffer(IB_SIZE * sizeof(ImDrawIdx), /*D3DUSAGE_DYNAMIC | */D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32,
		D3DPOOL_MANAGED, &s_pIB, NULL)))
	{
		return false;
	}

	if (!CreateFontTexture())
		return false;

	D3DXCreateTextureFromFile ( device, "White.png", &s_pWhiteTexture ) ;

	//((ImGuiState*)ImGui::GetInternalState())->Style.Colors[ImGuiCol_WindowBg] = ImColor(255, 255, 0, 255);
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(32, 32, 32, 255));

	return true;
}

void CGuiRenderer::CleanUp()
{
	if (s_pVB)
		s_pVB->Release();

	if (s_pIB)
		s_pIB->Release();

	if (s_pFontTexture)
		s_pFontTexture->Release();

	if (s_pDevice)
		s_pDevice->Release();

	if ( s_pShader )
		s_pShader->Release () ;
	s_pShader = NULL ;

	ImGui::Shutdown();
}

void CGuiRenderer::Update( float dt )
{
	ImGuiIO& io = ImGui::GetIO();

	io.DeltaTime = dt;

	// Read keyboard modifiers inputs
	//BYTE keystate[256];
	//GetKeyboardState(keystate);
	//for (int i = 0; i < 256; i++){
	//	io.KeysDown[i] = (keystate[i] & 0x80) != 0;
	//}

	//bool b1 = (GetAsyncKeyState('A') & 0x8000) != 0;
	//bool b2 = (keystate['A'] & 0x80 ) != 0;
	//if (b1 == true && b2 == true)
	//	b1 = 0;

	//io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	//io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	//io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
	// io.KeysDown : filled by WM_KEYDOWN/WM_KEYUP events
	// io.MousePos : filled by WM_MOUSEMOVE events
	// io.MouseDown : filled by WM_*BUTTON* events
	// io.MouseWheel : filled by WM_MOUSEWHEEL events

	// Hide OS mouse cursor if ImGui is drawing it
	//HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW);
	//SetCursor(io.MouseDrawCursor ? NULL : hCursor );

	// Start the frame
	ImGui::NewFrame();
}

bool CGuiRenderer::CreateFontTexture()
{
	ImGuiIO& io = ImGui::GetIO();

	ImWchar ranges[] = {
		0x0020, 0x00ff,
		0x0600, 0x06ff,
		0xFB50, 0xFDFF,
		0xFE70, 0xFEFF,
		0
	};

	//ImFont* pFont = io.Fonts->AddFontFromFileTTF("bbc-nassim-regular.ttf", 26, NULL, ranges);

	// Build
	unsigned char* pixels;
	int width, height, bytes_per_pixel;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

	// Create DX9 texture
	s_pFontTexture = NULL;
	if (D3DXCreateTexture(s_pDevice, width, height, 1, 0/*D3DUSAGE_DYNAMIC*/, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &s_pFontTexture) < 0)
		return false;
	D3DLOCKED_RECT tex_locked_rect;
	if (s_pFontTexture->LockRect(0, &tex_locked_rect, NULL, 0) != D3D_OK)
		return false;
	for (int y = 0; y < height; y++)
		memcpy((unsigned char *)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, pixels + (width * bytes_per_pixel) * y, (width * bytes_per_pixel));
	s_pFontTexture->UnlockRect(0);

	// Store our identifier
	io.Fonts->TexID = (void *)s_pFontTexture;

	// Cleanup (don't clear the input data if you want to append new fonts later)
	io.Fonts->ClearInputData();
	io.Fonts->ClearTexData();
	return true;
}


void CGuiRenderer::Render()
{
	ImGui::Render();
}

void CGuiRenderer::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_LBUTTONDOWN)
		ImGui::GetIO().MouseDown[0] = true;
	else if (message == WM_LBUTTONUP)
		ImGui::GetIO().MouseDown[0] = false;
	else if (message == WM_RBUTTONDOWN)
		ImGui::GetIO().MouseDown[1] = true;
	else if (message == WM_RBUTTONUP)
		ImGui::GetIO().MouseDown[1] = false;
	else if (message == WM_MOUSEWHEEL)
		ImGui::GetIO().MouseWheel = GET_WHEEL_DELTA_WPARAM(wParam);
	else if ( message == WM_MOUSEMOVE ) {
		POINT pt;
		pt.x = GET_X_LPARAM ( lParam );
		pt.y = GET_Y_LPARAM ( lParam ) ;
		//::ScreenToClient ( hWnd, &pt ) ;
		ImGui::GetIO().MousePos.x = (float)pt.x/*GET_X_LPARAM(lParam)*/;
		ImGui::GetIO().MousePos.y = (float)pt.y/*GET_Y_LPARAM(lParam)*/;
	}
	else if (message == WM_KEYDOWN) {
		if (wParam < 256)
			ImGui::GetIO().KeysDown[wParam] = true;
	}
	else if (message == WM_KEYUP) {
		if (wParam < 256)
			ImGui::GetIO().KeysDown[wParam] = false;
	}

}

