#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>
#include <cglm/cglm.h>

void renderer_init(void);
void renderer_draw_scene(mat4 view, mat4 projection);
void renderer_cleanup(void);

#endif // RENDERER_H