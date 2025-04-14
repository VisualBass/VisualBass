#ifndef FFT_H
#define FFT_H

// Declare InitFFT with parameters
void InitFFT(const float* audioBuffer, int bufferSize);
void CloseFFT();
float ProcessFFT(const float* audioBuffer, int bufferSize);

#endif // FFT_H
