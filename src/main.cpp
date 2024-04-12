#include "render.h"
#include "window.h"

void draw_things(){

    Eye eye = create_eye();

    draw_3d_point(eye, glm::vec3(2,0,0));
    draw_3d_point(eye, glm::vec3(-2,0,0));

    draw_3d_point(eye, glm::vec3(0,0,2));
    draw_3d_point(eye, glm::vec3(0,0,-2));
}

int main() {
    initGLFW(500, 500, "test render");
    start_gl();

    
    
    while(!glfwWindowShouldClose(win)){

       
        update_window();
        update_gl();

        draw_things();
        
    }
    return 0;
}