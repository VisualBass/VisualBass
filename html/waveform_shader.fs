// waveform_shader.fs
#version 330

in vec2 fragTexCoord;
in vec4 vertexColor;

uniform float hueShift;
uniform float glowValue;

out vec4 finalColor;

void main() {
    // Convert the hue shift to RGB colors using a simple HSV to RGB function
    float r = mod(hueShift + 0.1, 1.0);
    float g = mod(hueShift + 0.2, 1.0);
    float b = mod(hueShift + 0.3, 1.0);

    // Apply a simple glow effect by modifying the brightness based on glowValue
    r = clamp(r * glowValue, 0.0, 1.0);
    g = clamp(g * glowValue, 0.0, 1.0);
    b = clamp(b * glowValue, 0.0, 1.0);

    finalColor = vec4(r, g, b, 1.0);
}
