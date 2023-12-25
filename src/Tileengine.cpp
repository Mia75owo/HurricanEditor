// Datei : Tileengine.cpp

// --------------------------------------------------------------------------------------
//
// 2D Tile-Engine für Hurrican
// bestehend aus einem Vordergrund-Layer in verschiedenen Helligkeitsstufen
// und einem Overlay-Layer
//
// (c) 2002 Jörg M. Winterstein
//
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// Includes
// --------------------------------------------------------------------------------------

#include <cmath>
#include <cstring>
#include <filesystem>
#include <string>
#include <algorithm>
#include "Gegner.hpp"
#include "ObjectList.hpp"
namespace fs = std::filesystem;

#include "DX8Graphics.hpp"
#include "DX8Sprite.hpp"
#include "Globals.hpp"
#include "Logdatei.hpp"
#include "Tileengine.hpp"
#include "Timer.hpp"
#include "Globals.hpp"

// --------------------------------------------------------------------------------------
// externe Variablen
// --------------------------------------------------------------------------------------

extern Logdatei Protokoll;
extern TimerClass Timer;
extern ObjectListClass ObjectList;

D3DCOLOR Col1, Col2, Col3;  // Farben für Wasser/Lava etc
bool DrawDragon;            // Für den Drachen im Turm Level
float ShadowAlpha;

// --------------------------------------------------------------------------------------
// Klassendefunktionen
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// TileEngine Klasse
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// Konstruktor
// --------------------------------------------------------------------------------------

TileEngineClass::TileEngineClass() {
    Scale = 1.0f;

    bDrawShadow = false;

    XOffset = 0.0f;
    YOffset = 0.0f;

    RenderPosX = 0;
    RenderPosY = 0;
    RenderPosXTo = 0;
    RenderPosYTo = 0;

    LEVELSIZE_X = 128;
    LEVELSIZE_Y = 96;

    //CloudMovement = 0.0f;
    TileAnimCount = 0.0f;
    TileAnimPhase = 0;
    LoadedTilesets = 0;

    memset(Tiles, 0, sizeof(Tiles));

    for (int i = 0; i < MAX_LEVELSIZE_X; i++)
        for (int j = 0; j < MAX_LEVELSIZE_Y; j++) {
            LevelTileStruct& tile = Tiles[i][j];
            tile.Red = 255;
            tile.Green = 255;
            tile.Blue = 255;
            tile.Alpha = 255;

            tile.move_v1 = tile.move_v2 = tile.move_v3 = tile.move_v4 = false;
        }

    for (auto &gfx : TileGfx)
        gfx.itsTexIdx = -1;

    // Texturkoordinaten für das Wasser vorberechnen
    for (int i = 0; i < 9; i++) {
        float const w = 128.0f / 8.0f * static_cast<float>(i) / 128.0f;
        WasserU[i] = w;
        WasserV[i] = w;
    }

    WasserfallOffset = 0.0f;

    // Tile Ausschnitte vorberechnen
    //
    for (int i = 0; i < MAX_TILERECTS; i++) {
        TileRects[i].top = (i / 12) * ORIGINAL_TILE_SIZE_X;
        TileRects[i].left = (i % 12) * ORIGINAL_TILE_SIZE_Y;
        TileRects[i].right = TileRects[i].left + ORIGINAL_TILE_SIZE_X;
        TileRects[i].bottom = TileRects[i].top + ORIGINAL_TILE_SIZE_Y;
    }
}

// --------------------------------------------------------------------------------------
// Destruktor
// --------------------------------------------------------------------------------------

TileEngineClass::~TileEngineClass() {}

void TileEngineClass::LoadSprites() {
    // Wasserfall Textur laden
    Wasserfall[0].LoadImage("wasserfall.png", 60, 240, 60, 240, 1, 1);
    Wasserfall[1].LoadImage("wasserfall2.png", 640, 480, 640, 480, 1, 1);

    LiquidGfx[0].LoadImage("water.png", 128, 128, 128, 128, 1, 1);
    LiquidGfx[1].LoadImage("water2.png", 128, 128, 128, 128, 1, 1);

    // GameOver Schriftzug laden
    //GameOver.LoadImage("gameover.png", 400, 90, 400, 90, 1, 1);

    // Shatten für das Alien Level laden
    Shadow.LoadImage("shadow.png", 512, 512, 512, 512, 1, 1);
}

void TileEngineClass::Zoom(float times) {
    float OriginalScale = Scale;

    Scale *= times;

    CalcRenderRange();

    // Cap the zoom
    if (times < 1.0f && LEVELPIXELSIZE_X <= DirectGraphics.RenderWidth
      ||times < 1.0f && LEVELPIXELSIZE_Y <= DirectGraphics.RenderHeight) {
        Scale = OriginalScale;
        CalcRenderRange();
        return;
    }

    Scale = OriginalScale;

    // Center screen again
    const float aspect = (Scale * times) / Scale;
    const float screenaspectx = (DirectGraphics.RenderWidth * aspect) - DirectGraphics.RenderWidth;
    const float screenaspecty = (DirectGraphics.RenderHeight * aspect) - DirectGraphics.RenderHeight;
    XOffset = XOffset * aspect + screenaspectx / 2.0f;
    YOffset = YOffset * aspect + screenaspecty / 2.0f;

    Scale *= times;
}

// --------------------------------------------------------------------------------------
// Neues, leeres Level der Grösse xSize/ySize erstellen
// --------------------------------------------------------------------------------------

void TileEngineClass::InitNewLevel(int xSize, int ySize) {
    LEVELSIZE_X = xSize;
    LEVELSIZE_Y = ySize;

    memset(&Tiles, 0, sizeof(Tiles));

    WaterSinTable.ResetPosition();

    DrawDragon = true;

    XOffset = 0.0f;
    YOffset = 0.0f;
}

// --------------------------------------------------------------------------------------
// Level freigeben
// --------------------------------------------------------------------------------------

void TileEngineClass::ClearLevel() {}

// --------------------------------------------------------------------------------------
// Level laden
// --------------------------------------------------------------------------------------

