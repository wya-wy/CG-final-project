#ifndef APPLICATION_H
#define APPLICATION_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

#include "SceneContext.h"
#include "Camera.h"
#include "Shader.h"

class Application {
public:
    Application(const std::string& title, int width, int height);
    ~Application();

    void Run();

private:
    GLFWwindow* window;
    int scrWidth;
    int scrHeight;
    std::string appTitle;

    Camera* camera;
    SceneContext* scene;
    Shader* mainShader;

    // 状态
    float deltaTime;
    float lastFrame;
    bool isMousePressed;      // 右键漫游状态
    bool isDragging;          // [新增] 左键拖拽物体状态
    bool firstMouse;
    float lastX, lastY;

    // UI 缓存变量 [新增]
    char objPathBuffer[256] = "assets/models/teapot.obj";
    char texturePathBuffer[256] = "assets/textures/wood.png";

    // 初始化
    bool InitGLFW();
    bool InitImGui();
    void InitScene();

    // 逻辑
    void ProcessInput();
    void RenderUI();
    void RenderScene();
    void DeleteSelectedObject();

    // 射线检测算法
    void SelectObjectFromMouse(double xpos, double ypos);
    bool IntersectRayAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, 
                          const glm::vec3& boxMin, const glm::vec3& boxMax, float& t);
    
    // [新增] 射线与平面相交 (用于拖拽移动)
    bool IntersectRayPlane(const glm::vec3& rayOrigin, const glm::vec3& rayDir, 
                           const glm::vec3& planeNormal, const glm::vec3& planePoint, 
                           float& t);
    // [新增] 处理拖拽逻辑
    void ProcessDrag(double xpos, double ypos);

    // 回调
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
};

#endif