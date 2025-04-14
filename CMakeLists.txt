cmake_minimum_required(VERSION 3.10)

# Project name
project(VisualBassSync)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

# External library configuration for KissFFT
set(KISSFFT_PKGCONFIG OFF CACHE BOOL "Disable pkg-config for KissFFT" FORCE)
set(KISSFFT_TEST OFF CACHE BOOL "Disable KissFFT tests" FORCE)
set(KISSFFT_TOOLS OFF CACHE BOOL "Disable KissFFT tools" FORCE)
# Set root library path

set(LIB_ROOT "C:/Packages")

# Set library paths for KissFFT
set(KISSFFT_PATH "${LIB_ROOT}/kissfft-master/kissfft-master")

# Include KissFFT as a subproject (it must have its own valid CMakeLists.txt)
add_subdirectory(${KISSFFT_PATH} ${CMAKE_BINARY_DIR}/kissfft)
# Set Bullet library path
set(BULLET_PATH "${LIB_ROOT}/bullet3-master/bullet3-master/cmake-build-debug")
set(RAYLIB_PATH "${LIB_ROOT}/raylib")
set(PORTAUDIO_PATH "N:/portaudio-master/portaudio")
set(KISSFFT_PATH "N:/kissfft-master/kissfft-master")
set(LIBWINPTHREAD_PATH "N:/path_to_libwinpthread")
set(PKG_CONFIG_EXECUTABLE "C:/msys64/usr/bin/pkg-config.exe")

# Set PKG_CONFIG_PATH for libpng
set(ENV{PKG_CONFIG_PATH} "C:/msys64/mingw64/lib/pkgconfig;C:/msys64/usr/share/pkgconfig")

include_directories(${PORTAUDIO_PATH}/include)
# Include directories for external libraries
include_directories(
        ${RAYLIB_PATH}/include
        ${KISSFFT_PATH}/include
        ${BULLET_PATH}/src
)

# Link directories for external libraries
link_directories(
        ${RAYLIB_PATH}/lib
        ${PORTAUDIO_PATH}/build
        ${BULLET_PATH}/lib
)

# Add executable target and source files
add_executable(VisualBassSync
        main.cpp
        gravityorbs.cpp
        networking.cpp
        waveform.cpp
        globals.cpp
        menu.cpp
        cube.cpp
        particle01.cpp
        particle01.h
        Menu/ColorPicker.cpp
        Menu/ColorPicker.h
        Menu/GetColorFromHue.cpp
        Menu/GetColorFromHue.h
        Menu/OrbControl.cpp
        Menu/OrbControl.h
        Menu/SliderControl.cpp
        Menu/SliderControl.h
)

# Link libraries to your executable
target_link_libraries(VisualBassSync
        raylib
        portaudio
        kissfft::kissfft  # Link KissFFT here
        BulletDynamics_Debug   # Link Bullet Dynamics library
        BulletCollision_Debug  # Link Bullet Collision library
        LinearMath_Debug      # Link LinearMath library
        winmm
        gdi32
        opengl32
        setupapi
        ws2_32
        kernel32
        user32
        winspool
        shell32
        ole32
        oleaut32
        uuid
        comdlg32
        advapi32
        winpthread
)

# Optional SIMD config for KissFFT
if(KISSFFT_DATATYPE MATCHES "^simd$")
    target_compile_definitions(VisualBassSync PRIVATE USE_SIMD)
    if (NOT MSVC)
        target_compile_options(VisualBassSync PRIVATE -msse)
    else()
        target_compile_options(VisualBassSync PRIVATE "/arch:SSE")
    endif()
endif()

# For MinGW (optional if using MSVC or other compilers)
if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
endif()

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
