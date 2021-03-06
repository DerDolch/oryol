//------------------------------------------------------------------------------
//  resourceMgr.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Render/base/meshLoaderBase.h"
#include "resourceMgr.h"

namespace Oryol {
namespace Render {

using namespace Core;
using namespace Resource;
    
//------------------------------------------------------------------------------
resourceMgr::resourceMgr() :
isValid(false),
stateWrapper(nullptr) {
    // empty
}

//------------------------------------------------------------------------------
resourceMgr::~resourceMgr() {
    o_assert(!this->isValid);
}

//------------------------------------------------------------------------------
void
resourceMgr::AttachLoader(const Core::Ptr<meshLoaderBase>& loader) {
    this->meshFactory.AttachLoader(loader);
}
    
//------------------------------------------------------------------------------
void
resourceMgr::Setup(const RenderSetup& setup, class stateWrapper* stWrapper) {
    o_assert(!this->isValid);
    o_assert(nullptr != stWrapper);
    
    this->isValid = true;
    this->stateWrapper = stWrapper;

    this->meshFactory.Setup(this->stateWrapper);
    this->meshPool.Setup(&this->meshFactory, setup.GetPoolSize(ResourceType::Mesh), setup.GetThrottling(ResourceType::Mesh), 'MESH');
    this->shaderFactory.Setup();
    this->shaderPool.Setup(&this->shaderFactory, setup.GetPoolSize(ResourceType::Shader), 0, 'SHDR');
    this->programBundleFactory.Setup(this->stateWrapper, &this->shaderPool);
    this->programBundlePool.Setup(&this->programBundleFactory, setup.GetPoolSize(ResourceType::ProgramBundle), 0, 'PRGB');
    
    this->resourceRegistry.Setup(setup.GetResourceRegistryCapacity());
}

//------------------------------------------------------------------------------
void
resourceMgr::Discard() {
    o_assert(this->isValid);
    this->isValid = false;
    this->resourceRegistry.Discard();
    this->programBundlePool.Discard();
    this->programBundleFactory.Discard();
    this->shaderPool.Discard();
    this->shaderFactory.Discard();
    this->meshPool.Discard();
    this->meshFactory.Discard();
    this->stateWrapper = nullptr;
}

//------------------------------------------------------------------------------
bool
resourceMgr::IsValid() const {
    return this->isValid;
}

//------------------------------------------------------------------------------
void
resourceMgr::Update() {
    o_assert(this->isValid);
    
    // only call Update on pools which support asynchronous resource loading
    this->meshPool.Update();
}

//------------------------------------------------------------------------------
template<> Id
resourceMgr::CreateResource(const MeshSetup& setup) {
    o_assert(this->isValid);
    const Locator& loc = setup.GetLocator();
    Id resId = this->resourceRegistry.LookupResource(loc);
    if (resId.IsValid()) {
        o_assert(resId.Type() == ResourceType::Mesh);
        return resId;
    }
    else {
        resId = this->meshPool.AllocId();
        this->resourceRegistry.AddResource(loc, resId);
        this->meshPool.Assign(resId, setup);
        return resId;
    }
}

//------------------------------------------------------------------------------
template<> Id
resourceMgr::CreateResource(const MeshSetup& setup, const Ptr<IO::Stream>& data) {
    o_assert(this->isValid);
    const Locator& loc = setup.GetLocator();
    Id resId = this->resourceRegistry.LookupResource(setup.GetLocator());
    if (!resId.IsValid()) {
        resId = this->meshPool.AllocId();
        this->resourceRegistry.AddResource(loc, resId);
        this->meshPool.Assign(resId, setup, data);
    }
    return resId;
}

//------------------------------------------------------------------------------
template<> Id
resourceMgr::CreateResource(const ShaderSetup& setup) {
    o_assert(this->isValid);
    const Locator& loc = setup.GetLocator();
    Id resId = this->resourceRegistry.LookupResource(loc);
    if (resId.IsValid()) {
        o_assert(resId.Type() == ResourceType::Shader);
        return resId;
    }
    else {
        resId = this->shaderPool.AllocId();
        this->resourceRegistry.AddResource(loc, resId);
        this->shaderPool.Assign(resId, setup);
        return resId;
    }
}

//------------------------------------------------------------------------------
template<> Id
resourceMgr::CreateResource(const ProgramBundleSetup& setup) {
    o_assert(this->isValid);
    const Locator& loc = setup.GetLocator();
    Id resId = this->resourceRegistry.LookupResource(loc);
    if (resId.IsValid()) {
        o_assert(resId.Type() == ResourceType::ProgramBundle);
        return resId;
    }
    else {
        resId = this->programBundlePool.AllocId();
        // add vertex/fragment shaders as dependencies
        Array<Id> deps;
        const int32 numProgs = setup.GetNumPrograms();
        deps.Reserve(numProgs * 2);
        for (int32 i = 0; i < setup.GetNumPrograms(); i++) {
            deps.AddBack(setup.GetVertexShader(i));
            deps.AddBack(setup.GetFragmentShader(i));
        }
        this->resourceRegistry.AddResource(loc, resId, deps);
        this->programBundlePool.Assign(resId, setup);
        return resId;
    }
}
    
//------------------------------------------------------------------------------
Id
resourceMgr::LookupResource(const Locator& loc) {
    o_assert(this->isValid);
    return this->resourceRegistry.LookupResource(loc);
}

//------------------------------------------------------------------------------
void
resourceMgr::DiscardResource(const Id& resId) {
    o_assert(this->isValid);
    if (this->resourceRegistry.ReleaseResource(resId, this->removedIds) > 0) {
        // removedIds now has the resources which need to be destroyed
        for (const Id& removeId : this->removedIds) {
            switch (removeId.Type()) {
                case ResourceType::Texture:
                    o_assert2(false, "FIXME!!!\n");
                    break;
                case ResourceType::Mesh:
                    this->meshPool.Unassign(removeId);
                    break;
                case ResourceType::Shader:
                    this->shaderPool.Unassign(removeId);
                    break;
                case ResourceType::ProgramBundle:
                    this->programBundlePool.Unassign(removeId);
                    break;
                case ResourceType::StateBlock:
                    o_assert2(false, "FIXME!!!\n");
                    break;
                case ResourceType::ConstantBlock:
                    o_assert2(false, "FIXME!!!\n");
                    break;
                default:
                    o_assert(false);
                    break;
            }
        }
    }
}

//------------------------------------------------------------------------------
State::Code
resourceMgr::QueryResourceState(const Id& resId) {
    o_assert(this->isValid);
    switch (resId.Type()) {
        case ResourceType::Texture:
            o_assert2(false, "FIXME!!!\n");
            break;
        case ResourceType::Mesh:
            return this->meshPool.QueryState(resId);
            break;
        case ResourceType::Shader:
            return this->shaderPool.QueryState(resId);
        case ResourceType::ProgramBundle:
            return this->programBundlePool.QueryState(resId);
        case ResourceType::StateBlock:
            o_assert2(false, "FIXME!!!\n");
            break;
        case ResourceType::ConstantBlock:
            o_assert2(false, "FIXME!!!\n");
            break;
        default:
            o_assert(false);
            break;
    }
}

} // namespace Render
} // namespace Oryol