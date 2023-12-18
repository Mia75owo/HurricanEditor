/**
 *
 *  Copyright (C) 2011-2015 Scott R. Smith
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 *
 */

#include <cstdint>
#include <cstring>
#include <filesystem>
namespace fs = std::filesystem;

#include "DX8Graphics.hpp"
#include "DX8Texture.hpp"
#include "texture.hpp"

// DKS - Textures are now managed in DX8Sprite.cpp in new TexturesystemClass.
//      This function now returns a TextureHandle object, and if it fails to load the file,
//      all of the returned TextureHandle's data members will be set to 0.
//      In th.npot_scalex and th.npot_scaley, it sets a correction
//      factor to apply to each dimension to compensate for any increases in size
//      from power-of-two expansion (each will remain 1.0 if none occurred).
bool SDL_LoadTexture(const std::string &path,
                     const std::string &filename,
                     void *buf,
                     unsigned int buf_size,
                     TextureHandle &th) {
    image_t image;
    bool success = false;
    bool load_from_memory = buf_size > 0;
    std::string fullpath;
    std::string filename_sans_ext(filename);
    ReplaceAll(filename_sans_ext, ".png", "");

    if (load_from_memory && !buf) {
        Protokoll << "Error in SDL_LoadTexture() loading texture from memory!" << std::endl;
        GameRunning = false;
        return false;
    } else if (!load_from_memory && filename.empty()) {
        Protokoll << "Error in SDL_LoadTexture(): empty filename parameter" << std::endl;
        GameRunning = false;
        return false;
    }

    fullpath = path + "/" + filename;
    success = loadImageSDL(image, fullpath, buf, buf_size) && load_texture(image, th.tex);
    image.data = std::vector<char>();

    if (success)
        goto loaded;
    else
        th.tex = 0;

loaded:
    if (success) {
        th.instances = 1;
        th.npot_scalex = image.npot_scalex;
        th.npot_scaley = image.npot_scaley;
    } else {
        Protokoll << "Error loading texture " << filename << " in SDL_LoadTexture()" << std::endl;
        GameRunning = false;
    }

    return success;
}

void SDL_UnloadTexture(TextureHandle &th) {
    glDeleteTextures(1, &th.tex);
    th.tex = 0;
    th.instances = 0;
}

bool load_texture(image_t &image, GLuint &new_texture) {
    GLuint texture;

    if (!image.data.empty()) {
        // Have OpenGL generate a texture object handle for us
        glGenTextures(1, &texture);

        // Bind the texture object
        glBindTexture(GL_TEXTURE_2D, texture);

        // Set the texture's stretching properties
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // DKS - this fixes the lightning-beam drawing in low,med-detail modes..
        //      and from looking at its code in Player.cpp, I am convinced this is
        //      correct edge setting for textures for this game. My play-testing
        //      shows no unwanted side effects from the change to GL_REPEAT.
        // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        if (image.compressed) {
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, image.format, image.w, image.h, 0, image.data.size(),
                                   image.data.data() + image.offset);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, image.format, image.w, image.h, 0, image.format, image.type,
                         image.data.data());
        }

#ifndef NDEBUG
        int error = glGetError();
        if (error != 0) {
            Protokoll << "GL load_texture Error " << error << std::endl;
            Protokoll << "Format " << std::hex << image.format << std::dec << " W " << image.w << " H " << image.h
                      << " S " << image.data.size() << " Data " << std::hex << reinterpret_cast<std::uintptr_t>(image.data.data())
                      << std::dec << " + " << image.offset << std::endl;
            return false;
        }
#endif

    } else {
        Protokoll << "ERROR Image data reference is NULL" << std::endl;
        return false;
    }

    new_texture = texture;
    return true;
}

