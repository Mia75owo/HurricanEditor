// Datei : DX8Graphics.cpp

// --------------------------------------------------------------------------------------
//
// Direct Graphics Klasse
// zum initialisieren von DirectX8
// beinhaltet zudem verschiedene Grafik-Funktionen zum Speichern von Screenshots usw
//
// (c) 2002 Jörg M. Winterstein
//
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// Include Dateien
// --------------------------------------------------------------------------------------

#include "DX8Graphics.hpp"
#include <time.h>
#include <string>
#include "DX8Texture.hpp"
#include "Globals.hpp"
#include "Logdatei.hpp"
#include "Mathematics.hpp"

// --------------------------------------------------------------------------------------
// sonstige Variablen
// --------------------------------------------------------------------------------------

glm::mat4x4 matProj;   // Projektionsmatrix
glm::mat4x4 matWorld;  // Weltmatrix
float DegreetoRad[360];  // Tabelle mit Rotationswerten

// --------------------------------------------------------------------------------------
// Klassenfunktionen
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// DirectGraphics Klasse
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// Konstruktor
// --------------------------------------------------------------------------------------

DirectGraphicsClass::DirectGraphicsClass() {
    SupportedETC1 = false;
    SupportedPVRTC = false;
    use_shader = shader_t::COLOR;

    ProgramCurrent = PROGRAM_NONE;
}

// --------------------------------------------------------------------------------------
// Desktruktor
// --------------------------------------------------------------------------------------

DirectGraphicsClass::~DirectGraphicsClass() {}

