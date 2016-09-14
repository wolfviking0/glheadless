#include <glheadless/Context.h>

#include <cassert>

#ifdef _WIN32
#include "wgl/Implementation.h"
#elif defined(__APPLE__)
#include "cgl/Implementation.h"
#elif defined(__linux__)
#if defined(GLHEADLESS_LINUX_USE_EGL)
#include "egl/Implementation.h"
#else
#include "glx/Implementation.h"
#endif // defined(GLHEADLESS_LINUX_USE_EGL)
#endif


namespace glheadless {


Context Context::currentContext() {
    return Implementation::currentContext();
}


Context::Context()
: m_version(0, 0)
, m_profile(ContextProfile::NONE)
, m_debugContext(false)
, m_exceptionTriggers(ExceptionTrigger::NONE)
, m_implementation(std::make_unique<Implementation>(this)) {
}


Context::Context(Context&& other) {
    *this = std::move(other);
}


Context::~Context() {
}


ContextProfile Context::profile() const {
    return m_profile;
}


void Context::setProfile(const ContextProfile profile) {
    m_profile = profile;
}


const Context::Version& Context::version() const {
    return m_version;
}


void Context::setVersion(int major, int minor) {
    m_version = { major, minor };
}


bool Context::debugContext() const {
    return m_debugContext;
}


void Context::setDebugContext(const bool debugContext) {
    m_debugContext = debugContext;
}


const std::map<int, int>& Context::attributes() const {
    return m_attributes;
}


void Context::setAttribute(int name, int value) {
    m_attributes[name] = value;
}


bool Context::create() {
    return m_implementation->create();
}


bool Context::create(const Context& shared) {
    return m_implementation->create(&shared);
}


bool Context::destroy() {
    return m_implementation->destroy();
}


bool Context::makeCurrent() {
    return m_implementation->makeCurrent();
}


bool Context::doneCurrent() {
    return m_implementation->doneCurrent();
}


Implementation* Context::implementation() {
    return m_implementation.get();
}


const Implementation* Context::implementation() const {
    return m_implementation.get();
}


Context& Context::operator=(Context&& other) {
    m_version = other.m_version;
    m_profile = other.m_profile;
    m_debugContext = other.m_debugContext;
    m_attributes = std::move(other.m_attributes);

    m_implementation = std::move(other.m_implementation);

    return *this;
}


bool Context::valid() const {
    return m_implementation->valid();
}


const std::error_code& Context::lastErrorCode() const {
    return m_implementation->lastErrorCode();
}


const std::string& Context::lastErrorMessage() const {
    return m_implementation->lastErrorMessage();
}


void Context::setExceptionTriggers(ExceptionTrigger exceptions) {
    m_exceptionTriggers = exceptions;
}

ExceptionTrigger Context::exceptionTriggers() const {
    return m_exceptionTriggers;
}


}  // namespace glheadless
