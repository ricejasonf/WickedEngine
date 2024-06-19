#include "MyRenderPath.h"
#include "wiBacklog.h"
#include "wiGraphics.h"
#include "wiInitializer.h"
#include "wiScene.h"
#include "wiShaderCompiler.h"

namespace {

class PuzzleCube {
  using Entity = wi::ecs::Entity;
  using Scene = wi::scene::Scene;

  static constexpr float scale_out = 2.1f;
  static constexpr float sticker_thickness = 0.025f;

  Scene& scene;

public:
  Entity entity;  // The root object thing.
  int my_shader_index = -1;
private:
  std::array<Entity, 27> pieces;

  // Each sticker color represents a face
  // of the cube that we can project stuff to.
  // Orient opposite faces as upside down to
  // each other (except Green and Blue).
  std::array<Entity, 9> stickers_green;
  std::array<Entity, 9> stickers_blue;
  std::array<Entity, 9> stickers_red;
  std::array<Entity, 9> stickers_orange;
  std::array<Entity, 9> stickers_white;
  std::array<Entity, 9> stickers_yellow;

  Entity addSticker(Scene& scene, Entity piece,
                    XMFLOAT3 const& scale,
                    XMFLOAT3 const& translate) {
      Entity sticker = scene.Entity_CreateCube(std::string{});
      scene.Component_Attach(sticker, piece);
      if (wi::scene::TransformComponent* transform
            = scene.transforms.GetComponent(sticker)) {
        transform->Scale(scale);
        transform->Translate(translate);
      }
      return sticker;
  }
  void addStickers(Scene& scene, Entity piece,
                   unsigned x, unsigned y, unsigned z) {
    Entity sticker = scene.Entity_CreateCube(std::string{});
    x %= 3;
    y %= 3;
    z %= 3;

    if (x == 0) // Green
      stickers_green[z * 3 + y] = addSticker(scene, piece,
        XMFLOAT3{sticker_thickness, 0.9f, 0.9f},
        XMFLOAT3{-1.0f - sticker_thickness, 0.0f, 0.0f});
    else if (x == 2) // Blue
      stickers_blue[z * 3 + y] = addSticker(scene, piece,
        XMFLOAT3{sticker_thickness, 0.9f, 0.9f},
        XMFLOAT3{1.0f + sticker_thickness, 0.0f, 0.0f});
    if (y == 0) // Red
      stickers_red[x * 3 + z] = addSticker(scene, piece,
        XMFLOAT3{0.9f, sticker_thickness, 0.9f},
        XMFLOAT3{0.0f, -1.0f - sticker_thickness, 0.0f});
    else if (y == 2) // Orange
      stickers_orange[8 - (x * 3 + z)] = addSticker(scene, piece,
        XMFLOAT3{0.9f, sticker_thickness, 0.9f},
        XMFLOAT3{0.0f, 1.0f + sticker_thickness, 0.0f});
    if (z == 0) // Yellow
      stickers_yellow[x * 3 + y] = addSticker(scene, piece,
        XMFLOAT3{0.9f, 0.9f, sticker_thickness},
        XMFLOAT3{0.0f, 0.0f, -1.0f - sticker_thickness});
    else if (z == 2) // White
      stickers_white[8 - (x * 3 + y)] = addSticker(scene, piece,
        XMFLOAT3{0.9f, 0.9f, sticker_thickness},
        XMFLOAT3{0.0f, 0.0f, 1.0f + sticker_thickness});
  }

  Entity createPiece(wi::scene::Scene& scene, unsigned x, unsigned y, unsigned z) {
    Entity piece = scene.Entity_CreateCube(std::string{});
    scene.Component_Attach(piece, entity);
    if (wi::scene::TransformComponent* transform
          = scene.transforms.GetComponent(piece)) {
      transform->Translate(XMFLOAT3{scale_out * (float(x % 3) - 1.0f),
                                    scale_out * (float(y % 3) - 1.0f),
                                    scale_out * (float(z % 3) - 1.0f)});
    }
    if (wi::scene::MaterialComponent* material
          = scene.materials.GetComponent(piece)) {
      material->SetBaseColor(XMFLOAT4{0.0f, 0.0f, 0.0f, 0.0f});
      material->SetCustomShaderID(my_shader_index);
    }
    return piece;
  }

  void createPieces(wi::scene::Scene& scene) {
    // Start at the left, top, back corner.
    unsigned x = 0;
    unsigned y = 0;
    unsigned z = 0;
    for (auto& piece : pieces) {
      if (!(x % 3 == 1 && y % 3 == 1 && z % 3 == 1)) {
        piece = createPiece(scene, x, y, z);
        addStickers(scene, piece, x, y, z);
      }

      x += 1;
      if (x % 3 == 0)
        y += 1;
      if (x % 9 == 0)
        z += 1;
    }

    applyStickerColor(stickers_green,  0.0f, 1.0f, 0.0f);
    applyStickerColor(stickers_blue,   0.0f, 0.0f, 1.0f);
    applyStickerColor(stickers_red  ,  1.0f, 0.0f, 0.0f);
    applyStickerColor(stickers_orange, 1.0f, 0.50f, 0.0f);
    applyStickerColor(stickers_white,  1.0f, 1.0f, 1.0f);
    applyStickerColor(stickers_yellow, 1.0f, 1.0f, 0.0f);
  }

