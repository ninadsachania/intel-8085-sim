#include <stdio.h>
#include <stdint.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include "SDL3/SDL.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include "SDL3/SDL_opengles2.h"
#else
#include "SDL3/SDL_opengl.h"
#endif
#include "log.h"

#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

#include "chip.h"

Chip chip;

const char *title = "8085 Simulator";
bool done = false;

char tutorial_text[8192] = R"(Assembly Language Guide
=======================

:Date: 2026-04-17
:Version: 0.1
:Authors: Ninad Sachania <ninad.sachania@gmail.com>


Introduction
------------

TODO: Write the introduction and the rest of the tutorial.
)";

//
// Helper functions.
//
template<typename T>
void clamp(T *value, T min, T max) {
    if (*value < min) { *value = min; }
    if (*value > max) { *value = max; }
}

void draw_menu_bar() {
    static bool open_tutorial = false;
    bool open_about    = false;

    if (ImGui::BeginMainMenuBar()) {
        //
        // Help.
        //
        if (ImGui::BeginMenu("Help")) {
            // Assembler Tutorial.
            if (ImGui::MenuItem("Assembler Tutorial")) {
                open_tutorial = true;
            }

            ImGui::Separator();

            // About.
            if (ImGui::MenuItem("About")) {
                open_about = true;
            }

            ImGui::EndMenu();
        }


        ImGui::EndMainMenuBar();
    }

    if (open_tutorial) {
        ImGui::Begin("Assembler Tutorial", &open_tutorial);
        ImGui::InputTextMultiline("##text", tutorial_text, sizeof(tutorial_text), ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
        ImGui::End();
    }

    if (open_about) {
        ImGui::OpenPopup("About");
    }

    // Always center this window when appearing.
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("About", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Made with love by Ninad Sachania :-)");

        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }

        ImGui::EndPopup();
    }
}

void draw_registers() {
    auto register_label_color = ImVec4(1.0f, 0.0f, 0.0f, 1.00f);

    ImGui::Begin("CPU State"); // , NULL, ImGuiWindowFlags_AlwaysAutoResize);

    //
    // Register A.
    //
    ImGui::TextColored(register_label_color, "A:");
    ImGui::SameLine();
    ImGui::Text("%02X", chip.reg_a);

    //
    // Register B & C.
    //
    ImGui::TextColored(register_label_color, "B:");
    ImGui::SameLine();
    ImGui::Text("%02X", chip.reg_b);
    ImGui::SameLine();
    ImGui::TextColored(register_label_color, "C:");
    ImGui::SameLine();
    ImGui::Text("%02X", chip.reg_c);

    //
    // Register D & E.
    //
    ImGui::TextColored(register_label_color, "D:");
    ImGui::SameLine();
    ImGui::Text("%02X", chip.reg_d);
    ImGui::SameLine();
    ImGui::TextColored(register_label_color, "E:");
    ImGui::SameLine();
    ImGui::Text("%02X", chip.reg_e);

    //
    // Register H & L.
    //
    ImGui::TextColored(register_label_color, "H:");
    ImGui::SameLine();
    ImGui::Text("%02X", chip.reg_h);
    ImGui::SameLine();
    ImGui::TextColored(register_label_color, "L:");
    ImGui::SameLine();
    ImGui::Text("%02X", chip.reg_l);

    //
    // Register stack pointer.
    //
    ImGui::TextColored(register_label_color, "SP:");
    ImGui::SameLine();
    ImGui::Text("%04X", chip.reg_sp);

    //
    // Register program counter.
    //
    ImGui::TextColored(register_label_color, "PC:");
    ImGui::SameLine();
    ImGui::Text("%04X", chip.reg_pc);

    //
    // Register program status word.
    //
    ImGui::TextColored(register_label_color, "PSW:");
    ImGui::SameLine();
    ImGui::Text("%04X", chip.reg_psw);

    ImGui::End();
}

