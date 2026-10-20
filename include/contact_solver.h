#ifndef CONTACT_SOLVER_H
#define CONTACT_SOLVER_H

#include "collision.h"

// Вирішує всі контактні обмеження за допомогою імпульсного методу
void contact_solver_solve(ContactManifold *manifolds, int manifoldCount, float deltaTime);

#endif // CONTACT_SOLVER_H