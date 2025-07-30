#include <stream/OutputStream.h>
#include <keyboard/Keyboard.h>
#include <graphics/gfx.h>

int main(int argc, char** argv)
{
    
    gfx_draw_text(screen_buffer, (gfx_point){10, 10}, "Hello, AtomOS!", (gfx_color){255, 255, 255, 255});

}
