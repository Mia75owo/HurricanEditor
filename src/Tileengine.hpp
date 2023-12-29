// Datei : Tileengine.hpp

// --------------------------------------------------------------------------------------
//
// 2D Tile-Engine für Hurrican
// bestehend aus einem Vordergrund-Layer in verschiedenen Helligkeitsstufen
//
// (c) 2002 Jörg M. Winterstein
//
// --------------------------------------------------------------------------------------

#ifndef _TILEENGINE_HPP_
#define _TILEENGINE_HPP_

// --------------------------------------------------------------------------------------
// Includes
// --------------------------------------------------------------------------------------
#include "DX8Graphics.hpp"
#include "DX8Sprite.hpp"
#include "Globals.hpp"

#include <cstdlib>

// --------------------------------------------------------------------------------------
// Defines
// --------------------------------------------------------------------------------------

//----- Flags für den Blockwert

enum BlockValue : uint32_t {
  BLOCKWERT_WAND           = 0x000001,    // Solide Wand
  BLOCKWERT_GEGNERWAND     = 0x000002,    // Wand nur für Gegner
  BLOCKWERT_PLATTFORM      = 0x000004,    // Plattform
  BLOCKWERT_LIGHT          = 0x000008,    // Licht bei Objekten verändern
  BLOCKWERT_VERDECKEN      = 0x000010,    // Spieler und Objekte verdecken
  BLOCKWERT_ANIMIERT_BACK  = 0x000020,    // Animiert Hintergrund
  BLOCKWERT_ANIMIERT_FRONT = 0x000040,    // Animiert Overlay
  BLOCKWERT_WASSER         = 0x000080,    // Wasser (macht platsch :D )
  BLOCKWERT_SCHADEN        = 0x000100,    // Schaden
  BLOCKWERT_FLIESSBANDL    = 0x000200,    // Fliessband Links
  BLOCKWERT_FLIESSBANDR    = 0x000400,    // Fliessband Rechts
  BLOCKWERT_WENDEPUNKT     = 0x000800,    // Bewegte Plattformen umdrehen lassen
  BLOCKWERT_DESTRUCTIBLE   = 0x001000,    // Zerstörbare Wand
  BLOCKWERT_MOVELINKS      = 0x002000,    // Textur nach links bewegen
  BLOCKWERT_OVERLAY_LIGHT  = 0x004000,    // Overlay nimmt Licht an
  BLOCKWERT_SUMPF          = 0x008000,    // Einsinken
  BLOCKWERT_EIS            = 0x010000,    // Eis, auf dem man ausrutscht
  BLOCKWERT_MOVEVERTICAL   = 0x020000,    // Vertikale Texturbewegung
  BLOCKWERT_WASSERFALL     = 0x040000,    // Wasserfall
  BLOCKWERT_MOVERECHTS     = 0x080000,    // Textur nach rechts bewegen

  BLOCKWERT_SCHRAEGE_L     = 0x200000,    // Schräge Rechts
  BLOCKWERT_SCHRAEGE_R     = 0x400000,    // Schräge Rechts
  BLOCKWERT_LIQUID         = 0x800000     // Flüssigkeit (Wasser, Säure, Lava, Magensäure)
};

//--- Animationsgeschwindigkeit der animierten Level-Tiles

constexpr float TILEANIM_SPEED = 0.8f;

//--- Werte zur Levelgrösse

constexpr int ORIGINAL_TILE_SIZE_X = 20;         // Grösse eines
constexpr int ORIGINAL_TILE_SIZE_Y = 20;         // einzelnen Tiles
constexpr float TILESETSIZE_X = 256.0f;  // Grösse eines
constexpr float TILESETSIZE_Y = 256.0f;  // Tilesets

constexpr int MAX_LEVELSIZE_X = 1024;  // Gesamtgrösse des Level
constexpr int MAX_LEVELSIZE_Y = 1600;

constexpr int MAX_TILESETS = 64;     // Maximalzahl der Tilesets
constexpr int INCLUDE_ZEROTILE = 1;  // Tile = 0;,0 im Tileset mit verwenden ?

constexpr int MAX_TILERECTS = 144;

//----- Grösse des nicht scrollbaren Bereichs

constexpr int SCROLL_BORDER_EXTREME_LEFT = 0;
constexpr int SCROLL_BORDER_EXTREME_TOP = 0;
constexpr int SCROLL_BORDER_EXTREME_RIGHT = 570;
constexpr int SCROLL_BORDER_EXTREME_BOTTOM = 400;

