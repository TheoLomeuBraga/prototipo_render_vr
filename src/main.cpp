

#include "render.h"
#include "window.h"
#include "vr.h"

void draw_things(){

    Eye eye = create_eye();

    draw_3d_point(eye, glm::vec3(2,0,0));
    draw_3d_point(eye, glm::vec3(-2,0,0));
    draw_3d_point(eye, glm::vec3(0,0,2));
    draw_3d_point(eye, glm::vec3(0,0,-2));

    draw_3d_point(eye, glm::vec3(2,0,2));
    draw_3d_point(eye, glm::vec3(2,0,-2));
    draw_3d_point(eye, glm::vec3(-2,0,2));
    draw_3d_point(eye, glm::vec3(-2,0,-2));
}

int main() {

    initGLFW(500, 500, "test render");
    while(start_openxr()){}
    
    start_gl();
    
    while(the_window_show_should_continue() && the_vr_show_should_continue()){
       
        update_window();
        update_openxr();
        update_gl();

        draw_things();
        
    }

    end_window();
    end_openxr();
    //end_gl();
    return 0;
}