//------------------------------------------------------------------------------
//  glfwDisplayMgr.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Render/gl/gl_impl.h"
#include "glfwDisplayMgr.h"
#include "Core/Log.h"
#if ORYOL_OSX
#define GLFW_INCLUDE_GLCOREARB
#endif
#include "GLFW/glfw3.h"

namespace Oryol {
namespace Render {
    
//------------------------------------------------------------------------------
void
glfwDisplayMgr::glfwErrorCallback(int error, const char* desc) {
    Core::Log::Error("GLFW error: '%d', '%s'\n", error, desc);
}

//------------------------------------------------------------------------------
glfwDisplayMgr::glfwDisplayMgr() :
glfwWindow(nullptr) {
    // empty
}

//------------------------------------------------------------------------------
glfwDisplayMgr::~glfwDisplayMgr() {
    if (this->IsDisplayValid()) {
        this->DiscardDisplay();
    }
}

//------------------------------------------------------------------------------
void
glfwDisplayMgr::SetupDisplay(const RenderSetup& setup) {
    o_assert(!this->IsDisplayValid());
    o_assert(nullptr == this->glfwWindow);
    
    displayMgrBase::SetupDisplay(setup);
    
    // setup GLFW
    if (!glfwInit()) {
        o_error("glfwDisplayMgr: failed to setup GLFW!\n");
    }
    glfwSetErrorCallback(glfwErrorCallback);
    
    // setup the GLFW window
    int redBits=0, greenBits=0, blueBits=0, alphaBits=0;
    switch (setup.GetColorPixelFormat()) {
        case PixelFormat::R8G8B8:
            redBits=greenBits=blueBits=8; alphaBits=0;
            break;
        case PixelFormat::R5G6B5:
            redBits=5; greenBits=6; blueBits=5; alphaBits=0;
            break;
        case PixelFormat::R5G5B5A1:
            redBits=greenBits=blueBits=5; alphaBits=1;
            break;
        case PixelFormat::R4G4B4A4:
            redBits=greenBits=blueBits=alphaBits=4;
            break;
        case PixelFormat::R8G8B8A8:
        default:
            redBits=greenBits=blueBits=alphaBits=8;
            break;
    }
    int depthBits=0, stencilBits=0;
    switch (setup.GetDepthPixelFormat()) {
        case PixelFormat::D16:
            depthBits=16; stencilBits=0;
            break;
        case PixelFormat::D32:
            depthBits=32; stencilBits=0;
            break;
        case PixelFormat::D24S8:
            depthBits=24; stencilBits=8;
            break;
        default:
            depthBits=0; stencilBits=0;
            break;
    }
    glfwWindowHint(GLFW_RED_BITS, redBits);
    glfwWindowHint(GLFW_GREEN_BITS, greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, blueBits);
    glfwWindowHint(GLFW_ALPHA_BITS, alphaBits);
    glfwWindowHint(GLFW_DEPTH_BITS, depthBits);
    glfwWindowHint(GLFW_STENCIL_BITS, stencilBits);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // windowed or fullscreen mode?
    GLFWmonitor* glfwMonitor = nullptr;
    if (setup.IsFullscreen()) {
        glfwMonitor = glfwGetPrimaryMonitor();
    }
    
    // now actually create the window
    this->glfwWindow = glfwCreateWindow(setup.GetWindowWidth(),
                                        setup.GetWindowHeight(),
                                        setup.GetWindowTitle().AsCStr(),
                                        glfwMonitor,
                                        0);
    o_assert(nullptr != this->glfwWindow);
    
    // and make the window's GL context current
    glfwMakeContextCurrent(this->glfwWindow);
    glfwSwapInterval(setup.GetSwapInterval());
    
    // initialize GLEW
    #if !ORYOL_OSX
    glewInit();
    o_assert(GLEW_VERSION_3_2);
    #endif
    
    // now set the actual display attributes
    int fbWidth = 0, fbHeight = 0;
    int posX = 0, posY = 0;
    int width = 0, height = 0;
    glfwGetFramebufferSize(this->glfwWindow, &fbWidth, &fbHeight);
    glfwGetWindowPos(this->glfwWindow, &posX, &posY);
    glfwGetWindowSize(this->glfwWindow, &width, &height);
    this->displayAttrs.SetFramebufferWidth(fbWidth);
    this->displayAttrs.SetFramebufferHeight(fbHeight);
    this->displayAttrs.SetWindowPosX(posX);
    this->displayAttrs.SetWindowPosY(posY);
    this->displayAttrs.SetWindowWidth(width);
    this->displayAttrs.SetWindowHeight(height);
}

//------------------------------------------------------------------------------
void
glfwDisplayMgr::DiscardDisplay() {
    o_assert(this->IsDisplayValid());
    o_assert(nullptr != this->glfwWindow);
    
    glfwDestroyWindow(this->glfwWindow);
    this->glfwWindow = nullptr;
    glfwTerminate();
    
    displayMgrBase::DiscardDisplay();
}

//------------------------------------------------------------------------------
bool
glfwDisplayMgr::QuitRequested() const {
    o_assert(nullptr != this->glfwWindow);
    return glfwWindowShouldClose(this->glfwWindow);
}

//------------------------------------------------------------------------------
void
glfwDisplayMgr::ProcessSystemEvents() {
    o_assert(nullptr != this->glfwWindow);
    glfwPollEvents();
    displayMgrBase::ProcessSystemEvents();
}


//------------------------------------------------------------------------------
void
glfwDisplayMgr::Present() {
    o_assert(nullptr != this->glfwWindow);
    glfwSwapBuffers(this->glfwWindow);
    displayMgrBase::Present();
}

} // namespace Render
} // namespace Oryol