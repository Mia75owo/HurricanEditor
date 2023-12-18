An Editor for [Hurrican](https://github.com/HurricanGame/Hurrican).

---

### NOTICE
This Editor is currently (and probably for ever) not meant to be used.
The only purpose of me making this is: fun.

---

### Dependencies

The code depends on SDL2 with the image component, and libepoxy.
A compiler with c++17 support is required.

### Building

The code can be built using cmake (tested on Linux)

    git clone --recurse-submodules https://github.com/Nils75owo/HurricanEditor.git
    cd HurricanEditor
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build .

