#include "physics.h"
#include "rigid_body.h"
#include "collision.h"
#include <cglm/cglm.h>

#define MAX_CONTACTS 256

void physics_update(float deltaTime, float gravity) {
    // 1. Применяем гравитацию и сбрасываем силы
    for (int i = 0; i < cubeCount; i++) {
        if (!cubes[i].active) continue;

        vec3 gravityForce = {0.0f, -cubes[i].mass * gravity, 0.0f};
        glm_vec3_copy(gravityForce, cubes[i].force);
        glm_vec3_zero(cubes[i].torque);
    }

    // 2. Ищем все контакты через отдельный модуль коллизий
    Contact contacts[MAX_CONTACTS];
    int contactCount = collision_detect_boxes(contacts, MAX_CONTACTS);

    // 3. Обрабатываем контакты (решатель сил и трения)
    for (int c = 0; c < contactCount; c++) {
        int a = contacts[c].bodyA;
        int b = contacts[c].bodyB;

        vec3 rA;
        glm_vec3_sub(contacts[c].point, cubes[a].position, rA);

        vec3 pointVelA, omegaCrossRA;
        glm_cross(cubes[a].angularVelocity, rA, omegaCrossRA);
        glm_vec3_add(cubes[a].velocity, omegaCrossRA, pointVelA);

        vec3 pointVelB = {0.0f, 0.0f, 0.0f};
        if (b != -1) {
            vec3 rB;
            glm_vec3_sub(contacts[c].point, cubes[b].position, rB);
            vec3 omegaCrossRB;
            glm_cross(cubes[b].angularVelocity, rB, omegaCrossRB);
            glm_vec3_add(cubes[b].velocity, omegaCrossRB, pointVelB);
        }

        // Относительная скорость в точке контакта
        vec3 relativeVel;
        glm_vec3_sub(pointVelA, pointVelB, relativeVel);

        float normalVel = glm_vec3_dot(relativeVel, contacts[c].normal);

        // Сила упругости (Spring + Damping)
        float springK = 500.0f;
        float dampingK = 30.0f;

        vec3 normalForce;
        float normalMagnitude = (contacts[c].penetration * springK) - (normalVel * dampingK);
        if (normalMagnitude < 0.0f) normalMagnitude = 0.0f;

        glm_vec3_scale(contacts[c].normal, normalMagnitude, normalForce);

        // Применяем силы и моменты для тела A
        glm_vec3_add(cubes[a].force, normalForce, cubes[a].force);
        vec3 torqueA;
        glm_cross(rA, normalForce, torqueA);
        glm_vec3_add(cubes[a].torque, torqueA, cubes[a].torque);

        // Трение в плоскости контакта
        vec3 tangentVel;
        vec3 normalComponent;
        glm_vec3_scale(contacts[c].normal, normalVel, normalComponent);
        glm_vec3_sub(relativeVel, normalComponent, tangentVel);

        vec3 frictionForce;
        glm_vec3_scale(tangentVel, -15.0f, frictionForce);
        glm_vec3_add(cubes[a].force, frictionForce, cubes[a].force);

        vec3 frictionTorque;
        glm_cross(rA, frictionForce, frictionTorque);
        glm_vec3_add(cubes[a].torque, frictionTorque, cubes[a].torque);

        // Если второе тело — это другой куб (не платформа), передаем силу и ему по третьему закону Ньютона
        if (b != -1) {
            vec3 forceB;
            glm_vec3_scale(normalForce, -1.0f, forceB);
            glm_vec3_add(cubes[b].force, forceB, cubes[b].force);

            vec3 rB;
            glm_vec3_sub(contacts[c].point, cubes[b].position, rB);
            vec3 torqueB;
            glm_cross(rB, forceB, torqueB);
            glm_vec3_add(cubes[b].torque, torqueB, cubes[b].torque);
        }
    }

    // 4. Интегрирование движения для всех активных тел
    for (int i = 0; i < cubeCount; i++) {
        if (!cubes[i].active) continue;

        // Линейное интегрирование
        vec3 acceleration;
        glm_vec3_scale(cubes[i].force, cubes[i].invMass, acceleration);
        vec3 deltaVel;
        glm_vec3_scale(acceleration, deltaTime, deltaVel);
        glm_vec3_add(cubes[i].velocity, deltaVel, cubes[i].velocity);

        vec3 deltaPos;
        glm_vec3_scale(cubes[i].velocity, deltaTime, deltaPos);
        glm_vec3_add(cubes[i].position, deltaPos, cubes[i].position);

        // Угловое интегрирование
        vec3 angularAcceleration;
        glm_vec3_scale(cubes[i].torque, cubes[i].invMass, angularAcceleration);
        vec3 deltaAngVel;
        glm_vec3_scale(angularAcceleration, deltaTime, deltaAngVel);
        glm_vec3_add(cubes[i].angularVelocity, deltaAngVel, cubes[i].angularVelocity);

        float angle = glm_vec3_norm(cubes[i].angularVelocity) * deltaTime;
        if (angle > 0.0001f) {
            vec3 axis;
            glm_vec3_copy(cubes[i].angularVelocity, axis);
            glm_vec3_normalize(axis);
            glm_rotate(cubes[i].orientation, angle, axis);
        }

        // Общее аэродинамическое затухание
        glm_vec3_scale(cubes[i].angularVelocity, 0.98f, cubes[i].angularVelocity);
        glm_vec3_scale(cubes[i].velocity, 0.99f, cubes[i].velocity);
    }
}