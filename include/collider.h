#ifndef COLLIDER_H
#define COLLIDER_H

#include <cglm/cglm.h>

typedef enum {
    COLLIDER_BOX = 0,
    COLLIDER_SPHERE
} ColliderType;

typedef struct {
    ColliderType type;
    int bodyIndex;     // Індекс тіла, якому належить колайдер
    vec3 localPos;     // Зміщення від центру мас тіла
    
    union {
        struct {
            vec3 halfSizes; // Напіврозміри по осях X, Y, Z (для Box)
        } box;
        struct {
            float radius;   // Радіус (для Sphere)
        } sphere;
    };
} Collider;

#define MAX_COLLIDERS 128

extern Collider colliders[MAX_COLLIDERS];
extern int colliderCount;

void collider_init_system(void);
int collider_create_box(int bodyIndex, vec3 halfSizes);
int collider_create_sphere(int bodyIndex, float radius);

#endif // COLLIDER_H