#include <math.h>

#include "cglp.h"
#include "machineDependent.h"
#include "random.h"
#include "vector.h"

typedef struct {
  Vector pos;
  Vector vel;
  int ticks;
  int colorIndex;
} Particle;

#define MAX_PARTICLE_COUNT 64
static Particle particles[MAX_PARTICLE_COUNT];
int particleIndex = 0;
static Random particleRandom;

void initParticle() {
  setRandomSeedWithTime(&particleRandom);
  for (int i = 0; i < MAX_PARTICLE_COUNT; i++) {
    particles[i].ticks = -1;
  }
}

void addParticle(float x, float y, float count, float speed, float angle,
                 float angleWidth) {
  if (color == TRANSPARENT) {
    return;
  }
  if (count < 1) {
    if (getRandom(&particleRandom, 0, 1) > count) {
      return;
    }
    count = 1;
  }
  for (int i = 0; i < count; i++) {
    float a =
        angle + getRandom(&particleRandom, 0, angleWidth) - angleWidth / 2;
    Particle *p = &particles[particleIndex];
    vectorSet(&p->pos, x, y);
    rotate(vectorSet(&p->vel, speed * getRandom(&particleRandom, 0.5f, 1), 0),
           a);
    p->ticks =
        clamp(getRandom(&particleRandom, 10, 20) + sqrtf(fabsf(speed)), 10, 60);
    p->colorIndex = color;
    particleIndex++;
    if (particleIndex >= MAX_PARTICLE_COUNT) {
      particleIndex = 0;
    }
  }
}

void updateParticles() {
  for (int i = 0; i < MAX_PARTICLE_COUNT; i++) {
    Particle *p = &particles[i];
    if (p->ticks < 0) {
      continue;
    }
    vectorAdd(&p->pos, p->vel.x, p->vel.y);
    vectorMul(&p->vel, 0.98f);
    ColorRgb *rgb = &colorRgbs[p->colorIndex];
    md_drawRect(p->pos.x, p->pos.y, 1, 1, rgb->r, rgb->g, rgb->b);
    p->ticks--;
  }
}
