#include "Core/Engine.hpp"
#include "RenderSpriteSystem.hpp"
#include "TransformSystem.hpp"
#include "../Core/Engine.hpp"
#include "../Core/Scene.hpp"
#include "../Core/Space.hpp"
#include "../Utilities/SettingsFile.hpp"
#include "../AssetManagers/TextureManager.hpp"
#include "../GraphicsCommon/GraphicsWrapper.hpp"
#include "LightPointSystem.hpp"
#include "LightSpotSystem.hpp"
#include "LightDirectionalSystem.hpp"

RenderSpriteComponent::RenderSpriteComponent(GameObjectHandle object_handle, ComponentHandle handle) :
	Component(COMPONENT_RENDER_SPRITE, object_handle, handle), color_(1.0f, 1.0f, 1.0f, 1.0f), aspect_(1.0f) {}

RenderSpriteSubSystem::RenderSpriteSubSystem(Space *space) : SubSystem(COMPONENT_RENDER_SPRITE, space) {
}

ComponentHandle RenderSpriteSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	auto &component = components_.back();

	return component_handle;
}

RenderSpriteSubSystem::~RenderSpriteSubSystem() {}

void RenderSpriteSubSystem::renderSprite(bool render_ortho, float aspect, glm::vec4 color, TextureBinding *binding, glm::mat4 model) {
	auto graphics_wrapper = engine.getGraphicsWrapper();
	auto system = ((RenderSpriteSystem *)engine.getSystem(COMPONENT_RENDER_SPRITE));
	
	//Texture *tex = engine.getTextureManager()->getTexture(comp.texture_handle_);

	// Bind Shader + UBO
	SpriteUniformBuffer ubo;
	ubo.aspect = aspect;
	ubo.color = color;
	ubo.model = model;
	ubo.render_ortho = render_ortho;

	system->pipeline_->Bind();
	system->ubo_->UpdateUniformBuffer(&ubo);
	system->ubo_->Bind();
	engine.getUniformBuffer()->Bind();

	// Bind Texture	
	graphics_wrapper->BindTextureBinding(binding);

	// Render Quad
	graphics_wrapper->BindVertexArrayObject(engine.getPlaneVAO());
	graphics_wrapper->DrawImmediateVertices(0, 6);
}

void RenderSpriteSubSystem::renderSprites(bool render_ortho, glm::vec3 cam_pos, DepthTarget *depth_target) {
	auto graphics_wrapper = engine.getGraphicsWrapper();
	graphics_wrapper->SetImmediateBlending(BLEND_ADD_ALPHA);
	graphics_wrapper->CopyToDepthBuffer(depth_target);
	graphics_wrapper->EnableDepth(true);

	RenderSpriteSystem *system = ((RenderSpriteSystem *)engine.getSystem(COMPONENT_RENDER_SPRITE));

	TransformSubSystem *sub = (TransformSubSystem *)space_->getSubsystem(COMPONENT_TRANSFORM);

	for (auto &comp : components_) {
		// Get Position
		GameObject game_object = space_->getObject(comp.game_object_handle_);
		ComponentHandle comp_handle = game_object.getComponentHandle(COMPONENT_TRANSFORM);
		auto model = sub->getModelMatrix(comp_handle);
		auto pos = sub->getPosition(comp_handle);

		renderSprite(render_ortho, comp.aspect_, comp.color_, comp.texture_binding_, model);
	}

	LightPointSubSystem *lightpointsys = (LightPointSubSystem *)space_->getSubsystem(COMPONENT_LIGHT_POINT);
	for (int i = 0; i < lightpointsys->getNumComponents(); ++i) {
		auto &comp = lightpointsys->getComponent(i);
		handleDebugSprite(render_ortho, comp.properties_.color, cam_pos, comp.game_object_handle_, system->debug_light_pos_sprite_);
	}

	LightSpotSubSystem *lightspotsys = (LightSpotSubSystem *)space_->getSubsystem(COMPONENT_LIGHT_SPOT);
	for (int i = 0; i < lightspotsys->getNumComponents(); ++i) {
		auto &comp = lightspotsys->getComponent(i);
		handleDebugSprite(render_ortho, comp.properties_.color, cam_pos, comp.game_object_handle_, system->debug_light_pos_sprite_);
	}

	LightDirectionalSubSystem *lightdirectionalsys = (LightDirectionalSubSystem *)space_->getSubsystem(COMPONENT_LIGHT_DIRECTIONAL);
	for (int i = 0; i < lightdirectionalsys->getNumComponents(); ++i) {
		auto &comp = lightdirectionalsys->getComponent(i);
		handleDebugSprite(render_ortho, comp.properties_.color, cam_pos, comp.game_object_handle_, system->debug_light_pos_sprite_);
	}
}

void RenderSpriteSubSystem::handleDebugSprite(bool render_ortho, glm::vec3 color, glm::vec3 cam_pos, GameObjectHandle game_obj_handle, TextureBinding *tex_binding) {
	TransformSubSystem *sub = (TransformSubSystem *)space_->getSubsystem(COMPONENT_TRANSFORM);
	GameObject game_object = space_->getObject(game_obj_handle);
	ComponentHandle comp_handle = game_object.getComponentHandle(COMPONENT_TRANSFORM);
	auto model = sub->getModelMatrix(comp_handle);
	auto pos = sub->getPosition(comp_handle);

	float d = 1.0f - glm::clamp((glm::distance(cam_pos, pos) / 2.0f - 10.0f), 0.0f, 1.0f);

	glm::vec4 final_color = glm::vec4(color.r, color.g, color.b, d);

	renderSprite(render_ortho, 1.0f, final_color, tex_binding, model);
}

