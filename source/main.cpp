#define NOMINMAX

#include "graphics/geometry.hpp"
#include "graphics/graphicsdevice.hpp"
#include "graphics/material.hpp"
#include "graphics/program.hpp"
#include "graphics/texturecube.hpp"
#include "graphics/textures.hpp"
#include "graphics/light.hpp"
#include "graphics/framebuffer.hpp"
#include "graphics/meshdata.hpp"
#include "io/filewatcher.hpp"
#include "math/frustum.hpp"
#include "physics.hpp"
#include "scenes/scenenode.hpp"
#include "scenes/spacescene.hpp"
#include "types.hpp"
#include "camera.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>


static f64 g_MousePosXOld = {};
static f64 g_MousePosYOld = {};

GLFWwindow* g_Window{ nullptr };

PhysicsScene* g_PhysicsScene{ nullptr };
glm::mat4 g_Camera_View{ glm::mat4(1.0f) };
Light* g_Camera_Light{ nullptr };

Program* g_FinalProgram{ nullptr };
Program* g_GeometryProgram{ nullptr };
Program* g_MotionBlurProgram{ nullptr };
Program* g_LightProgram{ nullptr };
Program* g_QuadProgram{ nullptr };
Program* g_EmissionProgram{ nullptr };

Geometry* g_EmptyGeometry{ nullptr };
Geometry* g_CubeGeometry{ nullptr };
Geometry* g_PlaneGeometry{ nullptr };
Geometry* g_ShipGeometry{ nullptr };
Geometry* g_PointLightGeometry{ nullptr };

Texture* g_gBufferFinalTexture{ nullptr };
Texture* g_gBufferPositionTexture{ nullptr };
Texture* g_gBufferNormalTexture{ nullptr };
Texture* g_gBufferAlbedoTexture{ nullptr };
Texture* g_gBufferDepthTexture{ nullptr };
Texture* g_gBufferVelocityTexture{ nullptr };
Texture* g_LightBufferTexture{ nullptr };
Texture* g_MotionBlurTexture{ nullptr };
Texture* g_TransitionTexture{ nullptr };
Texture* g_EmissionTexture{ nullptr };

Framebuffer* g_GeometryFramebuffer{ };
Framebuffer* g_EmissionFramebuffer{ };
Framebuffer* g_FinalFramebuffer{ };
Framebuffer* g_MotionBlurFramebuffer{ };
Framebuffer* g_LightsFramebuffer{ };
Framebuffer* g_TransitionFramebuffer{ };

TextureCube* g_SkyboxTextureCube{ };

std::vector<Material*> g_Materials;
std::vector<Scene*> g_Scenes;
Scene* g_Scene_Current{ nullptr };

Frustum g_Frustum;

bool g_IsMotionBlurEnabled{ true };
bool g_IsVsyncEnabled{ true };

bool g_IsTransitionEffectEnabled{ false };
glm::vec4 g_Transition_Factor{ 0.0f, 0.0f, 0.0f, 0.0f };

inline float Lerp(const f32 a, const f32 b, const f32 f)
{
    return a + f * (b - a);
}

void Cleanup()
{
    delete g_GeometryProgram;
    delete g_FinalProgram;
    delete g_MotionBlurProgram;
    delete g_LightProgram;
    delete g_QuadProgram;
    delete g_EmissionProgram;

    delete g_CubeGeometry;
    delete g_PlaneGeometry;
    delete g_EmptyGeometry;
    delete g_ShipGeometry;
    delete g_PointLightGeometry;

    delete g_GeometryFramebuffer;
    delete g_FinalFramebuffer;
    delete g_MotionBlurFramebuffer;
    delete g_LightsFramebuffer;
    delete g_TransitionFramebuffer;
    delete g_EmissionFramebuffer;

    delete g_gBufferFinalTexture;
    delete g_gBufferPositionTexture;
    delete g_gBufferNormalTexture;
    delete g_gBufferAlbedoTexture;
    delete g_gBufferDepthTexture;
    delete g_gBufferVelocityTexture;
    delete g_LightBufferTexture;
    delete g_MotionBlurTexture;
    delete g_TransitionTexture;
    delete g_EmissionTexture;

    delete g_SkyboxTextureCube;

    for (auto material : g_Materials)
    {
        delete material;
    }
    for (auto scene : g_Scenes)
    {
        delete scene;
    }

    delete g_PhysicsScene;
}

