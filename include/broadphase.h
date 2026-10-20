#ifndef BROADPHASE_H
#define BROADPHASE_H

#include <cglm/cglm.h>
#include "collider.h"

typedef struct {
    int bodyA;
    int bodyB;
} PotentialPair;

#define MAX_PAIRS 1024

// Визначає потенційні пари для зіткнення за допомогою AABB
int broadphase_collect_pairs(PotentialPair *pairs, int maxPairs);

#endif // BROADPHASE_H