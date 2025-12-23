#include "waveform.h"
#include "raylib.h"
#include <vector>
#include <deque>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include "globals.h" // Include globals to access enableInterpolation

// Global variables
extern float glow_value;
extern float hue_value;
extern float control_waveform_points;
extern float waveform_height_scale;
extern float waveform_smoothing_factor;
extern float control_sensitivity;
extern float control_brightness_floor;
extern std::vector<std::deque<float>> waveform_buffers;

// Constant for maximum line thickness
const float MAX_LINE_THICKNESS = 16.0f;

float smoothstep(float edge0, float edge1, float x) {
    x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

float lerp(float start, float end, float t) {
    return start + t * (end - start);
}

Waveform::Waveform(int control_waveform_points,
                   float waveform_smoothing_factor,
                   float waveform_height_scale,
                   float control_brightness_floor,
                   float glow_value,
                   float control_sensitivity,
                   float hue_value)
    : control_waveform_points(control_waveform_points),
      waveform_smoothing_factor(waveform_smoothing_factor),
      waveform_height_scale(waveform_height_scale),
      control_brightness_floor(control_brightness_floor),
      glow_value(glow_value),
      control_sensitivity(control_sensitivity),
      hue_value(hue_value) {
    waveform_buffers.resize(control_waveform_points);
}

void Waveform::updateWaveformBuffers() {
    for (size_t i = 0; i < waveform_buffers.size(); ++i) {
        if (waveform_buffers[i].size() > 100) {
            waveform_buffers[i].pop_front();
        }
    }
}

void Waveform::drawWaveformWrapper(const std::vector<float>& audio_data, Color orbColor) {
    drawWaveform(audio_data, orbColor);
}

void Waveform::drawWaveform(const std::vector<float>& latest_audio_data, Color orbColor) {
    try {
        if (latest_audio_data.empty() || latest_audio_data.size() < 2) {
            return;
        }

        int downsample_factor = std::max(1, static_cast<int>(latest_audio_data.size()) / control_waveform_points);
        std::vector<float> downsampled_waveform(latest_audio_data.size() / downsample_factor);

        for (size_t i = 0; i < downsampled_waveform.size(); ++i) {
            downsampled_waveform[i] = latest_audio_data[i * downsample_factor];
        }

        // --- GLOBAL SMOOTHING CHECK ---
        // Only apply the buffer-based smoothing if the toggle is ON
        if (enableInterpolation) {
            for (size_t i = 0; i < downsampled_waveform.size(); ++i) {
                if (i < waveform_buffers.size()) {
                    if (downsampled_waveform[i] > 0) {
                        waveform_buffers[i].push_back(downsampled_waveform[i]);
                    }
                    float smoothed_val = std::accumulate(waveform_buffers[i].begin(), waveform_buffers[i].end(), 0.0f) / waveform_buffers[i].size();
                    downsampled_waveform[i] = (waveform_smoothing_factor * smoothed_val) +
                                              (1.0f - waveform_smoothing_factor) * downsampled_waveform[i];
                }
            }
        }

        float bass_energy = 0.0f;
        size_t bass_samples = 10;
        for (size_t i = 0; i < std::min(bass_samples, downsampled_waveform.size()); ++i) {
            bass_energy += std::abs(downsampled_waveform[i]);
        }
        bass_energy /= bass_samples;

        float line_thickness = std::min(MAX_LINE_THICKNESS, bass_energy * MAX_LINE_THICKNESS);

        int num_points = downsampled_waveform.size();
        if (num_points < 2) {
            return;
        }

        float x_step = static_cast<float>(GetScreenWidth()) / (num_points - 1);
        std::vector<Vector2> points;

        for (size_t i = 0; i < num_points - 1; ++i) {
            // Internal interpolation factor: if smoothing is OFF, we use 1.0 to skip extra points
            float interpolation_factor = enableInterpolation ? 0.5f : 1.0f;

            int x0 = static_cast<int>(i * x_step);
            int y0_raw = static_cast<int>(GetScreenHeight() / 2 + downsampled_waveform[i] * (GetScreenHeight() / 2) * waveform_height_scale * control_sensitivity);
            int x1 = static_cast<int>((i + 1) * x_step);
            int y1_raw = static_cast<int>(GetScreenHeight() / 2 + downsampled_waveform[i + 1] * (GetScreenHeight() / 2) * waveform_height_scale * control_sensitivity);

            for (float t = 0.0f; t <= 1.0f; t += interpolation_factor) {
                int x = static_cast<int>(lerp(x0, x1, t));
                int y = static_cast<int>(lerp(y0_raw, y1_raw, t));
                points.push_back({ static_cast<float>(x), static_cast<float>(y) });
            }
        }

        points.push_back({ static_cast<float>((num_points - 1) * x_step),
                           static_cast<float>(GetScreenHeight() / 2 + downsampled_waveform.back() * (GetScreenHeight() / 2) * waveform_height_scale * control_sensitivity) });

        for (size_t i = 0; i < points.size() - 1; ++i) {
            DrawLineEx(points[i], points[i + 1], line_thickness, orbColor);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in drawWaveform: " << e.what() << std::endl;
    }
}