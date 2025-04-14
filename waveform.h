#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <vector>
#include <deque>
#include "raylib.h"

extern Shader waveformShader;  // Declare the shader variable

class Waveform {
public:
    // Constructor
    Waveform(int control_waveform_points,
             float waveform_smoothing_factor,
             float waveform_height_scale,
             float control_brightness_floor,
             float glow_value,
             float control_sensitivity,
             float hue_value);

    // Method to update waveform buffers if needed (e.g., for smoothing or other processing)
    void updateWaveformBuffers();

    // Method to draw the waveform
    void drawWaveformWrapper(const std::vector<float>& audio_data, Color orbColor);  // Add orbColor parameter
    void drawWaveform(const std::vector<float>& latest_audio_data, Color orbColor);  // Modify drawWaveform

private:
    int control_waveform_points;
    float waveform_smoothing_factor;
    float waveform_height_scale;
    float control_brightness_floor;
    float control_sensitivity;
    float glow_value;
    float hue_value;
    std::vector<std::deque<float>> waveform_buffers;
};

#endif  // WAVEFORM_H