bool TileEngineClass::LoadLevel(const std::string &Filename) {
    ObjectList.ClearObjects();

    LevelObjectStruct LoadObject;

    // Dann checken, ob sich das File im Standard Ordner befindet
    if (!fs::exists(Filename) && !fs::is_regular_file(Filename)) {
        Protokoll << "\n-> Error loading level " << Filename << "!" << std::endl;
        GameRunning = false;
        return false;
    }

    Protokoll << "\n-> Loading Level <-\n" << std::endl;

    ClearLevel();

    // File öffnen
    std::ifstream Datei(Filename, std::ifstream::binary);

    if (!Datei) {
        Protokoll << " \n-> Error loading level !" << std::endl;
        return false;
    }

    // DateiHeader auslesen
    Datei.read(reinterpret_cast<char *>(&DateiHeader), sizeof(DateiHeader));

    // und Werte übertragen
    LEVELSIZE_X = FixEndian(DateiHeader.SizeX);
    LEVELSIZE_Y = FixEndian(DateiHeader.SizeY);
    LoadedTilesets = DateiHeader.UsedTilesets;
    strcpy_s(Beschreibung, DateiHeader.Beschreibung);
    bScrollBackground = DateiHeader.ScrollBackground;

    // Benutzte Tilesets laden
    for (int i = 0; i < LoadedTilesets; i++)
        TileGfx[i].LoadImage(DateiHeader.SetNames[i], 256, 256, TileSizeX, TileSizeY, 12, 12);

    // Benutzte Hintergrundgrafiken laden

    Background.LoadImage(DateiHeader.BackgroundFile, 640, 480, 640, 480, 1, 1);
    //ParallaxLayer[0].LoadImage(DateiHeader.ParallaxAFile, 640, 480, 640, 480, 1, 1);
    //ParallaxLayer[1].LoadImage(DateiHeader.ParallaxBFile, 640, 480, 640, 480, 1, 1);
    //CloudLayer.LoadImage(DateiHeader.CloudFile, 640, 240, 640, 240, 1, 1);
    //Timelimit = static_cast<float>(FixEndian(DateiHeader.Timelimit));
    DateiHeader.NumObjects = FixEndian(DateiHeader.NumObjects);

    if (Timelimit <= 0.0f)
        Timelimit = 500.0f;

    MaxSecrets = 0;
    MaxDiamonds = 0;
    MaxOneUps = 0;
    MaxBlocks = 0;

    // LevelDaten laden
    InitNewLevel(LEVELSIZE_X, LEVELSIZE_Y);

    LevelTileLoadStruct LoadTile;

    for (int i = 0; i < LEVELSIZE_X; i++)
        for (int j = 0; j < LEVELSIZE_Y; j++) {
            Datei.read(reinterpret_cast<char *>(&LoadTile), sizeof(LoadTile));

            if (LoadTile.TileSetBack > LoadedTilesets)
                LoadTile.TileSetBack = LoadedTilesets;
            if (LoadTile.TileSetFront > LoadedTilesets)
                LoadTile.TileSetFront = LoadedTilesets;

            LevelTileStruct& tile = TileAt(i, j);

            // Geladenes Leveltile übernehmen
            //
            tile.Alpha = LoadTile.Alpha;
            tile.BackArt = LoadTile.BackArt;
            tile.Block = FixEndian(LoadTile.Block);
            tile.Blue = LoadTile.Blue;
            tile.FrontArt = LoadTile.FrontArt;
            tile.Green = LoadTile.Green;
            tile.Red = LoadTile.Red;
            tile.TileSetBack = LoadTile.TileSetBack;
            tile.TileSetFront = LoadTile.TileSetFront;

            tile.Color[0] = D3DCOLOR_RGBA(LoadTile.Red, LoadTile.Green, LoadTile.Blue, LoadTile.Alpha);
            tile.Color[1] = D3DCOLOR_RGBA(LoadTile.Red, LoadTile.Green, LoadTile.Blue, LoadTile.Alpha);
            tile.Color[2] = D3DCOLOR_RGBA(LoadTile.Red, LoadTile.Green, LoadTile.Blue, LoadTile.Alpha);
            tile.Color[3] = D3DCOLOR_RGBA(LoadTile.Red, LoadTile.Green, LoadTile.Blue, LoadTile.Alpha);

            // Eine Flüssigkeit als Block?
            // damit man nicht immer auf alle vier möglichen Flüssigkeiten checken muss,
            // sondern nur auf BLOCKWERT_LIQUID

            if (tile.Block & BLOCKWERT_WASSER || tile.Block & BLOCKWERT_SUMPF)
                tile.Block ^= BLOCKWERT_LIQUID;
        }

    // eventuelle Schrägen ermitteln und Ecken für die Wasseranim festlegen

    for (int i = 1; i < LEVELSIZE_X - 1; i++)
        for (int j = 2; j < LEVELSIZE_Y - 1; j++) {
      #if 0
            // Schräge links hoch
            if (TileAt(i + 0, j + 0).Block & BLOCKWERT_WAND && !(TileAt(i + 1, j + 0).Block & BLOCKWERT_WAND) &&
                TileAt(i + 1, j + 1).Block & BLOCKWERT_WAND && !(TileAt(i + 0, j - 1).Block & BLOCKWERT_WAND)) {
                if (!(TileAt(i + 1, j + 0).Block & BLOCKWERT_SCHRAEGE_L))
                    TileAt(i + 1, j + 0).Block ^= BLOCKWERT_SCHRAEGE_L;
            }

            // Schräge rechts hoch
            if (TileAt(i + 0, j + 0).Block & BLOCKWERT_WAND && !(TileAt(i - 1, j + 0).Block & BLOCKWERT_WAND) &&
                TileAt(i - 1, j + 1).Block & BLOCKWERT_WAND && !(TileAt(i + 0, j - 1).Block & BLOCKWERT_WAND)) {
                if (!(TileAt(i - 1, j + 0).Block & BLOCKWERT_SCHRAEGE_R))
                    TileAt(i - 1, j + 0).Block ^= BLOCKWERT_SCHRAEGE_R;
            }
      #endif

            // Wasseranim
            //
            uint32_t bl = TileAt(i - 1, j + 0).Block;
            uint32_t br = TileAt(i + 1, j + 0).Block;
            uint32_t bo = TileAt(i + 0, j - 1).Block;
            uint32_t bu = TileAt(i + 0, j + 1).Block;

            LevelTileStruct& tile = TileAt(i, j);

            if (!(TileAt(i - 1, j - 1).Block & BLOCKWERT_WAND) && !(TileAt(i, j - 1).Block & BLOCKWERT_WASSERFALL) &&
                !(TileAt(i - 1, j - 1).Block & BLOCKWERT_WASSERFALL) &&
                (bl & BLOCKWERT_LIQUID && (!(bo & BLOCKWERT_WAND))))
                tile.move_v1 = true;
            else
                tile.move_v1 = false;

            if (!(TileAt(i - 1, j + 1).Block & BLOCKWERT_WAND) && (bl & BLOCKWERT_LIQUID && bu & BLOCKWERT_LIQUID))
                tile.move_v3 = true;
            else
                tile.move_v3 = false;

            if (!(TileAt(i + 1, j - 1).Block & BLOCKWERT_WAND) && !(TileAt(i, j - 1).Block & BLOCKWERT_WASSERFALL) &&
                !(TileAt(i + 1, j - 1).Block & BLOCKWERT_WASSERFALL) &&
                (br & BLOCKWERT_LIQUID && (!(bo & BLOCKWERT_WAND))))
                tile.move_v2 = true;
            else
                tile.move_v2 = false;

            if (!(TileAt(i + 1, j + 1).Block & BLOCKWERT_WAND) && (br & BLOCKWERT_LIQUID && bu & BLOCKWERT_LIQUID))
                tile.move_v4 = true;
            else
                tile.move_v4 = false;
        }

    // Objekt Daten laden und gleich Liste mit Objekten erstellen
    for (int i = 0; i < static_cast<int>(DateiHeader.NumObjects); i++) {
        Datei.read(reinterpret_cast<char *>(&LoadObject), sizeof(LoadObject));  // Objekt laden

        LoadObject.ObjectID = FixEndian(LoadObject.ObjectID);
        LoadObject.XPos = FixEndian(LoadObject.XPos);
        LoadObject.YPos = FixEndian(LoadObject.YPos);
        LoadObject.Value1 = FixEndian(LoadObject.Value1);
        LoadObject.Value2 = FixEndian(LoadObject.Value2);

        // Gegner und andere Objekte laden und ins Level setzen
        switch (LoadObject.ObjectID) {
            // Count Secrets, OneUps etc., for Summary-Box
            case SECRET:
                MaxSecrets++;
                break;
            case DIAMANT:
                MaxDiamonds++;
                break;
            case ONEUP:
                MaxOneUps++;
                break;
            case POWERBLOCK:
                MaxBlocks++;
                break;

            default:
                break;
        }

        // Startposition des Spielers
        if (LoadObject.ObjectID == 0) {
            XOffset = static_cast<float>(LoadObject.XPos) - static_cast<float>(DirectGraphics.RenderWidth) / 2;
            YOffset = static_cast<float>(LoadObject.YPos) - static_cast<float>(DirectGraphics.RenderHeight) / 2;
        } 

        // Gegner laden, wenn er nicht schon geladen wurde
        ObjectList.LoadObjectGraphic(LoadObject.ObjectID);
        Object object {
              LoadObject.ObjectID,
              LoadObject.XPos,
              LoadObject.YPos,
              LoadObject.ChangeLight,
              LoadObject.Skill,
              LoadObject.Value1,
              LoadObject.Value2,
        };
        ObjectList.PushObject(object);
    }

    Datei.read(reinterpret_cast<char *>(&DateiAppendix), sizeof(DateiAppendix));

    DateiAppendix.UsedPowerblock = FixEndian(DateiAppendix.UsedPowerblock);

    bDrawShadow = DateiAppendix.Taschenlampe;
    ShadowAlpha = 255.0f;

    // Datei schliessen
    Datei.close();

    // Temp Datei löschen und speicher freigeben
    fs::remove(fs::path("temp.map"));

    // Liquid Farben setzen
    ColR1 = std::stoi(std::string(&DateiAppendix.Col1[0], 2), nullptr, 16);
    ColG1 = std::stoi(std::string(&DateiAppendix.Col1[2], 2), nullptr, 16);
    ColB1 = std::stoi(std::string(&DateiAppendix.Col1[4], 2), nullptr, 16);

    ColR2 = std::stoi(std::string(&DateiAppendix.Col2[0], 2), nullptr, 16);
    ColG2 = std::stoi(std::string(&DateiAppendix.Col2[2], 2), nullptr, 16);
    ColB2 = std::stoi(std::string(&DateiAppendix.Col2[4], 2), nullptr, 16);
    
    ColA1 = std::stoi(std::string(&DateiAppendix.Col1[6], 2), nullptr, 16);
    ColA2 = std::stoi(std::string(&DateiAppendix.Col2[6], 2), nullptr, 16);

    Col1 = D3DCOLOR_RGBA(ColR1, ColG1, ColB1, ColA1);
    Col2 = D3DCOLOR_RGBA(ColR2, ColG2, ColB2, ColA2);

    ColR3 = ColR1 + ColR2 + 32;
    if (ColR3 > 255)
        ColR3 = 255;
    ColG3 = ColG1 + ColG2 + 32;
    if (ColG3 > 255)
        ColG3 = 255;
    ColB3 = ColB1 + ColB2 + 32;
    if (ColB3 > 255)
        ColB3 = 255;
    int ColA3 = ColA1 + ColA2;
    if (ColA3 > 255)
        ColA3 = 255;

    Col3 = D3DCOLOR_RGBA(ColR3, ColG3, ColB3, ColA3);

    ComputeCoolLight();

    // Level korrekt geladen
    Protokoll << "-> Load Level : " << Filename << " successful ! <-\n" << std::endl;

    return true;
}


void TileEngineClass::SaveLevel(const std::string &Filename) {
    // File öffnen
    std::ofstream Datei(Filename, std::ofstream::binary);

    Datei.write(reinterpret_cast<char *>(&DateiHeader), sizeof(DateiHeader));

    LevelTileLoadStruct SaveTile;
    for(int i = 0; i < LEVELSIZE_X; i++) {
        for(int j = 0; j < LEVELSIZE_Y; j++) {
          //if (tile.Block & BLOCKWERT_WASSER || tile.Block & BLOCKWERT_SUMPF)
              //tile.Block ^= BLOCKWERT_LIQUID;

            SaveTile.TileSetBack = Tiles[i][j].TileSetBack;
            SaveTile.TileSetFront = Tiles[i][j].TileSetFront;
            SaveTile.BackArt = Tiles[i][j].BackArt;
            SaveTile.FrontArt = Tiles[i][j].FrontArt;
            SaveTile.Red = Tiles[i][j].Red;
            SaveTile.Green = Tiles[i][j].Green;
            SaveTile.Blue = Tiles[i][j].Blue;
            SaveTile.Alpha = Tiles[i][j].Alpha;
            SaveTile.Block = Tiles[i][j].Block & ~BLOCKWERT_LIQUID;

            Datei.write(reinterpret_cast<char *>(&SaveTile), sizeof(SaveTile));
        }
    }

    LevelObjectStruct SaveObject;
    for (auto& object: ObjectList.Objects) {
        if (object.ObjectID == NULLENEMY) {
            break;
        }

        SaveObject.ObjectID = object.ObjectID;
        SaveObject.XPos = object.XPos;
        SaveObject.YPos = object.YPos;
        SaveObject.ChangeLight = object.ChangeLight;
        SaveObject.Skill = object.Skill;
        SaveObject.Value1 = object.Value1;
        SaveObject.Value2 = object.Value2;

        SaveObject.PADDING_CHUNK_1[0] = 0xcc;
        SaveObject.PADDING_CHUNK_1[1] = 0xcc;

        Datei.write(reinterpret_cast<char *>(&SaveObject), sizeof(SaveObject));
    }

    DateiAppendix.Taschenlampe = bDrawShadow;
    DateiAppendix.UsedPowerblock = FixEndian(DateiAppendix.UsedPowerblock);


    memset(DateiAppendix.Col1, 0, 8);
    memset(DateiAppendix.Col2, 0, 8);

    auto ColToString = [](int col) {
        std::ostringstream TmpString;
        TmpString << std::setfill('0') << std::setw(2) << std::hex << col;
        auto str = TmpString.str();
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    };

    std::memcpy(&DateiAppendix.Col1[0], ColToString(ColR1).c_str(), 2);
    std::memcpy(&DateiAppendix.Col1[2], ColToString(ColG1).c_str(), 2);
    std::memcpy(&DateiAppendix.Col1[4], ColToString(ColB1).c_str(), 2);

    std::memcpy(&DateiAppendix.Col2[0], ColToString(ColR2).c_str(), 2);
    std::memcpy(&DateiAppendix.Col2[2], ColToString(ColG2).c_str(), 2);
    std::memcpy(&DateiAppendix.Col2[4], ColToString(ColB2).c_str(), 2);

    std::memcpy(&DateiAppendix.Col1[6], ColToString(ColA1).c_str(), 2);
    std::memcpy(&DateiAppendix.Col2[6], ColToString(ColA2).c_str(), 2);

    Datei.write(reinterpret_cast<char *>(&DateiAppendix), sizeof(DateiAppendix));

    Datei.close();
}

