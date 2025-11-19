#include <Grindstone.Renderer.RenderGraph/include/pch.hpp>

#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/EngineCore.hpp>

#include <Grindstone.Renderer.RenderGraph/include/RenderGraphBuilder.hpp>

using namespace Grindstone;
using namespace Grindstone::Memory;

namespace Grindstone::Renderer {
	namespace PassNames {
		constexpr Grindstone::ConstHashedString gbuffer = "Grindstone.Pass.Gbuffer";
		constexpr Grindstone::ConstHashedString shadow = "Grindstone.Pass.Shadow";
		constexpr Grindstone::ConstHashedString lighting = "Grindstone.Pass.Lighting";
		constexpr Grindstone::ConstHashedString forward = "Grindstone.Pass.Forward";
		constexpr Grindstone::ConstHashedString ssao = "Grindstone.Pass.SSAO";
		constexpr Grindstone::ConstHashedString ssaoBlur = "Grindstone.Pass.SSAO Blur";
		constexpr Grindstone::ConstHashedString bloom = "Grindstone.Pass.Bloom";
		constexpr Grindstone::ConstHashedString bloomDownSample1 = "Grindstone.Pass.BloomDownsample-1";
		constexpr Grindstone::ConstHashedString bloomDownSample2 = "Grindstone.Pass.BloomDownsample-2";
		constexpr Grindstone::ConstHashedString bloomDownSample3 = "Grindstone.Pass.BloomDownsample-3";
		constexpr Grindstone::ConstHashedString bloomDownSample4 = "Grindstone.Pass.BloomDownsample-4";
		constexpr Grindstone::ConstHashedString bloomDownSample5 = "Grindstone.Pass.BloomDownsample-5";
		constexpr Grindstone::ConstHashedString bloomDownSample6 = "Grindstone.Pass.BloomDownsample-6";
		constexpr Grindstone::ConstHashedString bloomUpSample6 = "Grindstone.Pass.BloomDownsample-6";
		constexpr Grindstone::ConstHashedString bloomUpSample5 = "Grindstone.Pass.BloomDownsample-5";
		constexpr Grindstone::ConstHashedString bloomUpSample4 = "Grindstone.Pass.BloomDownsample-4";
		constexpr Grindstone::ConstHashedString bloomUpSample3 = "Grindstone.Pass.BloomDownsample-3";
		constexpr Grindstone::ConstHashedString bloomUpSample2 = "Grindstone.Pass.BloomDownsample-2";
		constexpr Grindstone::ConstHashedString bloomUpSample1 = "Grindstone.Pass.BloomDownsample-1";
		constexpr Grindstone::ConstHashedString tonemap = "Grindstone.Pass.Tonemap";

		namespace Gizmos {
			constexpr Grindstone::ConstHashedString primGizmo = "Grindstone.Pass.PrimitiveGizmo";
			constexpr Grindstone::ConstHashedString grid = "Grindstone.Pass.Grid";
		}

		namespace Debug {
			constexpr Grindstone::ConstHashedString debug = "Grindstone.Pass.Debug";
		}
	}

	namespace AttachmentNames {
		constexpr Grindstone::ConstHashedString albedo = "Grindstone.Attachment.Gbuffer.Albedo";
		constexpr Grindstone::ConstHashedString normal = "Grindstone.Attachment.Gbuffer.Normal";
		constexpr Grindstone::ConstHashedString specularRoughness = "Grindstone.Attachment.Gbuffer.SpecularRoughness";
		constexpr Grindstone::ConstHashedString depthPostGbuffer = "Grindstone.Attachment.Depth::PostGbuffer";
		constexpr Grindstone::ConstHashedString depthPostForward = "Grindstone.Attachment.Depth::PostForward";
		constexpr Grindstone::ConstHashedString shadowAtlas = "Grindstone.Attachment.ShadowAtlas";
		constexpr Grindstone::ConstHashedString ssao = "Grindstone.Attachment.SSAO";
		constexpr Grindstone::ConstHashedString ssaoBlurred = "Grindstone.Attachment.SSAO Blurred";
		constexpr Grindstone::ConstHashedString hdrPreForward = "Grindstone.Attachment.Hdr::PreForward";
		constexpr Grindstone::ConstHashedString hdr = "Grindstone.Attachment.Hdr::PostForward";
		constexpr Grindstone::ConstHashedString bloom = "Grindstone.Attachment.Bloom";
		constexpr Grindstone::ConstHashedString tonemapped = "Grindstone.Attachment.Tonemapped";