constexpr int SCROLL_BORDER_HORIZ = 0;
constexpr int SCROLL_BORDER_TOP = 50;
constexpr int SCROLL_BORDER_BOTTOM = 50;

constexpr int LOOK_BORDER_TOP = 60;
constexpr int LOOK_BORDER_BOTTOM = 400;

// --------------------------------------------------------------------------------------
// Strukturen
// --------------------------------------------------------------------------------------

// Struktur für ein Level Tile wie es aus dem Level geladen wird
//
struct LevelTileLoadStruct {
    uint8_t TileSetBack;              // Back  aus welchem Tileset ?
    uint8_t TileSetFront;             // Front aus welchem Tileset ?
    uint8_t BackArt;                  // Tile im Hintergrund
    uint8_t FrontArt;                 // Tile im Vordergrund
    uint8_t Red, Green, Blue, Alpha;  // Farbwert des Tiles
    uint32_t Block;                   // Blockierungsart (siehe #defines)
};

static_assert(sizeof(LevelTileLoadStruct) == 12, "Size of LevelTileLoadStruct is wrong");

// Struktur für ein Level Tile wie es im Level vorkommt (wie beim Laden, nur noch mit Extra Farben für alle Ecken)
//
struct LevelTileStruct {
    unsigned char TileSetBack;                // Back  aus welchem Tileset ?
    unsigned char TileSetFront;               // Front aus welchem Tileset ?
    unsigned char BackArt;                    // Tile im Hintergrund
    unsigned char FrontArt;                   // Tile im Vordergrund
    unsigned char Red, Green, Blue, Alpha;    // Farbwert des Tiles
    D3DCOLOR Color[4];                        // Farbwert des Tiles (Alle vier Ecken)
    uint32_t Block;                           // Blockierungsart (siehe #defines)
    bool move_v1, move_v2, move_v3, move_v4;  // Die Ecken eines Tiles bei der Wasseranim bewegen?
};

static_assert(sizeof(LevelTileStruct) == 32, "Size of LevelTileStruct is wrong");

// --------------------------------------------------------------------------------------
// Struktur für ein aus dem Level zu ladendes Objekte
// --------------------------------------------------------------------------------------

struct LevelObjectStruct {
    uint32_t ObjectID;           // Welche Objekt ID ?
    int32_t XPos;                // x-Position
    int32_t YPos;                // y-Position
    bool ChangeLight;         // Umgebungslicht annehmen ?
    uint8_t Skill;               // 0 = Easy, 1 = Medium, 2 = Hard, 3 = Hurrican
    uint8_t PADDING_CHUNK_1[2];  // 2 PADDING BYTES
    int32_t Value1;              // Werte für diverse Trigger
    int32_t Value2;              // Werte für diverse Trigger
};

static_assert(sizeof(LevelObjectStruct) == 24, "Size of LevelObjectStruct is wrong");

// --------------------------------------------------------------------------------------
// Level-Datei Header
// --------------------------------------------------------------------------------------

struct FileHeader {
    char Kennung[46];            // Level-Kennung
    char Beschreibung[100];      // Level-Beschreibung
    char BackgroundFile[24];     // Dateiname des Hintergrundes
    char ParallaxAFile[24];      // Dateiname des 1. Parallax Layers
    char ParallaxBFile[24];      // Dateiname des 2. Parallax Layers
    char CloudFile[24];          // Dateiname des WolkenLayers
    uint8_t PADDING_CHUNK_1[2];  // 2 PADDING  BYTES
    uint32_t Timelimit;          // aktuelles Zeitlimit des Levels
    uint8_t UsedTilesets;        // Anzahl der Tilesets
    char SetNames[64][16];       // Namen der benutzten Sets
    uint8_t PADDING_CHUNK_2[3];  // 3 PADDING BYTES
    uint32_t SizeX, SizeY;       // Größe des Levels
    uint32_t NumObjects;         // Anzahl der Objekte
    uint8_t ScrollBackground;    // Hintergrundbild srollen oder statisch ?
    uint8_t PADDING_CHUNK_3[3];  // 3 PADDING BYTES
};

static_assert(sizeof(FileHeader) == 1292, "Size of FileHeader is wrong");

// --------------------------------------------------------------------------------------
// Anhang am File nach den Level Daten
// damit nicht alle levels nochmal konvertiert werden, hänge ich einfach alle dinge, die
// noch ins level format reinmüssen, dahinter =)
// --------------------------------------------------------------------------------------

