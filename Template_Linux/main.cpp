#include "MyRenderPath.h"
#include <WickedEngine.h>
#include <SDL2/SDL.h>

int sdl_loop(wi::Application &application)
{
    SDL_Event event;

    bool quit = false;
    while (!quit)
    {
        SDL_PumpEvents();
        application.Run();

        while( SDL_PollEvent(&event)) 
        {
            switch (event.type) 
            {
                case SDL_KEYDOWN:
                    if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                      quit = true;
                    break;
                case SDL_QUIT:      
                    quit = true;
                    break;
                case SDL_WINDOWEVENT:
                    switch (event.window.event) 
                    {
                    case SDL_WINDOWEVENT_CLOSE:
                        quit = true;
                        break;
                    case SDL_WINDOWEVENT_RESIZED:
                        application.SetWindow(application.window);
                        break;
                    default:
                        break;
                    }
                default:
                    break;
            }
        }
    }

    return 0;

}

int main(int argc, char *argv[])
{
    wi::Application application;
    #ifdef WickedEngine_SHADER_DIR
    wi::renderer::SetShaderSourcePath(WickedEngine_SHADER_DIR);
    #endif

    application.infoDisplay.active = true;
    application.infoDisplay.watermark = true;
    application.infoDisplay.resolution = true;
    application.infoDisplay.fpsinfo = true;

    sdl2::sdlsystem_ptr_t system = sdl2::make_sdlsystem(SDL_INIT_EVERYTHING | SDL_INIT_EVENTS);
    sdl2::window_ptr_t window = sdl2::make_window(
            "Template",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            1920, 1080,
            SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI);

    SDL_Event event;

    if (!window) {
      std::exit(1);
    }

    SDL_SetWindowFullscreen(window.get(), SDL_WINDOW_FULLSCREEN_DESKTOP);
    application.SetWindow(window.get());
    my::RenderPath RenderPath;
    application.ActivatePath(&RenderPath);

    int ret = sdl_loop(application);

    SDL_Quit();

    return ret;
}
