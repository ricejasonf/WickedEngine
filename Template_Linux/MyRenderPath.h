#pragma once

#include <wiApplication.h>
#include <wiEventHandler.h>
#include <wiRenderPath3D.h>

namespace wi::ecs {
  using Entity = uint32_t;
}

namespace my {

class RenderPath : public wi::RenderPath3D {
  friend class Application;
  wi::ecs::Entity box = {};
  int my_shader_index = 0;
public:
  void Start() override;
  void Update(float dt) override;
};

class Application : public wi::Application {
  RenderPath render_path;
  wi::eventhandler::Handle load_shaders_event_handler;
  void LoadShaders();
public:
  void Run();
};


}
