#include "camera.h"

Camera camera;

static bool firstMouse = true;
static float lastX = 400.0f;
static float lastY = 300.0f;

void camera_init(void) {
    camera.position[0] = 0.0f;
    camera.position[1] = 2.0f;
    camera.position[2] = 6.0f;

    camera.front[0] = 0.0f;
    camera.front[1] = 0.0f;
    camera.front[2] = -1.0f;

    camera.up[0] = 0.0f;
    camera.up[1] = 1.0f;
    camera.up[2] = 0.0f;

    camera.yaw = -90.0f;
    camera.pitch = 0.0f;
    camera.movementSpeed = 5.0f;
    camera.mouseSensitivity = 0.1f;
}

void camera_process_keyboard(GLFWwindow *window, float deltaTime) {
    float velocity = camera.movementSpeed * deltaTime;
    vec3 temp;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm_vec3_scale(camera.front, velocity, temp);
        glm_vec3_add(camera.position, temp, camera.position);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm_vec3_scale(camera.front, velocity, temp);
        glm_vec3_sub(camera.position, temp, camera.position);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm_vec3_cross(camera.front, camera.up, temp);
        glm_vec3_normalize(temp);
        glm_vec3_scale(temp, velocity, temp);
        glm_vec3_sub(camera.position, temp, camera.position);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm_vec3_cross(camera.front, camera.up, temp);
        glm_vec3_normalize(temp);
        glm_vec3_scale(temp, velocity, temp);
        glm_vec3_add(camera.position, temp, camera.position);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        glm_vec3_scale(camera.up, velocity, temp);
        glm_vec3_add(camera.position, temp, camera.position);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        glm_vec3_scale(camera.up, velocity, temp);
        glm_vec3_sub(camera.position, temp, camera.position);
    }
}

void camera_process_mouse(float xoffset, float yoffset) {
    xoffset *= camera.mouseSensitivity;
    yoffset *= camera.mouseSensitivity;

    camera.yaw += xoffset;
    camera.pitch += yoffset;

    if (camera.pitch > 89.0f)  camera.pitch = 89.0f;
    if (camera.pitch < -89.0f) camera.pitch = -89.0f;

    vec3 direction;
    direction[0] = cosf(glm_rad(camera.yaw)) * cosf(glm_rad(camera.pitch));
    direction[1] = sinf(glm_rad(camera.pitch));
    direction[2] = sinf(glm_rad(camera.yaw)) * cosf(glm_rad(camera.pitch));
    glm_vec3_normalize_to(direction, camera.front);
}

void camera_get_view_matrix(mat4 dest) {
    vec3 target;
    glm_vec3_add(camera.position, camera.front, target);
    vec3 worldUp = {0.0f, 1.0f, 0.0f};
    glm_lookat(camera.position, target, worldUp, dest);
}