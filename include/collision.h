#ifndef COLLISION_H
#define COLLISION_H

#include <cglm/cglm.h>
#include "rigid_body.h"
#include "collider.h"
#include "broadphase.h"

typedef struct {
    vec3 point;         // Точка контакту у світових координатах
    vec3 normal;        // Нормаль від тіла A до тіла B
    float penetration;  // Глубина проникнення

    // Накопичені імпульси для warm starting і вирішення контактів
    float normalImpulse;
    float tangentImpulse1;
    float tangentImpulse2;
} ContactPoint;

#define MAX_CONTACTS_PER_PAIR 8

typedef struct {
    int bodyA;
    int bodyB;
    ContactPoint contacts[MAX_CONTACTS_PER_PAIR];
    int contactCount;
    vec3 normal;        // Загальна нормаль контакту
} ContactManifold;

#define MAX_MANIFOLDS 256

extern ContactManifold manifolds[MAX_MANIFOLDS];
extern int manifoldCount;

void collision_detect_all(const PotentialPair *pairs, int pairCount);

#endif // COLLISION_H