#include "rigid_body.h"
#include <string.h>
#include <math.h>

RigidBody bodies[MAX_BODIES];
int bodyCount = 0;

void rigid_body_init_system(void) {
    memset(bodies, 0, sizeof(bodies));
    bodyCount = 0;
}

/**
 * @brief Обчислює світовий тензор інерції для тіла на основі його кватерниона орієнтації.
 * Формула: I_world = R * I_local * R^T
 */
void rigid_body_update_world_inertia(RigidBody *body) {
    if (body->type == BODY_TYPE_STATIC) {
        glm_mat3_zero(body->invWorldInertiaTensor);
        return;
    }

    mat4 rotMat4;
    glm_quat_mat4(body->orientation, rotMat4);

    mat3 rotMat3;
    glm_mat4_pick3(rotMat4, rotMat3);

    mat3 rotMat3Trans;
    glm_mat3_transpose_to(rotMat3, rotMat3Trans);

    // Temp = R * I_local_inv
    mat3 temp;
    glm_mat3_mul(rotMat3, body->invLocalInertiaTensor, temp);

    // I_world_inv = Temp * R^T
    glm_mat3_mul(temp, rotMat3Trans, body->invWorldInertiaTensor);
}

int rigid_body_create(BodyType type, float mass, vec3 pos) {
    if (bodyCount >= MAX_BODIES) return -1;

    int id = bodyCount++;
    RigidBody *b = &bodies[id];

    b->type = type;
    b->active = true;

    glm_vec3_copy(pos, b->position);
    glm_quat_identity(b->orientation);

    glm_vec3_zero(b->velocity);
    glm_vec3_zero(b->angularVelocity);
    glm_vec3_zero(b->force);
    glm_vec3_zero(b->torque);

    b->mass = (type == BODY_TYPE_STATIC) ? 0.0f : mass;
    b->invMass = (type == BODY_TYPE_STATIC || mass == 0.0f) ? 0.0f : (1.0f / mass);

    b->friction = 0.4f;
    b->restitution = 0.2f;

    // Для куба розміром 1x1x1 тензор інерції обчислюється за формулою: I = 1/6 * m * side^2
    float side = 1.0f;
    float inertiaVal = (type == BODY_TYPE_STATIC) ? 0.0f : (1.0f / 6.0f) * mass * (side * side);

    glm_mat3_identity(b->localInertiaTensor);
    if (type != BODY_TYPE_STATIC && inertiaVal > 0.0f) {
        b->localInertiaTensor[0][0] = inertiaVal;
        b->localInertiaTensor[1][1] = inertiaVal;
        b->localInertiaTensor[2][2] = inertiaVal;
        glm_mat3_inv(b->localInertiaTensor, b->invLocalInertiaTensor);
    } else {
        glm_mat3_zero(b->localInertiaTensor);
        glm_mat3_zero(b->invLocalInertiaTensor);
    }

    rigid_body_update_world_inertia(b);

    return id;
}