// --------------------------------------------------------------------------------------
// Ausrechnen, welcher Levelausschnitt gerendert werden soll
// --------------------------------------------------------------------------------------

void TileEngineClass::CalcRenderRange() {
    TileSizeX = ORIGINAL_TILE_SIZE_X * Scale;
    TileSizeY = ORIGINAL_TILE_SIZE_Y * Scale;

    ScreenSizeTilesX = DirectGraphics.RenderWidth / TileSizeX;
    ScreenSizeTilesY = DirectGraphics.RenderHeight / TileSizeY;

    LEVELPIXELSIZE_X = LEVELSIZE_X * TileSizeX;
    LEVELPIXELSIZE_Y = LEVELSIZE_Y * TileSizeY;

    if (XOffset < 0)
        XOffset = 0;
    if (YOffset < 0)
        YOffset = 0;
    if (XOffset + DirectGraphics.RenderWidth >= LEVELPIXELSIZE_X - TileSizeX)
        XOffset = LEVELPIXELSIZE_X - DirectGraphics.RenderWidth - TileSizeX;
    if (YOffset + DirectGraphics.RenderHeight >= LEVELPIXELSIZE_Y - TileSizeY)
        YOffset = LEVELPIXELSIZE_Y - DirectGraphics.RenderHeight - TileSizeY;

    // Ausschnittgröße berechnen
    //
    int xo = static_cast<int>(XOffset * (1.0f / TileSizeX));
    int yo = static_cast<int>(YOffset * (1.0f / TileSizeY));

    if (xo < 0)
        xo = 0;
    if (yo < 0)
        yo = 0;

    RenderPosX = -1;
    RenderPosY = -1;

    if (RenderPosX + xo < 0)
        RenderPosX = 0;

    if (RenderPosY + yo < 0)
        RenderPosY = 0;

    RenderPosXTo = ScreenSizeTilesX + 2;
    RenderPosYTo = ScreenSizeTilesY + 2;

    if (xo + RenderPosXTo > LEVELSIZE_X)
        RenderPosXTo = ScreenSizeTilesX;
    if (yo + RenderPosYTo > LEVELSIZE_Y)
        RenderPosYTo = ScreenSizeTilesY;

    if (xo + RenderPosXTo > LEVELSIZE_X)
        RenderPosXTo = LEVELSIZE_X - xo;
    if (yo + RenderPosYTo > LEVELSIZE_Y)
        RenderPosYTo = LEVELSIZE_Y - yo;

    // Sonstige Ausgangswerte berechnen
    xLevel = xo;
    yLevel = yo;

    // Offsets der Tiles berechnen (0-19)
    xTileOffs = fmod(XOffset, TileSizeX);
    yTileOffs = fmod(YOffset, TileSizeY);

    WaterSinTable.UpdateTableIndexes(xLevel, yLevel);
}

// --------------------------------------------------------------------------------------
// Hintergrund Parallax Layer anzeigen
// --------------------------------------------------------------------------------------

void TileEngineClass::DrawBackground() {
    #if 0

    // Hintergrund nicht rotieren
    //
    glm::mat4x4 matView = glm::mat4x4(1.0f);
    g_matView = matView;

    //----- Hintergrund-Bild

    if (bScrollBackground)  // Hintergrundbild mitscrollen
    {
        int const x_off = static_cast<int>(XOffset / 5.0f) % RENDERWIDTH;
        // Linke Hälfte
        Background.SetRect(0, 0, x_off, RENDERHEIGHT);
        Background.RenderSprite(static_cast<float>(RENDERWIDTH - x_off), 0.0f, 0xFFFFFFFF);

        // Rechte Hälfte
        Background.SetRect(x_off, 0, RENDERWIDTH, RENDERHEIGHT);
        Background.RenderSprite(0.0f, 0.0f, 0xFFFFFFFF);
    } else  // oder statisch ?
    {
        Background.SetRect(0, 0, RENDERWIDTH, RENDERHEIGHT);
        Background.RenderSprite(0.0f, 0.0f, 0xFFFFFFFF);
    }

    int xoff;
    float yoff;

    //----- Layer ganz hinten (ausser im Flugsack Level)

    xoff = static_cast<int>(XOffset / 3.0f) % RENDERWIDTH;
    yoff = static_cast<float>((LEVELSIZE_Y - ScreenSizeTiles_Y) * TILESIZE_Y);  // Grösse des Levels in Pixeln (-1 Screen)
    yoff = 220.0f - 150.0f / yoff * YOffset;                               // y-Offset des Layers berechnen
    yoff -= 40.0f;

    // Linke Hälfte
    ParallaxLayer[0].SetRect(0, 0, xoff, RENDERHEIGHT);
    ParallaxLayer[0].RenderSprite(static_cast<float>(RENDERWIDTH - xoff), yoff, 0xFFFFFFFF);

    // Rechte Hälfte
    ParallaxLayer[0].SetRect(xoff, 0, RENDERWIDTH, RENDERHEIGHT);
    ParallaxLayer[0].RenderSprite(0.0f, yoff, 0xFFFFFFFF);

    //----- vorletzter Layer

    yoff = static_cast<float>((LEVELSIZE_Y - ScreenSizeTiles_Y) * TILESIZE_Y);  // Grösse des Levels in Pixeln (-1 Screen)
    yoff = 200.0f - 200.0f / yoff * YOffset;                               // y-Offset des Layers berechnen
    xoff = static_cast<int>(XOffset / 2.0f) % RENDERWIDTH;

    // Linke Hälfte
    ParallaxLayer[1].SetRect(0, 0, xoff, RENDERHEIGHT);
    ParallaxLayer[1].RenderSprite(static_cast<float>(RENDERWIDTH - xoff), yoff, 0xFFFFFFFF);

    // Rechte Hälfte
    ParallaxLayer[1].SetRect(xoff, 0, RENDERWIDTH, RENDERHEIGHT);
    ParallaxLayer[1].RenderSprite(0.0f, yoff, 0xFFFFFFFF);

    //----- Im Fahrstuhl-Level noch den vertikalen Parallax-Layer anzeigen

    if (IsElevatorLevel) {
        int const y_off = static_cast<int>(YOffset / 1.5f) % RENDERHEIGHT;

        // Obere Hälfte
        ParallaxLayer[2].SetRect(0, 0, RENDERWIDTH, y_off);
        ParallaxLayer[2].RenderSprite(390.0f - XOffset, static_cast<float>(RENDERHEIGHT - y_off), 0xFFFFFFFF);

        // Untere Hälfte
        ParallaxLayer[2].SetRect(0, y_off, RENDERWIDTH, RENDERHEIGHT);
        ParallaxLayer[2].RenderSprite(390.0f - XOffset, 0.0f, 0xFFFFFFFF);
    }

    //----- Wolken Layer (Wenn Focus des Level GANZ oben, dann wird er GANZ angezeigt)

    // Wolken bewegen
    CloudMovement += Timer.getSpeedFactor();
    if (CloudMovement > RENDERWIDTH)
        CloudMovement = 0.0f;

    DirectGraphics.SetAdditiveMode();

    xoff = static_cast<int>(XOffset / 4.0f + CloudMovement) % RENDERWIDTH;
    yoff = static_cast<float>((LEVELSIZE_Y - ScreenSizeTiles_Y) * 40);  // Grösse des Levels in Pixeln (-1 Screen)
    yoff = 240.0f / yoff * YOffset;               // y-Offset des Layers berechnen

    // Linke Hälfte
    CloudLayer.SetRect(0, static_cast<int>(yoff), xoff, 240);
    CloudLayer.RenderSprite(static_cast<float>(RENDERWIDTH - xoff), 0.0f, 0xFFFFFFFF);

    // Rechte Hälfte
    CloudLayer.SetRect(xoff, static_cast<int>(yoff), RENDERWIDTH, 240);
    CloudLayer.RenderSprite(0.0f, 0.0f, 0xFFFFFFFF);

    DirectGraphics.SetColorKeyMode();
#endif

    // Hintergrund nicht rotieren
    glm::mat4x4 matView = glm::mat4x4(1.0f);
    g_matView = matView;

    //----- Hintergrund-Bild

    Background.SetRect(0, 0, RENDERWIDTH, RENDERHEIGHT);
    Background.RenderSpriteScaled(0.0f, 0.0f, DirectGraphics.RenderWidth, DirectGraphics.RenderHeight, 0xFFFFFFFF);

    DirectGraphics.SetColorKeyMode();
}

