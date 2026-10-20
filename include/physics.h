#ifndef PHYSICS_H
#define PHYSICS_H

#include "rigid_body.h"
#include "collider.h"
#include "broadphase.h"
#include "collision.h"
#include "contact_solver.h"

void physics_init(void);
void physics_update(float deltaTime, float gravity);

#endif // PHYSICS_H