cmake_minimum_required(VERSION 3.20)
project(Observer3D C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

# Підключаємо GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
)

FetchContent_Declare(
        cglm
        GIT_REPOSITORY https://github.com/recp/cglm.git
        GIT_TAG v0.9.1
)
FetchContent_MakeAvailable(cglm)
FetchContent_MakeAvailable(glfw)

# Створюємо список наших вихідних файлів модулів
set(SOURCES
        main.c
        src/glad.c
        src/camera.c
        src/renderer.c
        src/physics.c
        src/rigid_body.c
        src/camera.c
        include/rigid_body.h
        src/rigid_body.c
        include/renderer.h
        src/renderer.c
        include/physics.h
        src/physics.c
        include/collision.h
        src/collision.c
)

# Створюємо виконуваний файл із нашими модулями
add_executable(My_Prog ${SOURCES})

# Вказуємо шлях до заголовочних файлів (.h)
target_include_directories(My_Prog PRIVATE include)

# Підключаємо бібліотеки до нашого проєкту
target_link_libraries(My_Prog
        PRIVATE
        glfw
        opengl32
        glm::glm
        cglm
)