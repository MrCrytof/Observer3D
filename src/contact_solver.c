#include "contact_solver.h"
#include "rigid_body.h"
#include <math.h>
#include <stdio.h>

void contact_solver_solve(ContactManifold *manifolds, int manifoldCount, float deltaTime) {
    int iterations = 10;

    for (int iter = 0; iter < iterations; iter++) {
        for (int m = 0; m < manifoldCount; m++) {
            ContactManifold *manifold = &manifolds[m];
            RigidBody *bA = &bodies[manifold->bodyA];
            RigidBody *bB = (manifold->bodyB >= 0) ? &bodies[manifold->bodyB] : NULL;

            for (int c = 0; c < manifold->contactCount; c++) {
                ContactPoint *cp = &manifold->contacts[c];

                vec3 rA;
                glm_vec3_sub(cp->point, bA->position, rA);

                vec3 rB = {0.0f, 0.0f, 0.0f};
                if (bB) {
                    glm_vec3_sub(cp->point, bB->position, rB);
                }

                vec3 velA, crossA;
                glm_cross(bA->angularVelocity, rA, crossA);
                glm_vec3_add(bA->velocity, crossA, velA);

                vec3 velB = {0.0f, 0.0f, 0.0f};
                if (bB) {
                    vec3 crossB;
                    glm_cross(bB->angularVelocity, rB, crossB);
                    glm_vec3_add(bB->velocity, crossB, velB);
                }

                vec3 relativeVel;
                glm_vec3_sub(velA, velB, relativeVel);

                float normalVelocity = glm_vec3_dot(relativeVel, cp->normal);

                if (normalVelocity > 0.0f) continue;

                float restitution = fminf(bA->restitution, bB ? bB->restitution : 0.0f);
                float impulseNumerator = -(1.0f + restitution) * normalVelocity;

                float invMassSum = bA->invMass;

                vec3 rA_cross_n, rA_invI;
                glm_cross(rA, cp->normal, rA_cross_n);
                glm_mat3_mulv(bA->invWorldInertiaTensor, rA_cross_n, rA_invI);
                vec3 termA;
                glm_cross(rA_invI, rA, termA);
                float angularComponentA = glm_vec3_dot(cp->normal, termA);

                float angularComponentB = 0.0f;
                if (bB) {
                    invMassSum += bB->invMass;
                    vec3 rB_cross_n, rB_invI;
                    glm_cross(rB, cp->normal, rB_cross_n);
                    glm_mat3_mulv(bB->invWorldInertiaTensor, rB_cross_n, rB_invI);
                    vec3 termB;
                    glm_cross(rB_invI, rB, termB);
                    angularComponentB = glm_vec3_dot(cp->normal, termB);
                }

                float denominator = invMassSum + angularComponentA + angularComponentB;
                if (denominator <= 0.0001f) continue;

                float deltaImpulse = impulseNumerator / denominator;

                float oldImpulse = cp->normalImpulse;
                cp->normalImpulse = fmaxf(oldImpulse + deltaImpulse, 0.0f);
                deltaImpulse = cp->normalImpulse - oldImpulse;

                // Застосування лінійних імпульсів
                vec3 linearImpulseA;
                glm_vec3_scale(cp->normal, deltaImpulse * bA->invMass, linearImpulseA);
                glm_vec3_add(bA->velocity, linearImpulseA, bA->velocity);

                // Застосування кутових імпульсів для A
                vec3 rA_cross_norm_scaled;
                glm_vec3_scale(rA_cross_n, deltaImpulse, rA_cross_norm_scaled);
                vec3 angDeltaA;
                glm_mat3_mulv(bA->invWorldInertiaTensor, rA_cross_norm_scaled, angDeltaA);
                glm_vec3_add(bA->angularVelocity, angDeltaA, bA->angularVelocity);

                if (bB) {
                    vec3 linearImpulseB;
                    glm_vec3_scale(cp->normal, deltaImpulse * bB->invMass, linearImpulseB);
                    glm_vec3_sub(bB->velocity, linearImpulseB, bB->velocity);

                    vec3 rB_cross_n;
                    glm_cross(rB, cp->normal, rB_cross_n);
                    vec3 rB_cross_norm_scaled;
                    glm_vec3_scale(rB_cross_n, deltaImpulse, rB_cross_norm_scaled);
                    vec3 angDeltaB;
                    glm_mat3_mulv(bB->invWorldInertiaTensor, rB_cross_norm_scaled, angDeltaB);
                    glm_vec3_sub(bB->angularVelocity, angDeltaB, bB->angularVelocity);
                }

                // Позиційна корекція (Baumgarte)
                float allowedPenetration = 0.001f;
                float baumgarte = 0.2f; // Зменшено для більшої плавності
                if (cp->penetration > allowedPenetration) {
                    float positionCorrection = (baumgarte / deltaTime) * (cp->penetration - allowedPenetration);

                    float totalInvMass = bA->invMass + (bB ? bB->invMass : 0.0f);
                    if (totalInvMass > 0.0f) {
                        if (bA->type == BODY_TYPE_DYNAMIC) {
                            vec3 correctionA;
                            glm_vec3_scale(cp->normal, positionCorrection * (bA->invMass / totalInvMass), correctionA);
                            glm_vec3_add(bA->position, correctionA, bA->position);
                        }
                        if (bB && bB->type == BODY_TYPE_DYNAMIC) {
                            vec3 correctionB;
                            glm_vec3_scale(cp->normal, -positionCorrection * (bB->invMass / totalInvMass), correctionB);
                            glm_vec3_add(bB->position, correctionB, bB->position);
                        }
                    }
                }
            }
        }
    }
}