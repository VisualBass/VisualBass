cmake_minimum_required(VERSION 3.10)

project(untitled1)

# External libraries (raylib, portaudio, kissfft)
set(RAYLIB_PATH "N:/Raylib")
set(PORTAUDIO_PATH "N:/portaudio-master/portaudio")
set(KISSFFT_PATH "N:/kissfft-master/kissfft-master")
set(LIBWINPTHREAD_PATH "N:/path_to_libwinpthread")  # Set the path to libwinpthread

# Include directories
include_directories(${RAYLIB_PATH}/include)
include_directories(${PORTAUDIO_PATH}/include)
include_directories(${KISSFFT_PATH}/include)

# Link directories
link_directories(${RAYLIB_PATH}/lib)
link_directories(${PORTAUDIO_PATH}/build)
link_directories(${LIBWINPTHREAD_PATH})  # Link directory for libwinpthread

# Add KissFFT subdirectory (if needed)
add_subdirectory(${KISSFFT_PATH} ${CMAKE_BINARY_DIR}/kissfft)

# Add source files for the project
add_executable(untitled1
        main.cpp
        gravityorbs.cpp
        gravityorbs.h
        networking.cpp
        networking.h
        waveform.cpp
        waveform.h
        globals.cpp
        globals.h
        menu.cpp
        menu.h
        cube.cpp
        cube.h
)

# Link libraries
target_link_libraries(untitled1
        raylib
        portaudio
        kissfft::kissfft
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
        winpthread  # Link the libwinpthread library dynamically
)

# For MinGW (optional if using MSVC or other compilers)
if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
endif()
# --- Output folder ---
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# Enable debug symbols
set(CMAKE_BUILD_TYPE Debug)  # Change to Release when ready for production