// --------------------------------------------------------------------------------------
// Level Hintergrund anzeigen
//
// Das Level wird Tile für Tile durchgegangen das Vertex-Array solange mit
// Vertices gefüllt, die das selbe Tileset verwenden, bis ein anderes Tileset
// gesetzt werden muss, worauf alle Vetices im Array mit der alten Textur gerendert
// werden und alles mit der neuen Textur von vorne wieder losgeht, bis alle Tiles
// durchgenommen wurden.

// So funzt das auch bei FrontLevel und (vielleicht bald) Overlay
//
// Hier werden alle Tiles im Back-Layer gesetzt, die KEINE Wand sind,
// da die Wände später gesetzt werden, da sie alles verdecken, was in sie reinragt
// --------------------------------------------------------------------------------------

void TileEngineClass::DrawBackLevel() {
    // Am Anfang noch keine Textur gewählt
    int ActualTexture = -1;

    // x und ypos am screen errechnen
    xScreen = static_cast<float>(-xTileOffs + RenderPosX * TileSizeX);
    yScreen = static_cast<float>(-yTileOffs + RenderPosY * TileSizeY);

    DirectGraphics.SetColorKeyMode();

    // Noch keine Tiles zum rendern
    int NumToRender = 0;

    for (int j = RenderPosY; j < RenderPosYTo; j++) {
        xScreen = static_cast<float>(-xTileOffs) + RenderPosX * TileSizeX;

        for (int i = RenderPosX; i < RenderPosXTo; i++) {
            const LevelTileStruct& tile = TileAt(xLevel + i, yLevel + j);

            if (tile.BackArt > 0 &&  // Überhaupt ein Tile drin ?
                (!(tile.Block & BLOCKWERT_WAND) ||
                 (tile.FrontArt > 0 &&
                  tile.Block & BLOCKWERT_VERDECKEN))) {
                // Neue Textur ?
                if (tile.TileSetBack != ActualTexture) {
                    // Aktuelle Textur sichern
                    ActualTexture = tile.TileSetBack;

                    // Tiles zeichnen
                    if (NumToRender > 0)
                        DirectGraphics.RendertoBuffer(GL_TRIANGLES, NumToRender * 2, &TilesToRender[0]);

                    // Neue aktuelle Textur setzen
                    DirectGraphics.SetTexture(TileGfx[ActualTexture].itsTexIdx);

                    // Und beim rendern wieder von vorne anfangen
                    NumToRender = 0;
                }

                if (NumToRender >= TilesToRenderMax) {
                    DirectGraphics.RendertoBuffer(GL_TRIANGLES, NumToRender * 2, &TilesToRender[0]);
                    NumToRender = 0;
                }

                unsigned int Type = tile.BackArt - INCLUDE_ZEROTILE;

                // Animiertes Tile ?
                if (tile.Block & BLOCKWERT_ANIMIERT_BACK)
                    Type += 36 * TileAnimPhase;

                // richtigen Ausschnitt für das aktuelle Tile setzen
                RECT_struct const Rect = TileRects[Type];

                // Screen-Koordinaten der Vertices
                float const l = xScreen;               // Links
                float const o = yScreen;               // Oben
                float const r = xScreen + TileSizeX;  // Rechts
                float const u = yScreen + TileSizeY;  // Unten

                // Textur-Koordinaten
                float const tl = Rect.left / TILESETSIZE_X;    // Links
                float const tr = Rect.right / TILESETSIZE_X;   // Rechts
                float const to = Rect.top / TILESETSIZE_Y;     // Oben
                float const tu = Rect.bottom / TILESETSIZE_Y;  // Unten

                // Vertices definieren
                v1.color = tile.Color[0];
                v2.color = tile.Color[1];
                v3.color = tile.Color[2];
                v4.color = tile.Color[3];

                v1.x = l;  // Links oben
                v1.y = o;
                v1.tu = tl;
                v1.tv = to;

                v2.x = r;  // Rechts oben
                v2.y = o;
                v2.tu = tr;
                v2.tv = to;

                v3.x = l;  // Links unten
                v3.y = u;
                v3.tu = tl;
                v3.tv = tu;

                v4.x = r;  // Rechts unten
                v4.y = u;
                v4.tu = tr;
                v4.tv = tu;

                // Hintergrund des Wasser schwabbeln lassen

                if (tile.move_v1 || tile.move_v2 ||
                    tile.move_v3 || tile.move_v4) {
                    float x_offs[2];
                    WaterSinTable.GetNonWaterSin(j, x_offs);

                    if (yLevel + j > 0 &&  // DKS Added this check to above line
                        TileAt(xLevel + i, yLevel + j - 1).Block & BLOCKWERT_LIQUID) {
                        if (tile.move_v1 == true)
                            v1.x += x_offs[0];
                        if (tile.move_v2 == true)
                            v2.x += x_offs[0];
                    }

                    if (tile.move_v3 == true)
                        v3.x += x_offs[1];
                    if (tile.move_v4 == true)
                        v4.x += x_offs[1];
                }

                // Zu rendernde Vertices ins Array schreiben
                TilesToRender[NumToRender * 6 + 0] = v1;  // Jeweils 2 Dreicke als
                TilesToRender[NumToRender * 6 + 1] = v2;  // als ein viereckiges
                TilesToRender[NumToRender * 6 + 2] = v3;  // Tile ins Array kopieren
                TilesToRender[NumToRender * 6 + 3] = v3;
                TilesToRender[NumToRender * 6 + 4] = v2;
                TilesToRender[NumToRender * 6 + 5] = v4;

                NumToRender++;  // Weiter im Vertex Array
            }
            xScreen += TileSizeX;  // Am Screen weiter
        }
        yScreen += TileSizeY;  // Am Screen weiter
    }

    if (NumToRender > 0)
        DirectGraphics.RendertoBuffer(GL_TRIANGLES, NumToRender * 2, &TilesToRender[0]);
}

// --------------------------------------------------------------------------------------
// Level Vodergrund anzeigen
// --------------------------------------------------------------------------------------

void TileEngineClass::DrawFrontLevel() {
    // Am Anfang noch keine Textur gewählt
    int ActualTexture = -1;

    // x und ypos am screen errechnen
    xScreen = static_cast<float>(-xTileOffs + RenderPosX * TileSizeX);
    yScreen = static_cast<float>(-yTileOffs + RenderPosY * TileSizeY);

    DirectGraphics.SetColorKeyMode();

    // Noch keine Tiles zum rendern
    int NumToRender = 0;

    for (int j = RenderPosY; j < RenderPosYTo; j++) {
        xScreen = static_cast<float>(-xTileOffs + RenderPosX * TileSizeX);

        for (int i = RenderPosX; i < RenderPosXTo; i++) {
            const LevelTileStruct& tile = TileAt(xLevel + i, yLevel + j);

            if (tile.FrontArt > 0 &&
                !(tile.Block & BLOCKWERT_VERDECKEN) &&
                !(tile.Block & BLOCKWERT_WAND)) {
                // Neue Textur ?
                if (tile.TileSetFront != ActualTexture) {
                    // Aktuelle Textur sichern
                    ActualTexture = tile.TileSetFront;

                    // Tiles zeichnen
                    if (NumToRender > 0)
                        DirectGraphics.RendertoBuffer(GL_TRIANGLES, NumToRender * 2, &TilesToRender[0]);

                    // Neue aktuelle Textur setzen
                    DirectGraphics.SetTexture(TileGfx[ActualTexture].itsTexIdx);

                    // Und beim rendern wieder von vorne anfangen
                    NumToRender = 0;
                }

                if (NumToRender >= TilesToRenderMax) {
                    DirectGraphics.RendertoBuffer(GL_TRIANGLES, NumToRender * 2, &TilesToRender[0]);
                    NumToRender = 0;
                }

                unsigned int Type = tile.FrontArt - INCLUDE_ZEROTILE;

                // Animiertes Tile ?
                if (tile.Block & BLOCKWERT_ANIMIERT_FRONT)
                    Type += 36 * TileAnimPhase;

                // richtigen Ausschnitt für das aktuelle Tile setzen
                RECT_struct const Rect = TileRects[Type];

                // Screen-Koordinaten der Vertices
                float const l = xScreen;               // Links
                float const o = yScreen;               // Oben
                float const r = xScreen + TileSizeX;  // Rechts
                float const u = yScreen + TileSizeY;  // Unten

                // Textur-Koordinaten
                float const tl = Rect.left / TILESETSIZE_X;    // Links
                float const tr = Rect.right / TILESETSIZE_X;   // Rechts
                float const to = Rect.top / TILESETSIZE_Y;     // Oben
                float const tu = Rect.bottom / TILESETSIZE_Y;  // Unten

                // Licht setzen (prüfen auf Overlay light, wegen hellen Kanten)
                if (tile.Block & BLOCKWERT_OVERLAY_LIGHT) {
                    v1.color = tile.Color[0];
                    v2.color = tile.Color[1];
                    v3.color = tile.Color[2];
                    v4.color = tile.Color[3];
                } else {
                    v1.color = v2.color = v3.color = v4.color =
                        D3DCOLOR_RGBA(255, 255, 255, tile.Alpha);
                }

                v1.x = l;  // Links oben
                v1.y = o;
                v1.tu = tl;
                v1.tv = to;

                v2.x = r;  // Rechts oben
                v2.y = o;
                v2.tu = tr;
                v2.tv = to;

                v3.x = l;  // Links unten
                v3.y = u;
                v3.tu = tl;
                v3.tv = tu;

                v4.x = r;  // Rechts unten
                v4.y = u;
                v4.tu = tr;
                v4.tv = tu;

                // Hintergrund des Wasser schwabbeln lassen
                if (tile.move_v1 || tile.move_v2 ||
                    tile.move_v3 || tile.move_v4) {
                    float x_offs[2];
                    WaterSinTable.GetNonWaterSin(j, x_offs);

                    if (yLevel + j > 0 &&  // DKS Added this check to above line
                        TileAt(xLevel + i, yLevel + j - 1).Block & BLOCKWERT_LIQUID) {
                        if (tile.move_v1 == true)
                            v1.x += x_offs[0];
                        if (tile.move_v2 == true)
                            v2.x += x_offs[0];
                    }

                    if (tile.move_v3 == true)
                        v3.x += x_offs[1];
                    if (tile.move_v4 == true)
                        v4.x += x_offs[1];
                }

                // Zu rendernde Vertices ins Array schreiben
                TilesToRender[NumToRender * 6 + 0] = v1;  // Jeweils 2 Dreicke als
                TilesToRender[NumToRender * 6 + 1] = v2;  // als ein viereckiges
                TilesToRender[NumToRender * 6 + 2] = v3;  // Tile ins Array kopieren
                TilesToRender[NumToRender * 6 + 3] = v3;
                TilesToRender[NumToRender * 6 + 4] = v2;
                TilesToRender[NumToRender * 6 + 5] = v4;

                NumToRender++;  // Weiter im Vertex Array
            }
            xScreen += TileSizeX;  // Am Screen weiter
        }
        yScreen += TileSizeY;  // Am Screen weiter
    }

    if (NumToRender > 0)
        DirectGraphics.RendertoBuffer(GL_TRIANGLES, NumToRender * 2, &TilesToRender[0]);
}