// --------------------------------------------------------------------------------------
// D3D Initialisieren
// --------------------------------------------------------------------------------------
bool DirectGraphicsClass::Init(std::uint32_t dwBreite, std::uint32_t dwHoehe, std::uint32_t dwZ_Bits, bool VSync) {
    int ScreenWidth = SCREENWIDTH;
    int ScreenHeight = SCREENHEIGHT;
    RenderWidth = SCREENWIDTH;
    RenderHeight = SCREENHEIGHT;

    int const ScreenDepth = 32;

    uint32_t flags = SDL_WINDOW_OPENGL;

    Protokoll << "\n--> SDL/OpenGL init <--\n";
    Protokoll << "---------------------\n" << std::endl;

    // Initialize defaults, Video and Audio subsystems
    Protokoll << "Initializing SDL." << std::endl;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) == -1) {
        Protokoll << "Failed to initialize SDL: " << SDL_GetError() << "." << std::endl;
        return false;
    }
    Protokoll << "SDL initialized." << std::endl;

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);  //      (Can now be changed via command line switch)
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);          // DKS - No need for a depth buffer in this game
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);  // DKS - Changed this to 0 (Game would not load w/ GL1.2 laptop)
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Setup SDL Screen
    //if (isFullscreen) {
        //flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    //}

    flags |= SDL_WINDOW_RESIZABLE;

    // Create a window. Window mode MUST include SDL_WINDOW_OPENGL for use with OpenGL.
    Window =
        SDL_CreateWindow("Hurrican", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ScreenWidth, ScreenHeight, flags);
    if (Window == nullptr) {
        Protokoll << "Failed to create " << ScreenWidth << "x" << ScreenHeight
                  << " window: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create an OpenGL context associated with the window.
    GLcontext = SDL_GL_CreateContext(Window);
    if (GLcontext == nullptr) {
        Protokoll << "Failed to create GL context: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_ShowCursor(SDL_DISABLE);

    // If not using EGL, i.e. using SDL's GL handling, some more handling of
    //  Vsync is necessary now that context has been created:
    {
        int retval = -1;
        if (VSync) {
            // Beginning with SDL 2.0, we set vsync directly with function.
            // First, try setting it to -1, which requests 'late swap tearing', which
            //  will not wait for vsync if previous frame was missed:

            Protokoll << "-> Requesting SDL2 GL to enable VSync with 'late swap tearing' (optimal)" << std::endl;
            retval = SDL_GL_SetSwapInterval(-1);
            if (retval < 0) {
                Protokoll << "-> 'Late swap tearing' VSync not supported:\n" << SDL_GetError() << std::endl;
                Protokoll << "-> Requesting SDL2 GL to enable standard VSync" << std::endl;
                retval = SDL_GL_SetSwapInterval(1);
                if (retval < 0) {
                    Protokoll << "-> *** SDL2 GL failed to enable VSync:\n" << SDL_GetError() << std::endl;
                    VSyncEnabled = false;
                } else {
                    Protokoll << "-> VSync enabled successfully" << std::endl;
                    VSyncEnabled = true;
                }
            } else {
                Protokoll << "-> VSync with late-swap-tearing enabled successfully" << std::endl;
                VSyncEnabled = true;
            }
        } else {
            Protokoll << "-> Requesting SDL2 GL to disable VSync" << std::endl;
            retval = SDL_GL_SetSwapInterval(0);
            if (retval < 0) {
                Protokoll << "-> *** SDL2 GL failed to disable VSync:\n" << SDL_GetError() << std::endl;
                VSyncEnabled = true;
            } else {
                VSyncEnabled = false;
            }
        }
    }

    if (!SetDeviceInfo())
        return false;

    Protokoll << "\n-> OpenGL init successful!\n" << std::endl;

    // DegreetoRad-Tabelle füllen
    for (int i = 0; i < 360; i++)
        DegreetoRad[i] = PI * static_cast<float>(i) / 180.0f;

    SetColorKeyMode();

    return true;
}

// --------------------------------------------------------------------------------------
// Direct3D beenden
// --------------------------------------------------------------------------------------

bool DirectGraphicsClass::Exit() {
    Shaders[PROGRAM_COLOR].Close();
    Shaders[PROGRAM_TEXTURE].Close();
    Shaders[PROGRAM_RENDER].Close();

    SDL_GL_DeleteContext(GLcontext);
    SDL_DestroyWindow(Window);
    SDL_Quit();
    Protokoll << "-> SDL/OpenGL shutdown successfully completed !" << std::endl;
    return true;
}

void DirectGraphicsClass::ResizeToWindow() {
  int width;
  int height;
  SDL_GetWindowSize(Window, &width, &height);

  WindowView.w = width;
  WindowView.h = height;
  RenderRect.w = width;
  RenderRect.h = height;


  glViewport(WindowView.x, WindowView.y, WindowView.w, WindowView.h); /* Setup our viewport. */
}

// --------------------------------------------------------------------------------------
// Infos für Device Objekt setzen
// Für Init und nach Task Wechsel
// --------------------------------------------------------------------------------------

bool DirectGraphicsClass::SetDeviceInfo() {
    std::string vert;
    std::string frag;
    const char* glsl_version = "100";

    SetupFramebuffers();

    /* OpenGL Information */
    Protokoll << "GL_VENDOR: " << glGetString(GL_VENDOR) << std::endl;
    Protokoll << "GL_RENDERER: " << glGetString(GL_RENDERER) << std::endl;
    Protokoll << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
    Protokoll << "GL_SHADING_LANGUAGE_VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    //glextensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
    //Protokoll << "GL_EXTENSIONS: " << glextensions << std::endl;

    /* Init OpenGL */
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); /* Set the background black */

    glClearDepth(1.0);                     /* Depth buffer setup */

    glDisable(GL_DEPTH_TEST); /* No Depth Testing */
    glEnable(GL_BLEND);

    // Compile the shader code and link into a program
    vert = g_storage_ext + "/data/shaders/" + glsl_version + "/shader_color.vert";
    frag = g_storage_ext + "/data/shaders/" + glsl_version + "/shader_color.frag";

    if (!Shaders[PROGRAM_COLOR].Load(vert, frag)) {
        return false;
    }

    vert = g_storage_ext + "/data/shaders/" + glsl_version + "/shader_texture.vert";
    frag = g_storage_ext + "/data/shaders/" + glsl_version + "/shader_texture.frag";

    if (!Shaders[PROGRAM_TEXTURE].Load(vert, frag)) {
        return false;
    }

    vert = g_storage_ext + "/data/shaders/" + glsl_version + "/shader_render.vert";
    frag = g_storage_ext + "/data/shaders/" + glsl_version + "/shader_render.frag";

    Shaders[PROGRAM_RENDER].AddConstant("c_WindowWidth", RenderRect.w);
    Shaders[PROGRAM_RENDER].AddConstant("c_WindowHeight", RenderRect.h);

    if (!Shaders[PROGRAM_RENDER].Load(vert, frag)) {
        return false;
    }

    // Get names for attributes and uniforms
    Shaders[PROGRAM_COLOR].NamePos = Shaders[PROGRAM_COLOR].GetAttribute("a_Position");
    Shaders[PROGRAM_COLOR].NameClr = Shaders[PROGRAM_COLOR].GetAttribute("a_Color");
    Shaders[PROGRAM_COLOR].NameMvp = Shaders[PROGRAM_COLOR].GetUniform("u_MVPMatrix");

    Shaders[PROGRAM_TEXTURE].NamePos = Shaders[PROGRAM_TEXTURE].GetAttribute("a_Position");
    Shaders[PROGRAM_TEXTURE].NameClr = Shaders[PROGRAM_TEXTURE].GetAttribute("a_Color");
    Shaders[PROGRAM_TEXTURE].NameTex = Shaders[PROGRAM_TEXTURE].GetAttribute("a_Texcoord0");
    Shaders[PROGRAM_TEXTURE].NameMvp = Shaders[PROGRAM_TEXTURE].GetUniform("u_MVPMatrix");

    Shaders[PROGRAM_RENDER].NamePos = Shaders[PROGRAM_RENDER].GetAttribute("a_Position");
    Shaders[PROGRAM_RENDER].NameClr = Shaders[PROGRAM_RENDER].GetAttribute("a_Color");
    Shaders[PROGRAM_RENDER].NameTex = Shaders[PROGRAM_RENDER].GetAttribute("a_Texcoord0");
    Shaders[PROGRAM_RENDER].NameMvp = Shaders[PROGRAM_RENDER].GetUniform("u_MVPMatrix");
    NameTime                        = Shaders[PROGRAM_RENDER].GetUniform("u_Time");

    /* Matrices setup */
    g_matView = glm::mat4x4(1.0f);
    g_matModelView = glm::mat4x4(1.0f);

    matProjWindow = glm::ortho(0.0f, static_cast<float>(WindowView.w), static_cast<float>(WindowView.h), 0.0f, 0.0f, 1.0f);

    matProj = matProjWindow;

    return true;
}

