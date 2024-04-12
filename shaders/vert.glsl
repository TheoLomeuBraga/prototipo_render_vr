#version 330 core
layout (location = 0) in vec3 aPos; // Posição do vértice

uniform mat4 model; // Matriz de transformação do modelo
uniform mat4 view;  // Matriz de visão
uniform mat4 projection; // Matriz de projeção

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}