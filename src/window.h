#pragma once
#include <iostream>
#include <GLFW/glfw3.h>
#include "render.h"



GLFWwindow* win = NULL;

GLFWwindow* initGLFW(int width, int height, const char* title) {
    if (!glfwInit()) {
        std::cerr << "Erro ao inicializar GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Erro ao criar janela GLFW" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Erro ao inicializar GLEW" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSwapInterval(1);
    win = window;
    return window;
}

void update_window() {
    glfwPollEvents();
    glfwSwapBuffers(win);
}

bool the_window_show_should_continue(){return !glfwWindowShouldClose(win);}

void end_window(){glfwTerminate();}

