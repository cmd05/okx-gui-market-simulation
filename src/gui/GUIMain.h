#pragma once

#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <gui/GUIState.h>

constexpr float INPUT_PANEL_X = 300;
constexpr float INPUT_PANEL_Y = 200;

constexpr float OUTPUT_PANEL_X = 1100;
constexpr float OUTPUT_PANEL_Y = 200;

constexpr float PANEL_WIDTH = 600;
constexpr float PANEL_HEIGHT = 350;

extern InputWindowState g_input_window_state;
extern float g_curr_time;
extern float g_last_time;
extern float g_tick_latency;
extern float g_fps;

class GUIMain {
private:
    GLFWwindow* m_window;
public:
    GUIMain() {
        m_window = create_window();
        init_imgui();
    }

    ~GUIMain() {
        imgui_shutdown();
        glfwTerminate();
    }

    void calc_frame_times() {
        g_last_time = g_curr_time;
        g_curr_time = glfwGetTime();
        g_tick_latency = g_curr_time - g_last_time;
        g_fps = 1 / g_tick_latency;
    }

    void fill_input_data_gui(InputData& input_data) {
        input_data.instrument = g_input_window_state.instrument;
        input_data.order_sz = g_input_window_state.order_sz;
        input_data.fee_pct = g_input_window_state.fee_pct[g_input_window_state.selected_tier];
        input_data.volatility_pct = g_input_window_state.volatility_pct;
    }

    void init_imgui() {
        // Initialize ImGUI
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        set_imgui_style();
    }

    void imgui_new_frame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void imgui_render() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void imgui_shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void imgui_left_window() {
        
        ImGui::SetNextWindowPos(ImVec2(INPUT_PANEL_X, INPUT_PANEL_Y), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(PANEL_WIDTH, PANEL_HEIGHT));

        ImGui::Begin("Input Panel");
        
        ImGui::Text("Exchange: %s", g_input_window_state.exchange[0]);
        ImGui::InputText("SPOT Instrument (USDT-SWAP)", &g_input_window_state.instrument);
        ImGui::Text("Order Type: %s", g_input_window_state.order_type);
        ImGui::Text("Quantity: %i", g_input_window_state.order_sz);
    
        ImGui::SliderFloat("Volatility (%)", &g_input_window_state.volatility_pct, 0.01, 3.00);
        
        static const char* current_item = g_input_window_state.tiers[g_input_window_state.selected_tier];
        if (ImGui::BeginCombo("Fee Tier", current_item))
        {
            for (int n = 0; n < IM_ARRAYSIZE(g_input_window_state.tiers); n++)
            {
                bool is_selected = (current_item == g_input_window_state.tiers[n]);
                if (ImGui::Selectable(g_input_window_state.tiers[n], is_selected))
                    current_item = g_input_window_state.tiers[n];
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                    g_input_window_state.selected_tier = n;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));
        ImGui::Text("%s", g_input_window_state.error_txt.c_str());
        ImGui::PopStyleColor();

        // vertical spacing
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        // use seperate button color
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));       // Normal
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f)); // Hover
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.5f, 0.1f, 1.0f));  // Clicked

        float button_width = 100.0f; // Set your desired button width
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float available_width = ImGui::GetContentRegionAvail().x;

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (available_width - button_width));
        if (ImGui::Button("Update", ImVec2(button_width, 0))) {
            // Button clicked
            g_input_window_state.update_btn_clicked = true;
        }

        // pop the new button colors
        ImGui::PopStyleColor(3);

        ImGui::End();
    }

    void imgui_right_window(InputData& input_data, OutputData& output_data) {
        ImGui::SetNextWindowPos(ImVec2(OUTPUT_PANEL_X, OUTPUT_PANEL_Y), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(PANEL_WIDTH, PANEL_HEIGHT));
        ImGui::Begin("Output Panel");

        ImGui::Text("Selected %s for %iUSD quantity", input_data.instrument.c_str(), input_data.order_sz);
        ImGui::Dummy(ImVec2(0.0f, 20.0f));
        
        ImGui::Text("Mid Price : %f", output_data.mid_price);
        ImGui::Text("Slippage (USD): %f", output_data.slippage);
        ImGui::Text("Market Impact (USD): %f", output_data.market_impact);
        ImGui::Text("Fees (USD): %f", output_data.fees);
        ImGui::Text("Net Cost (USD): %f", output_data.net_cost);
        
        ImGui::Dummy(ImVec2(0.0f, 20.0f));
        ImGui::Text("Tick Latency (s): %f", g_tick_latency);
        ImGui::Text("FPS: %f", g_fps);

        ImGui::End();
    }
    
    GLFWwindow* create_window() {
        // glfw: initialize and configure
        // ------------------------------
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE); // maximize window when created

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

        // glfw window creation
        // --------------------
        GLFWwindow* window = glfwCreateWindow(1920, 1080, "OKX Exchange Trading", NULL, NULL);
        if (window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            // return -1;
        }
        glfwMakeContextCurrent(window);

        // glad: load all OpenGL function pointers
        // ---------------------------------------
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
        }

        return window;
    }

    void window_swap_buffers() {
        glfwSwapBuffers(m_window);
    }

    void poll_events() {
        glfwPollEvents();
    }

    bool window_should_close() {
        return glfwWindowShouldClose(m_window);
    }

    void clear_buffers() {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void process_input() {
        if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(m_window, true);
    }

    void set_imgui_style() {
        // Future Dark style by rewrking from ImThemes
        ImGuiStyle& style = ImGui::GetStyle();

        style.Alpha = 1.0f;
        style.DisabledAlpha = 0.5f;
        style.WindowPadding = ImVec2(13.0f, 10.0f);
        style.WindowRounding = 0.0f;
        style.WindowBorderSize = 1.0f;
        style.WindowMinSize = ImVec2(32.0f, 32.0f);
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
        style.WindowMenuButtonPosition = ImGuiDir_Right;
        style.ChildRounding = 3.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupRounding = 5.0f;
        style.PopupBorderSize = 1.0f;
        style.FramePadding = ImVec2(20.0f, 8.100000381469727f);
        style.FrameRounding = 2.0f;
        style.FrameBorderSize = 0.0f;
        style.ItemSpacing = ImVec2(3.0f, 10.0f);
        style.ItemInnerSpacing = ImVec2(3.0f, 8.0f);
        style.CellPadding = ImVec2(6.0f, 14.10000038146973f);
        style.IndentSpacing = 0.0f;
        style.ColumnsMinSpacing = 10.0f;
        style.ScrollbarSize = 10.0f;
        style.ScrollbarRounding = 2.0f;
        style.GrabMinSize = 12.10000038146973f;
        style.GrabRounding = 1.0f;
        style.TabRounding = 2.0f;
        style.TabBorderSize = 0.0f;
        style.ColorButtonPosition = ImGuiDir_Right;
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
        style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
        
        style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.2745098173618317f, 0.3176470696926117f, 0.4509803950786591f, 1.0f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5372549295425415f, 0.5529412031173706f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
        style.Colors[ImGuiCol_Tab] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_TabActive] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
        style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.5215686559677124f, 0.6000000238418579f, 0.7019608020782471f, 1.0f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.03921568766236305f, 0.9803921580314636f, 0.9803921580314636f, 1.0f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.2901960909366608f, 0.5960784554481506f, 1.0f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.9960784316062927f, 0.4745098054409027f, 0.6980392336845398f, 1.0f);
        style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
        style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
        style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
    }
};