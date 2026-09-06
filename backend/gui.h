#pragma once

#include "app.h"

namespace gui
{
    struct DearGUI
    {
        ID3D11Device* device;
        ID3D11DeviceContext* context;
        IDXGISwapChain* swap_chain;
        ID3D11RenderTargetView* render_target_view;

        ImGuiIO* io;
    };

    bool create_device_D3D(DearGUI* gui, HWND handle);
    void cleanup_device_D3D(DearGUI* gui);
    void create_render_target(DearGUI* gui);
    void destroy_render_target(DearGUI* gui);

    void draw_ui(const AppState& state);
}