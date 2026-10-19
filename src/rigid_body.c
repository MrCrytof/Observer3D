#include "rigid_body.h"

RigidBody cubes[MAX_CUBES];
int cubeCount = 0;
bool mousePressedLast = false;

void rigid_body_init_all(void) {
    cubeCount = 0;
    mousePressedLast = false;
    for (int i = 0; i < MAX_CUBES; i++) {
        cubes[i].active = false;
    }
}

void rigid_body_spawn(vec3 pos, vec3 initialVelocity) {
    if (cubeCount >= MAX_CUBES) return;

    glm_vec3_copy(pos, cubes[cubeCount].position);
    glm_vec3_copy(initialVelocity, cubes[cubeCount].velocity);
    glm_vec3_zero(cubes[cubeCount].force);

    cubes[cubeCount].mass = 2.0f;
    cubes[cubeCount].invMass = 1.0f / cubes[cubeCount].mass;

    glm_mat4_identity(cubes[cubeCount].orientation);
    
    // Випадковий початковий імпульс обертання
    cubes[cubeCount].angularVelocity[0] = 3.0f;
    cubes[cubeCount].angularVelocity[1] = 5.0f;
    cubes[cubeCount].angularVelocity[2] = 2.0f;
    
    glm_vec3_zero(cubes[cubeCount].torque);

    cubes[cubeCount].active = true;
    cubeCount++;
}