// --------------------------------------------------------------------------------------
// Wandstücke, die den Spieler vedecken, erneut zeichnen
// --------------------------------------------------------------------------------------

void TileEngineClass::DrawBackLevelOverlay() {

    // Am Anfang noch keine Textur gewählt
    int ActualTexture = -1;

    // x und ypos am screen errechnen
    xScreen = static_cast<float>(-xTileOffs + RenderPosX * TileSizeX);
    yScreen = static_cast<float>(-yTileOffs + RenderPosY * TileSizeY);

    DirectGraphics.SetColorKeyMode();

    // Noch keine Tiles zum rendern
    int NumToRender = 0;

    for (int j = RenderPosY; j < RenderPosYTo; j++) {
        xScreen = static_cast<float>(-xTileOffs + RenderPosX * TileSizeX);

        for (int i = RenderPosX; i < RenderPosXTo; i++) {
            const LevelTileStruct& tile = TileAt(xLevel + i, yLevel + j);

            // Hintergrundtile nochmal neu setzen?
            //
            if (tile.BackArt > 0 && tile.Block & BLOCKWERT_WAND &&
                (!(tile.FrontArt > 0 &&
                   tile.Block & BLOCKWERT_VERDECKEN))) {
                // Neue Textur ?
                if (tile.TileSetBack != ActualTexture) {
                    // Aktuelle Textur sichern
                    ActualTexture = tile.TileSetBack;

                    // Tiles zeichnen
                    if (NumToRender > 0)
                        DirectGraphics.RendertoBuffer(GL_TRIANGLES, NumToRender * 2, &TilesToRender[0]);

                    // Neue aktuelle Textur setzen
                    DirectGraphics.SetTexture(TileGfx[ActualTexture].itsTexIdx);

                    // Und beim rendern wieder von vorne anfangen
                    NumToRender = 0;
                }

                if (NumToRender >= TilesToRenderMax) {
                    DirectGraphics.RendertoBuffer(GL_TRIANGLES, NumToRender * 2, &TilesToRender[0]);
                    NumToRender = 0;
                }

                unsigned int Type = tile.BackArt - INCLUDE_ZEROTILE;

                // Animiertes Tile ?
                if (tile.Block & BLOCKWERT_ANIMIERT_BACK)
                    Type += 36 * TileAnimPhase;

                // richtigen Ausschnitt für das aktuelle Tile setzen
                RECT_struct const Rect = TileRects[Type];

                // Screen-Koordinaten der Vertices
                float const l = xScreen;               // Links
                float const o = yScreen;               // Oben
                float const r = xScreen + TileSizeX;  // Rechts
                float const u = yScreen + TileSizeY;  // Unten

                // Textur-Koordinaten
                float const tl = Rect.left / TILESETSIZE_X;    // Links
                float const tr = Rect.right / TILESETSIZE_X;   // Rechts
                float const to = Rect.top / TILESETSIZE_Y;     // Oben
                float const tu = Rect.bottom / TILESETSIZE_Y;  // Unten

                v1.color = tile.Color[0];
                v2.color = tile.Color[1];
                v3.color = tile.Color[2];
                v4.color = tile.Color[3];

                v1.x = l;  // Links oben
                v1.y = o;
                v1.tu = tl;
                v1.tv = to;

                v2.x = r;  // Rechts oben
                v2.y = o;
                v2.tu = tr;
                v2.tv = to;

                v3.x = l;  // Links unten
                v3.y = u;
                v3.tu = tl;
                v3.tv = tu;

                v4.x = r;  // Rechts unten
                v4.y = u;
                v4.tu = tr;
                v4.tv = tu;

                // Zu rendernde Vertices ins Array schreiben
                TilesToRender[NumToRender * 6 + 0] = v1;  // Jeweils 2 Dreicke als
                TilesToRender[NumToRender * 6 + 1] = v2;  // als ein viereckiges
                TilesToRender[NumToRender * 6 + 2] = v3;  // Tile ins Array kopieren
                TilesToRender[NumToRender * 6 + 3] = v3;
                TilesToRender[NumToRender * 6 + 4] = v2;
                TilesToRender[NumToRender * 6 + 5] = v4;

                NumToRender++;  // Weiter im Vertex Array
            }
            xScreen += TileSizeX;  // Am Screen weiter
        }
        yScreen += TileSizeY;  // Am Screen weiter
    }

    if (NumToRender > 0)
        DirectGraphics.RendertoBuffer(GL_TRIANGLES, NumToRender * 2, &TilesToRender[0]);
}

// --------------------------------------------------------------------------------------
// Die Levelstücke zeigen, die den Spieler verdecken
// --------------------------------------------------------------------------------------

void TileEngineClass::DrawOverlayLevel() {

    // Am Anfang noch keine Textur gewählt
    int ActualTexture = -1;

    // x und ypos am screen errechnen
    xScreen = static_cast<float>(-xTileOffs + RenderPosX * TileSizeX);
    yScreen = static_cast<float>(-yTileOffs + RenderPosY * TileSizeY);

    DirectGraphics.SetColorKeyMode();

    // Noch keine Tiles zum rendern
    int NumToRender = 0;

    for (int j = RenderPosY; j < RenderPosYTo; j++) {
        xScreen = static_cast<float>(-xTileOffs + RenderPosX * TileSizeX);

        for (int i = RenderPosX; i < RenderPosXTo; i++) {
            const LevelTileStruct& tile = TileAt(xLevel + i, yLevel + j);

            // Vordergrund Tiles setzen, um Spieler zu verdecken
            if ((tile.FrontArt > 0 &&
                 (tile.Block & BLOCKWERT_VERDECKEN ||
                  tile.Block & BLOCKWERT_WAND)) ||
                tile.Block & BLOCKWERT_WASSERFALL ||
                tile.Block & BLOCKWERT_MOVELINKS ||
                tile.Block & BLOCKWERT_MOVERECHTS ||
                tile.Block & BLOCKWERT_MOVEVERTICAL) {
                // Screen-Koordinaten der Vertices
                float const l = xScreen;               // Links
                float const o = yScreen;               // Oben
                float const r = xScreen + TileSizeX;  // Rechts
                float const u = yScreen + TileSizeY;  // Unten
                
                if ((tile.FrontArt > 0 &&
                     (tile.Block & BLOCKWERT_VERDECKEN ||
                      tile.Block & BLOCKWERT_MOVEVERTICAL ||
                      tile.Block & BLOCKWERT_MOVELINKS ||
                      tile.Block & BLOCKWERT_MOVERECHTS ||
                      tile.Block & BLOCKWERT_WAND))) {
                    // Neue Textur ?
                    if (tile.TileSetFront != ActualTexture) {
                        // Aktuelle Textur sichern
                        ActualTexture = tile.TileSetFront;

                        // Tiles zeichnen
                        if (NumToRender > 0)
                            DirectGraphics.RendertoBuffer(GL_TRIANGLES, NumToRender * 2, &TilesToRender[0]);

                        // Neue aktuelle Textur setzen
                        DirectGraphics.SetTexture(TileGfx[ActualTexture].itsTexIdx);

                        // Und beim rendern wieder von vorne anfangen
                        NumToRender = 0;
                    }

                    if (NumToRender >= TilesToRenderMax) {
                        DirectGraphics.RendertoBuffer(GL_TRIANGLES, NumToRender * 2, &TilesToRender[0]);
                        NumToRender = 0;
                    }

                    // "normales" Overlay Tile setzen
                    unsigned int Type = tile.FrontArt - INCLUDE_ZEROTILE;

                    // Animiertes Tile ?
                    if (tile.Block & BLOCKWERT_ANIMIERT_FRONT)
                        Type += 36 * TileAnimPhase;

                    // richtigen Ausschnitt für das aktuelle Tile setzen
                    RECT_struct const Rect = TileRects[Type];

                    // Textur-Koordinaten
                    float tl = Rect.left / TILESETSIZE_X;    // Links
                    float tr = Rect.right / TILESETSIZE_X;   // Rechts
                    float to = Rect.top / TILESETSIZE_Y;     // Oben
                    float tu = Rect.bottom / TILESETSIZE_Y;  // Unten

                    // bewegtes Tile vertikal
                    if (tile.Block & BLOCKWERT_MOVEVERTICAL) {
                        to -= 60.0f / 256.0f * WasserfallOffset / 120.0f;
                        tu -= 60.0f / 256.0f * WasserfallOffset / 120.0f;
                    }

                    // bewegtes Tile links
                    if (tile.Block & BLOCKWERT_MOVELINKS) {
                        tl += 60.0f / 256.0f * WasserfallOffset / 120.0f;
                        tr += 60.0f / 256.0f * WasserfallOffset / 120.0f;
                    }

                    // bewegtes Tile rechts
                    if (tile.Block & BLOCKWERT_MOVERECHTS) {
                        tl -= 60.0f / 256.0f * WasserfallOffset / 120.0f;
                        tr -= 60.0f / 256.0f * WasserfallOffset / 120.0f;
                    }

                    // al = tile.Alpha;

                    if (tile.Block & BLOCKWERT_OVERLAY_LIGHT) {
                        v1.color = tile.Color[0];
                        v2.color = tile.Color[1];
                        v3.color = tile.Color[2];
                        v4.color = tile.Color[3];
                    } else {
                        v1.color = v2.color = v3.color = v4.color =
                            D3DCOLOR_RGBA(255, 255, 255, tile.Alpha);
                    }

                    v1.x = l;  // Links oben
                    v1.y = o;
                    v1.tu = tl;
                    v1.tv = to;

                    v2.x = r;  // Rechts oben
                    v2.y = o;
                    v2.tu = tr;
                    v2.tv = to;

                    v3.x = l;  // Links unten
                    v3.y = u;
                    v3.tu = tl;
                    v3.tv = tu;

                    v4.x = r;  // Rechts unten
                    v4.y = u;
                    v4.tu = tr;
                    v4.tv = tu;

                    // Zu rendernde Vertices ins Array schreiben
                    TilesToRender[NumToRender * 6 + 0] = v1;  // Jeweils 2 Dreicke als
                    TilesToRender[NumToRender * 6 + 1] = v2;  // als ein viereckiges
                    TilesToRender[NumToRender * 6 + 2] = v3;  // Tile ins Array kopieren
                    TilesToRender[NumToRender * 6 + 3] = v3;
                    TilesToRender[NumToRender * 6 + 4] = v2;
                    TilesToRender[NumToRender * 6 + 5] = v4;

                    NumToRender++;  // Weiter im Vertex Array
                }
            }

            xScreen += TileSizeX;  // Am Screen weiter
        }

        yScreen += TileSizeY;  // Am Screen weiter
    }

    if (NumToRender > 0)
        DirectGraphics.RendertoBuffer(GL_TRIANGLES, NumToRender * 2, &TilesToRender[0]);
}

