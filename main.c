// Підключаємо стандартну бібліотеку введення-виведення (для printf)
#include <stdio.h>
// Підключаємо системну бібліотеку Windows (для налаштування кодування консолі)
#include <windows.h>
// Підключаємо тип даних bool (true/false)
#include <stdbool.h>

// Підключаємо заголовочний файл GLAD для завантаження OpenGL-функцій
#include <glad/glad.h>
// Підключаємо GLFW для створення вікна та обробки подій клавіатури/миші
#include <GLFW/glfw3.h>
// Підключаємо математичну бібліотеку CGLM для роботи з матрицями та векторами в C
#include <cglm/cglm.h>

// Задаємо константи для ширини та висоти вікна програми
#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 800

// Створюємо структуру даних для опису параметрів камери
typedef struct {
    vec3 position;   // Позиція камери у тривимірному світі (координати X, Y, Z)
    float yaw;       // Кут повороту камери вліво/вправо (горизонтальна площина)
    float pitch;     // Кут повороту камери вгору/вниз (вертикальна площина)
    float speed;     // Швидкість пересування камери по світу
} Camera;

// Оголошуємо глобальні змінні для відстеження руху миші
float lastX = SCREEN_WIDTH / 2.0f;  // Попередня координата курсору по осі X (по центру екрана)
float lastY = SCREEN_HEIGHT / 2.0f;  // Попередня координата курсору по осі Y (по центру екрана)
bool firstMouse = true;             // Прапорець, щоб уникнути різкого ривка камери при першому русі миші

// Ініціалізуємо об'єкт камери за допомогою нашої структури
Camera camera = {
    .position = {0.0f, 0.0f, 3.0f},  // Ставимо камеру трохи назад по осі Z, щоб бачити центр світу
    .yaw = -90.0f,                    // Початковий кут по горизонталі (дивимося вздовж осі -Z)
    .pitch = 0.0f,                    // Початковий кут по вертикалі (дивимося прямо)
    .speed = 5.0f                     // Задаємо швидкість руху камери
};

vec3 front = {0.0f, 0.0f, -1.0f};     // Вектор, що вказує, куди саме дивиться камера (напрямок погляду)
vec3 worldUp = {0.0f, 1.0f, 0.0f};    // Постійний вектор світу, що вказує строго вгору (вісь Y)

// Функція зворотного виклику (callback), яка викликається автоматично при русі миші
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    // Якщо це перший рух миші після запуску програми
    if (firstMouse) {
        lastX = (float)xpos; // Записуємо поточну позицію як попередню
        lastY = (float)ypos;
        firstMouse = false;   // Вимикаємо прапорець
    }

    // Обчислюємо зміщення курсору миші відносно попереднього кадру
    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos; // Інвертуємо Y, бо в екранних координатах вісь Y йде зверху вниз
    lastX = (float)xpos; // Оновлюємо попередню позицію X
    lastY = (float)ypos; // Оновлюємо попередню позицію Y

    float sensitivity = 0.1f; // Коефіцієнт чутливості миші (щоб огляд не був занадто швидким)
    xoffset *= sensitivity;   // Множимо зміщення X на чутливість
    yoffset *= sensitivity;   // Множимо зміщення Y на чутливість

    camera.yaw += xoffset;    // Додаємо зміщення до загального горизонтального кута
    camera.pitch += yoffset;  // Додаємо зміщення до загального вертикального кута

    // Обмежуємо вертикальний кут (pitch), щоб камера не робила перевертання «догори дриґом»
    if (camera.pitch > 89.0f)  camera.pitch = 89.0f;
    if (camera.pitch < -89.0f) camera.pitch = -89.0f;

    // Математично обчислюємо новий вектор напрямку (front) через тригонометричні функції (сферічні координати)
    vec3 direction;
    direction[0] = cosf(glm_rad(camera.yaw)) * cosf(glm_rad(camera.pitch));
    direction[1] = sinf(glm_rad(camera.pitch));
    direction[2] = sinf(glm_rad(camera.yaw)) * cosf(glm_rad(camera.pitch));
    glm_vec3_normalize_to(direction, front); // Нормалізуємо вектор (робимо його довжину рівною 1)
}

