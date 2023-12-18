#ifndef GLOBALS_HPP_
#define GLOBALS_HPP_

#include <cstdint>
#include <string>
#include <SDL2/SDL.h>

constexpr int RENDERWIDTH = 640;
constexpr int RENDERHEIGHT = 480;
constexpr int SCREENWIDTH = 640;
constexpr int SCREENHEIGHT = 480;

extern std::string g_storage_ext;
extern bool GameRunning;

static inline uint32_t FixEndian(uint32_t x) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    return SDL_Swap32(x);
#else
    return x;
#endif
}

static inline int32_t FixEndian(int32_t x) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    uint32_t val = SDL_Swap32(*reinterpret_cast<uint32_t *>(&x));
    return *reinterpret_cast<int32_t *>(&val);
#else
    return x;
#endif
}

void ReplaceAll(std::string &str, const std::string &from, const std::string &to);

template<typename T>
inline void clampAngle(T& angle) {
    static_assert(std::is_arithmetic<T>::value, "Arithmetic value required in clampAngle.");

    constexpr T MIN_ANGLE = static_cast<T>(0);
    constexpr T MAX_ANGLE = static_cast<T>(360);

    while (angle < MIN_ANGLE)
      angle += MAX_ANGLE;
    while (angle >= MAX_ANGLE)
      angle -= MAX_ANGLE;
}

#endif