struct FileAppendix {
    char Songs[2][30];       // Namen der benutzten Songs (Stage und Boss)
    int32_t UsedPowerblock;  // ID der benutzten Powerblock Art
    char Col1[8], Col2[8];   // Farben für Liquid
    uint8_t Taschenlampe;
    uint8_t PADDING_CHUNK_1[3];  // 3 padding bytes
};

static_assert(sizeof(FileAppendix) == 84, "Size of FileAppendix is wrong");

// --------------------------------------------------------------------------------------
// Unions
// --------------------------------------------------------------------------------------

struct Vector2D {
    float x;
    float y;
};

// --------------------------------------------------------------------------------------
// Klassendeklaration
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// WaterSinTableClass
// --------------------------------------------------------------------------------------

class WaterSinTableClass {
  public:
    WaterSinTableClass() {
        ResetPosition();
        for (int i = 0; i < 17; ++i) {
            SinTable[i] = 2.5f * sinf(i * static_cast<float>(M_PI) / 32.0f);
            NonWaterSinTable[i] = 2.0f * SinTable[i];
        }
    }

    // Called in ctor and also when a level is loaded:
    void ResetPosition() {
        SinTablePos = 0;
        WaterSinTableIdx = 0;
        NonWaterSinTableIdx = 0;
    }

    // Compute individual table indexes based on SinTablePos and the
    //  current screen position within overall map. (Called from
    //  TileEngineClass::CalcRenderRange() where xLevel/yLevel are
    //  updated each frame before tiles are rendered)
    void UpdateTableIndexes(const int xlev, const int ylev) {
        // This is for the two water layers that sway up/down:
        WaterSinTableIdx = (static_cast<int>(SinTablePos) + xlev * 3 + ylev * 16) % 64;
        // This is for any artwork that sways left/right in the water, like plants or backgrounds
        NonWaterSinTableIdx = (static_cast<int>(SinTablePos) + ylev * 3) % 64;
    }

    // Called once per frame in TileEngineClass::UpdateLevel()
    void AdvancePosition(const float speed_faktor) {
        // Advance primary offset by 3.20, which is the same as
        // advancing the original code's sin table offset by
        // 2, which was the original position advancement, i.e.
        // 2 * (pi/20)Timer.sync() == (16/5) *(pi/32)Timer.sync()
        SinTablePos += static_cast<float>(16.0 / 5.0) * speed_faktor;
        while (SinTablePos >= 64.0f)
            SinTablePos -= 64.0f;
    }

    // This is for the two layers of water textures that sway up/down.
    // (Called from TileEngineClass::DrawWaterVectors())
    // The top two vertices of a tile use sin_pair[0]
    // and the bottom two vertices use sin_pair[1].
    void GetWaterSin(const int i, const int j, float (&sin_pair)[2]) {
        // Originally, every tile on a row would have specify a sin
        //  value pi/10 past the one to the left, but we'll instead
        //  make that 3(pi/32) past the one to the left. Close enough
        //  to be unnoticeable.
        int ext_idx = WaterSinTableIdx + (i + 1) * 3 + (j + 1) * 16;

        for (int ctr = 0; ctr < 2; ++ctr) {
            int table_idx = ext_idx;
            bool negate_result = ConvertToTableIndex(table_idx);
            sin_pair[ctr] = negate_result ? -SinTable[table_idx] : SinTable[table_idx];
            ext_idx += 16;  // Second sine value is one quadrant past first one
        }
    }

    // This is for the two layers of water textures that sway up/down.
    // (Called from TileEngineClass::DrawTileGroupedForwardList())
    // The top two vertices of a tile use sin_pair[0]
    // and the bottom two vertices use sin_pair[1].
    void GetNonWaterSin(const int j, float (&sin_pair)[2]) {
        int ext_idx = NonWaterSinTableIdx + (j + 1) * 3;

        for (int ctr = 0; ctr < 2; ++ctr) {
            int table_idx = ext_idx;
            bool negate_result = ConvertToTableIndex(table_idx);
            sin_pair[ctr] = negate_result ? -NonWaterSinTable[table_idx] : NonWaterSinTable[table_idx];

            ext_idx += 3;  // Second sine value originally is pi/10 past first one (2*j),but
                           //  we'll end up moving just 3*pi/32 using new trig table. That's
                           //  close enough to be unnoticable. (16.87 deg instead of 18 deg)
        }
    }

