#include "collider.h"
#include <string.h>

Collider colliders[MAX_COLLIDERS];
int colliderCount = 0;

void collider_init_system(void) {
    memset(colliders, 0, sizeof(colliders));
    colliderCount = 0;
}

int collider_create_box(int bodyIndex, vec3 halfSizes) {
    if (colliderCount >= MAX_COLLIDERS) return -1;

    int id = colliderCount++;
    Collider *c = &colliders[id];

    c->type = COLLIDER_BOX;
    c->bodyIndex = bodyIndex;
    glm_vec3_zero(c->localPos);
    glm_vec3_copy(halfSizes, c->box.halfSizes);

    return id;
}

int collider_create_sphere(int bodyIndex, float radius) {
    if (colliderCount >= MAX_COLLIDERS) return -1;

    int id = colliderCount++;
    Collider *c = &colliders[id];

    c->type = COLLIDER_SPHERE;
    c->bodyIndex = bodyIndex;
    glm_vec3_zero(c->localPos);
    c->sphere.radius = radius;

    return id;
}