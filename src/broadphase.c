#include "broadphase.h"
#include "rigid_body.h"
#include <math.h>

// Допоміжна функція для отримання AABB коробки або сфери у світових координатах
static void get_collider_aabb(const Collider *c, vec3 minOut, vec3 maxOut) {
    const RigidBody *b = &bodies[c->bodyIndex];

    if (c->type == COLLIDER_BOX) {
        // Спрощений консервативний AABB для коробки з урахуванням орієнтації
        // Для точного розрахунку беремо радіус описаної сфери або максимальні габарити
        float maxDim = fmaxf(c->box.halfSizes[0], fmaxf(c->box.halfSizes[1], c->box.halfSizes[2])) * 1.732f;
        
        minOut[0] = b->position[0] - maxDim;
        minOut[1] = b->position[1] - maxDim;
        minOut[2] = b->position[2] - maxDim;

        maxOut[0] = b->position[0] + maxDim;
        maxOut[1] = b->position[1] + maxDim;
        maxOut[2] = b->position[2] + maxDim;
    } else if (c->type == COLLIDER_SPHERE) {
        float r = c->sphere.radius;
        minOut[0] = b->position[0] - r;
        minOut[1] = b->position[1] - r;
        minOut[2] = b->position[2] - r;

        maxOut[0] = b->position[0] + r;
        maxOut[1] = b->position[1] + r;
        maxOut[2] = b->position[2] + r;
    }
}

// Перевірка перетину двох AABB
static bool aabb_intersect(vec3 minA, vec3 maxA, vec3 minB, vec3 maxB) {
    return (minA[0] <= maxB[0] && maxA[0] >= minB[0]) &&
           (minA[1] <= maxB[1] && maxA[1] >= minB[1]) &&
           (minA[2] <= maxB[2] && maxA[2] >= minB[2]);
}

int broadphase_collect_pairs(PotentialPair *pairs, int maxPairs) {
    int pairCount = 0;

    for (int i = 0; i < colliderCount; i++) {
        for (int j = i + 1; j < colliderCount; j++) {
            Collider *cA = &colliders[i];
            Collider *cB = &colliders[j];

            // Не перевіряємо тіла, які належать одному об'єкту або обидва статики
            if (cA->bodyIndex == cB->bodyIndex) continue;
            
            RigidBody *bA = &bodies[cA->bodyIndex];
            RigidBody *bB = &bodies[cB->bodyIndex];

            if (!bA->active && !bB->active) continue;
            if (bA->type == BODY_TYPE_STATIC && bB->type == BODY_TYPE_STATIC) continue;

            vec3 minA, maxA, minB, maxB;
            get_collider_aabb(cA, minA, maxA);
            get_collider_aabb(cB, minB, maxB);

            if (aabb_intersect(minA, maxA, minB, maxB)) {
                if (pairCount < maxPairs) {
                    pairs[pairCount].bodyA = cA->bodyIndex;
                    pairs[pairCount].bodyB = cB->bodyIndex;
                    pairCount++;
                }
            }
        }
    }

    return pairCount;
}