// Функція для перевірки натискань клавіш клавіатури (WASD, Space, Shift)
void processInput(GLFWwindow *window, float deltaTime) {
    // Обчислюємо дистанцію переміщення за цей кадр (швидкість помножена на час кадру)
    float velocity = camera.speed * deltaTime;

    // Проєктуємо вектор напрямку на горизонтальну площину (ігноруємо Y), щоб рух вперед/назад був «по землі»
    vec3 tempFront = {front[0], 0.0f, front[2]};
    glm_vec3_normalize(tempFront);

    // Якщо натиснута клавіша W — рухаємося вперед
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        vec3 scaleVec;
        glm_vec3_scale(tempFront, velocity, scaleVec); // Множимо напрямок на швидкість кадру
        glm_vec3_add(camera.position, scaleVec, camera.position); // Додаємо до поточної позиції
    }
    // Якщо натиснута клавіша S — рухаємося назад
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        vec3 scaleVec;
        glm_vec3_scale(tempFront, velocity, scaleVec);
        glm_vec3_sub(camera.position, scaleVec, camera.position); // Віднімаємо від поточної позиції
    }

    // Обчислюємо вектор «праворуч» за допомогою векторного добутку напрямку та світового «вгору»
    vec3 right;
    glm_vec3_cross(tempFront, worldUp, right);
    glm_vec3_normalize(right);
    
    // Якщо натиснута клавіша A — рухаємося вліво
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        vec3 scaleVec;
        glm_vec3_scale(right, velocity, scaleVec);
        glm_vec3_sub(camera.position, scaleVec, camera.position);
    }
    // Якщо натиснута клавіша D — рухаємося вправо
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        vec3 scaleVec;
        glm_vec3_scale(right, velocity, scaleVec);
        glm_vec3_add(camera.position, scaleVec, camera.position);
    }

    // Якщо натиснута клавіша Space — летимо вгору (збільшуємо координату Y)
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.position[1] += velocity;
    }
    // Якщо натиснутий Left Shift — летимо вниз (зменшуємо координату Y)
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera.position[1] -= velocity;
    }
    //якщо натиснута escape програма завершуеться
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwDestroyWindow(window);
    }
}