ComponentHandle RenderSpriteSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value &params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	setComponent(component_handle, params);

	return component_handle;
}

void RenderSpriteSubSystem::setComponent(ComponentHandle component_handle, rapidjson::Value & params) {
	auto &component = components_[component_handle];

	if (params.HasMember("path")) {
		auto path = params["path"].GetString();
		component.path_ = path;
		component.texture_handle_ = engine.getTextureManager()->loadTexture(path);
		Texture *tex = engine.getTextureManager()->getTexture(component.texture_handle_);
		
		SingleTextureBind stb;
		stb.address = 0;
		stb.texture = tex;

		TextureBindingCreateInfo ci;
		ci.layout = ((RenderSpriteSystem *)engine.getSystem(COMPONENT_RENDER_SPRITE))->tbl_;
		ci.textures = &stb;
		ci.textureCount = 1;
		component.texture_binding_ = engine.getGraphicsWrapper()->CreateTextureBinding(ci);
	}
}

void RenderSpriteSubSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

RenderSpriteComponent &RenderSpriteSubSystem::getComponent(ComponentHandle id) {
	return components_[id];
}

Component * RenderSpriteSubSystem::getBaseComponent(ComponentHandle component_handle) {
	return &components_[component_handle];
}

size_t RenderSpriteSubSystem::getNumComponents() {
	return components_.size();
}

void RenderSpriteSubSystem::writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) {
	auto &c = getComponent(handle);

	w.Key("path");
	w.String(c.path_.c_str());
}

void RenderSpriteSystem::update(double dt) {
}

RenderSpriteSystem::RenderSpriteSystem() : System(COMPONENT_RENDER_SPRITE) {
	auto graphics_wrapper = engine.getGraphicsWrapper();

	UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 1;
	ubbci.shaderLocation = "SpriteUBO";
	ubbci.size = sizeof(SpriteUniformBuffer);
	ubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	ubb_ = engine.getGraphicsWrapper()->CreateUniformBufferBinding(ubbci);

	UniformBufferCreateInfo uboci;
	uboci.isDynamic = false;
	uboci.size = sizeof(SpriteUniformBuffer);
	uboci.binding = ubb_;
	ubo_ = graphics_wrapper->CreateUniformBuffer(uboci);

	TextureSubBinding subbinding;
	subbinding.shaderLocation = "image";
	subbinding.textureLocation = 0;

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 0;
	tblci.bindings = &subbinding;
	tblci.bindingCount = 1;
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	tbl_ = engine.getGraphicsWrapper()->CreateTextureBindingLayout(tblci);

	ShaderStageCreateInfo vi;
	ShaderStageCreateInfo fi;
	if (engine.getSettings()->graphics_language_ == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/spriteVert.glsl";
		fi.fileName = "../assets/shaders/spriteFrag.glsl";
	}
	else if (engine.getSettings()->graphics_language_ == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/spriteVert.fxc";
		fi.fileName = "../assets/shaders/spriteFrag.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/spriteVert.spv";
		fi.fileName = "../assets/shaders/spriteFrag.spv";
	}
	std::vector<char> vfile;
	if (!readFile(vi.fileName, vfile))
		return;
	vi.content = vfile.data();
	vi.size = (uint32_t)vfile.size();
	vi.type = SHADER_VERTEX;

	std::vector<char> ffile;
	if (!readFile(fi.fileName, ffile))
		return;
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = SHADER_FRAGMENT;

	std::vector<ShaderStageCreateInfo> stages = { vi, fi };

	auto vbd = engine.getPlaneVBD();
	auto vad = engine.getPlaneVAD();

	GraphicsPipelineCreateInfo spriteGPCI;
	spriteGPCI.cullMode = CULL_BACK;
	spriteGPCI.bindings = &vbd;
	spriteGPCI.bindingsCount = 1;
	spriteGPCI.attributes = &vad;
	spriteGPCI.attributesCount = 1;
	spriteGPCI.width = (float)engine.getSettings()->resolution_x_;
	spriteGPCI.height = (float)engine.getSettings()->resolution_y_;
	spriteGPCI.scissorW = engine.getSettings()->resolution_x_;
	spriteGPCI.scissorH = engine.getSettings()->resolution_y_;
	spriteGPCI.primitiveType = PRIM_TRIANGLE_STRIPS;
	spriteGPCI.shaderStageCreateInfos = stages.data();
	spriteGPCI.shaderStageCreateInfoCount = (uint32_t)stages.size();
	std::vector<TextureBindingLayout *> tbls_ = { };
	spriteGPCI.textureBindings = tbls_.data();
	spriteGPCI.textureBindingCount = (uint32_t)tbls_.size();
	std::vector<UniformBufferBinding *> ubbs = { engine.getUniformBufferBinding(), ubb_ };
	spriteGPCI.uniformBufferBindings = ubbs.data();
	spriteGPCI.uniformBufferBindingCount = (uint32_t)ubbs.size();
	pipeline_ = engine.getGraphicsWrapper()->CreateGraphicsPipeline(spriteGPCI);

	loadDebugSprites();
}

void RenderSpriteSystem::loadDebugSprites() {
	TextureHandler t = engine.getTextureManager()->loadTexture("../engineassets/materials/debug/light_point_sprite.png");
	Texture *tex = engine.getTextureManager()->getTexture(t);

	SingleTextureBind stb;
	stb.address = 0;
	stb.texture = tex;

	TextureBindingCreateInfo ci;
	ci.layout = tbl_;
	ci.textures = &stb;
	ci.textureCount = 1;
	debug_light_pos_sprite_ = engine.getGraphicsWrapper()->CreateTextureBinding(ci);
}