		namespace Debug {
			constexpr Grindstone::ConstHashedString sceneWithGizmos = "Grindstone.Attachment.Debug.SceneWithGizmos";
			constexpr Grindstone::ConstHashedString postDebug = "Grindstone.Attachment.PostDebug";
		}
	}

	namespace AttachmentInfos {
		const AttachmentInfo albedo{};
		const AttachmentInfo normal{};
		const AttachmentInfo specularRoughness{};
		const AttachmentInfo depth{};
		const AttachmentInfo shadowAtlas{};
		const AttachmentInfo ssao{};
		const AttachmentInfo hdr{};
		const AttachmentInfo bloom{};
		const AttachmentInfo swapchain{};
	}
}

namespace Grindstone {
	using ObserverHandle = size_t;

	template<typename... Args>
	class SinglecastObservable {
	public:
		using ObserverFunction = std::function<void(Args...)>;

		void Broadcast(Args... args) {
			std::scoped_lock lck(mutex);
			function(args...);
		}

		void Subscribe(ObserverFunction func) {
			std::scoped_lock lck(mutex);
			function = func;
		}

		void Unsubscribe() {
			std::scoped_lock lck(mutex);
			function = nullptr;
		}

	protected:

		std::mutex mutex;
		ObserverFunction function;

	};

	template<typename... Args>
	class MulticastObservable {
	public:
		using ObserverFunction = std::function<void(Args...)>;

		void Broadcast(Args... args) {
			std::scoped_lock lck(mutex);
			for (auto& [_, func] : observers) {
				func(args...);
			}
		}

		ObserverHandle Subscribe(ObserverFunction func) {
			std::scoped_lock lck(mutex);
			ObserverHandle handle = ++currentHandle;
			observers.emplace(handle, func);
			return handle;
		}

		void Unsubscribe(ObserverHandle handle) {
			std::scoped_lock lck(mutex);
			observers.erase(handle);
		}

	protected:

		std::mutex mutex;
		ObserverHandle currentHandle = 0;
		std::unordered_map<ObserverHandle, ObserverFunction> observers;

	};
}

static void BuildGbufferPass(Grindstone::Renderer::RenderGraphBuilder& builder) {
	using namespace Grindstone::Renderer;

	RenderGraphPass& gbufferPass = builder.AddPass(PassNames::gbuffer, GpuQueue::Graphics);
	gbufferPass.AddOutputImage(AttachmentNames::albedo, AttachmentInfos::albedo);
	gbufferPass.AddOutputImage(AttachmentNames::normal, AttachmentInfos::normal);
	gbufferPass.AddOutputImage(AttachmentNames::specularRoughness, AttachmentInfos::specularRoughness);
	gbufferPass.AddOutputImage(AttachmentNames::depthPostGbuffer, AttachmentInfos::depth);
}

static void BuildLightingPass(Grindstone::Renderer::RenderGraphBuilder& builder) {
	using namespace Grindstone::Renderer;

	RenderGraphPass& lightingPass = builder.AddPass(PassNames::lighting, GpuQueue::Graphics);
	lightingPass.AddInputImage(AttachmentNames::ssao, AttachmentInfos::ssao);
	lightingPass.AddInputImage(AttachmentNames::albedo, AttachmentInfos::albedo);
	lightingPass.AddInputImage(AttachmentNames::normal, AttachmentInfos::normal);
	lightingPass.AddInputImage(AttachmentNames::specularRoughness, AttachmentInfos::specularRoughness);
	lightingPass.AddInputImage(AttachmentNames::shadowAtlas, AttachmentInfos::shadowAtlas);
	lightingPass.AddInputImage(AttachmentNames::depthPostGbuffer, AttachmentInfos::depth);
	lightingPass.AddOutputImage(AttachmentNames::hdrPreForward, AttachmentInfos::hdr);
}

static void BuildForwardPass(Grindstone::Renderer::RenderGraphBuilder& builder) {
	using namespace Grindstone::Renderer;

	RenderGraphPass& forwardPass = builder.AddPass(PassNames::forward, GpuQueue::Graphics);
	forwardPass.AddInputOutputImage(AttachmentNames::depthPostGbuffer, AttachmentNames::depthPostForward, AttachmentInfos::hdr);
	forwardPass.AddInputOutputImage(AttachmentNames::hdrPreForward, AttachmentNames::hdr, AttachmentInfos::hdr);
}