bool loadImageSDL(image_t &image, const std::string &fullpath, void *buf, unsigned int buf_size) {
    SDL_Surface *rawSurf = nullptr;  // This surface will tell us the details of the image
    SDL_Surface *finSurf = nullptr;

    // Init
    image.data = std::vector<char>();
    image.compressed = false;
    image.format = GL_RGBA;
    image.npot_scalex = 1.0;
    image.npot_scaley = 1.0;

    if (buf_size == 0)  // Load from file
    {
        if (fullpath.empty() || !fs::exists(fullpath) || !fs::is_regular_file(fullpath)) {
            Protokoll << "Error in loadImageSDL loading " << fullpath << std::endl;
            GameRunning = false;
            return false;
        }

        rawSurf = IMG_Load(fullpath.c_str());
    } else  // Load from memory
    {
        SDL_RWops *sdl_rw = SDL_RWFromConstMem(reinterpret_cast<const void *>(buf), buf_size);

        if (sdl_rw != nullptr) {
            rawSurf = IMG_Load_RW(sdl_rw, 1);
        } else {
            Protokoll << "ERROR Texture: Failed to load texture: " << SDL_GetError() << std::endl;
            GameRunning = false;
            return false;
        }
    }

    if (rawSurf != nullptr) {
        SDL_Rect rawDimensions;
        //  Store dimensions of original RAW surface
        rawDimensions.x = rawSurf->w;
        rawDimensions.y = rawSurf->h;

        if (!isPowerOfTwo(rawSurf->w)) {
            rawDimensions.x = nextPowerOfTwo(rawSurf->w);
            image.npot_scalex = static_cast<double>(rawSurf->w) / static_cast<double>(rawDimensions.x);
        }

        if (!isPowerOfTwo(rawSurf->h)) {
            rawDimensions.y = nextPowerOfTwo(rawSurf->h);
            image.npot_scaley = static_cast<double>(rawSurf->h) / static_cast<double>(rawDimensions.y);
        }

        finSurf = SDL_CreateRGBSurface(SDL_SWSURFACE, rawDimensions.x, rawDimensions.y, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN  // OpenGL RGBA masks
                                       0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
#else
                                       0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
#endif
        );

        SDL_SetSurfaceAlphaMod(rawSurf, 255);

        SDL_BlitSurface(rawSurf, nullptr, finSurf, nullptr);
        SDL_FreeSurface(rawSurf);

        uint8_t factor = 1;

        // Blacklist of image filenames (sub-strings) that shouldn't ever be resized, because of
        // resulting graphics glitches
        if (fullpath.find("font") != std::string::npos
            //|| fullpath.find("lightmap")     != std::string::npos           // Lightmaps were never actually used in
            //the game
            || fullpath.find("hurrican_rund") != std::string::npos  // Menu star/nebula background (ugly)
            || fullpath.find("roboraupe") != std::string::npos      // Flat spiky enemy worm-like thing (glitches)
            || fullpath.find("enemy-walker") != std::string::npos   // Frog-like robotic walker (glitches)
            || fullpath.find("stelzsack") != std::string::npos      // Stilt-walker enemy on elevator level
        ) {
            factor = 1;
        }

        if (factor > 1)
            image.data = LowerResolution(finSurf, factor);

        image.w = finSurf->w / factor;
        image.h = finSurf->h / factor;

        if (image.data.empty()) {
            image.data.resize(
                static_cast<size_t>(finSurf->w) * static_cast<size_t>(finSurf->h) * sizeof(uint32_t)
            );
            std::memcpy(image.data.data(), finSurf->pixels, image.data.size());
            image.type = GL_UNSIGNED_BYTE;
        }

        SDL_FreeSurface(finSurf);
    } else {
        Protokoll << "Error in loadImageSDL: Could not read image data into rawSurf" << std::endl;
        GameRunning = false;
        return false;
    }

    return true;
}

// DKS - Assumes rgba8888 input, rgba8888 output
std::vector<char> LowerResolution(SDL_Surface *surface, int factor) {
    if (factor != 2 && factor != 4) {
        Protokoll << "ERROR call to LowerResolution() with factor not equal to 2 or 4" << std::endl;
        return std::vector<char>();
    }

    if (surface->format->BytesPerPixel != 4) {
        Protokoll << "ERROR call to LowerResolution() with source surface bpp other than 4" << std::endl;
        return std::vector<char>();
    }

    std::vector<char> dataout;
    dataout.reserve(
        static_cast<size_t>(surface->h / factor) * static_cast<size_t>(surface->w / factor) * sizeof(uint32_t)
    );

    uint32_t *dataout32 = reinterpret_cast<uint32_t *>(dataout.data());

    for (int y = 0; y < surface->h; y += factor) {
        uint32_t *datain32 = (reinterpret_cast<uint32_t *>(surface->pixels)) + surface->w * y;
        for (int x = 0; x < surface->w; x += factor) {
            *dataout32 = *datain32;
            datain32 += factor;
            dataout32++;
        }
    }

    return dataout;
}
