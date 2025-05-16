#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <common/shader.hpp>
#include "ImGuiConsole.hpp"
#include "SceneObject.hpp"
#include "RenderSystem.hpp"

#include <exception>
#include <unistd.h>
#include <chrono>  // Add at the top with other includes

class SceneRenderer {
private:
    GLuint programID;
    GLuint VertexArrayID;
    GLuint vertexbuffer;
    GLuint elementbuffer;
    GLuint LightID;

    unsigned int framebuffer = 0;
    unsigned int textureColorbuffer = 0;
    unsigned int rbo = 0;
    int framebufferWidth = 1024;
    int framebufferHeight = 768;

    bool initialized = false;
    bool pauseAnimations = false;

    SceneRenderer();

public:
    static SceneRenderer& getInstance();

    bool isInitialized();
    int getframebufferWidth();
    int getframebufferHeight();
    unsigned int getTextureColorbuffer();

    void setPauseAnimations(bool pause);
    bool getPauseAnimations();

    void resizeFramebuffer(int width, int height, float scale);
    bool setupFramebuffer(float width, float height, float scale);
    bool render(float deltaTime, bool paused, RenderSystem & renderSystem, Registry & registry);
    void cleanup();

};

#endif