  void applyStickerColor(std::array<Entity, 9> stickers,
                            float red, float green, float blue) {
    for (Entity sticker : stickers) {
      if (wi::scene::MaterialComponent* material
            = scene.materials.GetComponent(sticker)) {
        material->SetBaseColor(XMFLOAT4{red, green, blue, 0.0f});
        material->SetClearcoatFactor(1.0f);
        material->SetCustomShaderID(my_shader_index);
      }
    }
  }

public:

  PuzzleCube(wi::scene::Scene& scene_, int shader)
    : scene(scene_),
      my_shader_index(shader)
  {
    entity = wi::ecs::CreateEntity();
    wi::scene::TransformComponent& transform = scene.transforms.Create(entity);
    wi::scene::NameComponent& name = scene.names.Create(entity);
    name.name = "cube";
    createPieces(scene);
  }

};

}

namespace my {

void Application::Run() {
  // The event handler being set also indicates
  // that we loaded the stuff;
  if (!load_shaders_event_handler.IsValid() &&
      IsInitializeFinished(wi::initializer::INITIALIZED_SYSTEM_RENDERER)) {
    LoadShaders();
    render_path.Load();
    ActivatePath(&render_path);
    load_shaders_event_handler = wi::eventhandler::Subscribe(
      wi::eventhandler::EVENT_RELOAD_SHADERS,
      [this](auto) { 
          this->LoadShaders();
          this->render_path.SetBoxShader();
        });
  }

  wi::Application::Run();
}

// Load/Reload our custom shaders.
void Application::LoadShaders() {
  wi::graphics::ShaderStage stage = wi::graphics::ShaderStage::PS;
  wi::graphics::PipelineStateDesc desc;
  desc.vs = wi::renderer::GetShader(wi::enums::VSTYPE_OBJECT_COMMON);
  assert(desc.vs);
  assert(desc.vs->internal_state.get());
  wi::graphics::GraphicsDevice& device = *wi::graphics::GetDevice();

  // Compile the shader.
  wi::shadercompiler::CompilerInput input;
  wi::shadercompiler::CompilerOutput output;
  input.format = device.GetShaderFormat();
  input.stage = stage;
  input.shadersourcefilename = "../../Template_Linux/my_shader.hlsl";
  input.include_directories.push_back("../WickedEngine/shaders/");

  wi::shadercompiler::Compile(input, output);
  if (!output.IsValid()) {
    wi::backlog::post(output.error_message, wi::backlog::LogLevel::Warning);
    return;
  }
  if (!device.CreateShader(stage, output.shaderdata, output.shadersize, &my_shader)) {
    wi::backlog::post("unable to create shader", wi::backlog::LogLevel::Warning);
    return;
  }

  desc.ps = &my_shader;
  desc.bs = wi::renderer::GetBlendState(wi::enums::BSTYPE_ADDITIVE);
  desc.rs = wi::renderer::GetRasterizerState(wi::enums::RSTYPE_FRONT);
  desc.pt = wi::graphics::PrimitiveTopology::TRIANGLELIST;


  wi::graphics::PipelineState pso;
  wi::graphics::GetDevice()->CreatePipelineState(&desc, &pso);
  wi::renderer::CustomShader shader;
  shader.name = std::string("my_shader");
  shader.filterMask = wi::enums::FILTER_TRANSPARENT;
  shader.pso[wi::enums::RENDERPASS_MAIN] = pso;
  render_path.my_shader_index = wi::renderer::RegisterCustomShader(shader);
  wi::backlog::post(std::string("manually compiled/registered shader: ") +
                    std::string(input.shadersourcefilename));
}

void RenderPath::SetBoxShader() {
  if (wi::scene::MaterialComponent* material
        = scene->materials.GetComponent(box)) {
    material->SetCustomShaderID(my_shader_index);
    wi::backlog::post("shader id updated: " + std::to_string(my_shader_index));
    
  } else {
    wi::backlog::post("unable to update shader id",
        wi::backlog::LogLevel::Warning);
  }
}

void RenderPath::Load() {
#if 0
  PuzzleCube cube(*scene, /*shader_index=*/-1);
  box = cube.entity;
#endif
  box = scene->Entity_CreateCube(std::string{});

  // Get outside of the box!
  if (wi::scene::TransformComponent* transform
        = scene->transforms.GetComponent(box)) {
    transform->Translate(XMFLOAT3{0.0f, 0.0f, 5.0f});
  }
  SetBoxShader();

  // Make an interesting light.
  scene->Entity_CreateLight(std::string{},
    XMFLOAT3{15.0f, 5.0f, 0.0f}, // pos
    XMFLOAT3{1.0f, 1.0f, 1.0f}, // color
    1000.0f, // intensity
    100.0f); // range
}

void RenderPath::Update(float dt) {
  // Rotate the box!
  if (wi::scene::TransformComponent* transform
        = scene->transforms.GetComponent(box)) {
    transform->RotateRollPitchYaw(XMFLOAT3{1.0f * dt, 0.1f * dt, 0.0f});
  }

  RenderPath3D::Update(dt);
}

}