// --------------------------------------------------------------------------------------
// Die Levelstücke zeigen, die den Spieler verdecken
// --------------------------------------------------------------------------------------

void TileEngineClass::DrawWater() {
    // x und ypos am screen errechnen
    xScreen = static_cast<float>(-xTileOffs + RenderPosX * TileSizeX);
    yScreen = static_cast<float>(-yTileOffs + RenderPosY * TileSizeY);

    // Noch keine Tiles zum rendern
    int NumToRender = 0;

    DirectGraphics.SetFilterMode(true);
    DirectGraphics.SetColorKeyMode();

    // zwei Schichten Wasser rendern
    for (int schicht = 0; schicht < 2; schicht++) {
        // Offsets der Tiles berechnen (0-19)
        xTileOffs = fmod(XOffset, TileSizeX);
        yTileOffs = fmod(YOffset, TileSizeY);

        // ypos am screen errechnen
        yScreen = static_cast<float>(-yTileOffs + RenderPosY * TileSizeY);

        for (int j = RenderPosY; j < RenderPosYTo; j++) {
            xScreen = static_cast<float>(-xTileOffs + RenderPosX * TileSizeX);

            for (int i = RenderPosX; i < RenderPosXTo; i++) {
                const LevelTileStruct& tile = TileAt(xLevel + i, yLevel + j);

                if (NumToRender >= TilesToRenderMax) {
                    DirectGraphics.RendertoBuffer(GL_TRIANGLES, NumToRender * 2, &TilesToRender[0]);
                    NumToRender = 0;
                }

                // Vordergrund Tiles setzen um Spieler zu verdecken
                if (tile.Block & BLOCKWERT_LIQUID) {
                    // Screen-Koordinaten der Vertices
                    float const l = xScreen;               // Links
                    float const o = yScreen;               // Oben
                    float const r = xScreen + TileSizeX;  // Rechts
                    float const u = yScreen + TileSizeY;  // Unten

                    // Vertices definieren
                    v1.x = l;  // Links oben
                    v1.y = o;
                    v2.x = r;  // Rechts oben
                    v2.y = o;
                    v3.x = l;  // Links unten
                    v3.y = u;
                    v4.x = r;  // Rechts unten
                    v4.y = u;

                    if (schicht == 0) {
                        v1.color = v2.color = v3.color = v4.color = Col1;
                    } else {
                        v1.color = v2.color = v3.color = v4.color = Col2;
                    }

                    // Oberfläche des Wassers aufhellen
                    if (yLevel + j - 1 >= 0 && !(TileAt(xLevel + i, yLevel + j - 1).Block & BLOCKWERT_LIQUID) &&
                        !(TileAt(xLevel + i, yLevel + j - 1).Block & BLOCKWERT_WASSERFALL)) {
                        if (xLevel + i - 1 >= 0 &&
                            !(TileAt(xLevel + i - 1, yLevel + j - 1).Block & BLOCKWERT_LIQUID) &&
                            !(TileAt(xLevel + i - 1, yLevel + j - 1).Block & BLOCKWERT_WASSERFALL))
                            v1.color = Col3;

                        if (xLevel + i + 1 < LEVELSIZE_X &&
                            !(TileAt(xLevel + i + 1, yLevel + j - 1).Block & BLOCKWERT_LIQUID) &&
                            !(TileAt(xLevel + i + 1, yLevel + j - 1).Block & BLOCKWERT_WASSERFALL))
                            v2.color = Col3;
                    }

                    int const xo = (i + xLevel) % 8;
                    int const yo = (j + yLevel) % 8;

                    // Schicht 0 == langsam, gespiegelt
                    if (schicht == 0) {
                        v1.tu = WasserU[8 - xo];
                        v1.tv = WasserV[8 - yo];
                        v2.tu = WasserU[8 - xo - 1];
                        v2.tv = WasserV[8 - yo];
                        v3.tu = WasserU[8 - xo];
                        v3.tv = WasserV[8 - yo - 1];
                        v4.tu = WasserU[8 - xo - 1];
                        v4.tv = WasserV[8 - yo - 1];
                    }

                    // Schicht 0 == schnell
                    else {
                        v1.tu = WasserU[xo];
                        v1.tv = WasserV[yo];
                        v2.tu = WasserU[xo + 1];
                        v2.tv = WasserV[yo];
                        v3.tu = WasserU[xo];
                        v3.tv = WasserV[yo + 1];
                        v4.tu = WasserU[xo + 1];
                        v4.tv = WasserV[yo + 1];
                    }

                    if (tile.move_v1 || tile.move_v2 ||
                        tile.move_v3 || tile.move_v4) {
                        float y_offs[2];
                        WaterSinTable.GetWaterSin(i, j, y_offs);
                        if (tile.move_v1 == true)
                            v1.y += y_offs[0];
                        if (tile.move_v2 == true)
                            v2.y += y_offs[0];
                        if (tile.move_v3 == true)
                            v3.y += y_offs[1];
                        if (tile.move_v4 == true)
                            v4.y += y_offs[1];
                    }

                    // Zu rendernde Vertices ins Array schreiben
                    TilesToRender[NumToRender * 6 + 0] = v1;  // Jeweils 2 Dreicke als
                    TilesToRender[NumToRender * 6 + 1] = v2;  // als ein viereckiges
                    TilesToRender[NumToRender * 6 + 2] = v3;  // Tile ins Array kopieren
                    TilesToRender[NumToRender * 6 + 3] = v3;
                    TilesToRender[NumToRender * 6 + 4] = v2;
                    TilesToRender[NumToRender * 6 + 5] = v4;

                    NumToRender++;  // Weiter im Vertex Array
                }

                xScreen += TileSizeX;  // Am Screen weiter
            }

            yScreen += TileSizeY;  // Am Screen weiter
        }

        if (NumToRender > 0) {
            if (schicht > 0) {
                DirectGraphics.SetAdditiveMode();
                DirectGraphics.SetTexture(LiquidGfx[1].itsTexIdx);
            } else {
                DirectGraphics.SetTexture(LiquidGfx[0].itsTexIdx);
            }

            DirectGraphics.RendertoBuffer(GL_TRIANGLES, NumToRender * 2, &TilesToRender[0]);
        }

        NumToRender = 0;
    }

    // Wasserfall rendern
    xScreen = static_cast<float>(-xTileOffs + RenderPosX * TileSizeX);
    yScreen = static_cast<float>(-yTileOffs + RenderPosY * TileSizeY);
    for (int j = RenderPosY; j < RenderPosYTo; j++) {
        xScreen = static_cast<float>(-xTileOffs + RenderPosX * TileSizeX);

        for (int i = RenderPosX; i < RenderPosXTo; i++) {
            // Ist ein Wasserfall teil?
            if (TileAt(i + xLevel, j + yLevel).Block & BLOCKWERT_WASSERFALL) {
                DirectGraphics.SetColorKeyMode();
                // Drei Schichten Wasserfall rendern =)
                //

                // Schicht 1
                int xoff = (i + xLevel) % 3 * ORIGINAL_TILE_SIZE_X;
                int yoff = (j + yLevel) % 3 * ORIGINAL_TILE_SIZE_Y + 120 - static_cast<int>(WasserfallOffset);

                Wasserfall[0].SetRect(xoff, yoff, xoff + ORIGINAL_TILE_SIZE_X, yoff + ORIGINAL_TILE_SIZE_Y);
                Wasserfall[0].RenderSpriteWithScale(static_cast<float>(i * TileSizeX - xTileOffs),
                                           static_cast<float>(j * TileSizeY - yTileOffs), Scale, Col1);

                // Schicht 2
                //
                xoff = (i + xLevel + 1) % 3 * ORIGINAL_TILE_SIZE_X;
                yoff = (j + yLevel) % 3 * ORIGINAL_TILE_SIZE_Y + 120 - static_cast<int>(WasserfallOffset / 2.0f);

                Wasserfall[0].SetRect(xoff, yoff, xoff + ORIGINAL_TILE_SIZE_X, yoff + ORIGINAL_TILE_SIZE_Y);
                Wasserfall[0].RenderSpriteWithScale(static_cast<float>(i * TileSizeX - xTileOffs),
                                           static_cast<float>(j * TileSizeY - yTileOffs), Scale, Col2);

                // Glanzschicht (Schicht 3) drüber
                //

                // FIXME: this has some displaying errors
#if 0
                Wasserfall[1].SetRect(
                    (i * ORIGINAL_TILE_SIZE_X - (int)(xTileOffs / Scale)) % 640,
                    (j * ORIGINAL_TILE_SIZE_Y - (int)(yTileOffs / Scale)) % 480,
                    (i * ORIGINAL_TILE_SIZE_X - (int)(xTileOffs / Scale)) % 640 + ORIGINAL_TILE_SIZE_X,
                    (j * ORIGINAL_TILE_SIZE_Y - (int)(yTileOffs / Scale)) % 480 + ORIGINAL_TILE_SIZE_Y);

                Wasserfall[1].RenderSpriteWithScale(
                                           static_cast<float>(i * TileSizeX - xTileOffs),
                                           static_cast<float>(j * TileSizeY - yTileOffs),
                                           Scale,
                                           D3DCOLOR_RGBA(180, 240, 255, 60));
#endif
 
            }

            xScreen += TileSizeX;  // Am Screen weiter
        }

        yScreen += TileSizeY;  // Am Screen weiter
    }

    DirectGraphics.SetFilterMode(false);
}

