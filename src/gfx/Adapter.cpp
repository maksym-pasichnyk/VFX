#include "Adapter.hpp"
#include "Device.hpp"
#include "Surface.hpp"

gfx::Adapter::Adapter(rc<Instance> instance, vk::PhysicalDevice handle)
    : instance(std::move(instance))
    , handle(handle) {}

auto gfx::Adapter::getSurfaceCapabilities(this Adapter& self, rc<Surface> const& surface) -> vk::SurfaceCapabilitiesKHR {
    return self.handle.getSurfaceCapabilitiesKHR(surface->handle, self.instance->dispatcher);
}

auto gfx::Adapter::createDevice(this Adapter& self, vk::DeviceCreateInfo const& create_info) -> rc<Device> {
    return MakeShared<Device>(self.shared_from_this(), create_info);
}
