#pragma once

#include <wiRenderPath3D.h>
#include <wiScene.h>

namespace my {

class RenderPath : public wi::RenderPath3D {
  wi::ecs::Entity box = {};
public:
  void Start() override;
  void Update(float dt) override;
};

}