// --------------------------------------------------------------------------------------
// Grenzen checken
// --------------------------------------------------------------------------------------

void TileEngineClass::CheckBounds() {
    const float xtilesize = static_cast<float>(TileSizeX);
    const float ytilesize = static_cast<float>(TileSizeY);

    // Grenzen des Levels checken
    XOffset = std::clamp(XOffset, xtilesize, LEVELPIXELSIZE_X - RENDERWIDTH - xtilesize);
    YOffset = std::clamp(YOffset, ytilesize, LEVELPIXELSIZE_Y - RENDERHEIGHT - ytilesize);
}

// --------------------------------------------------------------------------------------

void TileEngineClass::WertAngleichen(float &nachx, float &nachy, float vonx, float vony) {
    float const rangex = std::clamp(vonx - nachx, -50.0f, 50.0f);
    float const rangey = std::clamp(vony - nachy, -60.0f, 60.0f);

    nachx += Timer.sync(rangex * 0.8f);
}

// --------------------------------------------------------------------------------------
// Level scrollen und Tiles animieren usw
// --------------------------------------------------------------------------------------

void TileEngineClass::UpdateLevel() {
    // Tiles animieren
    TileAnimCount += Timer.getSpeedFactor();        // Counter erhöhen
    if (TileAnimCount > TILEANIM_SPEED)  // auf Maximum prüfen
                                         // if (TileAnimCount > 0.5f)			// auf Maximum prüfen
    {
        TileAnimCount = 0.0f;    // Counter wieder auf 0 setzen
        TileAnimPhase++;         // und nächste Animphase setzen
        if (TileAnimPhase >= 4)  // Animation wieer von vorne ?
            TileAnimPhase = 0;
    }

    // Wasserfall animieren
    WasserfallOffset += Timer.sync(16.0f);

    while (WasserfallOffset >= 120.0f)
        WasserfallOffset -= 120.0f;

    WaterSinTable.AdvancePosition(Timer.getSpeedFactor());
}

// --------------------------------------------------------------------------------------
// Zurückliefern, welche BlockArt sich Rechts vom übergebenen Rect befindet
// --------------------------------------------------------------------------------------

uint32_t TileEngineClass::BlockRechts(float &x, float y, float &xo, float /*yo*/, RECT_struct rect, bool resolve) {
    // Nach rechts muss nicht gecheckt werden ?
    if (xo > x)
        return 0;

    int xlev = static_cast<int>((x + rect.right + 1) * (1.0f / TileSizeX));
    if (xlev < 0 || xlev >= LEVELSIZE_X)
        return 0;

    uint32_t block = 0;

    for (int j = rect.top; j < rect.bottom; j += TileSizeY) {
        int ylev = static_cast<int>((y + j) * (1.0f / TileSizeY));

        if (ylev < 0)
            continue;
        else if (ylev >= LEVELSIZE_Y)
            break;

        bool blockWand = block & BLOCKWERT_WAND;

        if (!blockWand) {
            uint32_t const newBlock = TileAt(xlev, ylev).Block;
            if (newBlock != 0) {
                block = newBlock;
                blockWand = block & BLOCKWERT_WAND;
            }
        }

        if (blockWand) {
            if (resolve) {
                x = static_cast<float>((xlev * TileSizeX) - rect.right - 1);
                xo = x;
            }

            return block;
        }
    }

    return block;
}

// --------------------------------------------------------------------------------------
// Zurückliefern, welche BlockArt sich Links vom übergebenen Rect befindet
// --------------------------------------------------------------------------------------

uint32_t TileEngineClass::BlockLinks(float &x, float y, float &xo, float /*yo*/, RECT_struct rect, bool resolve) {
    // Nach links muss nicht gecheckt werden ?
    if (xo < x)
        return 0;

    int xlev = static_cast<int>((x + rect.left - 1) * (1.0f / TileSizeX));
    if (xlev < 0 || xlev >= LEVELSIZE_X)
        return 0;

    uint32_t block = 0;

    for (int j = rect.top; j < rect.bottom; j += TileSizeY) {
        int ylev = static_cast<int>((y + j) * (1.0f / TileSizeY));

        if (ylev < 0)
            continue;
        else if (ylev >= LEVELSIZE_Y)
            break;

        bool blockWand = block & BLOCKWERT_WAND;

        if (!blockWand) {
            uint32_t const newBlock = TileAt(xlev, ylev).Block;
            if (newBlock != 0) {
                block = newBlock;
                blockWand = block & BLOCKWERT_WAND;
            }
        }

        if (blockWand) {
            if (resolve) {
                x = static_cast<float>((xlev * TileSizeX) + TileSizeX - rect.left);
                xo = x;
            }
            return block;
        }
    }

    return block;
}

// --------------------------------------------------------------------------------------
// Zurückliefern, welche Blockblock sich oberhalb vom übergebenen Rect befindet
// --------------------------------------------------------------------------------------

uint32_t TileEngineClass::BlockOben(float x, float &y, float /*xo*/, float &yo, RECT_struct rect, bool resolve) {
    // Nach oben muss nicht gecheckt werden ?
    if (yo < y)
        return 0;

    int ylev = static_cast<int>((y + rect.top - 1) * (1.0f / TileSizeY));
    if (ylev < 0 || ylev >= LEVELSIZE_Y)
        return 0;

    uint32_t block = 0;

    for (int i = rect.left; i < rect.right; i += TileSizeX) {
        int xlev = static_cast<int>((x + i) * (1.0f / TileSizeX));
        if (xlev < 0)
            continue;
        else if (xlev >= LEVELSIZE_X)
            break;

        bool blockWand = block & BLOCKWERT_WAND;

        if (!blockWand) {
            uint32_t const newBlock = TileAt(xlev, ylev).Block;
            if (newBlock > 0) {
                block = newBlock;
                blockWand = block & BLOCKWERT_WAND;
            }
        }

        if (blockWand) {
            if (resolve) {
                y = static_cast<float>((ylev * TileSizeY) + TileSizeY - rect.top);
                yo = y;
            }

            return block;
        }
    }

    return block;
}

// --------------------------------------------------------------------------------------
// Zurückliefern, welche Blockblock sich unterhalb vom übergebenen Rect befindet
// und dabei nicht "begradigen" sprich die y-Position an das Tile angleichen
// --------------------------------------------------------------------------------------

uint32_t TileEngineClass::BlockUntenNormal(float x, float y, float /*xo*/, float yo, RECT_struct rect) {
    // Nach unten muss nicht gecheckt werden ?
    if (yo > y)
        return false;

    int ylev = static_cast<int>((y + rect.bottom + 1) * (1.0f / TileSizeY));
    if (ylev < 0 || ylev >= LEVELSIZE_Y)
        return 0;

    uint32_t block = 0;

    // BIG TODO: see if you can make this increment by TILESIZE_X
    for (int i = rect.left; i < rect.right; i++) {
        int xlev = static_cast<int>((x + i) * (1.0f / TileSizeX));

        if (xlev < 0)
            continue;
        else if (xlev >= LEVELSIZE_X)
            break;

        bool blockWand = block & BLOCKWERT_WAND;

        if (!blockWand) {
            uint32_t const newBlock = TileAt(xlev, ylev).Block;
            if (newBlock > 0) {
                block = newBlock;
                blockWand = block & BLOCKWERT_WAND;
            }
        }

        if (blockWand || block & BLOCKWERT_PLATTFORM)
            return block;
    }

    return block;
}

// --------------------------------------------------------------------------------------
// Zurückliefern, welche Blockblock sich unterhalb vom übergebenen Rect befindet
// --------------------------------------------------------------------------------------

