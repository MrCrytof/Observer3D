#include "collision.h"
#include <string.h>
#include <math.h>

// Вспомогательная функция для проверки диапазонов осей
int collision_detect_boxes(Contact *contacts, int maxContacts) {
    int contactCount = 0;

    // Платформа (фиксированная плоскость на Y = 0)
    float platformTopY = 0.0f;
    float platformMinX = -10.0f;
    float platformMaxX = 10.0f;
    float platformMinZ = -10.0f;
    float platformMaxZ = 10.0f;

    vec3 localVertices[8] = {
        {-0.5f, -0.5f, -0.5f},
        { 0.5f, -0.5f, -0.5f},
        { 0.5f,  0.5f, -0.5f},
        {-0.5f,  0.5f, -0.5f},
        {-0.5f, -0.5f,  0.5f},
        { 0.5f, -0.5f,  0.5f},
        { 0.5f,  0.5f,  0.5f},
        {-0.5f,  0.5f,  0.5f}
    };

    // 1. Коллизии кубов с платформой
    for (int i = 0; i < cubeCount; i++) {
        if (!cubes[i].active) continue;

        for (int v = 0; v < 8; v++) {
            vec3 worldVertex;
            glm_mat4_mulv3(cubes[i].orientation, localVertices[v], 1.0f, worldVertex);
            glm_vec3_add(worldVertex, cubes[i].position, worldVertex);

            bool withinX = (worldVertex[0] >= platformMinX) && (worldVertex[0] <= platformMaxX);
            bool withinZ = (worldVertex[2] >= platformMinZ) && (worldVertex[2] <= platformMaxZ);

            if (withinX && withinZ && (worldVertex[1] < platformTopY)) {
                if (contactCount < maxContacts) {
                    glm_vec3_copy(worldVertex, contacts[contactCount].point);
                    contacts[contactCount].normal[0] = 0.0f;
                    contacts[contactCount].normal[1] = 1.0f;
                    contacts[contactCount].normal[2] = 0.0f;
                    contacts[contactCount].penetration = platformTopY - worldVertex[1];
                    contacts[contactCount].bodyA = i;
                    contacts[contactCount].bodyB = -1; // -1 означает платформу
                    contactCount++;
                }
            }
        }
    }

    // 2. Коллизии куб с кубом (Vertex-in-Box для каждой пары)
    for (int i = 0; i < cubeCount; i++) {
        if (!cubes[i].active) continue;

        for (int j = 0; j < cubeCount; j++) {
            if (i >= j || !cubes[j].active) continue;

            mat4 invOrientationB;
            glm_mat4_inv(cubes[j].orientation, invOrientationB);

            for (int v = 0; v < 8; v++) {
                vec3 worldVertex;
                glm_mat4_mulv3(cubes[i].orientation, localVertices[v], 1.0f, worldVertex);
                glm_vec3_add(worldVertex, cubes[i].position, worldVertex);

                vec3 localToB;
                glm_vec3_sub(worldVertex, cubes[j].position, localToB);
                vec3 vertexInB;
                glm_mat4_mulv3(invOrientationB, localToB, 1.0f, vertexInB);

                if (vertexInB[0] >= -0.5f && vertexInB[0] <= 0.5f &&
                    vertexInB[1] >= -0.5f && vertexInB[1] <= 0.5f &&
                    vertexInB[2] >= -0.5f && vertexInB[2] <= 0.5f) {

                    if (contactCount < maxContacts) {
                        float dists[6] = {
                            0.5f - vertexInB[0], vertexInB[0] - (-0.5f),
                            0.5f - vertexInB[1], vertexInB[1] - (-0.5f),
                            0.5f - vertexInB[2], vertexInB[2] - (-0.5f)
                        };

                        float minPenetration = dists[0];
                        int minIdx = 0;
                        for (int d = 1; d < 6; d++) {
                            if (dists[d] < minPenetration) {
                                minPenetration = dists[d];
                                minIdx = d;
                            }
                        }

                        vec3 localNormal = {0.0f, 0.0f, 0.0f};
                        if (minIdx == 0) localNormal[0] = 1.0f;
                        else if (minIdx == 1) localNormal[0] = -1.0f;
                        else if (minIdx == 2) localNormal[1] = 1.0f;
                        else if (minIdx == 3) localNormal[1] = -1.0f;
                        else if (minIdx == 4) localNormal[2] = 1.0f;
                        else if (minIdx == 5) localNormal[2] = -1.0f;

                        vec3 worldNormal;
                        glm_mat4_mulv3(cubes[j].orientation, localNormal, 0.0f, worldNormal);
                        glm_vec3_normalize(worldNormal);

                        glm_vec3_copy(worldVertex, contacts[contactCount].point);
                        glm_vec3_copy(worldNormal, contacts[contactCount].normal);
                        contacts[contactCount].penetration = minPenetration;
                        contacts[contactCount].bodyA = i;
                        contacts[contactCount].bodyB = j;
                        contactCount++;
                    }
                }
            }
        }
    }

    return contactCount;
}