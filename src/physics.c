#include "physics.h"
#include <string.h>

void physics_init(void) {
    rigid_body_init_system();
    collider_init_system();
}

void physics_update(float deltaTime, float gravity) {
    // 1. Оновлення світових тензорів інерції для всіх активних тіл
    for (int i = 0; i < bodyCount; i++) {
        if (!bodies[i].active) continue;
        rigid_body_update_world_inertia(&bodies[i]);
    }

    // 2. Застосування гравітації до сил
    for (int i = 0; i < bodyCount; i++) {
        RigidBody *b = &bodies[i];
        if (!b->active || b->type == BODY_TYPE_STATIC) continue;

        vec3 gravityForce = {0.0f, -b->mass * gravity, 0.0f};
        glm_vec3_copy(gravityForce, b->force);
        glm_vec3_zero(b->torque);
    }

    // 3. Інтегрування рухів (Semi-Implicit Euler) ПЕРЕД пошуком колізій
    for (int i = 0; i < bodyCount; i++) {
        RigidBody *b = &bodies[i];
        if (!b->active || b->type == BODY_TYPE_STATIC) continue;

        // Лінійне інтегрування
        vec3 acceleration;
        glm_vec3_scale(b->force, b->invMass, acceleration);

        vec3 deltaVel;
        glm_vec3_scale(acceleration, deltaTime, deltaVel);
        glm_vec3_add(b->velocity, deltaVel, b->velocity);

        vec3 deltaPos;
        glm_vec3_scale(b->velocity, deltaTime, deltaPos);
        glm_vec3_add(b->position, deltaPos, b->position);

        // Кутове інтегрування
        vec3 angularAcc;
        glm_mat3_mulv(b->invWorldInertiaTensor, b->torque, angularAcc);

        vec3 deltaAngVel;
        glm_vec3_scale(angularAcc, deltaTime, deltaAngVel);
        glm_vec3_add(b->angularVelocity, deltaAngVel, b->angularVelocity);

        float angle = glm_vec3_norm(b->angularVelocity) * deltaTime;
        if (angle > 0.00001f) {
            vec3 axis;
            glm_vec3_copy(b->angularVelocity, axis);
            glm_vec3_normalize(axis);

            versor rotQuat;
            glm_quatv(rotQuat, angle, axis);
            glm_quat_mul(rotQuat, b->orientation, b->orientation);
            glm_quat_normalize(b->orientation);
        }

        // Демпфування
        glm_vec3_scale(b->velocity, 0.995f, b->velocity);
        glm_vec3_scale(b->angularVelocity, 0.99f, b->angularVelocity);
    }

    // 4. Широка фаза (Broad Phase): пошук потенційних пар після переміщення
    PotentialPair pairs[MAX_PAIRS];
    int pairCount = broadphase_collect_pairs(pairs, MAX_PAIRS);

    // 5. Вузька фаза (Narrow Phase): генерація контактних маніфолдів
    collision_detect_all(pairs, pairCount);

    // 6. Вирішення контактів (Contact Solver): виправлення швидкостей та позицій
    contact_solver_solve(manifolds, manifoldCount, deltaTime);
}