  private:
    // Convert an index x representing x/64 fraction of a circle,
    // 64 being 2*pi, to an internal table index 0-16, 16
    // representing pi/2, using symmetric property of sine function.
    // Boolean return value indicates if the value retrieved from the
    // table should be negated.
    bool ConvertToTableIndex(int &idx) {
        bool negate_result = false;
        idx %= 64;
        int increments_past_quad = idx % 16;
        int quad = idx / 16;

        switch (quad) {
            default:  // Default case shouldn't happen; fall through to case 0

            case 0:
                idx = increments_past_quad;
                break;
            case 1:
                idx = 16 - increments_past_quad;
                break;
            case 2:
                idx = increments_past_quad;
                negate_result = !negate_result;
                break;
            case 3:
                idx = 16 - increments_past_quad;
                negate_result = !negate_result;
                break;
        }

        return negate_result;
    }

    float SinTablePos;  // Value 0.0-63.999 that represents current base
                        //  used to generate offsets of water tile vertices.
                        //  Represents 0*(2*pi/64) through 64*(2*pi/64).
                        //  Incremented through UpdatePosition() function.

    // The two index vars, one for each table, are updated in UpdateTableIndexes() function:
    int WaterSinTableIdx;     // For the two layers of actual water that sway up/down
    int NonWaterSinTableIdx;  // For any artwork on a water tile like plants that sway left/right

    float SinTable[17], NonWaterSinTable[17];
};

// --------------------------------------------------------------------------------------
// TileEngine Klasse
// --------------------------------------------------------------------------------------

constexpr int TilesToRenderMax = 1024;

class TileEngineClass {
  private:
    FileHeader DateiHeader;  // Header der Level-Datei

    float TileAnimCount;  // Animations-Zähler und
    //float CloudMovement;
    int TileAnimPhase;                      // Phase der Tile Animation
    VERTEX2D TilesToRender[TilesToRenderMax * 6];    // Alle zu rendernden Leveltiles
    VERTEX2D v1, v2, v3, v4;                // Vertices zum Sprite rendern
    unsigned char LoadedTilesets;           // Anzahl geladener Sets

    WaterSinTableClass WaterSinTable;

    // Vorberechnung fürs Levelrendern

    int ScreenSizeTilesX;  // Leveltiles pro Screen
    int ScreenSizeTilesY;
    int RenderPosX;  // Bereich des Levels, der gerendert wird
    int RenderPosY;
    int RenderPosXTo;
    int RenderPosYTo;
    int xLevel;
    int yLevel;
    float xTileOffs;
    float yTileOffs;
    float xScreen;
    float yScreen;

    bool bScrollBackground;      // Hintegrundbild scrollen ?
    bool bDrawShadow;            // Taschenlampen Shatten im Alien Level rendern?

    RECT_struct TileRects[MAX_TILERECTS];        // vorberechnete Tile Ausschnitte
    DirectGraphicsSprite TileGfx[MAX_TILESETS];  // Tilegrafiken
    DirectGraphicsSprite LiquidGfx[2];           // Flüssigkeit
    DirectGraphicsSprite CloudLayer;             // Wolkenlayer
    DirectGraphicsSprite Shadow;                 // Schatten im Alien Level
    float WasserU[9];                            // vorberechnete TexturKoordinaten für das Wasser TU
    float WasserV[9];                            // vorberechnete TexturKoordinaten für das Wasser TV

    FileAppendix DateiAppendix;  // Anhang der Level-Datei

  public:
    LevelTileStruct Tiles[MAX_LEVELSIZE_X]  // Array mit Leveldaten
                         [MAX_LEVELSIZE_Y];

    float Scale = 1.0f;
    float TileSizeX;
    float TileSizeY;

    bool IsElevatorLevel;
    bool MustCenterPlayer;
    int ColR1, ColG1, ColB1, ColA1;  // Farben in RGB
    int ColR2, ColG2, ColB2, ColA2;
    int ColR3, ColG3, ColB3;

    char Beschreibung[100];      // Beschreibung des Levels
    int MaxBlocks;
    int MaxOneUps;
    int MaxSecrets;
    int MaxDiamonds;

    float SinPos2;  // Position in der SinusListe für den Wasserhintergrund

    std::vector<std::string> LoadedTilesetPaths;

