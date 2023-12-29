cmake_minimum_required(VERSION 3.9)
project(editor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/3rdparty/" "${CMAKE_SOURCE_DIR}/3rdparty/sdl2-cmake-scripts")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
        "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options ("$<$<CONFIG:DEBUG>:-Wall;-Wextra;-pedantic;-Wcast-align;-Wcast-qual;-Wctor-dtor-privacy;-Wdisabled-optimization;-Wformat=2;-Winit-self;-Wmissing-include-dirs;-Wno-old-style-cast;-Woverloaded-virtual;-Wredundant-decls;-Wshadow;-Wsign-promo;-Wundef;-Wno-unused;-Wno-variadic-macros;-Wno-parentheses;-Wdouble-promotion;-fdiagnostics-show-option>")
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT result OUTPUT output)
if(result)
    message(STATUS "IPO supported by compiler, enabled for release builds")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
else()
    message(WARNING "IPO is not supported: ${output}")
endif()

        #src/Main.cpp
set(EDITOR_SOURCES
        src/GUI/App.cpp
        src/GUI/App.hpp
        src/GUI/EditMenu.cpp
        src/GUI/EditMenu.hpp
        src/GUI/IDs.hpp
        src/GUI/MainFrame.cpp
        src/GUI/MainFrame.hpp
        src/GUI/TileCanvas.cpp
        src/GUI/TileCanvas.hpp
        src/GUI/TileSet.cpp
        src/GUI/TileSet.hpp

        src/ObjectList.cpp
        src/ObjectList.hpp

        src/Globals.cpp
        src/Globals.hpp

        src/Logdatei.cpp
        src/Logdatei.hpp

        src/Tileengine.cpp
        src/Tileengine.hpp

        src/Timer.cpp
        src/Timer.hpp

        src/DX8Graphics.cpp
        src/DX8Graphics.hpp
        src/DX8Texture.cpp
        src/DX8Texture.hpp
        src/DX8Font.cpp
        src/DX8Font.hpp
        src/DX8Sprite.cpp
        src/DX8Sprite.hpp
        src/Mathematics.cpp
        src/Mathematics.hpp

        src/SDLPort/keymap.hpp
        src/SDLPort/opengl.hpp
        src/SDLPort/SDL_fmod.cpp
        src/SDLPort/SDL_fmod.hpp
        src/SDLPort/SDL_port.cpp
        src/SDLPort/SDL_port.hpp
        src/SDLPort/texture.cpp
        src/SDLPort/texture.hpp
)

add_definitions(-DUSE_GL2)
set(gl2 ON)

list(APPEND EDITOR_SOURCES
    src/SDLPort/cshader.cpp
    src/SDLPort/cshader.hpp
)

include(CMakeDependentOption)

ADD_DEFINITIONS(-DUSE_FAST_RNG)

if(UNIX)
    add_definitions(-DUSE_HOME_DIR)
    add_definitions(-DUSE_STORAGE_PATH="${CMAKE_INSTALL_PREFIX}/share/hurrican/")
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/SDLPort)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/glm)

add_link_options("-lglut")
add_executable(${PROJECT_NAME} ${EDITOR_SOURCES})

set(SDL2_BUILDING_LIBRARY TRUE)
find_package(SDL2)
if (SDL2_FOUND)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_mixer REQUIRED)
endif()

find_package(LibEpoxy 1.2 REQUIRED)
include_directories(${LibEpoxy_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${LibEpoxy_LIBRARIES})

if (SDL2_FOUND)
    include_directories(${SDL2_INCLUDE_DIR}
                        ${SDL2_IMAGE_INCLUDE_DIR}
                        ${SDL2_MIXER_INCLUDE_DIR})

    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1)
    target_link_libraries(${PROJECT_NAME} stdc++fs)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(${PROJECT_NAME} c++fs)
endif()

#target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/IncludeEpoxy.hpp)

find_package(wxWidgets REQUIRED COMPONENTS net core base gl)
include(${wxWidgets_USE_FILE})
target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES})

if(UNIX)
    install(PROGRAMS  ${CMAKE_BINARY_DIR}/hurrican   DESTINATION bin/)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/      DESTINATION share/hurrican/data)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/lang/      DESTINATION share/hurrican/lang)
    install(FILES     ${CMAKE_SOURCE_DIR}/readme.txt DESTINATION share/doc/hurrican/)
else()
    install(PROGRAMS  ${CMAKE_BINARY_DIR}/hurrican   DESTINATION ./)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/      DESTINATION data)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/lang/      DESTINATION lang)
    install(FILES     ${CMAKE_SOURCE_DIR}/readme.txt DESTINATION ./)
endif()
