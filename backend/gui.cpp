#include "gui.h"
#include "networking.h"
#include "cogne.h"
#include "dex.h"

// >NOTE: these includes are only here to trigger intellisense for the dear imgui symbol recognition. They will need
//        to be eventually removed. 
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"

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

void gui::draw_ui(AppState* state)
{
    draw_main_window(state);
}


void gui::draw_main_window(AppState* state)
{
    bool connected = state->network->connected;

    ImGui::Begin("MainWindow");

    // Status bar
    //
    ImGui::BeginChild("StatusBar", ImVec2(0, 40), true);
    ImVec4 connection_status_color = connected ? ImVec4(0.f, 255.f, 0.f, 1.f) : ImVec4(255.f, 0.f, 0.f, 1.f);
    ImGui::TextColored(connection_status_color, connected ? "Connected" : "Not connected");
    ImGui::EndChild();

    // Left panel
    // 
    ImGui::BeginChild("LeftPanel", ImVec2(250, 0), true);
    ImGui::Text("Left Panel");
    ImGui::EndChild();
    ImGui::SameLine();
    //RightPanel
    //
    ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
    ImGui::Text("Right Panel");
    const char* str_disconnected = "Connect";
    const char* str_connnected = "Disconnect";
    if (ImGui::Button(connected ? str_connnected : str_disconnected))
    {
        if (connected)
        {
            net::stop(state->network);
            wnd::process_stop_node(state->window);
        }
        else
        {            
            if (!net::init(state->network, 5000))
            {
                ::WSACleanup();
                //return;
            }

            net::start(state->network);
            if (!wnd::process_start_node(state->window))
            {
                std::cerr << "Could not start node process !\n";
            }
        }
    }

    if (ImGui::Button("Paid Order"))
    {
        ImGui::OpenPopup("Dex - Paid Order");
        //ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        //ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }

    //draw_dex_token_pairs_popup();
    dex_paid_order_popup(state->dex->paid_order.internal_properties);

    if (ImGui::Button("Test"))
    {
        std::cout << stringify(state->dex->paid_order) << std::flush;
    }

    ImGui::EndChild();


    ImGui::End();
}

void gui::draw_dex_token_pairs_popup()
{
    if (ImGui::BeginPopupModal("Dex - TokenPairs", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Choose the data you want to retrieve");
        ImGui::Separator();

        static bool t = false;
        ImGui::Checkbox("test_checkbox", &t);

        if (ImGui::Button("Done"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}