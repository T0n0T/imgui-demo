// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "ImGui/imgui.h"
#include "ImGui/backends/imgui_impl_sdl2.h"
#include "ImGui/backends/imgui_impl_opengl3.h"
#include "ImGuiColorTextEdit/TextEditor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <fstream>
#include <sstream>
#include <SDL2/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL2/SDL_opengl.h>
#endif
#ifdef _WIN32
#include <windows.h>        // SetProcessDPIAware()
#include <commdlg.h>        // For file dialogs (GetOpenFileName, GetSaveFileName)
#include <shlobj.h>         // For SHBrowseForFolder
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "ImGui/examples/libs/emscripten/emscripten_mainloop_stub.h"
#endif

// Function declarations
std::string OpenFileDialog();
std::string SaveFileDialog();
bool LoadFile(const std::string& filePath, TextEditor& editor);
bool SaveFile(const std::string& filePath, const TextEditor& editor);

// Helper function to get filename from path
std::string GetFileName(const std::string& filePath) {
    size_t lastSlash = filePath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        return filePath.substr(lastSlash + 1);
    }
    return filePath;
}

// Windows file dialog implementation
#ifdef _WIN32
std::string OpenFileDialog() {
    char fileName[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "All Files (*.*)\0*.*\0C++ Files (*.cpp, *.h, *.hpp, *.cc)\0*.cpp;*.h;*.hpp;*.cc\0C Files (*.c, *.h)\0*.c;*.h\0Text Files (*.txt)\0*.txt\0\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        return std::string(ofn.lpstrFile);
    }
    return "";
}

std::string SaveFileDialog() {
    char fileName[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "All Files (*.*)\0*.*\0C++ Files (*.cpp, *.h, *.hpp, *.cc)\0*.cpp;*.h;*.hpp;*.cc\0C Files (*.c, *.h)\0*.c;*.h\0Text Files (*.txt)\0*.txt\0\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrDefExt = "txt";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameA(&ofn)) {
        return std::string(ofn.lpstrFile);
    }
    return "";
}
#else
// For non-Windows platforms, we'll need to implement alternatives or use a cross-platform library
// For now, we'll use a placeholder implementation that returns an empty string
std::string OpenFileDialog() {
    // On non-Windows systems, we'd typically use a library like nfd (nativefiledialog)
    // For this implementation, we'll just return an empty string
    // In a real application, you would integrate with GTK or another UI toolkit
    printf("Open file dialog not implemented for this platform\n");
    return "";
}

std::string SaveFileDialog() {
    // Same as above
    printf("Save file dialog not implemented for this platform\n");
    return "";
}
#endif

// Function to load file content into the editor
bool LoadFile(const std::string& filePath, TextEditor& editor) {
    if (filePath.empty()) {
        return false;
    }

    std::ifstream fileStream(filePath);
    if (!fileStream.is_open()) {
        return false;
    }

    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    std::string content = buffer.str();
    editor.SetText(content);
    fileStream.close();

    return true;
}

// Function to save editor content to a file
bool SaveFile(const std::string& filePath, const TextEditor& editor) {
    if (filePath.empty()) {
        return false;
    }

    std::ofstream fileStream(filePath);
    if (!fileStream.is_open()) {
        return false;
    }

    std::string content = editor.GetText();
    fileStream.write(content.c_str(), content.size());
    fileStream.close();

    return true;
}

