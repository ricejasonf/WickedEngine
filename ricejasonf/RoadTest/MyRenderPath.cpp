#include "MyRenderPath.h"
#include <wiBacklog.h>
#include <wiGraphics.h>
#include <wiRenderer.h>
#include <wiInitializer.h>
#include <wiInput.h>
#include <wiScene.h>
#include <cmath>

namespace {

class RoadTest {
  using Entity = wi::ecs::Entity;
  using Scene = wi::scene::Scene;

  static constexpr float scale_out = 2.1f;
  static constexpr float sticker_thickness = 0.025f;

  Scene& scene;
  Entity FirstStreet = wi::ecs::INVALID_ENTITY;

public:
  RoadTest(wi::scene::Scene& scene_)
    : scene(scene_)
  { }

  void LoadRoad() {
    constexpr float Pi = 3.14159265359;
    FirstStreet = scene.Entity_CreateCube("");
#if 0
    wi::scene::SplineComponent& SC = scene.splines.Create(FirstStreet);
    SC.SetLooped();
    for (float theta = 0.0; theta < 2.0 * Pi; theta += Pi / 4.0)
      LoadRoadNode(FirstStreet, SC, std::cos(theta), std::sin(theta));
#endif

    wi::scene::TransformComponent& TC = scene.transforms.Create(FirstStreet);
    TC.Translate(XMFLOAT3{0.0f, 0.0f, 50.0f});
    TC.Scale(XMFLOAT3{10.0f, 10.0f, 10.0f});
  }

  void LoadRoadNode(Entity Street,
                    wi::scene::SplineComponent& SC,
                    float x, float y) {
    Entity Node = wi::ecs::CreateEntity();
    wi::scene::TransformComponent& TC = scene.transforms.Create(Node);
    TC.Translate(XMVectorSet(x, y, 0, 0));
    TC.UpdateTransform();
    SC.spline_node_entities.push_back(Node);
    SC.spline_node_transforms.push_back(TC);
    scene.Component_Attach(Node, Street);
  }
};

}

namespace my {

void Application::Run() {
  wi::Application::Run();
}

void RenderPath::Load() {
  RoadTest RT(*scene);
  RT.LoadRoad();
}

void RenderPath::Update(float dt) {
  RenderPath3D::Update(dt);
}

}
