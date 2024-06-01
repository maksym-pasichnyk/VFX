#pragma once

#include "Device.hpp"
#include "Function.hpp"

#include <map>
#include <optional>

namespace gfx {
    struct CommandBuffer;
    struct RenderCommandEncoder;

    struct RenderPipelineColorBlendAttachmentStateArray {
        std::vector<vk::PipelineColorBlendAttachmentState> elements = {};

        auto operator[](size_t i) -> vk::PipelineColorBlendAttachmentState&;
    };

    struct RenderPipelineColorAttachmentFormatArray {
        std::vector<vk::Format> elements = {};

        auto operator[](size_t i) -> vk::Format&;
    };

    struct TessellationState : public ManagedObject {
        uint32_t patch_control_points = 3;
    };

    struct MultisampleState : public ManagedObject {
        bool                            sample_shading_enable       = false;
        float                           min_sample_shading          = 1.0F;
        vk::Optional<vk::SampleMask>    sample_mask                 = nullptr;
    };

    struct DepthStencilStateDescription {
        bool                    isDepthTestEnabled          = false;
        bool                    isDepthWriteEnabled         = false;
        vk::CompareOp           depthCompareFunction        = vk::CompareOp::eAlways;
        bool                    depth_bounds_test_enable    = false;
        bool                    stencil_test_enable         = false;
        vk::StencilOpState      frontFaceStencil            = {};
        vk::StencilOpState      backFaceStencil             = {};
        float                   min_depth_bounds            = 0.0F;
        float                   max_depth_bounds            = 1.0F;
    };

    struct VertexInputState final : ManagedObject {
        std::vector<vk::VertexInputBindingDescription>      bindings    = {};
        std::vector<vk::VertexInputAttributeDescription>    attributes  = {};
    };

    class RenderPipelineStateDescription : public ManagedObject {
    private:
        using Self = RenderPipelineStateDescription;

        rc<Function>                                    _vertexFunction;
        rc<Function>                                    _fragmentFunction;
        rc<TessellationState>                           _tessellationState;
        rc<MultisampleState>                            _multisampleState;
        rc<VertexInputState>                            _vertexInputState;
        uint32_t                                        _viewMask;
        vk::Format                                      _depthAttachmentFormat;
        vk::Format                                      _stencilAttachmentFormat;
        RenderPipelineColorAttachmentFormatArray        _colorAttachmentFormats;
        RenderPipelineColorBlendAttachmentStateArray    _colorBlendAttachments;
        vk::PrimitiveTopology                           _inputPrimitiveTopology;
        bool                                            _primitiveRestartEnable;
        uint32_t                                        _rasterSampleCount;
        bool                                            _isAlphaToCoverageEnabled;
        bool                                            _isAlphaToOneEnabled;

    private:
        RenderPipelineStateDescription() {
            this->_vertexFunction              = {};
            this->_fragmentFunction            = {};
            this->_tessellationState           = {};
            this->_multisampleState            = {};
            this->_vertexInputState            = {};
            this->_viewMask                    = {};
            this->_depthAttachmentFormat       = vk::Format::eUndefined;
            this->_stencilAttachmentFormat     = vk::Format::eUndefined;
            this->_colorAttachmentFormats      = {};
            this->_colorBlendAttachments       = {};
            this->_inputPrimitiveTopology      = vk::PrimitiveTopology::eTriangleList;
            this->_primitiveRestartEnable      = {};
            this->_rasterSampleCount           = 1;
            this->_isAlphaToCoverageEnabled    = {};
            this->_isAlphaToOneEnabled         = {};
        }

    public:
        static auto init() -> rc<RenderPipelineStateDescription> {
            return rc<RenderPipelineStateDescription>(new RenderPipelineStateDescription());
        }

        auto getVertexFunction(this Self& self) -> rc<Function> {
            return self._vertexFunction;
        }
        void setVertexFunction(this Self& self, rc<Function> vertexFunction) {
            self._vertexFunction = vertexFunction;
        }

        auto getFragmentFunction(this Self& self) -> rc<Function> {
            return self._fragmentFunction;
        }
        void setFragmentFunction(this Self& self, rc<Function> fragmentFunction) {
            self._fragmentFunction = fragmentFunction;
        }

        auto getTessellationState(this Self& self) -> rc<TessellationState> {
            return self._tessellationState;
        }
        void setTessellationState(this Self& self, rc<TessellationState> tessellationState) {
            self._tessellationState = tessellationState;
        }

