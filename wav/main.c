#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_KEYS 88 // Total number of piano keys (including both ends)
#define M_PI 3.14159265358979323846

double piano_freqs[9][12] = {
    // Octave 0
    {16.35, 17.32, 18.35, 19.45, 20.60, 21.83, 23.12, 24.50, 25.96, 27.50,
     29.14, 30.87},
    // Octave 1
    {32.70, 34.65, 36.71, 38.89, 41.20, 43.65, 46.25, 49.00, 51.91, 55.00,
     58.27, 61.74},
    // Octave 2
    {65.41, 69.30, 73.42, 77.78, 82.41, 87.31, 92.50, 98.00, 103.83, 110.00,
     116.54, 123.47},
    // Octave 3
    {130.81, 138.59, 146.83, 155.56, 164.81, 174.61, 185.00, 196.00, 207.65,
     220.00, 233.08, 246.94},
    // Octave 4
    {261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.00, 415.30,
     440.00, 466.16, 493.88},
    // Octave 5
    {523.25, 554.37, 587.33, 622.25, 659.25, 698.46, 739.99, 783.99, 830.61,
     880.00, 932.33, 987.77},
    // Octave 6
    {1046.50, 1108.73, 1174.66, 1244.51, 1318.51, 1396.91, 1479.98, 1567.98,
     1661.22, 1760.00, 1864.66, 1975.53},
    // Octave 7
    {2093.00, 2217.46, 2349.32, 2489.02, 2637.02, 2793.83, 2959.96, 3135.96,
     3322.44, 3520.00, 3729.31, 3951.07},
    // Octave 8 (only C8 exists on an 88-key piano)
    {4186.01, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};

typedef enum Scales {
  MAJOR,
  MINOR,
} Scales;

double random_note_from_scale(int octave, int note, Scales scale_type) {
  switch (scale_type) {
  case MAJOR: {
    int note = rand() % 8;
    switch (note) {
    case 0:
      return piano_freqs[octave][note];
      break;
    case 1:
      return piano_freqs[octave][note + 2];
      break;
    case 2:
      return piano_freqs[octave][note + 4];
      break;
    case 3:
      return piano_freqs[octave][note + 5];
      break;
    case 4:
      return piano_freqs[octave][note + 7];
      break;
    case 5:
      return piano_freqs[octave][note + 9];
      break;
    case 6:
      return piano_freqs[octave][note + 11];
      break;
    case 7:
      return piano_freqs[octave][note + 12];
      break;
    }
  } break;
  case MINOR:
    assert(false);
    break;
  }

  assert(false);
}

enum oscilators { SINE, SQUARE, TRIANGLE, SAW_TOOTH, NOISE } oscilators;

typedef struct Synthesizer {
  double osc;
  double amplitudes[5]; // We're going to assume these amplitudes are normalized
  double panning[5];    // Panning is [0,1], 0 being all the way left, 1 all the
                        // way right
} Synthesizer;

constexpr double two_pi = M_PI * 2;

float sine(double x) { return sin(x); }

float square(double x) { return signbit(sin(x)); }

float triangle(double x) {
  return 4 * fabs((x / two_pi) - floor((x / two_pi) + 0.5)) - 1;
}

float saw_tooth(double x) {
  return 2 * ((x / two_pi) - floor(0.5 + (x / two_pi)));
}

float noise(double x) { return rand() / ((double)RAND_MAX / 2) - 1; }

typedef struct Sample {
  float a;
  float b;
} Sample;

Sample sample(Synthesizer *s, double dt) {
  float raw_samples[5] = {0};
  raw_samples[SINE] = sine(s->osc);
  raw_samples[SQUARE] = square(s->osc);
  raw_samples[TRIANGLE] = triangle(s->osc);
  raw_samples[SAW_TOOTH] = saw_tooth(s->osc);
  raw_samples[NOISE] = noise(s->osc);

  s->osc += dt;

  Sample sample = {0};
  sample.a += raw_samples[SINE] * s->amplitudes[SINE] * (1 - s->panning[SINE]);
  sample.b += raw_samples[SINE] * s->amplitudes[SINE] * s->panning[SINE];
  sample.a +=
      raw_samples[SQUARE] * s->amplitudes[SQUARE] * (1 - s->panning[SQUARE]);
  sample.b += raw_samples[SQUARE] * s->amplitudes[SQUARE] * s->panning[SQUARE];
  sample.a += raw_samples[TRIANGLE] * s->amplitudes[TRIANGLE] *
              (1 - s->panning[TRIANGLE]);
  sample.b +=
      raw_samples[TRIANGLE] * s->amplitudes[TRIANGLE] * s->panning[TRIANGLE];
  sample.a += raw_samples[SAW_TOOTH] * s->amplitudes[SAW_TOOTH] *
              (1 - s->panning[SAW_TOOTH]);
  sample.b +=
      raw_samples[SAW_TOOTH] * s->amplitudes[SAW_TOOTH] * s->panning[SAW_TOOTH];
  sample.a +=
      raw_samples[NOISE] * s->amplitudes[NOISE] * (1 - s->panning[NOISE]);
  sample.b += raw_samples[NOISE] * s->amplitudes[NOISE] * s->panning[NOISE];

  return sample;
}

int main() {
  const int seconds = 4;
  const int num_samples = seconds * 44100;

  int format_size = 16;
  short format_type = 3;
  short num_channels = 2;
  int sample_rate = 44100;
  short bits_per_sample = 32;
  int bits_per_second = (sample_rate * bits_per_sample * num_channels) / 8;
  short bytes_across_channels = 8;
  int data_size = (num_samples * num_channels * bits_per_sample) / 8;
  int file_size = 36 + data_size;

  printf("RIFF");
  fwrite(&file_size, 4, 1, stdout);
  printf("WAVE");
  printf("fmt ");
  fwrite(&format_size, 4, 1, stdout);
  fwrite(&format_type, 2, 1, stdout);
  fwrite(&num_channels, 2, 1, stdout);
  fwrite(&sample_rate, 4, 1, stdout);
  fwrite(&bits_per_second, 4, 1, stdout);
  fwrite(&bytes_across_channels, 2, 1, stdout);
  fwrite(&bits_per_sample, 2, 1, stdout);
  printf("data");
  fwrite(&data_size, 4, 1, stdout);

  Synthesizer s = {
      .amplitudes = {0.23, 0.23, 0.23, 0.23, 0.05},
      .panning = {0.5, 0.5, 0.5, 0.5, 1},
  };

  double one_hertz = M_PI * 2 / sample_rate;
  double bpm = 60.0 / 60.0;
  double samples_per_beat = sample_rate / bpm;
  double samples_per_sixteenth = samples_per_beat / 4.0;
  int total_sixteenth_notes = num_samples / samples_per_sixteenth;

  for (size_t note = 0; note < total_sixteenth_notes; note++) {
    double increment = random_note_from_scale(4, 0, MAJOR);

    for (size_t n = 0; n < samples_per_sixteenth; n++) {
      Sample samp = sample(&s, increment * one_hertz);

      fwrite(&samp.a, 4, 1, stdout);
      fwrite(&samp.b, 4, 1, stdout);
    }
  }
}
