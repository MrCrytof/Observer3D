#ifndef COLLISION_H
#define COLLISION_H

#include <cglm/cglm.h>
#include "rigid_body.h"

typedef struct {
    vec3 point;         // Точка контакта в мировых координатах
    vec3 normal;        // Нормаль от первого ко второму телу
    float penetration;  // Глубина проникновения
    int bodyA;          // Индекс первого тела
    int bodyB;          // Индекс второго тела
} Contact;

// Функция поиска столкновений между всеми телами
int collision_detect_boxes(Contact *contacts, int maxContacts);

#endif // COLLISION_H