// Main code
int main(int, char**)
{
    // Setup SDL
#ifdef _WIN32
    ::SetProcessDPIAware();
#endif
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
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

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    float main_scale = ImGui_ImplSDL2_GetContentScaleForDisplay(0);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 Text Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)(1280 * main_scale), (int)(800 * main_scale), window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return 1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr)
    {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Create text editor instance
    TextEditor editor;
    editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus()); // Set language definition
    editor.SetPalette(TextEditor::GetDarkPalette()); // Use dark palette
    editor.SetText("/* Sample code */\n\nint main() {\n\tprintf(\"Hello, world!\");\n\treturn 0;\n}\n");
    editor.SetShowWhitespaces(false);

    // Our state
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    
    // File management state
    std::string currentFilePath = "";
    bool fileModified = false;

    // Main loop
    bool done = false;
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
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                // Check if file has been modified before quitting
                if (fileModified) {
                    // In a real application, you would show a dialog asking if the user wants to save
                    printf("Warning: File has been modified. Please save before quitting.\n");
                    // For now, we'll allow quitting without saving
                }
                done = true;
            }
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) {
                // Check if file has been modified before closing
                if (fileModified) {
                    // In a real application, you would show a dialog asking if the user wants to save
                    printf("Warning: File has been modified. Please save before closing.\n");
                    // For now, we'll allow closing without saving
                }
                done = true;
            }
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // 1. Show the text editor window
        {
            // Update window title to show current file and modification status
            std::string windowTitle = "Text Editor";
            if (!currentFilePath.empty()) {
                windowTitle = GetFileName(currentFilePath);
                if (fileModified) {
                    windowTitle += " *";
                }
                windowTitle += " - Text Editor";
            } else {
                if (fileModified) {
                    windowTitle = "* Text Editor";
                } else {
                    windowTitle = "Text Editor";
                }
            }
            
            ImGui::Begin(windowTitle.c_str(), nullptr, ImGuiWindowFlags_MenuBar);

            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("New", "Ctrl+N"))
                    {
                        editor.SetText("");
                        currentFilePath = "";
                        fileModified = false;
                    }
                    if (ImGui::MenuItem("Open", "Ctrl+O"))
                    {
                        std::string filePath = OpenFileDialog();
                        if (!filePath.empty()) {
                            if (LoadFile(filePath, editor)) {
                                currentFilePath = filePath;
                                fileModified = false;
                            } else {
                                // Show error message
                                printf("Error: Could not open file %s\n", filePath.c_str());
                            }
                        }
                    }
                    if (ImGui::MenuItem("Save", "Ctrl+S"))
                    {
                        if (!currentFilePath.empty()) {
                            // If we have a file path, save directly to it
                            if (SaveFile(currentFilePath, editor)) {
                                fileModified = false;
                            } else {
                                // Show error message
                                printf("Error: Could not save file %s\n", currentFilePath.c_str());
                            }
                        } else {
                            // If no file path, show save as dialog
                            std::string filePath = SaveFileDialog();
                            if (!filePath.empty()) {
                                if (SaveFile(filePath, editor)) {
                                    currentFilePath = filePath;
                                    fileModified = false;
                                } else {
                                    // Show error message
                                    printf("Error: Could not save file %s\n", filePath.c_str());
                                }
                            }
                        }
                    }
                    if (ImGui::MenuItem("Save As..."))
                    {
                        std::string filePath = SaveFileDialog();
                        if (!filePath.empty()) {
                            if (SaveFile(filePath, editor)) {
                                currentFilePath = filePath;
                                fileModified = false;
                            } else {
                                // Show error message
                                printf("Error: Could not save file %s\n", filePath.c_str());
                            }
                        }
                    }
                    if (ImGui::MenuItem("Exit", "Alt+F4"))
                    {
                        done = true;
                    }
                    ImGui::EndMenu();
                }
                
                if (ImGui::BeginMenu("Edit"))
                {
                    bool ro = editor.IsReadOnly();
                    if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
                        editor.SetReadOnly(ro);
                    ImGui::Separator();

                    if (ImGui::MenuItem("Undo", "Ctrl+Z"))
                        editor.Undo();
                    if (ImGui::MenuItem("Redo", "Ctrl+Y"))
                        editor.Redo();

                    ImGui::Separator();
                    
                    if (ImGui::MenuItem("Copy", "Ctrl+C"))
                        editor.Copy();
                    if (ImGui::MenuItem("Cut", "Ctrl+X"))
                        editor.Cut();
                    if (ImGui::MenuItem("Paste", "Ctrl+V"))
                        editor.Paste();
                    if (ImGui::MenuItem("Delete", "Del"))
                        editor.Delete();

                    ImGui::Separator();

                    if (ImGui::MenuItem("Select all", "Ctrl+A"))
                        editor.SelectAll();

                    ImGui::EndMenu();
                }
                
                if (ImGui::BeginMenu("View"))
                {
                    if (ImGui::MenuItem("Dark palette"))
                        editor.SetPalette(TextEditor::GetDarkPalette());
                    if (ImGui::MenuItem("Light palette"))
                        editor.SetPalette(TextEditor::GetLightPalette());
                    if (ImGui::MenuItem("Retro Blue palette"))
                        editor.SetPalette(TextEditor::GetRetroBluePalette());
                    ImGui::EndMenu();
                }
                
                if (ImGui::BeginMenu("Language"))
                {
                    if (ImGui::MenuItem("Plain text"))
                        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::C()); // Use C for plain text
                    if (ImGui::MenuItem("C++"))
                        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
                    if (ImGui::MenuItem("C"))
                        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::C());
                    if (ImGui::MenuItem("GLSL"))
                        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
                    if (ImGui::MenuItem("HLSL"))
                        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::HLSL());
                    if (ImGui::MenuItem("SQL"))
                        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::SQL());
                    if (ImGui::MenuItem("AngelScript"))
                        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::AngelScript());
                    if (ImGui::MenuItem("Lua"))
                        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            ImGui::Text("Line: %d, Column: %d | Selected: %d characters", 
                       editor.GetCursorPosition().mLine + 1, 
                       editor.GetCursorPosition().mColumn + 1,
                       editor.HasSelection() ? (int)editor.GetSelectedText().length() : 0);

            // Check if editor content has changed to update fileModified flag
            static std::string lastText = editor.GetText();
            std::string currentText = editor.GetText();
            if (lastText != currentText) {
                fileModified = true;
                lastText = currentText;
            }

            // Render the text editor
            editor.Render("TextEditor");

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
