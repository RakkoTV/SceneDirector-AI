# Build Guide - SceneDirector-AI

## Building on Windows

### Prerequisites

- Visual Studio 2019 or later
- CMake 3.15+
- OBS Studio source code
- Qt 5.15+

### Steps

1. Clone OBS Studio:
```bash
git clone https://github.com/obsproject/obs-studio.git
```

2. Clone SceneDirector-AI:
```bash
git clone https://github.com/RakkoTV/SceneDirector-AI.git
```

3. Build OBS Studio (follow official guide)

4. Build SceneDirector-AI:
```bash
mkdir build && cd build
cmake -D LIBOBS_INCLUDE_DIR=../../obs-studio/libobs ..
cmake --build . --config Release
```

5. Copy `SceneDirectorAI.dll` to OBS plugins folder

## Building on macOS

### Prerequisites

- Xcode 12+
- CMake 3.15+
- OBS Studio source code

### Steps

```bash
brew install cmake qt5
mkdir build && cd build
cmake -D LIBOBS_INCLUDE_DIR=../../obs-studio/libobs ..
make
```

## Building on Linux

### Prerequisites

- GCC 9+ or Clang 10+
- CMake 3.15+
- Qt 5 dev packages
- OBS Studio dev files

### Steps

```bash
sudo apt install obs-studio dev cmake qtbase5-dev
mkdir build && cd build
cmake -D LIBOBS_INCLUDE_DIR=/usr/include/obs ..
make -j4
```

## Dependencies

- libobs (OBS Studio core)
- Qt 5 (GUI)
- WebSocket library (libwebsockets)

## Troubleshooting Build

**"libobs not found"**:
- Set `LIBOBS_INCLUDE_DIR` to your obs-studio path
- Build OBS Studio first

**"Qt not found"**:
- Set `Qt5_DIR` to your Qt installation
- Install Qt development packages

---

For build issues, see [GitHub Issues](https://github.com/RakkoTV/SceneDirector-AI/issues)
