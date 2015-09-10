#pragma once

#include <glheadless/glheadless_api.h>

#include <memory>

#include <glheadless/PixelFormat.h>


namespace glheadless {


class Implementation;


enum class ContextProfile : unsigned int {
    CORE,
    COMPATIBILITY
};


class GLHEADLESS_API Context {
public:
    using Version = std::pair<int, int>;


public:
    Context();
    Context(const Context&) = delete;
    Context(Context&& other);
    ~Context();

    const PixelFormat& pixelFormat() const;
    void setPixelFormat(const PixelFormat& pixelFormat);

    const Version& version() const;
    void setVersion(int major, int minor);

    ContextProfile profile() const;
    void setProfile(const ContextProfile profile);

    bool debugContext() const;
    void setDebugContext(const bool debugContext);

    const std::map<int, int>& attributes() const;
    void setAttribute(int name, int value);

    void create();

    void makeCurrent() noexcept;
    void doneCurrent() noexcept;

    Context& operator=(const Context&) = delete;
    Context& operator=(Context&& other);


private:
    PixelFormat m_pixelFormat;
    Version m_version;
    ContextProfile m_profile;
    bool m_debugContext;
    std::map<int, int> m_attributes;

    std::unique_ptr<Implementation> m_implementation;
};

    
}  // namespace glheadless
