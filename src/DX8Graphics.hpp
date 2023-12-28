// Datei : DX8Graphics.hpp

// --------------------------------------------------------------------------------------
//
// Direct Graphics Klasse
// zum initialisieren von DirectX8
// beinhaltet zudem verschiedene Grafik-Funktionen zum Speichern von Screenshots usw
//
// (c) 2002 Jörg M. Winterstein
//
// --------------------------------------------------------------------------------------

#ifndef _DX8GRAPHICS_HPP_
#define _DX8GRAPHICS_HPP_

// --------------------------------------------------------------------------------------
// Include Dateien
// --------------------------------------------------------------------------------------

#include "SDL_port.hpp"
#if defined(USE_GL2) || defined(USE_GL3)
#  include "cshader.hpp"
#endif /* USE_GL2 || USE_GL3 */

// --------------------------------------------------------------------------------------
// Defines
// --------------------------------------------------------------------------------------

enum class BlendModeEnum {
  ADDITIV,
  COLORKEY,
  WHITE
};

#if defined(USE_GL2) || defined(USE_GL3)
enum { PROGRAM_COLOR = 0, PROGRAM_TEXTURE, PROGRAM_RENDER, PROGRAM_TOTAL, PROGRAM_NONE };
#endif

// --------------------------------------------------------------------------------------
// Strukturen
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// Struktur für einen 2D Vertex
// --------------------------------------------------------------------------------------

// DKS - Removed unnecessary z-coordinate:
struct VERTEX2D {
    float x, y;      // x,y Koordinaten
    D3DCOLOR color;  // Vertex-Color
    float tu, tv;    // Textur-Koordinaten
};

// DKS - Added
struct QUAD2D {
    VERTEX2D v1, v2, v3, v4;
};

// --------------------------------------------------------------------------------------
// Klassendeklaration
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// DirectGraphics Klasse
// --------------------------------------------------------------------------------------

class DirectGraphicsClass {
  private:
    enum class shader_t {COLOR, TEXTURE, RENDER};

  private:
    bool VSyncEnabled;  // VSync ein/aus ?
    bool FilterMode;    // Linearer Filter an/aus?
    bool CrtEnabled;
    const char *glextensions;
    shader_t use_shader;
    BlendModeEnum BlendMode;  // Additiv, Colorkey oder White mode aktiviert?
    int MaxTextureUnits;
    bool SupportedETC1;
    bool SupportedPVRTC;
    GLuint ProgramCurrent;
    GLuint NameTime;
    CShader Shaders[PROGRAM_TOTAL];
    glm::mat4x4 matProjWindow;
    glm::mat4x4 matProjRender;

    SDL_GLContext GLcontext;
    SDL_Rect WindowView;
    SDL_Rect RenderRect;

  public:
    int RenderWidth;
    int RenderHeight;
    void ResizeToWindow(int width, int height);

    void ShowBackBuffer();  // Present aufrufen

    DirectGraphicsClass();   // Konstruktor
    ~DirectGraphicsClass();  // Desktruktor

    bool Init();
    bool Exit();  // D3D beenden
    bool SetDeviceInfo();

    void SetColorKeyMode();         // Alpha für Colorkey oder
    void SetAdditiveMode();         // Additive-Blending nutzen
    void SetWhiteMode();            // Komplett weiss rendern
    void SetFilterMode(bool filteron);  // Linearer Textur Filter ein/aus

    void RendertoBuffer(GLenum PrimitiveType,          // Rendert in den Buffer, der am Ende
                        std::uint32_t PrimitiveCount,  // eines jeden Frames komplett in
                        void *pVertexStreamZeroData);  // den Backbuffer gerendert wird

    void DisplayBuffer();  // Render den Buffer auf den Backbuffer
    // DKS - SetTexture is now used for both GL and DirectX, and uses new TexturesystemClass:
    void SetTexture(int idx);
    bool ExtensionSupported(const char *ext);
    void SetupFramebuffers();
    void ClearBackBuffer();
    void SelectBuffer(bool active);

    inline BlendModeEnum GetBlendMode() const { return BlendMode; }
    inline bool IsETC1Supported() const { return SupportedETC1; }
    inline bool IsPVRTCSupported() const { return SupportedPVRTC; }
};



// --------------------------------------------------------------------------------------
// Externals
// --------------------------------------------------------------------------------------

extern DirectGraphicsClass DirectGraphics;  // DirectGraphics Klasse
extern glm::mat4x4 matProj;                  // Projektionsmatrix
extern glm::mat4x4 matWorld;                 // Weltmatrix
extern float DegreetoRad[360];              // Tabelle mit Rotationswerten

#endif
