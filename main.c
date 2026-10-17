#include <stdio.h>
#include <windows.h> // Потрібно для налаштування кодування консолі Windows (UTF-8)

#include <glad/glad.h>
#include <GLFW/glfw3.h>

int main(void) {
    // Примусово вмикаємо UTF-8 в консолі Windows, щоб кирилиця в printf виводилась без кракозябрів
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    // 1. Ініціалізація самої бібліотеки GLFW (готуємо систему вікон до роботи)
    if (!glfwInit()) {
        printf("Помилка: не вдалося запустити GLFW\n");
        return -1;
    }

    // 2. Встановлюємо правила для майбутнього OpenGL-контексту:
    // Кажемо, що хочемо саме OpenGL версії 3.3 з сучасним Core Profile (без застарілого функціоналу)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 3. Створюємо системне вікно заданого розміру (1200x800) з назвою "Observer3D"
    GLFWwindow* window = glfwCreateWindow(1200, 800, "Observer3D", NULL, NULL);
    if (!window) {
        printf("Помилка: не вдалося створити вікно\n");
        glfwTerminate(); // Якщо вікно не створилося — завершуємо роботу GLFW
        return -1;
    }

    // 4. Робимо контекст OpenGL поточним для щойно створеного вікна
    glfwMakeContextCurrent(window);

    // 5. Ініціалізуємо GLAD, щоб завантажити вказівники на функції OpenGL для поточної відеокарти
    if (!gladLoadGL()) {
        printf("Помилка: не вдалося ініціалізувати GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    printf("Успіх! Вікно та OpenGL готові до роботи.\n");

    // Задаємо колір, яким екран буде очищуватися (у нашому випадку — яскраво-фіолетовий: макс. червоний і синій)
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

    // Масив координат трьох вершин трикутника у тривимірному просторі (X, Y, Z) у діапазоні від -1.0 до 1.0
    float vertices[] = {
        0.0f,  0.5f, 0.0f,  // Верхня вершина
       -0.5f, -0.5f, 0.0f,  // Ліва нижня вершина
        0.5f, -0.5f, 0.0f   // Права нижня вершина
    };

    // --- ШЕЙДЕРИ (Маленькі програми, що виконуються прямо на відеокарті - GPU) ---

    // Вершинний шейдер: керує позицією та координатами кожної точки (вершини) у просторі
    const char* vertexShaderSource =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n" // Отримуємо координати X, Y, Z під індексом 0 з нашого буфера
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n" // Записуємо фінальну позицію вершини для екрана
        "}\0";

    // Фрагментний шейдер: керує кольором кожного окремого пікселя всередині фігури
    const char* fragmentShaderSource =
        "#version 330 core\n"
        "out vec4 FragColor;\n" // Вихідний колір пікселя (у форматі RGBA)
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n" // Зафарбовуємо пікселі у помаранчевий колір
        "}\0";

    // --- КОМПІЛЯЦІЯ ШЕЙДЕРІВ ---

    // 1. Створюємо контейнер для вершинного шейдера в пам'яті відеокарти
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // 2. Прив'язуємо текст нашого коду до цього контейнера
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    // 3. Компілюємо текст шейдера у внутрішній байт-код відеокарти
    glCompileShader(vertexShader);

    // 1. Створюємо контейнер для фрагментного шейдера
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // 2. Прив'язуємо текст фрагментного шейдера
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    // 3. Компілюємо його
    glCompileShader(fragmentShader);

    // --- СТВОРЕННЯ ПРОГРАМИ ШЕЙДЕРІВ ---

    // Створюємо головну програму шейдерів, яка об'єднає обидва скомпільовані шейдери разом
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);     // Прикріплюємо вершинний шейдер
    glAttachShader(shaderProgram, fragmentShader);   // Прикріплюємо фрагментний шейдер
    glLinkProgram(shaderProgram);                    // Зв'язуємо їх у єдиний робочий пайплайн відеокарти

    // Оскільки шейдери вже всередині програми, окремі контейнери нам більше не потрібні — очищаємо пам'ять
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // --- НАЛАШТУВАННЯ БУФЕРІВ (ПЕРЕДАЧА ДАНИХ НА ВІДЕОКАРТУ) ---

    unsigned int VAO, VBO;
    /*
      VBO (Vertex Buffer Object) — комірка (склад) у пам'яті відеокарти, де лежить наш масив координат 'vertices'.
      VAO (Vertex Array Object) — "папка-інструкція", яка запам'ятовує правила: як саме відеокарта повинна
      читати цей VBO (скільки чисел на одну вершину, який тип даних тощо).
    */

    // Генеруємо унікальні ID для наших буферів
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // 1. Робимо VAO активним (усі подальші налаштування буферів запишуться сюди)
    glBindVertexArray(VAO);

    // 2. Робимо VBO поточним буфером масиву
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // 3. Копіюємо координати з оперативної пам'яті ПК у швидку пам'ять відеокарти
    // GL_STATIC_DRAW вказує, що ці дані завантажуються один раз і змінюватися не будуть
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 4. Пояснюємо відеокарті, як саме читати завантажені дані з VBO:
    // 0 - індекс атрибута (відповідає layout (location = 0) у вершинному шейдері)
    // 3 - кількість чисел на одну вершину (X, Y, Z — це 3 числа)
    // GL_FLOAT - тип даних чисел (float)
    // GL_FALSE - чи потрібно нормалізувати дані (ні)
    // 3 * sizeof(float) - крок (stride): скільки байтів пропускати до наступної вершини
    // (void*)0 - з якого байта починається зчитування (з самого початку)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); // Вмикаємо цей атрибут під номером 0

    // 5. Відв'язуємо буфери, щоб випадково не перезаписати їх десь в іншому місці коду
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // --- ГОЛОВНИЙ ЦИКЛ ПРОГРАМИ (ІГРОВИЙ ЦИКЛ) ---
    // Працює постійно, кадр за кадром, поки користувач не закриє вікно хрестиком
    while (!glfwWindowShouldClose(window)) {

        // 1. Очищуємо екран кольором, заданим у glClearColor (стираємо старий кадр)
        glClear(GL_COLOR_BUFFER_BIT);

        // 2. РЕНДЕРИНГ (МАЛЮВАННЯ ТРИКУТНИКА):
        glUseProgram(shaderProgram);          // Кажемо відеокарті використовувати нашу програму шейдерів
        glBindVertexArray(VAO);                // Вибираємо нашу папку налаштувань із координатами
        glDrawArrays(GL_TRIANGLES, 0, 3);      // Даємо команду намалювати трикутники: почати з індекса 0, використати 3 вершини

        // 3. Подвійна буферизація: міняємо місцями прихований (задній) і видимий (передній) буфери вікна
        glfwSwapBuffers(window);

        // 4. Перевіряємо події ОС: чи натискались клавіші, чи рухалась миша, чи закривали вікно
        glfwPollEvents();
    }

    // --- ЗАВЕРШЕННЯ РОБОТИ ТА ОЧИЩЕННЯ ПАМ'ЯТІ ---
    // Коли цикл завершився (вікно закрили), акуратно звільняємо ресурси ОС
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window); // Знищуємо вікно
    glfwTerminate();           // Завершуємо роботу GLFW

    return 0;
}