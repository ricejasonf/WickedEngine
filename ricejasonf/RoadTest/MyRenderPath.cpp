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

  Scene& scene;
  Entity FirstStreet = wi::ecs::INVALID_ENTITY;

public:
  RoadTest(wi::scene::Scene& scene_)
    : scene(scene_)
  { }

  void LoadRoad() {
    FirstStreet = scene.Entity_CreateCube("");
    wi::scene::SplineComponent& SC = scene.splines.Create(FirstStreet);
    //SC.SetLooped();
    SC.mesh_generation_subdivision = 32;
    constexpr float Pi = 3.14159265359;
    float theta = 0.0;
    constexpr int NumNodes = 8;
    for (int i = 0; i < NumNodes; i++) {
      LoadRoadNode(FirstStreet, SC, std::cos(theta), std::sin(theta));
      theta += 2.0 * Pi / NumNodes;
      wilog("theta: %f", theta);
    }

    if (wi::scene::TransformComponent* TC
          = scene.transforms.GetComponent(FirstStreet)) {
      TC->Translate(XMFLOAT3{0.0f, 0.0f, 10.0f});
      //TC.Scale(XMFLOAT3{10.0f, 10.0f, 10.0f});
    }

    // Make an interesting light.
    scene.Entity_CreateLight(std::string{},
      XMFLOAT3{15.0f, 5.0f, 0.0f}, // pos
      XMFLOAT3{1.0f, 1.0f, 1.0f}, // color
      1000.0f, // intensity
      100.0f); // range
    wilog("LoadRoad complete");
  }

  void LoadRoadNode(Entity Street, wi::scene::SplineComponent& SC,
                    float x, float y) {
    Entity Node = wi::ecs::CreateEntity();
    wi::scene::TransformComponent& TC = scene.transforms.Create(Node);
    TC.Translate(XMVectorSet(x, y, 0, 0));
    //TC.UpdateTransform();
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

void Application::Initialize() {
  render_path.Load();
  ActivatePath(&render_path);
  wi::Application::Initialize();
}

void RenderPath::Load() {
  RoadTest RT(*scene);
  RT.LoadRoad();
}

void RenderPath::Update(float dt) {
  RenderPath3D::Update(dt);
}

}
