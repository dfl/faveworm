// Quick test to analyze waveform characteristics at different beta values
// Compile: clang++ -std=c++17 -I../src tests/beta_analysis.cpp -o beta_analysis

#include "../src/TestSignalGenerator.h"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

struct WaveStats {
  float min_val = 1e9f;
  float max_val = -1e9f;
  float avg_abs = 0.0f;
  int zero_crossings = 0;
  float avg_segment_dist = 0.0f;
  float max_segment_dist = 0.0f;
  int nan_count = 0;
  int inf_count = 0;
};

WaveStats analyzeAtBeta(double beta, int num_samples = 4096) {
  TestSignalGenerator gen;
  gen.setSampleRate(44100.0);
  gen.setFrequency(80.0);
  gen.setDetune(1.003);
  gen.setBeta(beta);
  gen.setExponent(1);

  std::vector<float> left(num_samples), right(num_samples);
  gen.generate(left.data(), right.data(), num_samples);

  WaveStats stats;
  float prev_left = 0.0f;
  float prev_right = 0.0f;
  float sum_abs = 0.0f;
  float sum_dist = 0.0f;

  for (int i = 0; i < num_samples; ++i) {
    float l = left[i];
    float r = right[i];

    // Check for NaN/Inf
    if (!std::isfinite(l) || !std::isfinite(r)) {
      if (std::isnan(l) || std::isnan(r))
        stats.nan_count++;
      if (std::isinf(l) || std::isinf(r))
        stats.inf_count++;
      continue;
    }

    // Min/max
    stats.min_val = std::min(stats.min_val, std::min(l, r));
    stats.max_val = std::max(stats.max_val, std::max(l, r));

    // Sum for average
    sum_abs += std::abs(l) + std::abs(r);

    // Zero crossings (for left channel)
    if (i > 0 && ((prev_left < 0 && l >= 0) || (prev_left >= 0 && l < 0))) {
      stats.zero_crossings++;
    }

    // XY distance between consecutive samples
    if (i > 0) {
      float dx = l - prev_left;
      float dy = r - prev_right;
      float dist = std::sqrt(dx * dx + dy * dy);
      sum_dist += dist;
      stats.max_segment_dist = std::max(stats.max_segment_dist, dist);
    }

    prev_left = l;
    prev_right = r;
  }

  stats.avg_abs = sum_abs / (2.0f * num_samples);
  stats.avg_segment_dist = sum_dist / (num_samples - 1);

  return stats;
}

void writeWavFile(const char *filename, double beta, int num_samples = 44100) {
  TestSignalGenerator gen;
  gen.setSampleRate(44100.0);
  gen.setFrequency(80.0);
  gen.setDetune(1.003);
  gen.setBeta(beta);
  gen.setExponent(1);

  std::vector<float> left(num_samples), right(num_samples);
  gen.generate(left.data(), right.data(), num_samples);

  // Simple WAV file writer (16-bit stereo)
  std::ofstream out(filename, std::ios::binary);

  // WAV header
  int sample_rate = 44100;
  int bits_per_sample = 16;
  int num_channels = 2;
  int byte_rate = sample_rate * num_channels * bits_per_sample / 8;
  int block_align = num_channels * bits_per_sample / 8;
  int data_size = num_samples * block_align;
  int file_size = 36 + data_size;

  out.write("RIFF", 4);
  out.write(reinterpret_cast<char *>(&file_size), 4);
  out.write("WAVE", 4);
  out.write("fmt ", 4);
  int fmt_size = 16;
  out.write(reinterpret_cast<char *>(&fmt_size), 4);
  short audio_format = 1; // PCM
  out.write(reinterpret_cast<char *>(&audio_format), 2);
  short channels = num_channels;
  out.write(reinterpret_cast<char *>(&channels), 2);
  out.write(reinterpret_cast<char *>(&sample_rate), 4);
  out.write(reinterpret_cast<char *>(&byte_rate), 4);
  short block = block_align;
  out.write(reinterpret_cast<char *>(&block), 2);
  short bits = bits_per_sample;
  out.write(reinterpret_cast<char *>(&bits), 2);
  out.write("data", 4);
  out.write(reinterpret_cast<char *>(&data_size), 4);

  // Write samples
  for (int i = 0; i < num_samples; ++i) {
    short l = static_cast<short>(std::clamp(left[i], -1.0f, 1.0f) * 32767);
    short r = static_cast<short>(std::clamp(right[i], -1.0f, 1.0f) * 32767);
    out.write(reinterpret_cast<char *>(&l), 2);
    out.write(reinterpret_cast<char *>(&r), 2);
  }

  out.close();
  std::cout << "Wrote: " << filename << std::endl;
}

int main() {
  std::cout << std::fixed << std::setprecision(4);
  std::cout << "Beta Analysis - RPM Oscillator Waveform Characteristics\n";
  std::cout << "========================================================\n\n";

  std::vector<double> test_betas = {0.0, 1.0, 2.0,  3.0,  4.0, 5.0,
                                    6.0, 7.0, 10.0, 20.0, 40.0};

  std::cout << std::setw(6) << "Beta" << std::setw(10) << "Min" << std::setw(10)
            << "Max" << std::setw(10) << "AvgAbs" << std::setw(10)
            << "ZeroCross" << std::setw(12) << "AvgDist" << std::setw(12)
            << "MaxDist" << std::setw(8) << "NaN" << std::setw(8) << "Inf"
            << "\n";
  std::cout << std::string(86, '-') << "\n";

  for (double beta : test_betas) {
    WaveStats stats = analyzeAtBeta(beta);
    std::cout << std::setw(6) << beta << std::setw(10) << stats.min_val
              << std::setw(10) << stats.max_val << std::setw(10)
              << stats.avg_abs << std::setw(10) << stats.zero_crossings
              << std::setw(12) << stats.avg_segment_dist << std::setw(12)
              << stats.max_segment_dist << std::setw(8) << stats.nan_count
              << std::setw(8) << stats.inf_count << "\n";
  }

  std::cout << "\nWriting WAV files for beta=4 and beta=40...\n";
  writeWavFile("beta_4.wav", 4.0);
  writeWavFile("beta_40.wav", 40.0);

  return 0;
}