// --------------------------------------------------------------------------------------
// Renderstates für Sprites mit ColorKey setzen
// --------------------------------------------------------------------------------------

void DirectGraphicsClass::SetColorKeyMode() {
    if (BlendMode == BlendModeEnum::COLORKEY)
        return;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    BlendMode = BlendModeEnum::COLORKEY;
}

// --------------------------------------------------------------------------------------
// Renderstates für Sprites setzen, die komplett weiss gerendert werden
// --------------------------------------------------------------------------------------

void DirectGraphicsClass::SetWhiteMode() {
    if (BlendMode == BlendModeEnum::WHITE)
        return;

    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA);

    BlendMode = BlendModeEnum::WHITE;
}

// --------------------------------------------------------------------------------------
// Renderstates für Sprites mit Additivem Alphablending setzen
// --------------------------------------------------------------------------------------

void DirectGraphicsClass::SetAdditiveMode() {
    if (BlendMode == BlendModeEnum::ADDITIV)
        return;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    BlendMode = BlendModeEnum::ADDITIV;
}

// --------------------------------------------------------------------------------------
// Renderstates für linearen Texturfilter ein/ausschalten
// --------------------------------------------------------------------------------------

void DirectGraphicsClass::SetFilterMode(bool filteron) {
    FilterMode = filteron;
}

// --------------------------------------------------------------------------------------
// Rendert in den Buffer, der am Ende eines jeden Frames komplett
// in den Backbuffer gerendert wird
// --------------------------------------------------------------------------------------