void HandleInput(const f32 deltaTime)
{
    if (glfwGetKey(g_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(g_Window, true);
    }

    f32 acceleration = 0.05f;
    if (glfwGetKey(g_Window, GLFW_KEY_LEFT_SHIFT))
    {
        acceleration *= 10;
    }

    if (glfwGetKey(g_Window, GLFW_KEY_W) == GLFW_PRESS)
    {
        g_PhysicsScene->Boost(Direction::Forward, acceleration);
    }

    if (glfwGetKey(g_Window, GLFW_KEY_S) == GLFW_PRESS)
    {
        g_PhysicsScene->Boost(Direction::Backward, acceleration);
    }

    if (glfwGetKey(g_Window, GLFW_KEY_A) == GLFW_PRESS)
    {
        g_PhysicsScene->Boost(Direction::Left, acceleration);
    }

    if (glfwGetKey(g_Window, GLFW_KEY_D) == GLFW_PRESS)
    {
        g_PhysicsScene->Boost(Direction::Right, acceleration);
    }

    if (glfwGetKey(g_Window, GLFW_KEY_E) == GLFW_PRESS)
    {
        g_PhysicsScene->Boost(Direction::RollCW, 0.05f);
    }

    if (glfwGetKey(g_Window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        g_PhysicsScene->Boost(Direction::RollCCW, 0.05f);
    }

    if (glfwGetKey(g_Window, GLFW_KEY_R) == GLFW_PRESS)
    {
        g_PhysicsScene->Boost(Direction::Stop, 0.0f);
    }

    if (glfwGetKey(g_Window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        g_PhysicsScene->Boost(Direction::Up, acceleration);
    }

    if (glfwGetKey(g_Window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        g_PhysicsScene->Boost(Direction::Down, acceleration);
    }

    if (glfwGetKey(g_Window, GLFW_KEY_F1) == GLFW_PRESS)
    {
        g_IsTransitionEffectEnabled = true;
    }
}

void Update(const float deltaTime)
{
    glfwPollEvents();
    HandleInput(deltaTime);

    g_PhysicsScene->Step(deltaTime);
    
    const auto physicsCamera = g_PhysicsScene->Camera;
    const auto transform = physicsCamera->getGlobalPose();

    const auto pos = glm::vec3(transform.p.x, transform.p.y, transform.p.z);
    const auto quat = glm::quat(transform.q.w, transform.q.x, transform.q.y, transform.q.z);
    
    auto euler = glm::eulerAngles(quat);

    auto viewMatrix = glm::translate(glm::mat4(1.0f), pos) * glm::toMat4(quat);

    viewMatrix = glm::inverse(viewMatrix);

    if (g_IsTransitionEffectEnabled)
    {
        g_Transition_Factor.w += 0.01f;
        if (g_Transition_Factor.w >= 1.0f)
        {
            g_IsTransitionEffectEnabled = false;
            g_Transition_Factor.w = 0.0f;
        }
    }

    g_Camera_View = viewMatrix;
}

void WindowOnFramebufferResized(GLFWwindow* window, const int width, const int height)
{
    //g_Window_Width = width;
    //g_Window_Height = height;
    glViewport(0, 0, width, height);
}

void WindowOnMouseMove(GLFWwindow* window, const double xPos, const double yPos)
{
    if (g_PhysicsScene != nullptr)
    {
        g_PhysicsScene->Tumble(static_cast<float>(xPos - g_MousePosXOld),
                               static_cast<float>(yPos - g_MousePosYOld));
        g_MousePosXOld = xPos;
        g_MousePosYOld = yPos;
    }
}

void GetWorkingArea(s32* screenWidth, s32* screenHeight)
{
    const auto primaryMonitor = glfwGetPrimaryMonitor();

    glfwGetMonitorWorkarea(primaryMonitor, nullptr, nullptr, screenWidth, screenHeight);
}

GLFWwindow* CreateMainWindow(
    const s32 screenWidth,
    const s32 screenHeight,
    const s32 windowWidth,
    const s32 windowHeight,
    const std::string_view title)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#if _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    const auto window = glfwCreateWindow(windowWidth, windowHeight, title.data(), nullptr, nullptr);
    if (window == nullptr)
    {
        glfwTerminate();
        return nullptr;
    }

    glfwSetFramebufferSizeCallback(window, WindowOnFramebufferResized);
    glfwSetCursorPosCallback(window, WindowOnMouseMove);

    glfwSetWindowPos(window, screenWidth / 2 - windowWidth / 2, screenHeight / 2 - windowHeight / 2);
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    return window;
}

void InitializeOpenGL(GLFWwindow* window)
{
    glfwMakeContextCurrent(window);
    if (!gladLoadGL())
    {
        std::cerr << "GLAD: Unable to initialize.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

void InitializePhysics()
{
    g_PhysicsScene = new PhysicsScene();
}

void RenderGBuffer(
    const s32 frameWidth,
    const s32 frameHeight,
    const glm::mat4& cameraProjection,
    const glm::mat4& cameraView)
{
    constexpr std::string_view renderGBufferDebugGroup = "Render GBuffer";
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, static_cast<GLsizei>(renderGBufferDebugGroup.length()), renderGBufferDebugGroup.data());
    auto constexpr depthClearValue = 1.0f;
    g_GeometryFramebuffer->Clear(0, glm::value_ptr(glm::vec3(0.0f)));
    g_GeometryFramebuffer->Clear(1, glm::value_ptr(glm::vec3(0.0f)));
    g_GeometryFramebuffer->Clear(2, glm::value_ptr(glm::vec4(0.0f)));
    g_GeometryFramebuffer->Clear(3, glm::value_ptr(glm::vec2(0.0f)));
    g_GeometryFramebuffer->Clear(4, glm::value_ptr(glm::vec4(0.0f)));
    g_GeometryFramebuffer->ClearDepth(depthClearValue);
    g_GeometryFramebuffer->Bind();
    glViewport(0, 0, frameWidth, frameHeight);

    g_GeometryProgram->Bind();
    //std::sort(g_Scene_Current->Objects().begin(), g_Scene_Current->Objects().end());
        
    ///////////////////////// SCENE RENDER BEGIN /////////////////////////
    //TODO(deccer): move to spacescene.cpp
    for (auto& object : g_Scene_Current->Objects())
    {
        object->ObjectMaterial->Apply();
        switch (object->ObjectShape)
        {
            case Shape::Cube: g_CubeGeometry->Bind(); break;
            case Shape::CubeInstanced:
            {
                g_CubeGeometry->Bind();
                //g_Buffer_Asteroids->BindAsStorageBuffer();
                reinterpret_cast<SpaceScene*>(g_Scene_Current)->GetAsteroidInstanceBuffer()->BindAsStorageBuffer(0);
                object->ExcludeFromMotionBlur = true;
                break;
            }
            case Shape::Ship: g_ShipGeometry->Bind(); break;
            case Shape::Quad: g_PlaneGeometry->Bind(); break;
        }

        auto const currentModelViewProjection = cameraProjection * cameraView * object->ModelViewProjection;

        g_GeometryProgram->SetVertexShaderUniform(2, object->ModelViewProjection);
        g_GeometryProgram->SetVertexShaderUniform(3, currentModelViewProjection);
        g_GeometryProgram->SetVertexShaderUniform(4, object->ModelViewProjectionPrevious);
        g_GeometryProgram->SetVertexShaderUniform(5, object->ExcludeFromMotionBlur);
        g_GeometryProgram->SetVertexShaderUniform(6, object->ObjectShape == Shape::CubeInstanced);

        object->ModelViewProjectionPrevious = currentModelViewProjection;

        switch (object->ObjectShape)
        {
            case Shape::Cube: g_CubeGeometry->Draw(); break;
            //case Shape::CubeInstanced: glDrawElementsInstancedBaseVertex(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr, 5000, 0); break;
            case Shape::CubeInstanced: g_CubeGeometry->DrawInstanced(5000); break;
            case Shape::Quad: g_PlaneGeometry->Draw(); break;
            case Shape::Ship: g_ShipGeometry->Draw(); break;
        }
    }
    glPopDebugGroup();
}

void RenderLights(
    const Texture& gBufferPosition,
    const Texture& gBufferNormal,
    const Texture& gBufferDepth,
    const glm::mat4& cameraProjection,
    const glm::vec3& cameraPosition,
    const glm::vec3& cameraDirection,
    int& visibleLights)
{
    constexpr std::string_view renderLightBufferDebugGroup = "Render LBuffer";
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 2, static_cast<GLsizei>(renderLightBufferDebugGroup.length()), renderLightBufferDebugGroup.data());

    auto constexpr depthClearValue = 1.0f;
    g_LightsFramebuffer->Clear(0, glm::value_ptr(glm::vec3(0.0f)));
    g_LightsFramebuffer->ClearDepth(depthClearValue);
    g_LightsFramebuffer->Bind();

    gBufferPosition.Bind(0);
    gBufferNormal.Bind(1);
    gBufferDepth.Bind(2);

    g_LightProgram->Bind();
    g_PointLightGeometry->Bind();

    glCullFace(GL_FRONT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    auto lightIndex = 0;
    visibleLights = 0;

    auto& lights = g_Scene_Current->Lights();
    for (auto& light : lights)
    {
        lightIndex++;

        if (!g_Frustum.SphereInFrustum(light.Position.x, light.Position.y, light.Position.z, light.Attenuation.z))
        {
            continue;
        }

        visibleLights++;
        auto model = glm::translate(glm::mat4(1.0f), glm::vec3(light.Position));
        model = glm::scale(model, glm::vec3(light.Attenuation.z, light.Attenuation.z, light.Attenuation.z));
        g_LightProgram->SetVertexShaderUniform(0, cameraProjection);
        g_LightProgram->SetVertexShaderUniform(1, g_Camera_View);
        g_LightProgram->SetVertexShaderUniform(2, model);

        g_LightProgram->SetFragmentShaderUniform(0, static_cast<s32>(light.Type));
        g_LightProgram->SetFragmentShaderUniform(1, light.Position);
        g_LightProgram->SetFragmentShaderUniform(2, light.Color);
        g_LightProgram->SetFragmentShaderUniform(3, light.Direction);
        g_LightProgram->SetFragmentShaderUniform(4, light.Attenuation);
        g_LightProgram->SetFragmentShaderUniform(5, light.CutOff);
        g_LightProgram->SetFragmentShaderUniform(6, cameraPosition);

        glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 240, 1, 0);
    }
    glDisable(GL_BLEND);
    glCullFace(GL_BACK);
    glPopDebugGroup();
}

// TODO(deccer): pass Camera, remove frameWidth/frameHeight/fieldOfView
void ResolveGBuffer(
    const TextureCube& skyboxTextureCube,
    const Texture& gBufferPosition,
    const Texture& gBufferNormal,
    const Texture& gBufferAlbedo,
    const Texture& gBufferDepth,
    const Texture& lightBufferTexture,
    const Texture& emissionTexture,
    const int frameWidth,
    const int frameHeight,
    const float fieldOfView)
{
    auto constexpr kUniformCameraDirection = 0;
    auto constexpr kUniformCameraFieldOfView = 1;
    auto constexpr kUniformCameraAspectRatio = 2;
    auto constexpr kUniformUvsDiff = 3;

    constexpr std::string_view resolveGBufferDebugGroup = "Resolve GBuffer";
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 3, static_cast<GLsizei>(resolveGBufferDebugGroup.length()), resolveGBufferDebugGroup.data());

    g_FinalFramebuffer->Clear(0, glm::value_ptr(glm::vec3(1.0f)));
    g_FinalFramebuffer->ClearDepth(1.0f);
    g_FinalFramebuffer->Bind();

    gBufferPosition.Bind(0);
    gBufferNormal.Bind(1);
    gBufferAlbedo.Bind(2);
    gBufferDepth.Bind(3);
    skyboxTextureCube.Bind(4);
    lightBufferTexture.Bind(5);
    emissionTexture.Bind(6);

    g_EmptyGeometry->Bind();
    g_FinalProgram->Bind();
    g_FinalProgram->SetVertexShaderUniform(kUniformCameraDirection, glm::inverse(glm::mat3(g_Camera_View)));
    g_FinalProgram->SetVertexShaderUniform(kUniformCameraFieldOfView, fieldOfView);
    g_FinalProgram->SetVertexShaderUniform(kUniformCameraAspectRatio, static_cast<f32>(frameWidth) / static_cast<f32>(frameHeight));
    g_FinalProgram->SetVertexShaderUniform(kUniformUvsDiff, glm::vec2(1.0f, 1.0f));

    glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);
    glPopDebugGroup();
}

void RenderEmission(const Texture& lightBufferTexture, const Texture& emissionTexture)
{
    constexpr std::string_view renderEmissionDebugGroup = "Render Emission";
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 4, static_cast<GLsizei>(renderEmissionDebugGroup.length()), renderEmissionDebugGroup.data());

    g_EmissionFramebuffer->Clear(0, glm::value_ptr(glm::vec3(0.0f)));
    g_EmissionFramebuffer->Bind();

    lightBufferTexture.Bind(0);
    emissionTexture.Bind(1);

    g_EmptyGeometry->Bind();
    g_EmissionProgram->Bind();
    g_EmissionProgram->SetFragmentShaderUniform(0, 0.7f);

    glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);
    glPopDebugGroup();
}