    DirectGraphicsSprite Background;             // Hintergrund
    //DirectGraphicsSprite ParallaxLayer[3];       // Anzahl der Layer
    //DirectGraphicsSprite GameOver;               // GameOver Symbol
    DirectGraphicsSprite Wasserfall[2];          // Wasserfall Grafiken
    float WasserfallOffset;                      // Wasserfall Offset
    float XOffset, YOffset;                      // Scrolloffset des Levels
    float Timelimit;                             // Zeitlimit
    int LEVELSIZE_X;                             // Grösse des Levels
    int LEVELSIZE_Y;                             // in Tiles
    float LEVELPIXELSIZE_X;                      // Levelgrösse in Pixeln
    float LEVELPIXELSIZE_Y;                      // (für XOffset und YOffset)

    TileEngineClass();   // Konstruktor
    ~TileEngineClass();  // Destruktor

    void Zoom(float times);
    void ZoomBy(float times);

    void LoadSprites();

    void ClearLevel();                            // Level freigeben
    bool LoadLevel(const std::string &Filename);  // Level laden
    void SaveLevel(const std::string &Filename);  // Save level
    void InitNewLevel(int xSize, int ySize);      // Neues Level initialisieren
    void CalcRenderRange();                       // Bereiche berechnen, die gerendert werden sollen
    void DrawBackground();                        // Hintergrund Layer zeichnen
    void DrawBackLevel();                         // Level hintergrund anzeigen
    void DrawFrontLevel();                        // Level vordergrund anzeigen
    void DrawBackLevelOverlay();                  // Boden Tiles, die verdecken
    void DrawOverlayLevel();                      // Sonstige, die verdecken
    void DrawWater();                             // Wasser Planes rendern
    void CheckBounds();
    void UpdateLevel();                           // Level evtl scrollen usw

    uint32_t BlockRechts(float &x, float y, float &xo, float yo, RECT_struct rect, bool resolve = false);
    uint32_t BlockLinks(float &x, float y, float &xo, float yo, RECT_struct rect, bool resolve = false);
    uint32_t BlockOben(float x, float &y, float xo, float &yo, RECT_struct rect, bool resolve = false);
    uint32_t BlockUnten(float x, float &y, float xo, float &yo, RECT_struct rect, bool resolve = false);
    uint32_t BlockUntenNormal(float x, float y, float xo, float yo, RECT_struct rect);

    uint32_t BlockSlopes(const float x, float &y, const RECT_struct rect, const float ySpeed);

    D3DCOLOR LightValue(float x, float y, RECT_struct rect, bool forced);  // Helligkeit an Stelle x/y

    void ComputeCoolLight();  // Coole   Lightberechnung

    void DrawShadow();  // Schatten im Alien Level zeichnen

    void WertAngleichen(float &nachx, float &nachy, float vonx, float vony);

#ifdef NDEBUG
    inline
#endif
    LevelTileStruct &TileAt(const int i, const int j) {
#ifndef NDEBUG
        if (i >= MAX_LEVELSIZE_X || i < 0 || j >= MAX_LEVELSIZE_Y || j < 0) {
            Protokoll << "-> Error: Out of bounds in TileEngineClass::TileAt():\n"
                      << "\tparam i: " << i << "\tLower bound: " << 0 << "\tUpper bound: " << MAX_LEVELSIZE_X - 1
                      << "\n"
                      << "\tparam j: " << j << "\tLower bound: " << 0 << "\tUpper bound: " << MAX_LEVELSIZE_Y - 1
                      << std::endl;
            GameRunning = false;
            exit(EXIT_FAILURE);  // WriteText above should do this for us (first param==true)
        }

        // Stricter bounds-check I use when optimizing
        if (i >= LEVELSIZE_X || j >= LEVELSIZE_Y) {
            Protokoll << "-> Warning: Out of level bound in TileEngineClass::TileAt():\n"
                      << "\tparam i: " << i << "\tUpper bound: " << LEVELSIZE_X - 1 << "\n"
                      << "\tparam j: " << j << "\tUpper bound: " << LEVELSIZE_Y - 1
                      << std::endl;
        }
#endif
        return Tiles[i][j];
    }

    int32_t GetUsedPowerBlock() const { return DateiAppendix.UsedPowerblock; }
    const char* GetSong(int i) const { return DateiAppendix.Songs[i]; }
    void ToggleLamp();
};

// --------------------------------------------------------------------------------------
// Externals
// --------------------------------------------------------------------------------------

extern TileEngineClass TileEngine;
extern D3DCOLOR Col1, Col2, Col3;
extern bool DrawDragon;
extern float ShadowAlpha;

#endif
