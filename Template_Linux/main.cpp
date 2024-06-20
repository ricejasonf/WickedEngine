#include "MyRenderPath.h"
#include <WickedEngine.h>
#include <SDL2/SDL.h>
#include <initializer_list>
#include <string>
#include <thread>

void watch_shaders_finish();
void watch_shaders_start(std::vector<char const*> files);

void hot_reload_script(my::Application& app) {
  wi::eventhandler::FireEvent(wi::eventhandler::EVENT_RELOAD_SHADERS, 0);
  static std::string file_path = wi::helper::GetCurrentPath() +
                                  "/startup.lua";
  if (wi::lua::RunFile(file_path))
    wi::backlog::post("Hot reloaded startup file: " + file_path);
}

int sdl_loop(my::Application& application) {
  SDL_Event event;
  bool ctrl_left = false;
  bool ctrl_right = false;

  bool quit = false;
  while (!quit)
  {
    SDL_PumpEvents();
    application.Run();

    while( SDL_PollEvent(&event)) 
    {
      switch (event.type) {
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE)
          quit = true;
        else if (event.key.keysym.sym == SDLK_SPACE &&
                 (ctrl_left || ctrl_right))
          hot_reload_script(application);
        else if (event.key.keysym.sym == SDLK_LCTRL)
          ctrl_left = true;
        else if (event.key.keysym.sym == SDLK_RCTRL)
          ctrl_right = true;
        break;
      case SDL_KEYUP:
        if (event.key.keysym.sym == SDLK_LCTRL)
          ctrl_left = false;
        else if (event.key.keysym.sym == SDLK_RCTRL)
          ctrl_right = false;
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

int main(int argc, char *argv[]) {
    my::Application application;

    application.infoDisplay.active = true;
    application.infoDisplay.watermark = true;
    application.infoDisplay.resolution = true;
    application.infoDisplay.fpsinfo = true;

    sdl2::sdlsystem_ptr_t system = sdl2::make_sdlsystem(SDL_INIT_EVERYTHING | SDL_INIT_EVENTS);
    sdl2::window_ptr_t window = sdl2::make_window(
            "Template",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            1920, 1080,
            SDL_WINDOW_SHOWN
              | SDL_WINDOW_VULKAN
              | SDL_WINDOW_RESIZABLE
              | SDL_WINDOW_ALLOW_HIGHDPI);

    SDL_Event event;

    if (!window) {
      std::exit(1);
    }

    // SDL_SetWindowFullscreen(window.get(), SDL_WINDOW_FULLSCREEN_DESKTOP);
    application.SetWindow(window.get());

    watch_shaders_start({"../../Template_Linux/my_shader.hlsl"});
    int ret = sdl_loop(application);
    watch_shaders_finish();

    SDL_Quit();

    return ret;
}