        auto getMultisampleState(this Self& self) -> rc<MultisampleState> {
            return self._multisampleState;
        }
        void setMultisampleState(this Self& self, rc<MultisampleState> multisampleState) {
            self._multisampleState = multisampleState;
        }

        auto getVertexInputState(this Self& self) -> rc<VertexInputState> {
            return self._vertexInputState;
        }
        void setVertexInputState(this Self& self, rc<VertexInputState> vertexInputState) {
            self._vertexInputState = vertexInputState;
        }

        auto getViewMask(this Self& self) -> uint32_t {
            return self._viewMask;
        }
        void setViewMask(this Self& self, uint32_t viewMask) {
            self._viewMask = viewMask;
        }

        auto getDepthAttachmentFormat(this Self& self) -> vk::Format {
            return self._depthAttachmentFormat;
        }
        void setDepthAttachmentFormat(this Self& self, vk::Format depthAttachmentFormat) {
            self._depthAttachmentFormat = depthAttachmentFormat;
        }

        auto getStencilAttachmentFormat(this Self& self) -> vk::Format {
            return self._stencilAttachmentFormat;
        }
        void setStencilAttachmentFormat(this Self& self, vk::Format stencilAttachmentFormat) {
            self._stencilAttachmentFormat = stencilAttachmentFormat;
        }

        auto colorAttachmentFormats(this Self& self) -> RenderPipelineColorAttachmentFormatArray& {
            return self._colorAttachmentFormats;
        }
        auto colorBlendAttachments(this Self& self) -> RenderPipelineColorBlendAttachmentStateArray& {
            return self._colorBlendAttachments;
        }

        auto getInputPrimitiveTopology(this Self& self) -> vk::PrimitiveTopology {
            return self._inputPrimitiveTopology;
        }
        void setInputPrimitiveTopology(this Self& self, vk::PrimitiveTopology inputPrimitiveTopology) {
            self._inputPrimitiveTopology = inputPrimitiveTopology;
        }

        auto getPrimitiveRestartEnable(this Self& self) -> bool {
            return self._primitiveRestartEnable;
        }
        void setPrimitiveRestartEnable(this Self& self, bool primitiveRestartEnable) {
            self._primitiveRestartEnable = primitiveRestartEnable;
        }

        auto getRasterSampleCount(this Self& self) -> uint32_t {
            return self._rasterSampleCount;
        }
        void setRasterSampleCount(this Self& self, uint32_t rasterSampleCount) {
            self._rasterSampleCount = rasterSampleCount;
        }

        auto getIsAlphaToCoverageEnabled(this Self& self) -> bool {
            return self._isAlphaToCoverageEnabled;
        }
        void setIsAlphaToCoverageEnabled(this Self& self, bool isAlphaToCoverageEnabled) {
            self._isAlphaToCoverageEnabled = isAlphaToCoverageEnabled;
        }

        auto getIsAlphaToOneEnabled(this Self& self) -> bool {
            return self._isAlphaToOneEnabled;
        }
        void setIsAlphaToOneEnabled(this Self& self, bool isAlphaToOneEnabled) {
            self._isAlphaToOneEnabled = isAlphaToOneEnabled;
        }
    };

    class DepthStencilState : public ManagedObject {
        friend Device;
        friend RenderCommandEncoder;

    private:
        bool                isDepthTestEnabled      = {};
        bool                isDepthWriteEnabled     = {};
        vk::CompareOp       depthCompareFunction    = {};
        bool                isDepthBoundsTestEnabled   = {};
        bool                isStencilTestEnabled       = {};
        vk::StencilOpState  frontFaceStencil        = {};
        vk::StencilOpState  backFaceStencil         = {};
        float               minDepthBounds          = {};
        float               maxDepthBounds          = {};
    };

    class RenderPipelineState : public ManagedObject {
        friend Device;
        friend CommandBuffer;
        friend RenderCommandEncoder;

    private:
        rc<Device>                           device                 = {};
        rc<RenderPipelineStateDescription>   description            = {};
        vk::PipelineCache                    pipelineCache          = {};
        vk::PipelineLayout                   pipelineLayout         = {};
        std::map<std::size_t, vk::Pipeline>  pipelines              = {};
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts   = {};

    public:
        explicit RenderPipelineState(rc<Device> device, rc<RenderPipelineStateDescription> description);
        ~RenderPipelineState() override;
    };
}