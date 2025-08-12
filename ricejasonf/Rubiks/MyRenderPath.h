#pragma once

#include <wiApplication.h>
#include <wiEventHandler.h>
#include <wiRenderPath3D.h>

namespace my {

class RenderPath : public wi::RenderPath3D {
public:
  unsigned entity = 0;
  int my_shader_index = -1;
  void Load() override;
  void Update(float dt) override;
  void SetBoxShader(wi::ecs::Entity);
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