int main(void)
{
    // Налаштовуємо кодування консолі Windows на UTF-8 для коректного виведення кирилиці
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    // Ініціалізуємо бібліотеку GLFW (управління вікнами та вводом)
    if (!glfwInit()) {
        printf("Помилка: не вдалося запустити GLFW\n");
        return -1;
    }

    // Вказуємо налаштування контексту: використовуємо OpenGL версії 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Створюємо головне вікно програми з заданими розмірами та назвою
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Observer3D", NULL, NULL);
    if (window == NULL) {
        printf("Помилка: не вдалося створити вікно\n");
        glfwTerminate();
        return -1;
    }

    // Робимо створений OpenGL-контекст поточним для потоку виконання
    glfwMakeContextCurrent(window);
    // Вмикаємо вертикальну синхронізацію (обмежуємо кадри частотою монітора)
    glfwSwapInterval(1);

    // Реєструємо функцію відстеження руху миші для створеного вікна
    glfwSetCursorPosCallback(window, mouse_callback);
    // Захоплюємо курсор усередині вікна (миша ховається і вільно обертається для огляду)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Ініціалізуємо бібліотеку GLAD для завантаження вкажчиків на функції OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Помилка: не вдалося завантажити OpenGL\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Виводимо в консоль версію завантаженого OpenGL
    printf("OpenGL: %s\n", glGetString(GL_VERSION));

    // Дозволяємо перевірку буфера глибини (щоб ближні об'єкти закривали дальні)
    glEnable(GL_DEPTH_TEST);
    // Встановлюємо колір очищення екрана (темно-синій фоновий колір)
    glClearColor(0.04f, 0.06f, 0.12f, 1.0f);

    // Оголошуємо масив вершин тривимірного куба (кожна вершина має 6 значень: 3 координати X,Y,Z та 3 кольори R,G,B)
    float vertices[] = {
        // Задня грань — червона
        -0.5f, -0.5f, -0.5f, 1, 0, 0,
         0.5f,  0.5f, -0.5f, 1, 0, 0,
         0.5f, -0.5f, -0.5f, 1, 0, 0,
        -0.5f, -0.5f, -0.5f, 1, 0, 0,
        -0.5f,  0.5f, -0.5f, 1, 0, 0,
         0.5f,  0.5f, -0.5f, 1, 0, 0,

        // Передня грань — зелена
        -0.5f, -0.5f,  0.5f, 0, 1, 0,
         0.5f, -0.5f,  0.5f, 0, 1, 0,
         0.5f,  0.5f,  0.5f, 0, 1, 0,
        -0.5f, -0.5f,  0.5f, 0, 1, 0,
         0.5f,  0.5f,  0.5f, 0, 1, 0,
        -0.5f,  0.5f,  0.5f, 0, 1, 0,

        // Ліва грань — синя
        -0.5f, -0.5f, -0.5f, 0, 0, 1,
        -0.5f, -0.5f,  0.5f, 0, 0, 1,
        -0.5f,  0.5f,  0.5f, 0, 0, 1,
        -0.5f, -0.5f, -0.5f, 0, 0, 1,
        -0.5f,  0.5f,  0.5f, 0, 0, 1,
        -0.5f,  0.5f, -0.5f, 0, 0, 1,

        // Права грань — жовта
         0.5f, -0.5f, -0.5f, 1, 1, 0,
         0.5f,  0.5f,  0.5f, 1, 1, 0,
         0.5f, -0.5f,  0.5f, 1, 1, 0,
         0.5f, -0.5f, -0.5f, 1, 1, 0,
         0.5f,  0.5f, -0.5f, 1, 1, 0,
         0.5f,  0.5f,  0.5f, 1, 1, 0,

        // Верхня грань — фіолетова
        -0.5f,  0.5f, -0.5f, 1, 0, 1,
        -0.5f,  0.5f,  0.5f, 1, 0, 1,
         0.5f,  0.5f,  0.5f, 1, 0, 1,
        -0.5f,  0.5f, -0.5f, 1, 0, 1,
         0.5f,  0.5f,  0.5f, 1, 0, 1,
         0.5f,  0.5f, -0.5f, 1, 0, 1,

        // Нижня грань — блакитна
        -0.5f, -0.5f, -0.5f, 0, 1, 1,
         0.5f, -0.5f,  0.5f, 0, 1, 1,
        -0.5f, -0.5f,  0.5f, 0, 1, 1,
        -0.5f, -0.5f, -0.5f, 0, 1, 1,
         0.5f, -0.5f, -0.5f, 0, 1, 1,
         0.5f, -0.5f,  0.5f, 0, 1, 1
    };

    // Вершини платформи: великий квадрат на площині XZ
    float floorVertices[] = {
        -10.0f, 0.0f, -10.0f,   0.5f, 0.5f, 0.5f,
         10.0f, 0.0f, -10.0f,   0.5f, 0.5f, 0.5f,
         10.0f, 0.0f,  10.0f,   0.5f, 0.5f, 0.5f,

        -10.0f, 0.0f, -10.0f,   0.5f, 0.5f, 0.5f,
         10.0f, 0.0f,  10.0f,   0.5f, 0.5f, 0.5f,
        -10.0f, 0.0f,  10.0f,   0.5f, 0.5f, 0.5f

    };

    // Рядок вихідного коду вершинного шейдера (обробляє позиції та матрицю MVP)
    const char* vertexShaderSource =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "uniform mat4 mvp;\n"
        "out vec3 vertexColor;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = mvp * vec4(aPos, 1.0);\n"
        "    vertexColor = aColor;\n"
        "}\n";

    // Рядок вихідного коду фрагментного шейдера (фарбує кожен піксель у потрібний колір)
    const char* fragmentShaderSource =
        "#version 330 core\n"
        "in vec3 vertexColor;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(vertexColor, 1.0);\n"
        "}\n";

    // Створюємо та компілюємо вершинний шейдер
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Створюємо та компілюємо фрагментний шейдер
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Створюємо шейдерну програму та об'єднуємо в неї обидва шейдери
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Видаляємо окремі шейдери, оскільки вони вже об'єднані в програму і більше не потрібні
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Знаходимо розташування уніфікованої змінної матриці mvp у шейдері
    GLint mvpLocation = glGetUniformLocation(shaderProgram, "mvp");

    // Створюємо буфери для передачі даних геометрії у відеопам'ять
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Активуємо VAO
    glBindVertexArray(VAO);
    // Активуємо VBO та копіюємо в нього масив вершин куба
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Налаштовуємо атрибут 0 (координати вершин X, Y, Z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Налаштовуємо атрибут 1 (кольори R, G, B)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Створюємо окремі буфери для платформи
    unsigned int floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);

    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    // Атрибут 0: координати (X, Y, Z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Атрибут 1: колір (R, G, B)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Оголошуємо матриці для тривимірних перетворень
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 viewModel;
    mat4 mvp;

    // Створюємо матрицю перспективної проєкції (кут огляду 45°, співвідношення сторін вікна, межі видимості 0.1 - 100.0)
    glm_perspective(
        glm_rad(45.0f),
        (float)SCREEN_WIDTH / SCREEN_HEIGHT,
        0.1f,
        100.0f,
        projection
    );

    float lastFrame = 0.0f; // Змінна для збереження часу попереднього кадру

    // Головний цикл програми (працює, поки вікно не закрите користувачем)
    while (!glfwWindowShouldClose(window)) {
        // Отримуємо поточний час роботи програми в секундах
        float currentFrame = (float)glfwGetTime();
        // Обчислюємо deltaTime (час, що витратився на рендеринг попереднього кадру)
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Викликаємо функцію обробки клавіатури для руху камери
        processInput(window, deltaTime);

        // Очищуємо екран від попереднього кадру (буфер кольору та буфер глибини)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Налаштовуємо матрицю моделі (розміщуємо об'єкт у світі та обертаємо його з часом навколо осі Y)
        glm_mat4_identity(model);
        // Поднимаємо куб на половину його висоти
        glm_translate(model, (vec3){0.0f, 0.5f, 0.0f});
        glm_rotate(model, (float)glfwGetTime(), (vec3){0.0f, 1.0f, 0.0f});

        mat4 floorModel;
        glm_mat4_identity(floorModel);

        // Формуємо матрицю огляду камери (view) за допомогою функції glm_lookat
        vec3 target;
        glm_vec3_add(camera.position, front, target); // Точка, куди дивиться камера = позиція камери + напрямок
        glm_lookat(camera.position, target, worldUp, view);

        // Множимо матриці для отримання підсумкової MVP матриці: PROJECTION * VIEW * MODEL
        glm_mat4_mul(view, model, viewModel);
        glm_mat4_mul(projection, viewModel, mvp);

        // Активуємо нашу шейдерну програму
        glUseProgram(shaderProgram);
        // Передаємо пораховану MVP матрицю у вершинний шейдер
        mat4 floorViewModel;
        mat4 floorMvp;

        glm_mat4_mul(view, floorModel, floorViewModel);
        glm_mat4_mul(projection, floorViewModel, floorMvp);

        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, (float*)floorMvp);

        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, (float*)mvp);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Активуємо VAO куба
        glBindVertexArray(VAO);
        // Малюємо об'єкт як трикутники (всього 36 вершин)
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Міняємо місцями буфери зображення (відображаємо намальований кадр на екрані)
        glfwSwapBuffers(window);
        // Опитуємо та обробляємо системні події (наприклад, кліки миші, закриття вікна)
        glfwPollEvents();
    }

    // Коли цикл завершився, коректно очищуємо пам'ять відеокарти та закриваємо вікно
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}