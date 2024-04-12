#version 330 core
out vec4 FragColor; // Cor de saída do fragmento

uniform vec3 color;

void main() {
    FragColor = vec4(color, 1.0); // Cor laranja
}