void DirectGraphicsClass::RendertoBuffer(GLenum PrimitiveType,
                                         std::uint32_t PrimitiveCount,
                                         void *pVertexStreamZeroData) {
    constexpr int STRIDE = sizeof(VERTEX2D);
    constexpr size_t CLR_OFFSET = offsetof(VERTEX2D, color);
    constexpr size_t TEX_OFFSET = offsetof(VERTEX2D, tu);

    uint8_t program_next;
    bool is_texture;

    // Determine the shader program to use
    switch (use_shader) {
    case shader_t::TEXTURE:
        program_next = PROGRAM_TEXTURE;
        is_texture = true;
        break;
    case shader_t::RENDER:
        program_next = PROGRAM_RENDER;
        is_texture = true;
        break;
    default:
        program_next = PROGRAM_COLOR;
        is_texture = false;
    }

    // Check if the program is already in use
    if (ProgramCurrent != program_next) {
        Shaders[program_next].Use();
        if (program_next==PROGRAM_RENDER) {
            glUniform1i(NameTime, 50*SDL_GetTicks()/1000);
        }
        ProgramCurrent = program_next;
    }

    if (PrimitiveType == GL_LINES) {
        PrimitiveCount *= 2;
    } else if (PrimitiveType == GL_LINE_STRIP) {
        PrimitiveCount += 2;
    } else if (PrimitiveType == GL_TRIANGLES) {
        PrimitiveCount *= 3;
    } else if (PrimitiveType == GL_TRIANGLE_STRIP) {
        PrimitiveCount += 2;
    } else {
        Protokoll << "Add type to count indinces" << std::endl;
        return;
    }

    // Enable attributes and uniforms for transfer
    if (is_texture) {
        glEnableVertexAttribArray(Shaders[ProgramCurrent].NameTex);
        glVertexAttribPointer(Shaders[ProgramCurrent].NameTex, 2, GL_FLOAT, GL_FALSE, STRIDE,
                              reinterpret_cast<uint8_t *>(pVertexStreamZeroData) + TEX_OFFSET);
    }

    glEnableVertexAttribArray(Shaders[ProgramCurrent].NamePos);
    glVertexAttribPointer(Shaders[ProgramCurrent].NamePos, 2, GL_FLOAT, GL_FALSE, STRIDE, pVertexStreamZeroData);

    glEnableVertexAttribArray(Shaders[ProgramCurrent].NameClr);
    glVertexAttribPointer(Shaders[ProgramCurrent].NameClr, 4, GL_UNSIGNED_BYTE, GL_TRUE, STRIDE,
                          reinterpret_cast<uint8_t *>(pVertexStreamZeroData) + CLR_OFFSET);

    glm::mat4x4 matMVP = matProj * g_matModelView;
    glUniformMatrix4fv(Shaders[ProgramCurrent].NameMvp, 1, GL_FALSE, glm::value_ptr(matMVP));

    glDrawArrays(PrimitiveType, 0, PrimitiveCount);

    // Disable attributes and uniforms
    glDisableVertexAttribArray(Shaders[ProgramCurrent].NamePos);
    glDisableVertexAttribArray(Shaders[ProgramCurrent].NameClr);

    if (is_texture) {
        glDisableVertexAttribArray(Shaders[ProgramCurrent].NameTex);
    }
}

// --------------------------------------------------------------------------------------
// Render den Buffer auf den Backbuffer
// --------------------------------------------------------------------------------------

void DirectGraphicsClass::DisplayBuffer() {
    // Backbuffer mit Frontbuffer tauschen
    ShowBackBuffer();
}

bool DirectGraphicsClass::ExtensionSupported(const char *ext) {
    if (strstr(glextensions, ext) != nullptr) {
        Protokoll << ext << " is supported" << std::endl;
        return true;
    }

    Protokoll << ext << " is not supported" << std::endl;
    return false;
}

void DirectGraphicsClass::SetTexture(int idx) {
    if (idx >= 0) {
        use_shader = shader_t::TEXTURE;
        TextureHandle &th = Textures[idx];
        glBindTexture(GL_TEXTURE_2D, th.tex);
    } else {
        use_shader = shader_t::COLOR;
    }
}

// --------------------------------------------------------------------------------------
// Present aufrufen
// --------------------------------------------------------------------------------------

void DirectGraphicsClass::ShowBackBuffer() {
    SDL_GL_SwapWindow(Window);

#ifndef NDEBUG
    int error = glGetError();

    if (error != 0) {
        Protokoll << "GL Error " << std::hex << error << " file " << __FILE__ << ": line " << std::dec << __LINE__
                  << std::endl;
    }
#endif
}

void DirectGraphicsClass::SetupFramebuffers() {
/* Read the current window size */
    {
        int tmp_w, tmp_h;
        SDL_GetWindowSize(Window, &tmp_w, &tmp_h);
        WindowView.w = tmp_w;
        WindowView.h = tmp_h;
    }
    Protokoll << "Window resolution: " << WindowView.w << "x" << WindowView.h << std::endl;

    WindowView.x = 0;
    WindowView.y = 0;
    WindowView.w = RenderWidth;
    WindowView.h = RenderHeight;

    glViewport(WindowView.x, WindowView.y, WindowView.w, WindowView.h); /* Setup our viewport. */
    Protokoll << "Window viewport: " << WindowView.w << "x" << WindowView.h << " at " << WindowView.x << "x"
              << WindowView.y << std::endl;
}

void DirectGraphicsClass::ClearBackBuffer() {
    glClear(GL_COLOR_BUFFER_BIT);
}
