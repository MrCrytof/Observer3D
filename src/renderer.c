#include "renderer.h"
#include "rigid_body.h"
#include <stdio.h>
#include <stdlib.h>

static unsigned int shaderProgram;
static unsigned int cubeVAO, cubeVBO;
static unsigned int platformVAO, platformVBO;
static int mvpLocation;

// Прості шейдери
static const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "uniform mat4 u_MVP;\n"
    "out vec3 vertexColor;\n"
    "void main() {\n"
    "   gl_Position = u_MVP * vec4(aPos, 1.0f);\n"
    "   vertexColor = aColor;\n"
    "}\0";

static const char *fragmentShaderSource = "#version 330 core\n"
    "in vec3 vertexColor;\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "   FragColor = vec4(vertexColor, 1.0f);\n"
    "}\0";

void renderer_init(void) {
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    mvpLocation = glGetUniformLocation(shaderProgram, "u_MVP");

    // Вершини куба
    float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f
    };

    // Вершини платформи (наприклад, сірий колір для всіх вершин)
    float platformVertices[] = {
        // Позиції                // Кольори (RGB)
        -0.5f, 0.0f, -0.5f,  0.5f, 0.5f, 0.5f,
         0.5f, 0.0f, -0.5f,  0.5f, 0.5f, 0.5f,
         0.5f, 0.0f,  0.5f,  0.5f, 0.5f, 0.5f,
         0.5f, 0.0f,  0.5f,  0.5f, 0.5f, 0.5f,
        -0.5f, 0.0f,  0.5f,  0.5f, 0.5f, 0.5f,
        -0.5f, 0.0f, -0.5f,  0.5f, 0.5f, 0.5f
    };

    glGenVertexArrays(1, &platformVAO);
    glGenBuffers(1, &platformVBO);

    glBindVertexArray(platformVAO);
    glBindBuffer(GL_ARRAY_BUFFER, platformVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(platformVertices), platformVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &platformVAO);
    glGenBuffers(1, &platformVBO);

    glBindVertexArray(platformVAO);
    glBindBuffer(GL_ARRAY_BUFFER, platformVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(platformVertices), platformVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void renderer_draw_scene(mat4 view, mat4 projection) {
    glUseProgram(shaderProgram);

    mat4 model, viewModel, mvp;

    // 1. Рендер платформи
    if (bodyCount > 0 && bodies[0].type == BODY_TYPE_STATIC) {
        glm_mat4_identity(model);
        glm_translate(model, bodies[0].position);

        // Масштабуємо платформу під її фізичний розмір (напіврозміри 10, 0.5, 10 -> повний розмір 20, 1, 20)
        vec3 scale = {20.0f, 1.0f, 20.0f};
        glm_scale(model, scale);

        glm_mat4_mul(view, model, viewModel);
        glm_mat4_mul(projection, viewModel, mvp);
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, (float*)mvp);

        glBindVertexArray(platformVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // 2. Рендер динамічних кубів
    glBindVertexArray(cubeVAO);
    for (int i = 1; i < bodyCount; i++) {
        if (!bodies[i].active) continue;

        glm_mat4_identity(model);
        glm_translate(model, bodies[i].position);

        mat4 rotMat;
        glm_quat_mat4(bodies[i].orientation, rotMat);
        glm_mul(model, rotMat, model);

        glm_mat4_mul(view, model, viewModel);
        glm_mat4_mul(projection, viewModel, mvp);

        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, (float*)mvp);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);
}

void renderer_cleanup(void) {
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &platformVAO);
    glDeleteBuffers(1, &platformVBO);
    glDeleteProgram(shaderProgram);
}