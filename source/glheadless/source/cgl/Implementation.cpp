#include "Implementation.h"

#include <vector>
#include <set>
#include <map>

#include <dlfcn.h>

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

#include <glheadless/Context.h>
#include <glheadless/ContextFormat.h>

#include "../InternalException.h"


namespace glheadless {
namespace cgl {


GLHEADLESS_REGISTER_IMPLEMENTATION(CGL, Implementation)


namespace {


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

const auto k_booleanAttributes = std::set<_CGLPixelFormatAttribute> {
        kCGLPFAAllRenderers,
        kCGLPFADoubleBuffer,
        kCGLPFAStereo,
        kCGLPFAMinimumPolicy,
        kCGLPFAMaximumPolicy,
        kCGLPFAOffScreen,
        kCGLPFAFullScreen,
        kCGLPFAAuxDepthStencil,
        kCGLPFAColorFloat,
        kCGLPFAMultisample,
        kCGLPFASupersample,
        kCGLPFASampleAlpha,
        kCGLPFASingleRenderer,
        kCGLPFANoRecovery,
        kCGLPFAAccelerated,
        kCGLPFAClosestPolicy,
        kCGLPFARobust,
        kCGLPFABackingStore,
        kCGLPFAMPSafe,
        kCGLPFAWindow,
        kCGLPFAMultiScreen,
        kCGLPFACompliant,
        kCGLPFAPBuffer,
        kCGLPFARemotePBuffer,
        kCGLPFAAllowOfflineRenderers,
        kCGLPFAAcceleratedCompute
};

#pragma GCC diagnostic pop


std::vector<_CGLPixelFormatAttribute> createPixelFormatAttributeList(const ContextFormat& format) {
    std::map<_CGLPixelFormatAttribute, int> attributes;

    attributes[kCGLPFAAccelerated] = GL_TRUE;
    attributes[kCGLPFAClosestPolicy] = GL_TRUE;

    if (format.versionMajor == 4) {
        attributes[kCGLPFAOpenGLProfile] = kCGLOGLPVersion_GL4_Core;
    } else if (format.versionMajor == 3) {
        attributes[kCGLPFAOpenGLProfile] = kCGLOGLPVersion_GL3_Core;
    } else if (format.versionMajor == 2 || format.versionMajor == 1 || format.versionMajor == 0) {
        attributes[kCGLPFAOpenGLProfile] = kCGLOGLPVersion_Legacy;
    } else {
        throw InternalException(Error::INVALID_CONFIGURATION, "Unsupported OpenGL version: " + std::to_string(format.versionMajor));
    }

    std::vector<_CGLPixelFormatAttribute> list;
    list.reserve(attributes.size() * 2 + 1);
    for (const auto& attribute : attributes) {
        if (k_booleanAttributes.find(static_cast<_CGLPixelFormatAttribute>(attribute.first)) != k_booleanAttributes.end()) {
            if (attribute.second == GL_TRUE) {
                list.push_back(attribute.first);
            }
        } else {
            list.push_back(attribute.first);
            list.push_back(static_cast<_CGLPixelFormatAttribute>(attribute.second));
        }
    }
    list.push_back(static_cast<_CGLPixelFormatAttribute>(0)); // finalize list

    return list;
}


} // unnamed namespace


Implementation::Implementation()
: m_contextHandle(nullptr)
, m_pixelFormatHandle(nullptr)
, m_owning(true) {
}


Implementation::~Implementation() {
}


std::unique_ptr<Context> Implementation::getCurrent() {
    auto context = std::unique_ptr<Context>(new Context(this));
    m_context = context.get();
    m_owning = false;

    m_contextHandle = CGLGetCurrentContext();
    if (m_contextHandle == nullptr) {
        context->setError(Error::INVALID_CONTEXT, "CGLGetCurrentContext returned nullptr");
        return context;
    }

    return context;
}


std::unique_ptr<Context> Implementation::create(const ContextFormat& format) {
    auto context = std::unique_ptr<Context>(new Context(this));
    m_context = context.get();

    try {
        setPixelFormat(format);
        createContext(nullptr);
    } catch (InternalException& e) {
        context->setError(e.code(), e.message());
    }

    return context;
}


std::unique_ptr<Context> Implementation::create(const Context* shared, const ContextFormat& format) {
    auto sharedImplementation = static_cast<const Implementation*>(shared->implementation());
    auto context = std::unique_ptr<Context>(new Context(this));
    m_context = context.get();

    try {
        setPixelFormat(format);
        createContext(sharedImplementation->m_contextHandle);
    } catch (InternalException& e) {
        context->setError(e.code(), e.message());
    }

    return context;
}


bool Implementation::destroy() {
    if (m_owning) {
        if (m_contextHandle != nullptr) {
            CGLReleaseContext(m_contextHandle);
        }
        if (m_pixelFormatHandle != nullptr) {
            CGLReleasePixelFormat(m_pixelFormatHandle);
        }
    }

    m_contextHandle = nullptr;
    m_pixelFormatHandle = nullptr;

    return true;
}


long long Implementation::nativeHandle() {
    return reinterpret_cast<long long>(m_contextHandle);
}


bool Implementation::valid() {
    return m_contextHandle != nullptr;
}


bool Implementation::makeCurrent() {
    if (m_contextHandle == nullptr) {
        return m_context->setError(Error::INVALID_CONTEXT, "Context not set up");
    }

    const auto error = CGLSetCurrentContext(m_contextHandle);
    if (error != kCGLNoError) {
        return m_context->setError(Error::INVALID_CONTEXT, "CGLSetCurrentContext failed");
    }

    return true;
}


bool Implementation::doneCurrent() {
    const auto error = CGLSetCurrentContext(nullptr);
    if (error != kCGLNoError) {
        return m_context->setError(Error::INVALID_CONTEXT, "CGLSetCurrentContext failed");
    }
    return true;
}


void (*Implementation::getProcAddress(const char * name))() {
    static auto framework = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);

