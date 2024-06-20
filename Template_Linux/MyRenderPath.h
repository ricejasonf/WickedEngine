#pragma once

#include <wiApplication.h>
#include <wiEventHandler.h>
#include <wiRenderPath3D.h>

namespace wi::ecs {
  using Entity = uint32_t;
}

namespace my {

class RenderPath : public wi::RenderPath3D {
public:
  wi::ecs::Entity box = {};
  int my_shader_index = -1;
  void Load() override;
  void Update(float dt) override;
  void SetBoxShader();
};

class Application : public wi::Application {
  RenderPath render_path;
  wi::eventhandler::Handle load_shaders_event_handler;
  wi::graphics::Shader my_shader;
  void LoadShaders();
public:
  void Run();
};


}