uint32_t TileEngineClass::BlockUnten(float x, float &y, float /*xo*/, float &yo, RECT_struct rect, bool resolve) {
    // Nach unten muss nicht gecheckt werden ?
    if (yo > y)
        return 0;

    int ylev = static_cast<int>((y + rect.bottom + 1) * (1.0f / TileSizeY));
    if (ylev < 0 || ylev >= LEVELSIZE_Y)
        return 0;

    uint32_t block = 0;

    // BIG TODO: see if you can make this increment by TILESIZE_X
    for (int i = rect.left; i < rect.right; i++) {
        int xlev = static_cast<int>((x + i) * (1.0f / TileSizeX));

        if (xlev < 0)
            continue;
        else if (xlev >= LEVELSIZE_X)
            break;

        bool blockWand = block & BLOCKWERT_WAND;
        bool blockPlatform = block & BLOCKWERT_PLATTFORM;

        if (!blockWand && !blockPlatform) {
            uint32_t const newBlock = TileAt(xlev, ylev).Block;
            if (newBlock > 0) {
                block = newBlock;
                blockWand = block & BLOCKWERT_WAND;
                blockPlatform = block & BLOCKWERT_PLATTFORM;
            }
        }

        if (blockWand || blockPlatform) {
            if (resolve) {
                y = static_cast<float>(ylev * TileSizeY - rect.bottom);
                yo = y;
            }

            return block;
        }
    }

    return block;
}

// --------------------------------------------------------------------------------------
// Auf Schrägen prüfen
// --------------------------------------------------------------------------------------

uint32_t TileEngineClass::BlockSlopes(const float x, float &y, const RECT_struct rect, const float ySpeed) {

    for (int j = rect.bottom; j < rect.bottom + TileSizeY; j++) {
        int ylev = static_cast<int>((y + (j - 1)) * (1.0f / TileSizeY));

        if (ylev < 0)
            continue;
        else if (ylev >= LEVELSIZE_Y)
            break;

        // Schräge links
        // von links anfangen mit der Block-Prüdung

        // DKS - TODO see if you can get this to increment faster, by tile:
        for (int i = rect.left; i < rect.right; i++) {
            int xlev = static_cast<int>((x + i) * (1.0f / TileSizeX));

            if (xlev < 0)
                continue;
            else if (xlev >= LEVELSIZE_X)
                break;

            uint32_t const block = TileAt(xlev, ylev).Block;

            if (block & BLOCKWERT_SCHRAEGE_L) {
                float newy = static_cast<float>((ylev + 1) * TileSizeY - rect.bottom -
                    (TileSizeY - (static_cast<int>(x + i) % (int)TileSizeX)) - 1);
                if (ySpeed == 0.0f || y > newy) {
                    y = newy;
                    return block;
                }
            }
        }

        // Schräge rechts
        // von rechts anfangen mit der Block-Prüdung

        // DKS TODO: try to get this to decrement faster, by tile:
        for (int i = rect.right; i > rect.left; i--) {
            int xlev = static_cast<int>((x + i) * (1.0f / TileSizeX));

            if (xlev >= LEVELSIZE_X)
                continue;
            else if (xlev < 0)
                break;

            uint32_t const block = TileAt(xlev, ylev).Block;

            if (block & BLOCKWERT_SCHRAEGE_R) {
                float newy = static_cast<float>((ylev + 1) * TileSizeY - rect.bottom -
                    (static_cast<int>(x + i) % (int)TileSizeX) - 1);
                if (ySpeed == 0.0f || y > newy) {
                    y = newy;
                    return block;
                }
            }
        }
    }

    return 0;
}

// --------------------------------------------------------------------------------------
// Zurückliefern, welcher Farbwert sich in der Mitte des RECTs an x/y im Level befindet
// damit die Gegner entsprechend dem Licht im Editor "beleuchtet" werden
// --------------------------------------------------------------------------------------

D3DCOLOR TileEngineClass::LightValue(float x, float y, RECT_struct rect, bool forced) {

    int const x_level = static_cast<int>((x + static_cast<float>(rect.right - rect.left) / 2) / TileSizeX);  // xPosition im Level
    int const y_level = static_cast<int>((y + static_cast<float>(rect.bottom - rect.top) / 2) / TileSizeY);  // yPosition im Level

    if ((x_level >= LEVELSIZE_X || y_level >= LEVELSIZE_Y) ||
        (!forced && !(TileAt(x_level, y_level).Block & BLOCKWERT_LIGHT)))  // Soll das Leveltile garnicht
        return 0xFFFFFFFF;                                               // das Licht des Objektes ändern

    const LevelTileStruct& tile = TileAt(x_level, y_level);
    unsigned int r = tile.Red;
    unsigned int g = tile.Green;
    unsigned int b = tile.Blue;

    r += 48;  // Farbewerte ein wenig erhöhen, damit man selbst bei 0,0,0
    g += 48;  // noch ein wenig was sehen kann und das Sprite nicht
    b += 48;  // KOMPLETT schwarz wird ... minimum ist 48, 48, 48

    if (r > 255)
        r = 255;  // Grenzen überschritten ? Dann angleichen
    if (g > 255)
        g = 255;
    if (b > 255)
        b = 255;

    return D3DCOLOR_RGBA(r, g, b, 255);
}

// --------------------------------------------------------------------------------------
// Neue Lichtberechnung
// Jedes Tile hat an allen vier Ecken eine interpolierte Farbe
// entsprechend der umliegenden Tiles -> smoothe Übergänge -> Geilomat!
// --------------------------------------------------------------------------------------

inline void interpolateColor(const LevelTileStruct& centralTile, const LevelTileStruct& otherTile, int& r, int& g, int& b) {
    if (!((otherTile.Block ^ centralTile.Block) & BLOCKWERT_WAND)) {
        r = otherTile.Red;
        g = otherTile.Green;
        b = otherTile.Blue;
    } else {
        r = centralTile.Red;
        g = centralTile.Green;
        b = centralTile.Blue;
    }
}

void TileEngineClass::ComputeCoolLight() {
    // Lichter im Level interpolieren
    // Dabei werden die Leveltiles in 2er Schritten durchgegangen
    // Dann werden die 4 Ecken des aktuellen Tiles auf die Farben der Nachbarfelder gesetzt
    // Farben der Nachbarfelder werden allerdings nur mit verrechnet, wenn es sich nicht um eine massive Wand handelt.
    // In diesem Falle wird die Standard-Tilefarbe verwendet
    //
    for (int i = 1; i < LEVELSIZE_X - 1; i += 1)
        for (int j = 1; j < LEVELSIZE_Y - 1; j += 1) {
            LevelTileStruct& tile = TileAt(i, j);

            int const al = tile.Alpha;

            int const r4 = tile.Red;
            int const g4 = tile.Green;
            int const b4 = tile.Blue;

            int rn, gn, bn, r1, r2, r3, g1, g2, g3, b1, b2, b3;

            // Ecke links oben
            //
            interpolateColor(tile, TileAt(i - 1, j - 1), r1, g1, b1);
            interpolateColor(tile, TileAt(i + 0, j - 1), r2, g2, b2);
            interpolateColor(tile, TileAt(i - 1, j + 0), r3, g3, b3);

            rn = (r1 + r2 + r3 + r4) / 4;
            gn = (g1 + g2 + g3 + g4) / 4;
            bn = (b1 + b2 + b3 + b4) / 4;

            tile.Color[0] = D3DCOLOR_RGBA(rn, gn, bn, al);

            // Ecke rechts oben
            //
            interpolateColor(tile, TileAt(i - 0, j - 1), r1, g1, b1);
            interpolateColor(tile, TileAt(i + 1, j - 1), r2, g2, b2);
            interpolateColor(tile, TileAt(i + 1, j + 0), r3, g3, b3);

            rn = (r1 + r2 + r3 + r4) / 4;
            gn = (g1 + g2 + g3 + g4) / 4;
            bn = (b1 + b2 + b3 + b4) / 4;

            tile.Color[1] = D3DCOLOR_RGBA(rn, gn, bn, al);

            // Ecke links unten
            //
            interpolateColor(tile, TileAt(i - 1, j - 0), r1, g1, b1);
            interpolateColor(tile, TileAt(i - 1, j + 1), r2, g2, b2);
            interpolateColor(tile, TileAt(i - 0, j + 1), r3, g3, b3);

            rn = (r1 + r2 + r3 + r4) / 4;
            gn = (g1 + g2 + g3 + g4) / 4;
            bn = (b1 + b2 + b3 + b4) / 4;

            tile.Color[2] = D3DCOLOR_RGBA(rn, gn, bn, al);

            // Ecke rechts unten
            //
            interpolateColor(tile, TileAt(i + 1, j - 0), r1, g1, b1);
            interpolateColor(tile, TileAt(i - 0, j + 0), r2, g2, b2);
            interpolateColor(tile, TileAt(i + 1, j + 1), r3, g3, b3);

            rn = (r1 + r2 + r3 + r4) / 4;
            gn = (g1 + g2 + g3 + g4) / 4;
            bn = (b1 + b2 + b3 + b4) / 4;

            tile.Color[3] = D3DCOLOR_RGBA(rn, gn, bn, al);
        }

}  // ComputeCoolLight

// --------------------------------------------------------------------------------------
// "Taschenlampen" Ausschnitt im Alien Level rendern
// --------------------------------------------------------------------------------------

void TileEngineClass::DrawShadow() {
    if (!bDrawShadow)
        return;

    float x = XOffset + 320.0f;
    float y = YOffset + 240.0f;

    D3DCOLOR col = D3DCOLOR_RGBA(255, 255, 255, static_cast<int>(ShadowAlpha));

    Shadow.RenderSprite(x, y, col);
    Shadow.RenderSprite(x + 512, y, 0, col, true);

    Shadow.RenderMirroredSprite(x, y + 512, col, false, true);
    Shadow.RenderMirroredSprite(x + 512, y + 512, col, true, true);

    col = D3DCOLOR_RGBA(0, 0, 0, static_cast<int>(ShadowAlpha));

    // Seitenränder
    RenderRect(x - 200, y - 200, 1420, 200, col);
    RenderRect(x - 200, y + 1420, 1420, 200, col);
    RenderRect(x - 200, y, 200, 1024, col);
    RenderRect(x + 1024, y, 200, 1024, col);
}

void TileEngineClass::ToggleLamp() {
    if (bDrawShadow == false) {
        bDrawShadow = true;
        ShadowAlpha = 255.0f;
    } else
        bDrawShadow = false;
}