int main(int argc, char* argv[])
{
    if (!glfwInit())
    {
        std::cerr << "GLFW: Unable to initialize.\n";
        return 1;
    }

    s32 screenWidth{};
    s32 screenHeight{};
    GetWorkingArea(&screenWidth, &screenHeight);

    const auto windowWidth = static_cast<u32>(0.8f * screenWidth);
    const auto windowHeight = static_cast<u32>(0.8f * screenHeight);

    g_Window = CreateMainWindow(screenWidth, screenHeight, windowWidth, windowHeight, "emptyspace");
    if (g_Window == nullptr)
    {
        std::cerr << "GLFW: Unable to create a window.\n";
        return 1;
    }

    InitializeOpenGL(g_Window);
    InitializePhysics();	

    const auto frameWidth = static_cast<s32>(windowWidth * 1.0f);
    const auto frameHeight = static_cast<s32>(windowHeight * 1.0f);

    const auto graphicsDevice = new GraphicsDevice();
    g_Scene_Current = new SpaceScene(*graphicsDevice);

    g_gBufferFinalTexture = graphicsDevice->CreateTexture(GL_RGB8, GL_RGB, frameWidth, frameHeight, nullptr, GL_NEAREST);
    
    g_gBufferPositionTexture = graphicsDevice->CreateTexture(GL_RGBA16F, GL_RGB, frameWidth, frameHeight, nullptr, GL_NEAREST);
    g_gBufferNormalTexture = graphicsDevice->CreateTexture(GL_RGB16F, GL_RGB, frameWidth, frameHeight, nullptr, GL_NEAREST);
    g_gBufferAlbedoTexture = graphicsDevice->CreateTexture(GL_RGBA8, GL_RGBA, frameWidth, frameHeight, nullptr, GL_NEAREST);
    g_gBufferDepthTexture = graphicsDevice->CreateTexture(GL_DEPTH_COMPONENT32, GL_DEPTH, frameWidth, frameHeight, nullptr, GL_NEAREST);
    g_gBufferVelocityTexture = graphicsDevice->CreateTexture(GL_RG16F, GL_RG, frameWidth, frameHeight, nullptr, GL_NEAREST);
    g_LightBufferTexture = graphicsDevice->CreateTexture(GL_RGB16F, GL_RGB, frameWidth, frameHeight, nullptr, GL_NEAREST);
    g_MotionBlurTexture = graphicsDevice->CreateTexture(GL_RGB8, GL_RGB, frameWidth, frameHeight, nullptr, GL_NEAREST);
    g_TransitionTexture = graphicsDevice->CreateTexture(GL_RGB8, GL_RGB, frameWidth, frameHeight, nullptr, GL_NEAREST);
    g_EmissionTexture = graphicsDevice->CreateTexture(GL_RGBA16F, GL_RGBA, frameWidth, frameHeight, nullptr, GL_NEAREST);

    g_GeometryFramebuffer = graphicsDevice->CreateFramebuffer("FB_Geometry",
        {
            g_gBufferPositionTexture,
            g_gBufferNormalTexture,
            g_gBufferAlbedoTexture,
            g_gBufferVelocityTexture,
            g_EmissionTexture,
        },
        g_gBufferDepthTexture);
    g_FinalFramebuffer = graphicsDevice->CreateFramebuffer("FB_Final", { g_gBufferFinalTexture });
    g_EmissionFramebuffer = graphicsDevice->CreateFramebuffer("FB_Emission", { g_EmissionTexture });
    g_MotionBlurFramebuffer = graphicsDevice->CreateFramebuffer("FB_Motionblur", { g_MotionBlurTexture });
    g_LightsFramebuffer = graphicsDevice->CreateFramebuffer("FB_Lights", { g_LightBufferTexture });
    g_TransitionFramebuffer = graphicsDevice->CreateFramebuffer("FB_Transition", { g_TransitionTexture });

    g_SkyboxTextureCube = graphicsDevice->CreateTextureCubeFromFiles({
    "data/textures/TC_SkySpace_Xn.png",
    "data/textures/TC_SkySpace_Xp.png",
    "data/textures/TC_SkySpace_Yn.png",
    "data/textures/TC_SkySpace_Yp.png",
    "data/textures/TC_SkySpace_Zn.png",
    "data/textures/TC_SkySpace_Zp.png"
        });

    g_EmptyGeometry = Geometry::CreateEmpty();
    g_CubeGeometry = Geometry::CreateCube(1, 1, 1);
    //g_CubeGeometry = Geometry::CreateFromFile("data/models/SM_Cube.fbx");
    g_PlaneGeometry = Geometry::CreatePlane(3, 3);
    g_ShipGeometry = Geometry::CreateFromFile("data/models/SM_ShipA_noWindshield.obj");
    g_PointLightGeometry = Geometry::CreateFromFile("data/models/SM_PointLight.obj");
        
    g_FinalProgram = graphicsDevice->CreateProgramFromFiles(
        "PP_Final",
        "data/shaders/main.vert.glsl",
        "data/shaders/main.frag.glsl");
    g_GeometryProgram = graphicsDevice->CreateProgramFromFiles(
        "PP_Geometry",
        "data/shaders/gbuffer.vert.glsl",
        "data/shaders/gbuffer.frag.glsl");
    g_MotionBlurProgram = graphicsDevice->CreateProgramFromFiles(
        "PP_MotionBlur",
        "data/shaders/motionblur.vert.glsl",
        "data/shaders/motionblur.frag.glsl");
    g_LightProgram = graphicsDevice->CreateProgramFromFiles(
        "PP_Light",
        "data/shaders/light.vert.glsl",
        "data/shaders/light.frag.glsl");
    g_QuadProgram = graphicsDevice->CreateProgramFromFiles(
        "PP_FSQ",
        "data/shaders/quad.vert.glsl",
        "data/shaders/quad.frag.glsl");
    g_EmissionProgram = graphicsDevice->CreateProgramFromFiles(
        "PP_Emission",
        "data/shaders/emission.vert.glsl",
        "data/shaders/emission.frag.glsl");

    /* uniforms */
    constexpr auto kUniformProjectionMatrix = 0;
    constexpr auto kUniformCameraPosition = 0;
    constexpr auto kUniformCameraDirection = 0;
    constexpr auto kUniformViewMatrix = 1;
    constexpr auto kUniformFieldOfView = 1;
    constexpr auto kUniformAspectRatio = 2;
    constexpr auto kUniformModel = 2;
    constexpr auto kUniformLight = 3;
    constexpr auto kUniformMotionBlurVelocityScale = 0;
    constexpr auto kUniformMotionBlurUvDiff = 3;
    constexpr auto kUniformModelViewProjection = 3;
    constexpr auto kUniformModelViewProjectionInverse = 4;
    constexpr auto kUniformMotionBlurExcept = 5;

    constexpr auto fieldOfView = glm::radians(60.0f);
    auto const cameraProjectionMatrix = glm::perspective(fieldOfView, static_cast<f32>(windowWidth) / static_cast<f32>(windowHeight), 0.1f, 1000.0f);
    
    g_GeometryProgram->SetVertexShaderUniform(kUniformProjectionMatrix, cameraProjectionMatrix);

    // SCENE SETUP BEGIN ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    g_Scene_Current->Initialize();

    // SCENE SETUP END //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    auto t1 = glfwGetTime();

    auto deltaTimeAverage = 0.0f;
    auto deltaTimeAverageSquared = 0.0f;

    auto framesToAverage = 100;
    auto frameCounter = 0;

    auto visibleLights = 0;

    glfwSwapInterval(g_IsVsyncEnabled ? 1 : 0);

    g_MousePosXOld = windowWidth / 2;
    g_MousePosYOld = windowHeight / 2;
    glfwSetCursorPos(g_Window, g_MousePosXOld, g_MousePosYOld);
    glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glViewport(0, 0, frameWidth, frameHeight);

    Material* currentMaterial = nullptr;
    
    while (!glfwWindowShouldClose(g_Window))
    {
        const auto t2 = glfwGetTime();
        const auto deltaTime = static_cast<f32>(t2 - t1);
        t1 = t2;

        deltaTimeAverage += deltaTime;
        deltaTimeAverageSquared += (deltaTime * deltaTime);
        frameCounter++;

        if (frameCounter == framesToAverage)
        {
            deltaTimeAverage /= static_cast<f32>(framesToAverage);
            deltaTimeAverageSquared /= static_cast<f32>(framesToAverage);
            const auto deltaTimeStandardError = sqrt(deltaTimeAverageSquared - deltaTimeAverage * deltaTimeAverage) /
                sqrt(framesToAverage);

            char str[128];
            snprintf(str, sizeof(str), "emptyspace, frame = %.3fms +/- %.4fms, fps = %.1f, %d frames, %d visible lights, %.3f", deltaTimeAverage * 1000.0f,
                1000.0f * deltaTimeStandardError, 1.0f / deltaTimeAverage, framesToAverage, visibleLights, g_Transition_Factor.r);
            glfwSetWindowTitle(g_Window, str);

            framesToAverage = static_cast<int>(1.0f / deltaTimeAverage);

            deltaTimeAverage = 0.0f;
            deltaTimeAverageSquared = 0.0f;
            frameCounter = 0;
        }

        Update(deltaTime);

        auto const camera = Camera::FromPhysicsScene(*g_PhysicsScene);

        ///////////////////////// SCENE UPDATE BEGIN /////////////////////////

        g_Scene_Current->Update(deltaTime, camera);

        ///////////////////////// SCENE UPDATE END /////////////////////////

        g_Frustum.CalculateFrustum(cameraProjectionMatrix, g_Camera_View);
        g_GeometryProgram->SetVertexShaderUniform(kUniformViewMatrix, g_Camera_View);

        RenderGBuffer(
            frameWidth,
            frameHeight,
            cameraProjectionMatrix,
            g_Camera_View);
        RenderLights(
            *g_gBufferPositionTexture,
            *g_gBufferNormalTexture,
            *g_gBufferDepthTexture,
            cameraProjectionMatrix,
            camera.Position,
            camera.Direction,
            visibleLights);
        RenderEmission(
            *g_LightBufferTexture,
            *g_EmissionTexture);
        ResolveGBuffer(
            *g_SkyboxTextureCube,
            *g_gBufferPositionTexture,
            *g_gBufferNormalTexture,
            *g_gBufferAlbedoTexture,
            *g_gBufferDepthTexture,
            *g_LightBufferTexture,
            *g_EmissionTexture,
            frameWidth,
            frameHeight,
            fieldOfView);

        if (g_Transition_Factor.w > 0.0f)
        {
            constexpr std::string_view transitionDebugGroup = "Transition";
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 5, static_cast<GLsizei>(transitionDebugGroup.length()), transitionDebugGroup.data());

            /* ============== TRANSITION EFFECT =================== */
            g_TransitionFramebuffer->Clear(0, glm::value_ptr(glm::vec3(0.0)));
            g_TransitionFramebuffer->Bind();

            g_gBufferFinalTexture->Bind(0);

            g_QuadProgram->Bind();
            g_QuadProgram->SetFragmentShaderUniform(0, g_Transition_Factor);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

            glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 3, 1, 0);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDisable(GL_BLEND);
        }
        /* ============== TRANSITION EFFECT =================== */

        if (g_IsMotionBlurEnabled)
        {
            constexpr std::string_view motionBlurDebugGroup = "MotionBlur";
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 6, static_cast<GLsizei>(motionBlurDebugGroup.length()), motionBlurDebugGroup.data());
            /* motion blur ========================================================================= begin */
            g_MotionBlurFramebuffer->Clear(0, glm::value_ptr(glm::vec3(0.0f)));
            g_MotionBlurFramebuffer->Bind();

            if (g_Transition_Factor.w > 0.0f)
            {
                g_TransitionTexture->Bind(0);
            }
            else
            {
                g_gBufferFinalTexture->Bind(0);
            }
            g_gBufferVelocityTexture->Bind(1);

            g_EmptyGeometry->Bind();
            g_MotionBlurProgram->Bind();
            g_MotionBlurProgram->SetVertexShaderUniform(kUniformMotionBlurUvDiff, glm::vec2(1.0f, 1.0f));
            g_MotionBlurProgram->SetFragmentShaderUniform(kUniformMotionBlurVelocityScale, 2.0f);

            glCullFace(GL_FRONT);
            glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 3, 1, 0);
            glCullFace(GL_BACK);
            glPopDebugGroup();
            /* motion blur =========================================================================== end */
        }

        /* final output */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, windowWidth, windowHeight);
        glBlitNamedFramebuffer(g_IsMotionBlurEnabled
            ? g_MotionBlurFramebuffer->Id()
            : g_IsTransitionEffectEnabled
                ? g_TransitionFramebuffer->Id()
                : g_FinalFramebuffer->Id(), 0, 0, 0, frameWidth, frameHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glFinish();
        glfwSwapBuffers(g_Window);
    }

    Cleanup();

    return 0;
}
