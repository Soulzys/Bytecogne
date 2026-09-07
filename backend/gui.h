#pragma once

// Win32 window procedure
//
// Forward declare based on the instructions in imgui_impl_win32.cpp
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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

    bool create_device_D3D     (DearGUI* gui, HWND handle);
    void cleanup_device_D3D    (DearGUI* gui);
    void create_render_target  (DearGUI* gui);
    void destroy_render_target (DearGUI* gui);
    void render                (DearGUI* gui, const real32 clear_color[4]);

    void draw_ui               (AppState* state);
}