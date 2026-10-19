#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>
#include <GLFW/glfw3.h>

/**
 * @brief Структура, що представляє камеру у тривимірному просторі.
 */
typedef struct {
    vec3 position;          ///< Позиція центру камери у світових координатах
    vec3 front;             ///< Вектор напрямку погляду вперед
    vec3 up;                ///< Вектор напрямку вгору для камери
    float yaw;              ///< Кут повороту навколо вертикальної осі (у градусах)
    float pitch;            ///< Кут нахилу вгору/вниз (у градусах)
    float movementSpeed;    ///< Базова швидкість переміщення камери
    float mouseSensitivity; ///< Чутливість обертання мишею
} Camera;

/**
 * @brief Глобальний екземпляр камери симулятора.
 */
extern Camera camera;

/**
 * @brief Ініціалізує параметри камери за замовчуванням.
 */
void camera_init(void);

/**
 * @brief Обробляє введення з клавіатури для переміщення камери.
 *
 * @param window Вказівник на вікно GLFW.
 * @param deltaTime Час, що минув з попереднього кадру (для плавності руху).
 */
void camera_process_keyboard(GLFWwindow *window, float deltaTime);

/**
 * @brief Обробляє зміщення курсору миші для огляду простору.
 *
 * @param xoffset Зміщення курсору по осі X.
 * @param yoffset Зміщення курсору по осі Y.
 */
void camera_process_mouse(float xoffset, float yoffset);

/**
 * @brief Обчислює та повертає матрицю вигляду для рендерингу.
 *
 * @param dest Масив mat4, у який буде записана результуюча матриця вигляду.
 */
void camera_get_view_matrix(mat4 dest);

#endif // CAMERA_H