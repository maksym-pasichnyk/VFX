#include "Assets.hpp"
#include "Application.hpp"
#include "ParticleSystem.hpp"
#include "ParticleEmitter.hpp"
#include "RocketParticleEmitter.hpp"

struct Game : Application {
public:
    Game() : Application("Particles-04") {
        explosionParticleSystem = sp<ParticleSystem>::of(device, 1000);
        explosionParticleSystem->updateEvent().connect<&Game::onExplosionParticleUpdate>(this);

        sparkleParticleSystem = sp<ParticleSystem>::of(device, 1000);
        sparkleParticleSystem->updateEvent().connect<&Game::onSparkleParticleUpdate>(this);

        rocketParticleSystem = sp<ParticleSystem>::of(device, 1000);
        rocketParticleSystem->updateEvent().connect<&Game::onRocketParticleUpdate>(this);
        rocketParticleSystem->deathEvent().connect<&Game::onRocketParticleDeath>(this);
        rocketParticleEmitter = sp<RocketParticleEmitter>::of(rocketParticleSystem, 2.5F);
    }

public:
    void update(float dt) override {
        camera_projection_matrix = getPerspectiveProjection(glm::radians(60.0F), getAspectRatio(), 0.03F, 1000.0F);
        world_to_camera_matrix = glm::lookAtLH(glm::vec3(25.0F, 10.0F, 0.0F), glm::vec3(0.0F, 10.0F, 0.0F), glm::vec3(0, 1, 0));

        rocketParticleEmitter->update(dt);
        rocketParticleSystem->update(dt, false);
        sparkleParticleSystem->update(dt, false);
        explosionParticleSystem->update(dt, true);
    }

    void render() override {
        auto drawable = swapchain.nextDrawable();
        auto drawableSize = swapchain.drawableSize();

        vk::Rect2D rendering_area = {};
        rendering_area.setOffset(vk::Offset2D{0, 0});
        rendering_area.setExtent(drawableSize);

        vk::Viewport rendering_viewport = {};
        rendering_viewport.setWidth(static_cast<float>(drawableSize.width));
        rendering_viewport.setHeight(static_cast<float>(drawableSize.height));
        rendering_viewport.setMinDepth(0.0F);
        rendering_viewport.setMaxDepth(1.0F);

        gfx::RenderingInfo rendering_info = {};
        rendering_info.renderArea = rendering_area;
        rendering_info.layerCount = 1;
        rendering_info.colorAttachments[0].texture = drawable.texture;
        rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;

        ShaderData shader_data = {};
        shader_data.g_proj_matrix = camera_projection_matrix;
        shader_data.g_view_matrix = world_to_camera_matrix;

        commandBuffer.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        commandBuffer.setImageLayout(drawable.texture, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2{}, vk::AccessFlagBits2::eColorAttachmentWrite);

        commandBuffer.setRenderPipelineState(rocketParticleSystem->renderPipelineState());
        commandBuffer.pushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(ShaderData), &shader_data);

        commandBuffer.beginRendering(rendering_info);
        commandBuffer.setScissor(0, rendering_area);
        commandBuffer.setViewport(0, rendering_viewport);

        rocketParticleSystem->draw(commandBuffer);
        sparkleParticleSystem->draw(commandBuffer);
        explosionParticleSystem->draw(commandBuffer);

        commandBuffer.endRendering();

        commandBuffer.setImageLayout(drawable.texture, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2{});
        commandBuffer.end();
        commandBuffer.submit();
        commandBuffer.present(drawable);
        commandBuffer.waitUntilCompleted();
    }

public:
    void onRocketParticleUpdate(Particle& particle, float dt) {
        particle.color.a = std::clamp(particle.lifetime / particle.time, 0.0F, 1.0F);

        glm::vec3 velocity = {};
        velocity.x = std::uniform_real_distribution(-1.0F, 1.0F)(random_engine) * 0.5F;
        velocity.y = 0.0F;
        velocity.z = std::uniform_real_distribution(-1.0F, 1.0F)(random_engine) * 0.5F;

        float lifetime = std::uniform_real_distribution(0.0F, 1.0F)(random_engine);

        sparkleParticleSystem->emit(particle.position, particle.color, velocity, lifetime);
    }

    void onRocketParticleDeath(Particle& particle) {
        for (int i = 0; i < 250; ++i) {
            glm::vec3 velocity = {};
            velocity.x = std::uniform_real_distribution(-1.0F, 1.0F)(random_engine);
            velocity.y = std::uniform_real_distribution(-1.0F, 1.0F)(random_engine);
            velocity.z = std::uniform_real_distribution(-1.0F, 1.0F)(random_engine);

            glm::vec4 color = particle.color;
            color.a = 1.0F;

            explosionParticleSystem->emit(particle.position, color, velocity * 5.0F, 1.0F);
        }
    }

    void onSparkleParticleUpdate(Particle& particle, float dt) {
        particle.color.a = std::clamp(particle.lifetime / particle.time, 0.0F, 1.0F);
        particle.velocity.y -= 9.8f * dt;
        particle.velocity.y = std::max(particle.velocity.y, -1.0F);
    }

    void onExplosionParticleUpdate(Particle& particle, float dt) {
        particle.color.a = std::clamp(particle.lifetime / particle.time, 0.0F, 1.0F);
    }

private:
    glm::mat4x4 camera_projection_matrix = {};
    glm::mat4x4 world_to_camera_matrix = {};

    sp<ParticleSystem> rocketParticleSystem = {};
    sp<ParticleSystem> sparkleParticleSystem = {};
    sp<ParticleSystem> explosionParticleSystem = {};
    sp<ParticleEmitter> rocketParticleEmitter = {};

    std::default_random_engine random_engine = {};
};

auto main(int argc, char** argv) -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);

    Game app{};
    app.run();

    return 0;
}