    return reinterpret_cast<void(*)()>(framework != nullptr ? dlsym(framework, name) : nullptr);
}


void Implementation::setPixelFormat(const ContextFormat& format) {
    
    // TODO Update the function create pixel format attribute list
    // const auto pixelFormatAttributes = createPixelFormatAttributeList(format);

    std::vector<CGLPixelFormatAttribute> attributes;

    if (format.versionMajor == 4) {
        attributes.push_back(kCGLPFAOpenGLProfile);
        attributes.push_back(static_cast<CGLPixelFormatAttribute>(kCGLOGLPVersion_GL4_Core));
    } else if (format.versionMajor == 3) {
        attributes.push_back(kCGLPFAOpenGLProfile);
        attributes.push_back(static_cast<CGLPixelFormatAttribute>(kCGLOGLPVersion_GL3_Core));
    } else if (format.versionMajor == 2 || format.versionMajor == 1 || format.versionMajor == 0) {
        attributes.push_back(kCGLPFAOpenGLProfile);
        attributes.push_back(static_cast<CGLPixelFormatAttribute>(kCGLOGLPVersion_Legacy));
    } else {
        throw InternalException(Error::INVALID_CONFIGURATION, "Unsupported OpenGL version: " + std::to_string(format.versionMajor));
    }

    attributes.push_back(kCGLPFAAccelerated);
    attributes.push_back(static_cast<CGLPixelFormatAttribute>(GL_TRUE));
    attributes.push_back(kCGLPFAClosestPolicy);
    attributes.push_back(static_cast<CGLPixelFormatAttribute>(GL_TRUE));
    attributes.push_back(kCGLPFAAllowOfflineRenderers);
    attributes.push_back(static_cast<CGLPixelFormatAttribute>(0));

    GLint numVirtualScreens;
    const auto error = CGLChoosePixelFormat(attributes.data(), &m_pixelFormatHandle, &numVirtualScreens);
    if (error != kCGLNoError) {
        throw InternalException(Error::INVALID_CONFIGURATION, "CGLChoosePixelFormat failed");
    }
}


void Implementation::createContext(CGLContextObj shared) {
    const auto error = CGLCreateContext(m_pixelFormatHandle, shared, &m_contextHandle);
    if (error != kCGLNoError) {
        throw InternalException(Error::INVALID_CONFIGURATION, "CGLCreateContext failed");
    }
}


}  // namespace cgl
}  // namespace glheadless