static void BuildScreenSpaceAmbientOcclusonPass(Grindstone::Renderer::RenderGraphBuilder& builder) {
	using namespace Grindstone::Renderer;

	RenderGraphPass& ssaoPass = builder.AddPass(PassNames::ssao, GpuQueue::Graphics);
	ssaoPass.AddInputImage(AttachmentNames::normal, AttachmentInfos::normal);
	ssaoPass.AddInputImage(AttachmentNames::depthPostGbuffer, AttachmentInfos::depth);
	ssaoPass.AddOutputImage(AttachmentNames::ssao, AttachmentInfos::ssao);

	// TODO: Split into horizontal and vertical blur
	RenderGraphPass& ssaoBlurPass = builder.AddPass(PassNames::ssaoBlur, GpuQueue::Graphics);
	ssaoPass.AddInputImage(AttachmentNames::ssao, AttachmentInfos::ssao);
	ssaoPass.AddOutputImage(AttachmentNames::ssaoBlurred, AttachmentInfos::ssao);

	RenderGraphPass& shadowPass = builder.AddPass(PassNames::shadow, GpuQueue::Graphics);
	shadowPass.AddOutputImage(AttachmentNames::shadowAtlas, AttachmentInfos::shadowAtlas);
}

static void BuildBloomPass(Grindstone::Renderer::RenderGraphBuilder& builder) {
	using namespace Grindstone::Renderer;

	RenderGraphPass& bloomPass = builder.AddPass(PassNames::bloom, GpuQueue::Compute);
	bloomPass.AddInputImage(AttachmentNames::hdr, AttachmentInfos::hdr);
	bloomPass.AddOutputImage(AttachmentNames::bloom, AttachmentInfos::bloom);
}

static void BuildTonemapPass(Grindstone::Renderer::RenderGraphBuilder& builder) {
	using namespace Grindstone::Renderer;

	RenderGraphPass& tonemapPass = builder.AddPass(PassNames::tonemap, GpuQueue::Graphics);
	tonemapPass.AddInputImage(AttachmentNames::hdr, AttachmentInfos::hdr);
	tonemapPass.AddInputImage(AttachmentNames::bloom, AttachmentInfos::bloom);
	tonemapPass.AddOutputImage(AttachmentNames::tonemapped, AttachmentInfos::swapchain);
}

static void BuildDebugPass(Grindstone::Renderer::RenderGraphBuilder& builder) {
	using namespace Grindstone::Renderer;

	RenderGraphPass& debugPass = builder.AddPass(PassNames::Debug::debug, GpuQueue::Graphics);
	debugPass.AddInputOutputImage(AttachmentNames::tonemapped, AttachmentNames::Debug::sceneWithGizmos, AttachmentInfos::swapchain);
	debugPass.AddInputOutputImage(AttachmentNames::Debug::sceneWithGizmos, AttachmentNames::Debug::postDebug, AttachmentInfos::swapchain);
}

using RenderGraphDelegate = MulticastObservable<Grindstone::Renderer::RenderGraphBuilder&>;

extern "C" {
	RENDER_GRAPH_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		using namespace Grindstone::Renderer;

		RenderGraphDelegate renderGraphBuildEvent;
		ObserverHandle gbufferFnHandle = renderGraphBuildEvent.Subscribe(BuildGbufferPass);
		ObserverHandle lightingFnHandle = renderGraphBuildEvent.Subscribe(BuildLightingPass);
		ObserverHandle forwardFnHandle = renderGraphBuildEvent.Subscribe(BuildForwardPass);
		ObserverHandle ssaoFnHandle = renderGraphBuildEvent.Subscribe(BuildScreenSpaceAmbientOcclusonPass);
		ObserverHandle bloomFnHandle = renderGraphBuildEvent.Subscribe(BuildBloomPass);
		ObserverHandle tonemapFnHandle = renderGraphBuildEvent.Subscribe(BuildTonemapPass);
		ObserverHandle debugFnHandle = renderGraphBuildEvent.Subscribe(BuildDebugPass);

		{
			RenderGraphBuilder builder;
			renderGraphBuildEvent.Broadcast(builder);
			builder.SetOutputAttachment(AttachmentNames::tonemapped);
			bool isDebug = false;
			if (isDebug) {
				builder.SetOutputAttachment(AttachmentNames::Debug::postDebug);
			}

			RenderGraph renderGraph = builder.Compile();
			renderGraph.Print();
		}

		renderGraphBuildEvent.Unsubscribe(gbufferFnHandle);
		renderGraphBuildEvent.Unsubscribe(lightingFnHandle);
		renderGraphBuildEvent.Unsubscribe(forwardFnHandle);
		renderGraphBuildEvent.Unsubscribe(ssaoFnHandle);
		renderGraphBuildEvent.Unsubscribe(bloomFnHandle);
		renderGraphBuildEvent.Unsubscribe(tonemapFnHandle);
		renderGraphBuildEvent.Unsubscribe(debugFnHandle);
	}

	RENDER_GRAPH_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
	}
}
