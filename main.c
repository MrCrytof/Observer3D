#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>

#include "camera.h"
#include "rigid_body.h"
#include "renderer.h"
#include "physics.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static float lastX = 400.0f;
    static float lastY = 300.0f;
    static bool firstMouse = true;

    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos; // Перевернуто: y іде від низу до верху
    lastX = (float)xpos;
    lastY = (float)ypos;

    camera_process_mouse(xoffset, yoffset);
}

int main(void) {
    // 1. Ініціалізація GLFW та створення вікна
    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Observer3D - Physics Simulator", NULL, NULL);
    if (!window) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 2. Ініціалізація GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // 3. Ініціалізація наших модулів
    camera_init();
    rigid_body_init_all();
    renderer_init();

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float gravity = 9.81f;

    // 4. Головний цикл програми
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (deltaTime > 0.05f) deltaTime = 0.05f; // Захист від лагів

        // Обробка введення клавіатури
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        camera_process_keyboard(window, deltaTime);

        // Спавн куба по кліку ЛКМ
        bool mousePressedNow = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
        if (mousePressedNow && !mousePressedLast) {
            vec3 spawnPos;
            glm_vec3_copy(camera.position, spawnPos);

            vec3 spawnOffset;
            glm_vec3_scale(camera.front, 2.0f, spawnOffset);
            glm_vec3_add(spawnPos, spawnOffset, spawnPos);

            vec3 initialVel;
            glm_vec3_scale(camera.front, 12.0f, initialVel);

            rigid_body_spawn(spawnPos, initialVel);
        }
        mousePressedLast = mousePressedNow;

        // Оновлення фізики
        physics_update(deltaTime, gravity);

        // Рендеринг
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 view, projection;
        camera_get_view_matrix(view);
        glm_perspective(glm_rad(45.0f), 800.0f / 600.0f, 0.1f, 100.0f, projection);

        renderer_draw_scene(view, projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Очищення ресурсів
    renderer_cleanup();
    glfwTerminate();
    return 0;
}