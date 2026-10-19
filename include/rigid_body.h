#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include <cglm/cglm.h>
#include <stdbool.h>

#define MAX_CUBES 100

typedef struct {
    vec3 position;          // Позиція центру мас
    vec3 velocity;          // Лінійна швидкість
    vec3 force;             // Сума сил

    mat4 orientation;       // Матриця орієнтації
    vec3 angularVelocity;   // Кутова швидкість
    vec3 torque;            // Крутний момент

    float mass;             // Маса
    float invMass;          // Обернена маса (1 / mass)
    bool active;
} RigidBody;

extern RigidBody cubes[MAX_CUBES];
extern int cubeCount;
extern bool mousePressedLast;

void rigid_body_init_all(void);
void rigid_body_spawn(vec3 pos, vec3 initialVelocity);

#endif // RIGID_BODY_H