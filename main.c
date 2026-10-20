#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stdio.h>

#include "camera.h"
#include "renderer.h"
#include "physics.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

float lastX = 800.0f / 2.0f;
float lastY = 600.0f / 2.0f;
bool firstMouse = true;

int main(void) {
    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Physics Engine 3D", NULL, NULL);
    if (!window) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    camera_init();
    renderer_init();

    physics_init();

    // Создание статической платформы
    int platformBody = rigid_body_create(BODY_TYPE_STATIC, 0.0f, (vec3){0.0f, -2.0f, 0.0f});
    collider_create_box(platformBody, (vec3){10.0f, 0.5f, 10.0f});

    // Создание динамических падающих кубов
    for (int i = 0; i < 4; i++) {
        int cubeBody = rigid_body_create(BODY_TYPE_DYNAMIC, 1.0f, (vec3){(float)(i * 0.2f), 2.0f + (float)i * 1.5f, 0.0f});
        collider_create_box(cubeBody, (vec3){0.5f, 0.5f, 0.5f});
    }

    float lastFrame = 0.0f;
    float accumulator = 0.0f;
    const float fixedTimeStep = 1.0f / 60.0f;

    while (!glfwWindowShouldClose(window)) {
        static bool mousePressedLastFrame = false;
        int currentMouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if (currentMouseState == GLFW_PRESS && !mousePressedLastFrame) {
            vec3 camPos, camFront;
            camera_get_position(camPos);
            camera_get_front(camFront);

            vec3 spawnPos;
            glm_vec3_scale(camFront, 2.5f, spawnPos);
            glm_vec3_add(camPos, spawnPos, spawnPos);

            int cubeBody = rigid_body_create(BODY_TYPE_DYNAMIC, 1.0f, spawnPos);
            collider_create_box(cubeBody, (vec3){0.5f, 0.5f, 0.5f});
        }
        mousePressedLastFrame = (currentMouseState == GLFW_PRESS);

        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        camera_process_keyboard(window, deltaTime);

        accumulator += deltaTime;
        if (accumulator > 0.25f) accumulator = 0.25f;
        while (accumulator >= fixedTimeStep) {
            int substeps = 8;
            float subDelta = fixedTimeStep / (float)substeps;
            for (int s = 0; s < substeps; s++) {
                physics_update(subDelta, 9.81f);
            }
            accumulator -= fixedTimeStep;
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 projection, view;
        glm_perspective(glm_rad(45.0f), 800.0f / 600.0f, 0.1f, 100.0f, projection);
        camera_get_view_matrix(view);

        // Якщо твій рендерер сам проходить по масиву bodies, достатньо одного виклику:
        renderer_draw_scene(view, projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = (float)xposIn;
    float ypos = (float)yposIn;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera_process_mouse(xoffset, yoffset);
}