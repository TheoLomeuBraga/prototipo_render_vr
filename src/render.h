#pragma once
#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/constants.hpp>

//#include "vr.h"

unsigned int main_shader;

GLuint loadShader(const char *vert_file, const char *frag_file)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Carregando e compilando shader de vértices
    std::ifstream vertStream(vert_file);
    std::stringstream vertBuffer;
    vertBuffer << vertStream.rdbuf();
    std::string vertSource = vertBuffer.str();
    const char *vertSourcePtr = vertSource.c_str();
    glShaderSource(vertexShader, 1, &vertSourcePtr, nullptr);
    glCompileShader(vertexShader);

    // Verificando erros de compilação do shader de vértices
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Erro ao compilar shader de vértices: " << infoLog << std::endl;
        exit(EXIT_FAILURE);
    }

    // Carregando e compilando shader de fragmentos
    std::ifstream fragStream(frag_file);
    std::stringstream fragBuffer;
    fragBuffer << fragStream.rdbuf();
    std::string fragSource = fragBuffer.str();
    const char *fragSourcePtr = fragSource.c_str();
    glShaderSource(fragmentShader, 1, &fragSourcePtr, nullptr);
    glCompileShader(fragmentShader);

    // Verificando erros de compilação do shader de fragmentos
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Erro ao compilar shader de fragmentos: " << infoLog << std::endl;
        exit(EXIT_FAILURE);
    }

    // Criando o programa de shader
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Verificando por erros de linkagem
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Erro ao linkar programa de shader: " << infoLog << std::endl;
        exit(EXIT_FAILURE);
    }

    // Limpando memória de shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint colorLoc;

// Vértices do quad (posições x, y)
const float vertices[] = {
    -0.5f, 0.5f, // Top-left
    0.5f, 0.5f,  // Top-right
    0.5f, -0.5f, // Bottom-right
    -0.5f, -0.5f // Bottom-left
};

// Índices dos triângulos do quad
const unsigned int indices[] = {
    0, 1, 2, // Primeiro triângulo
    0, 2, 3  // Segundo triângulo
};
unsigned int VAO, VBO, EBO;

unsigned int deeph_buffer;

void start_gl(int width, int height)
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    main_shader = loadShader("shaders/vert.glsl", "shaders/frag.glsl");

    modelLoc = glGetUniformLocation(main_shader, "model");
    viewLoc = glGetUniformLocation(main_shader, "view");
    projectionLoc = glGetUniformLocation(main_shader, "projection");
    colorLoc = glGetUniformLocation(main_shader, "color");

    // Criar e configurar um VAO (Vertex Array Object) e um VBO (Vertex Buffer Object)
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Vincular o VAO
    glBindVertexArray(VAO);

    // Vincular o VBO para armazenar os vértices do quad
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Vincular o EBO para armazenar os índices dos triângulos do quad
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Especificar layout dos dados do vértice
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Desvincular o VBO e o VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // deeph buffer
    glGenRenderbuffers(1, &deeph_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, deeph_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, deeph_buffer);
}

void update_gl()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

glm::mat4 getCameraViewMatrix(glm::mat4 transformMatrix)
{
    // Remove a informação de escala da matriz de transformação
    glm::mat4 viewMatrix = glm::mat4(glm::mat3(transformMatrix));

    // Remove a informação de transladação da matriz de transformação
    viewMatrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    // Inverte a matriz resultante
    viewMatrix = glm::affineInverse(viewMatrix);

    return viewMatrix;
}

struct eye_struct
{
    glm::mat4 view, projection;
};
typedef struct eye_struct Eye;

Eye create_eye(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::quat quaternion = glm::quat(1.0f, 0.0f, 0.0f, 0.0f))
{

    glm::mat4 view_transform = glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(quaternion);

    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 1000.0f);

    return {getCameraViewMatrix(view_transform), projection};
}

const glm::quat base_rotation(1.0f, 0.0f, 0.0f, 0.0f);
void draw_3d_point(Eye eye, glm::vec3 position, glm::quat quaternion = base_rotation, glm::vec3 scale = glm::vec3(1, 1, 1), glm::vec3 color = glm::vec3(0, 0, 0))
{
    glUseProgram(main_shader);

    // Configure outras uniformes
    bool rgb = color.x == 0 && color.y == 0 && color.z == 0;
    glUniform3f(colorLoc, color.x, color.y, color.z);

    if (rgb)
    {
        glUniform3f(colorLoc, 1, 0, 0);
    }

    // Configure os uniformes do shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(eye.view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(eye.projection));

    // Ativar o VAO para renderizar o quad
    glBindVertexArray(VAO);

    // Desenhar o quad
    glm::mat4 tf = glm::mat4_cast(quaternion) * glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), scale);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(tf));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    if (rgb)
    {
        glUniform3f(colorLoc, 0, 1, 0);
    }
    glm::mat4 tf2 = tf;
    glm::rotate(tf2, glm::radians(90.0f), glm::vec3(0, 0, 1));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(tf2));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    if (rgb)
    {
        glUniform3f(colorLoc, 0, 0, 1);
    }
    glm::mat4 tf3 = tf;
    glm::rotate(tf2, glm::radians(90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(tf3));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}