#include "MyRenderPath.h"
#include <iostream>

namespace my {

void RenderPath::Start() {
  box = scene->Entity_CreateCube("box");

  // Get outside of the box!
  if (wi::scene::TransformComponent* transform
        = scene->transforms.GetComponent(box)) {
    transform->Translate(XMFLOAT3{0.0f, 0.0f, 5.0f});
  }
}

void RenderPath::Update(float dt) {
  // Rotate the box!
  if (wi::scene::TransformComponent* transform
        = scene->transforms.GetComponent(box)) {
    transform->RotateRollPitchYaw(XMFLOAT3{1.0f * dt, 0.0f, 0.0f});
  }
  RenderPath3D::Update(dt);
}

}