void draw_flags() {
    auto flag_label_color = ImVec4(1.0f, 1.0f, 0.0f, 1.00f);

    ImGui::Begin("Flags");

    //
    // Flag S.
    //
    ImGui::TextColored(flag_label_color, " S:");
    ImGui::SameLine();
    ImGui::Text("%s", (chip.flag_s ? "1" : "0"));

    //
    // Flag Z.
    //
    ImGui::TextColored(flag_label_color, " Z:");
    ImGui::SameLine();
    ImGui::Text("%s", (chip.flag_z ? "1" : "0"));

    //
    // Flag AC.
    //
    ImGui::TextColored(flag_label_color, "AC:");
    ImGui::SameLine();
    ImGui::Text("%s", (chip.flag_ac ? "1" : "0"));

    //
    // Flag P.
    //
    ImGui::TextColored(flag_label_color, " P:");
    ImGui::SameLine();
    ImGui::Text("%s", (chip.flag_p ? "1" : "0"));

    //
    // Flag C.
    //
    ImGui::TextColored(flag_label_color, " C:");
    ImGui::SameLine();
    ImGui::Text("%s", (chip.flag_c ? "1" : "0"));

    ImGui::End();
}

void draw_view() {
    ImGui::Begin("View");

    if (ImGui::BeginTabBar("Tabs")) {
        //
        // Memory tab.
        //
        if (ImGui::BeginTabItem("Memory")) {
            if (ImGui::BeginTable("Memory", 3)) {

                ImGui::TableSetupColumn("Address (Hex)");
                ImGui::TableSetupColumn("Address (Decimal)");
                ImGui::TableSetupColumn("Value (Hex)");
                ImGui::TableHeadersRow();

                for (int row = 0; row < 1000 + 1; row += 1) { // @Hardcoded!!!
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%04X", row);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", row);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%02X", chip.memory[row]);
                }
                ImGui::EndTable();
            }

            ImGui::EndTabItem();
        }

        //
        // I/O ports tab.
        //
        if (ImGui::BeginTabItem("I/O Ports")) {
            if (ImGui::BeginTable("I/O Ports", 3)) {

                ImGui::TableSetupColumn("Address (Hex)");
                ImGui::TableSetupColumn("Address (Decimal)");
                ImGui::TableSetupColumn("Value (Hex)");
                ImGui::TableHeadersRow();

                for (int row = 0; row < sizeof(chip.io_ports); row += 1) {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%02X", row);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", row);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%02X", chip.io_ports[row]);
                }

                ImGui::EndTable();
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void draw_update_memory() {
    static int memory_address = 0x0;
    static int memory_value   = 0x0;

    ImGui::Begin("Update Memory");

    ImGui::InputScalar("Address (in hex)", ImGuiDataType_U16, &memory_address, NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
    clamp<int>(&memory_address, 0, chip.MEMORY_SIZE - 1);

    ImGui::InputScalar("Value (in hex)", ImGuiDataType_U16, &memory_value, NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal);
    clamp<int>(&memory_value, 0, 255);

    if (ImGui::Button("Update")) {
        log_info("Setting memory location %d to value %d.", memory_address, memory_value);
        chip.memory[memory_address] = memory_value;
    }

    ImGui::End();
}

void draw_ui() {
    draw_menu_bar();

    draw_registers();
    draw_flags();
    draw_view();
    draw_update_memory();
}

// Main code
int main(int argc, char **argv) {
    // Setup SDL
    // [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts would likely be your SDL_AppInit() function]
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        log_error("Error: SDL_Init(): %s", SDL_GetError());
        return 1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window* window = SDL_CreateWindow(title, (int)(1280 * main_scale), (int)(800 * main_scale), window_flags);
    if (window == nullptr) {
        log_error("Error: SDL_CreateWindow(): %s", SDL_GetError());
        return 1;
    }
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr) {
        log_error("Error: SDL_GL_CreateContext(): %s", SDL_GetError());
        return 1;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsClassic();
    // ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)
    io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
    io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If fonts are not explicitly loaded, Dear ImGui will select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
    //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
    // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefaultVector();
    //io.Fonts->AddFontDefaultBitmap();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!done)
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        // [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        draw_ui();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }

        SDL_GL_SwapWindow(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppQuit() function]
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
