#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include <cglm/cglm.h>
#include <stdbool.h>

typedef enum {
    BODY_TYPE_STATIC = 0,
    BODY_TYPE_DYNAMIC,
    BODY_TYPE_KINEMATIC
} BodyType;

typedef struct {
    BodyType type;
    bool active;          // Состояние активности (сон / бодрствование)

    vec3 position;        // Позиция в мировом пространстве
    versor orientation;   // Ориентация в виде кватерниона [x, y, z, w]

    vec3 velocity;        // Линейная скорость
    vec3 angularVelocity; // Угловая скорость

    vec3 force;           // Результирующая сила
    vec3 torque;          // Результирующий момент силы

    float mass;           // Масса
    float invMass;        // Обратная масса (0 для статических тел)

    mat3 localInertiaTensor;    // Локальный тензор инерции
    mat3 invLocalInertiaTensor; // Обратный локальный тензор инерции
    mat3 invWorldInertiaTensor; // Обратный тензор инерции в мировых координатах

    float friction;       // Коэффициент трения
    float restitution;    // Коэффициент восстановления (упругость)
} RigidBody;

#define MAX_BODIES 128

extern RigidBody bodies[MAX_BODIES];
extern int bodyCount;

void rigid_body_init_system(void);
int rigid_body_create(BodyType type, float mass, vec3 pos);
void rigid_body_update_world_inertia(RigidBody *body);

#endif // RIGID_BODY_H