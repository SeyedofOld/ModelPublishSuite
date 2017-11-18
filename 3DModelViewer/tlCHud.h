#include <d3dx9.h>
#include "imgui.h"

class tlGuiHud
{
private:
    struct Vertex
    {
        D3DXVECTOR3 pos;
        D3DCOLOR    col;
        D3DXVECTOR2 uv;
    };

private:
    static IDirect3DDevice9 *_device;
    static IDirect3DVertexBuffer9 *_vb;
    static IDirect3DTexture9 *_texture;    // Font texture

    static const UINT FVF = (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1);    // Drawing FVF
    static const UINT VB_SIZE = 10000;  // Number of vertices in vertex buffer

private:
    tlGuiHud() = delete;

    static void render_callback(ImDrawList** const cmd_lists, int cmd_lists_count)
    {
        size_t total_vtx_count = 0;
        for (int n = 0; n < cmd_lists_count; n++)
            total_vtx_count += cmd_lists[n]->vtx_buffer.size();
        if (total_vtx_count == 0)
            return;

        // Copy and convert all vertices into a single contiguous buffer
        Vertex* vtx_dst;
        if (_vb->Lock(0, total_vtx_count, (void**)&vtx_dst, D3DLOCK_DISCARD) < 0)
            return;
        for (int n = 0; n < cmd_lists_count; n++)
        {
            const ImDrawList* cmd_list = cmd_lists[n];
            const ImDrawVert* vtx_src = &cmd_list->vtx_buffer[0];
            for (size_t i = 0; i < cmd_list->vtx_buffer.size(); i++)
            {
                vtx_dst->pos.x = vtx_src->pos.x;
                vtx_dst->pos.y = vtx_src->pos.y;
                vtx_dst->pos.z = 0.0f;
                
                vtx_dst->col = (vtx_src->col & 0xFF00FF00) | ((vtx_src->col & 0xFF0000) >> 16) |
                    ((vtx_src->col & 0xFF) << 16);     // RGBA --> ARGB for DirectX9
                    
                vtx_dst->uv.x = vtx_src->uv.x;
                vtx_dst->uv.y = vtx_src->uv.y;
                vtx_dst++;
                vtx_src++;
            }
        }
        _vb->Unlock();

        _device->SetStreamSource(0, _vb, 0, sizeof(Vertex));
        _device->SetFVF(FVF);

        // Setup render state: alpha-blending, no face culling, no depth testing
        _device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        _device->SetRenderState(D3DRS_LIGHTING, false);
        _device->SetRenderState(D3DRS_ZENABLE, false);
        _device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
        _device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        _device->SetRenderState(D3DRS_ALPHATESTENABLE, false);
        _device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        _device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        _device->SetRenderState(D3DRS_SCISSORTESTENABLE, true);

        // Setup texture
        _device->SetTexture(0, _texture);
        _device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        _device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        _device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        _device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        _device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        _device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

        // Setup orthographic projection matrix (2D Rendering)
        D3DXMATRIXA16 mat;
        D3DXMatrixIdentity(&mat);
        _device->SetTransform(D3DTS_WORLD, &mat);
        _device->SetTransform(D3DTS_VIEW, &mat);
        D3DXMatrixOrthoOffCenterLH(&mat, 0.5f, ImGui::GetIO().DisplaySize.x + 0.5f, 
            ImGui::GetIO().DisplaySize.y + 0.5f, 0.5f, -1.0f, +1.0f);
        _device->SetTransform(D3DTS_PROJECTION, &mat);

        // Render command lists
        int vtx_offset = 0;
        for (int n = 0; n < cmd_lists_count; n++)
        {
            // Render command list
            const ImDrawList* cmd_list = cmd_lists[n];
            for (size_t cmd_i = 0; cmd_i < cmd_list->commands.size(); cmd_i++)
            {
                const ImDrawCmd* pcmd = &cmd_list->commands[cmd_i];
                const RECT r = { (LONG)pcmd->clip_rect.x, (LONG)pcmd->clip_rect.y, 
                    (LONG)pcmd->clip_rect.z, (LONG)pcmd->clip_rect.w };
                _device->SetScissorRect(&r);
                _device->DrawPrimitive(D3DPT_TRIANGLELIST, vtx_offset, pcmd->vtx_count / 3);
                vtx_offset += pcmd->vtx_count;
            }
        }

    }

public:
    static bool initialize(IDirect3DDevice9 *device, int width, int height)
    {
        // Set ImGui Params
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)width, (float)height);   
        io.DeltaTime = 1.0f / 60.0f;    
        io.PixelCenterOffset = 0.0f;
        io.KeyMap[ImGuiKey_Tab] = VK_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
        io.KeyMap[ImGuiKey_DownArrow] = VK_UP;
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

        io.RenderDrawListsFn = render_callback;

        // Create Graphics objects
        device->AddRef();
        _device = device;
        if (FAILED(device->CreateVertexBuffer(VB_SIZE*sizeof(Vertex), 
            D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, FVF, D3DPOOL_DEFAULT, &_vb, nullptr)))
        {
            return false;
        }

        // Load font texture
        const void* png_data;
        unsigned int png_size;
        ImGui::GetDefaultFontData(NULL, NULL, &png_data, &png_size);
        if (FAILED(D3DXCreateTextureFromFileInMemory(device, png_data, png_size, &_texture)))
        {
            return false;
        }

        return true;
    }

    static void release()
    {
        if(_vb)
            _vb->Release();

        if (_texture)
            _texture->Release();

        if (_device)
            _device->Release();        
    }

    static void update(float dt)
    {
        ImGuiIO& io = ImGui::GetIO();

        io.DeltaTime = dt;

        // Setup inputs
        // (we already got mouse position, buttons, wheel from the window message callback)
        BYTE keystate[256];
        GetKeyboardState(keystate);
        for (int i = 0; i < 256; i++)
            io.KeysDown[i] = (keystate[i] & 0x80) != 0;
        io.KeyCtrl = (keystate[VK_CONTROL] & 0x80) != 0;
        io.KeyShift = (keystate[VK_SHIFT] & 0x80) != 0;
        // io.MousePos : filled by WM_MOUSEMOVE event
        // io.MouseDown : filled by WM_*BUTTON* events
        // io.MouseWheel : filled by WM_MOUSEWHEEL events

        // Start the frame
        ImGui::NewFrame();
    }
};

