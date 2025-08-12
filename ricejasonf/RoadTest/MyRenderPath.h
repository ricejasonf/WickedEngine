#pragma once

#include <wiApplication.h>
#include <wiRenderPath3D.h>

namespace my {

class RenderPath : public wi::RenderPath3D {
public:
  unsigned current_entity = 0;
  std::array<wi::ecs::Entity, 2> entities;
  void Load() override;
  void Update(float dt) override;
  void SetBoxShader(wi::ecs::Entity);
};

class Application : public wi::Application {
  RenderPath render_path;
public:
  void Run();
};


}
