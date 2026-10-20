#include "collision.h"
#include <string.h>
#include <math.h>

ContactManifold manifolds[MAX_MANIFOLDS];
int manifoldCount = 0;

static const vec3 LOCAL_BOX_VERTICES[8] = {
    {-0.5f, -0.5f, -0.5f},
    { 0.5f, -0.5f, -0.5f},
    { 0.5f,  0.5f, -0.5f},
    {-0.5f,  0.5f, -0.5f},
    {-0.5f, -0.5f,  0.5f},
    { 0.5f, -0.5f,  0.5f},
    { 0.5f,  0.5f,  0.5f},
    {-0.5f,  0.5f,  0.5f}
};

static void collide_box_box(int indexA, int indexB, Collider *cA, Collider *cB, ContactManifold *m) {
    RigidBody *bA = &bodies[indexA];
    RigidBody *bB = &bodies[indexB];

    mat4 rotA, invRotA, rotB, invRotB;
    glm_quat_mat4(bA->orientation, rotA);
    glm_mat4_inv(rotA, invRotA);
    glm_quat_mat4(bB->orientation, rotB);
    glm_mat4_inv(rotB, invRotB);

    m->bodyA = indexA;
    m->bodyB = indexB;
    m->contactCount = 0;

    // 1. Перевірка вершин куба A всередині куба B (нормаль від B до A)
    for (int v = 0; v < 8; v++) {
        if (m->contactCount >= MAX_CONTACTS_PER_PAIR) break;

        vec3 scaledLocal;
        scaledLocal[0] = LOCAL_BOX_VERTICES[v][0] * (cA->box.halfSizes[0] * 2.0f);
        scaledLocal[1] = LOCAL_BOX_VERTICES[v][1] * (cA->box.halfSizes[1] * 2.0f);
        scaledLocal[2] = LOCAL_BOX_VERTICES[v][2] * (cA->box.halfSizes[2] * 2.0f);

        vec3 worldVertex;
        glm_mat4_mulv3(rotA, scaledLocal, 1.0f, worldVertex);
        glm_vec3_add(worldVertex, bA->position, worldVertex);

        vec3 localToB;
        glm_vec3_sub(worldVertex, bB->position, localToB);
        vec3 vertexInB;
        glm_mat4_mulv3(invRotB, localToB, 1.0f, vertexInB);

        vec3 halfB;
        glm_vec3_copy(cB->box.halfSizes, halfB);

        if (fabsf(vertexInB[0]) <= halfB[0] &&
            fabsf(vertexInB[1]) <= halfB[1] &&
            fabsf(vertexInB[2]) <= halfB[2]) {

            float dists[6] = {
                halfB[0] - vertexInB[0], vertexInB[0] - (-halfB[0]),
                halfB[1] - vertexInB[1], vertexInB[1] - (-halfB[1]),
                halfB[2] - vertexInB[2], vertexInB[2] - (-halfB[2])
            };

            float minPen = dists[0];
            int minIdx = 0;
            for (int d = 1; d < 6; d++) {
                if (dists[d] < minPen) {
                    minPen = dists[d];
                    minIdx = d;
                }
            }

            vec3 localNorm = {0.0f, 0.0f, 0.0f};
            if (minIdx == 0) localNorm[0] = 1.0f;
            else if (minIdx == 1) localNorm[0] = -1.0f;
            else if (minIdx == 2) localNorm[1] = 1.0f;
            else if (minIdx == 3) localNorm[1] = -1.0f;
            else if (minIdx == 4) localNorm[2] = 1.0f;
            else if (minIdx == 5) localNorm[2] = -1.0f;

            vec3 worldNorm;
            glm_mat4_mulv3(rotB, localNorm, 0.0f, worldNorm);
            glm_vec3_normalize(worldNorm);

            ContactPoint *cp = &m->contacts[m->contactCount];
            glm_vec3_copy(worldVertex, cp->point);
            glm_vec3_copy(worldNorm, cp->normal);
            cp->penetration = minPen;
            cp->normalImpulse = 0.0f;
            cp->tangentImpulse1 = 0.0f;
            cp->tangentImpulse2 = 0.0f;

            glm_vec3_copy(worldNorm, m->normal);
            m->contactCount++;
        }
    }

    // 2. Перевірка вершин куба B всередині куба A (нормаль від B до A, тобто інвертована від A)
    for (int v = 0; v < 8; v++) {
        if (m->contactCount >= MAX_CONTACTS_PER_PAIR) break;

        vec3 scaledLocal;
        scaledLocal[0] = LOCAL_BOX_VERTICES[v][0] * (cB->box.halfSizes[0] * 2.0f);
        scaledLocal[1] = LOCAL_BOX_VERTICES[v][1] * (cB->box.halfSizes[1] * 2.0f);
        scaledLocal[2] = LOCAL_BOX_VERTICES[v][2] * (cB->box.halfSizes[2] * 2.0f);

        vec3 worldVertex;
        glm_mat4_mulv3(rotB, scaledLocal, 1.0f, worldVertex);
        glm_vec3_add(worldVertex, bB->position, worldVertex);

        vec3 localToA;
        glm_vec3_sub(worldVertex, bA->position, localToA);
        vec3 vertexInA;
        glm_mat4_mulv3(invRotA, localToA, 1.0f, vertexInA);

        vec3 halfA;
        glm_vec3_copy(cA->box.halfSizes, halfA);

        if (fabsf(vertexInA[0]) <= halfA[0] &&
            fabsf(vertexInA[1]) <= halfA[1] &&
            fabsf(vertexInA[2]) <= halfA[2]) {

            float dists[6] = {
                halfA[0] - vertexInA[0], vertexInA[0] - (-halfA[0]),
                halfA[1] - vertexInA[1], vertexInA[1] - (-halfA[1]),
                halfA[2] - vertexInA[2], vertexInA[2] - (-halfA[2])
            };

            float minPen = dists[0];
            int minIdx = 0;
            for (int d = 1; d < 6; d++) {
                if (dists[d] < minPen) {
                    minPen = dists[d];
                    minIdx = d;
                }
            }

            vec3 localNorm = {0.0f, 0.0f, 0.0f};
            if (minIdx == 0) localNorm[0] = 1.0f;
            else if (minIdx == 1) localNorm[0] = -1.0f;
            else if (minIdx == 2) localNorm[1] = 1.0f;
            else if (minIdx == 3) localNorm[1] = -1.0f;
            else if (minIdx == 4) localNorm[2] = 1.0f;
            else if (minIdx == 5) localNorm[2] = -1.0f;

            vec3 worldNorm;
            glm_mat4_mulv3(rotA, localNorm, 0.0f, worldNorm);
            glm_vec3_normalize(worldNorm);
            glm_vec3_negate(worldNorm); // Нормаль повинна дивитися від B до A

            ContactPoint *cp = &m->contacts[m->contactCount];
            glm_vec3_copy(worldVertex, cp->point);
            glm_vec3_copy(worldNorm, cp->normal);
            cp->penetration = minPen;
            cp->normalImpulse = 0.0f;
            cp->tangentImpulse1 = 0.0f;
            cp->tangentImpulse2 = 0.0f;

            glm_vec3_copy(worldNorm, m->normal);
            m->contactCount++;
        }
    }
}

void collision_detect_all(const PotentialPair *pairs, int pairCount) {
    manifoldCount = 0;

    for (int i = 0; i < pairCount; i++) {
        int a = pairs[i].bodyA;
        int b = pairs[i].bodyB;

        Collider *cA = NULL;
        Collider *cB = NULL;

        for (int c = 0; c < colliderCount; c++) {
            if (colliders[c].bodyIndex == a) cA = &colliders[c];
            if (colliders[c].bodyIndex == b) cB = &colliders[c];
        }

        if (!cA || !cB) continue;

        if (cA->type == COLLIDER_BOX && cB->type == COLLIDER_BOX) {
            if (manifoldCount < MAX_MANIFOLDS) {
                ContactManifold *m = &manifolds[manifoldCount];
                collide_box_box(a, b, cA, cB, m);
                if (m->contactCount > 0) {
                    manifoldCount++;
                }
            }
        }
    }
}