#include "gui.h"

// Create / Destroy render target
//
void gui::create_render_target(DearGUI* gui)
{
    ID3D11Texture2D* back_buffer = nullptr;

    gui->swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    gui->device->CreateRenderTargetView(back_buffer, nullptr, &gui->render_target_view);
    back_buffer->Release();
}

void gui::destroy_render_target(DearGUI* gui)
{
    if (gui->render_target_view)
    {
        gui->render_target_view->Release();
        gui->render_target_view = nullptr;
    }
}


// D3D11 initialization
//
bool gui::create_device_D3D(DearGUI* gui, HWND handle)
{
    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};

    swap_chain_desc.BufferCount                        = 2;
    swap_chain_desc.BufferDesc.Width                   = 0;
    swap_chain_desc.BufferDesc.Height                  = 0;
    swap_chain_desc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferDesc.RefreshRate.Numerator   = 60;
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;

    swap_chain_desc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow                       = handle;
    swap_chain_desc.SampleDesc.Count                   = 1;
    swap_chain_desc.Windowed                           = TRUE;
    swap_chain_desc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL feature_level;
    const D3D_FEATURE_LEVEL feature_levels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };

    HRESULT hr = D3D11CreateDeviceAndSwapChain
    (
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        feature_levels,
        ARRAYSIZE(feature_levels),
        D3D11_SDK_VERSION,
        &swap_chain_desc,
        &gui->swap_chain,
        &gui->device,
        &feature_level,
        &gui->context
    );

    if (FAILED(hr))
    {
        return false;
    }

    create_render_target(gui);

    return true;
}

void gui::cleanup_device_D3D(DearGUI* gui)
{
    destroy_render_target(gui);

    if (gui->swap_chain)
    {
        gui->swap_chain->Release();
        gui->swap_chain = nullptr;
    }

    if (gui->context)
    {
        gui->context->Release();
        gui->context = nullptr;
    }

    if (gui->device)
    {
        gui->device->Release();
        gui->device = nullptr;
    }
}

void gui::render(DearGUI* gui, const real32 clear_color[4])
{
    ImGui::Render();

    gui->context->OMSetRenderTargets(1, &gui->render_target_view, nullptr);
    gui->context->ClearRenderTargetView(gui->render_target_view, clear_color);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    gui->swap_chain->Present(1, 0);
}

void draw_ui(const AppState& state)
{
	
}