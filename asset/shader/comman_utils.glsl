#ifndef COMMON_UTILS_GLSL
#define COMMON_UTILS_GLSL

#define PI 3.1415926535897932384626

#define NUM_SAMPLES 128
#define NUM_RINGS 4

vec2 poissonDisk[NUM_SAMPLES];

// Generate a random vec2 sequence with poisson disk sampling
void generateRandom(const in vec2 seed)
{

  float ANGLE_STEP = 2.0 * PI * float(NUM_RINGS) / float(NUM_SAMPLES);
  float INV_NUM_SAMPLES = 1.0 / float(NUM_SAMPLES);

  float angle = sin(seed.x + seed.y) * 2.0 * PI;
  float radius = INV_NUM_SAMPLES;
  float radiusStep = radius;

  for(int i = 0; i < NUM_SAMPLES; i++) {
    poissonDisk[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);
    radius += radiusStep;
    angle += ANGLE_STEP;
  }
}

#endif