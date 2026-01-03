#include "Application.h"
#include <iostream>
#include <algorithm>
#include <vector>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ModelLoader.h"
#include "GeometryUtils.h"
#include "Renderer.h"
#include "Texture.h"

Application::Application(const std::string &title, int width, int height)
    : appTitle(title), scrWidth(width), scrHeight(height),
      deltaTime(0.0f), lastFrame(0.0f),
      isMousePressed(false), isDragging(false), firstMouse(true),
      camera(nullptr), scene(nullptr), mainShader(nullptr)
{
    lastX = width / 2.0f;
    lastY = height / 2.0f;
}

Application::~Application()
{
    if (scene)
        delete scene;
    if (camera)
        delete camera;
    if (mainShader)
        delete mainShader;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Application::InitGLFW()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(scrWidth, scrHeight, appTitle.c_str(), NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    glEnable(GL_DEPTH_TEST);

    // [Part C] Init Shadow Map
    PartC::Renderer::InitShadowMap();

    return true;
}

bool Application::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.FontGlobalScale = 1.2f;
    ImGui::StyleColorsDark();

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.3f, 0.6f, 0.9f, 1.0f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    return true;
}

void Application::InitScene()
{
    camera = new Camera(glm::vec3(0.0f, 4.0f, 8.0f));
    scene = new SceneContext();
    mainShader = new Shader("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl");

    // 地面
    Mesh *floorMesh = GeometryUtils::CreateCube();
    SceneObject *floorObj = new SceneObject("Ground Plane", floorMesh);
    floorObj->scale = glm::vec3(20.0f, 0.01f, 20.0f);
    floorObj->position = glm::vec3(0.0f, -0.01f, 0.0f);
    floorObj->color = glm::vec3(0.25f, 0.25f, 0.25f);
    scene->AddObject(floorObj);

    // 默认测试物体
    Mesh *cubeMesh = GeometryUtils::CreateCube();

    // [Part C Test] Manually create a checkerboard texture to verify rendering pipeline
    unsigned int texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    const int w = 64, h = 64;
    unsigned char data[w * h * 3];
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            bool c = ((x / 8 + y / 8) % 2) == 0;
            int idx = (y * w + x) * 3;
            unsigned char val = c ? 255 : 50; // White and Dark Gray
            data[idx] = val;                  // R
            data[idx + 1] = val;              // G
            data[idx + 2] = val;              // B
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Texture diffuseMap;
    diffuseMap.id = texID;
    diffuseMap.type = "diffuse";
    diffuseMap.path = "generated_checkerboard";

    // Reuse same texture for specular for now
    Texture specularMap;
    specularMap.id = texID;
    specularMap.type = "specular";
    specularMap.path = "generated_checkerboard";

    cubeMesh->textures.push_back(diffuseMap);
    cubeMesh->textures.push_back(specularMap);

    SceneObject *cubeObj = new SceneObject("Cube", cubeMesh);
    cubeObj->position = glm::vec3(0.0f, 0.5f, 0.0f);
    cubeObj->color = glm::vec3(1.0f, 1.0f, 1.0f);
    scene->AddObject(cubeObj);

    scene->selectedObject = nullptr;
}

void Application::DeleteSelectedObject()
{
    if (!scene || !scene->selectedObject)
        return;
    auto &objs = scene->objects;
    for (auto it = objs.begin(); it != objs.end();)
    {
        if (*it == scene->selectedObject)
        {
            delete (*it)->mesh;
            delete *it;
            it = objs.erase(it);
            scene->selectedObject = nullptr;
            break;
        }
        else
        {
            ++it;
        }
    }
}

void Application::Run()
{
    if (!InitGLFW())
        return;
    if (!InitImGui())
        return;
    InitScene();

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        ProcessInput();

        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        RenderScene();
        RenderUI();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Application::ProcessInput()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (ImGui::GetIO().WantCaptureKeyboard)
        return;

    if (camera)
    {
        float speed = deltaTime;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            speed *= 3.0f;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera->ProcessKeyboard(FORWARD, speed);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera->ProcessKeyboard(BACKWARD, speed);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera->ProcessKeyboard(LEFT, speed);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera->ProcessKeyboard(RIGHT, speed);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera->ProcessKeyboard(DOWN, speed);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camera->ProcessKeyboard(UP, speed);
    }

    static bool deletePressed = false;
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS)
    {
        if (!deletePressed)
        {
            DeleteSelectedObject();
            deletePressed = true;
        }
    }
    else
    {
        deletePressed = false;
    }
}

// ---------------- 射线检测算法 ----------------

// 射线与平面相交检测
// P = rayOrigin + t * rayDir
// (P - planePoint) . planeNormal = 0
bool Application::IntersectRayPlane(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir,
                                    const glm::vec3 &planeNormal, const glm::vec3 &planePoint,
                                    float &t)
{
    float denom = glm::dot(planeNormal, rayDir);
    if (abs(denom) > 1e-6)
    { // 避免平行
        glm::vec3 p0l0 = planePoint - rayOrigin;
        t = glm::dot(p0l0, planeNormal) / denom;
        return (t >= 0);
    }
    return false;
}

bool Application::IntersectRayAABB(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir,
                                   const glm::vec3 &boxMin, const glm::vec3 &boxMax,
                                   float &t)
{
    glm::vec3 tMin = (boxMin - rayOrigin) / rayDir;
    glm::vec3 tMax = (boxMax - rayOrigin) / rayDir;
    glm::vec3 t1 = glm::min(tMin, tMax);
    glm::vec3 t2 = glm::max(tMin, tMax);
    float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
    float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);
    if (tNear > tFar || tFar < 0.0f)
        return false;
    t = tNear;
    return true;
}

void Application::SelectObjectFromMouse(double xpos, double ypos)
{
    if (!camera || !scene)
        return;

    // 1. 生成世界空间射线
    float x = (2.0f * xpos) / scrWidth - 1.0f;
    float y = 1.0f - (2.0f * ypos) / scrHeight;
    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);
    glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)scrWidth / (float)scrHeight, 0.1f, 100.0f);
    glm::vec4 rayEye = glm::inverse(projection) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
    glm::vec3 rayDirWorld = glm::normalize(glm::vec3(glm::inverse(camera->GetViewMatrix()) * rayEye));
    glm::vec3 rayOriginWorld = camera->Position;

    SceneObject *bestObj = nullptr;
    float minDistance = std::numeric_limits<float>::max();

    for (auto obj : scene->objects)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, obj->position);
        model = glm::rotate(model, glm::radians(obj->rotation.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(obj->rotation.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(obj->rotation.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, obj->scale);

        glm::mat4 invModel = glm::inverse(model);
        glm::vec3 rayOriginLocal = glm::vec3(invModel * glm::vec4(rayOriginWorld, 1.0f));
        glm::vec3 rayDirLocal = glm::vec3(invModel * glm::vec4(rayDirWorld, 0.0f));

        float t = 0.0f;
        if (IntersectRayAABB(rayOriginLocal, rayDirLocal, glm::vec3(-0.5f), glm::vec3(0.5f), t))
        {
            glm::vec3 hitPointLocal = rayOriginLocal + rayDirLocal * t;
            glm::vec3 hitPointWorld = glm::vec3(model * glm::vec4(hitPointLocal, 1.0f));
            float worldDist = glm::distance(camera->Position, hitPointWorld);
            if (worldDist < minDistance)
            {
                minDistance = worldDist;
                bestObj = obj;
            }
        }
    }

    if (bestObj)
    {
        scene->selectedObject = bestObj;
    }
}

// 拖拽逻辑：将鼠标投射到 XZ 平面上
void Application::ProcessDrag(double xpos, double ypos)
{
    if (!scene || !scene->selectedObject || !camera)
        return;

    // 计算射线
    float x = (2.0f * xpos) / scrWidth - 1.0f;
    float y = 1.0f - (2.0f * ypos) / scrHeight;
    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);
    glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)scrWidth / (float)scrHeight, 0.1f, 100.0f);
    glm::vec4 rayEye = glm::inverse(projection) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
    glm::vec3 rayDirWorld = glm::normalize(glm::vec3(glm::inverse(camera->GetViewMatrix()) * rayEye));
    glm::vec3 rayOriginWorld = camera->Position;

    // 构造一个通过物体当前Y坐标的水平面
    glm::vec3 planeNormal(0.0f, 1.0f, 0.0f);
    glm::vec3 planePoint(0.0f, scene->selectedObject->position.y, 0.0f);

    float t = 0.0f;
    if (IntersectRayPlane(rayOriginWorld, rayDirWorld, planeNormal, planePoint, t))
    {
        glm::vec3 hitPoint = rayOriginWorld + rayDirWorld * t;
        // 更新物体位置 (保持 Y 不变，只移动 XZ)
        scene->selectedObject->position.x = hitPoint.x;
        scene->selectedObject->position.z = hitPoint.z;
    }
}

// --------------------------------------------------------

void Application::RenderScene()
{
    if (!mainShader || !scene || !camera)
        return;

    // ------------------------------------------------
    // 1. Render Shadow Map (Pass 1)
    // ------------------------------------------------
    PartC::Renderer::BeginShadowMap();
    for (auto obj : scene->objects)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, obj->position);
        model = glm::rotate(model, glm::radians(obj->rotation.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(obj->rotation.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(obj->rotation.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, obj->scale);

        // Use depth shader (managed internally by Renderer)
        PartC::Renderer::depthShader->setMat4("model", model);
        if (obj->mesh)
            obj->mesh->Draw(*PartC::Renderer::depthShader);
    }
    PartC::Renderer::EndShadowMap(scrWidth, scrHeight);

    // ------------------------------------------------
    // 2. Render Scene Normally (Pass 2)
    // ------------------------------------------------
    glViewport(0, 0, scrWidth, scrHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mainShader->use();
    glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)scrWidth / (float)scrHeight, 0.1f, 100.0f);
    glm::mat4 view = camera->GetViewMatrix();
    mainShader->setMat4("projection", projection);
    mainShader->setMat4("view", view);

    // [Part C] Use Renderer to setup lights (includes shadow map binding)
    PartC::Renderer::SetupLights(*mainShader, camera->Position);

    for (auto obj : scene->objects)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, obj->position);
        model = glm::rotate(model, glm::radians(obj->rotation.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(obj->rotation.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(obj->rotation.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, obj->scale);

        // [Part C] Use Renderer to render mesh
        mainShader->setVec3("objectColor", obj->color);
        PartC::Renderer::RenderMesh(obj->mesh, *mainShader, model);

        if (obj == scene->selectedObject)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(2.5f);
            glm::mat4 highlightModel = glm::scale(model, glm::vec3(1.005f));
            mainShader->setMat4("model", highlightModel);

            // Note: Highlight shader logic might need adjustment if it relies on objectColor
            // For now, we just draw lines on top.
            // mainShader->setVec3("objectColor", glm::vec3(1.0f, 1.0f, 0.0f));

            if (obj->mesh)
                obj->mesh->Draw(*mainShader);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glLineWidth(1.0f);
        }
    }
}

void Application::RenderUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // ---------------- 编辑器主面板 ----------------
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 650), ImGuiCond_FirstUseEver); // 加高一点
    ImGui::Begin("Scene Editor");

    ImGui::Text("SCENE HIERARCHY");
    ImGui::Separator();

    ImGui::BeginChild("HierarchyList", ImVec2(0, 150), true);
    for (auto obj : scene->objects)
    {
        bool isSelected = (scene->selectedObject == obj);
        if (ImGui::Selectable(obj->name.c_str(), isSelected))
        {
            scene->selectedObject = obj;
        }
        if (isSelected)
            ImGui::SetItemDefaultFocus();
    }
    ImGui::EndChild();

    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Text("GLOBAL LIGHTING");
    ImGui::Separator();

    ImGui::DragFloat3("Light Dir", (float *)&PartC::Renderer::mainLight.direction, 0.05f, -1.0f, 1.0f);
    ImGui::ColorEdit3("Ambient", (float *)&PartC::Renderer::mainLight.ambient);
    ImGui::ColorEdit3("Diffuse", (float *)&PartC::Renderer::mainLight.diffuse);
    ImGui::ColorEdit3("Specular", (float *)&PartC::Renderer::mainLight.specular);

    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Text("CREATE & IMPORT");
    ImGui::Separator();

    // [新增] 添加物体 UI
    if (ImGui::Button("Cube", ImVec2(60, 0)))
    {
        Mesh *mesh = GeometryUtils::CreateCube();
        SceneObject *newObj = new SceneObject("New Cube", mesh);
        newObj->position = glm::vec3(0, 0.5f, 0); // 生成在地面上
        scene->AddObject(newObj);
    }
    ImGui::SameLine();
    if (ImGui::Button("Sphere", ImVec2(60, 0)))
    {
        Mesh *mesh = GeometryUtils::CreateSphere(20, 20);
        SceneObject *newObj = new SceneObject("New Sphere", mesh);
        newObj->position = glm::vec3(0, 0.5f, 0);
        scene->AddObject(newObj);
    }

    // [新增] 加载 OBJ UI
    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Text("Import Model (.obj)");
    ImGui::InputText("##objPath", objPathBuffer, sizeof(objPathBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Load"))
    {
        // 调用 Part B 接口
        Mesh *imported = ModelLoader::LoadMesh(objPathBuffer);
        if (imported)
        {
            SceneObject *newObj = new SceneObject("Imported Model", imported);
            scene->AddObject(newObj);
        }
        else
        {
            std::cout << "Failed to load model from " << objPathBuffer << std::endl;
        }
    }

    // ---------------- 属性面板 ----------------
    if (scene->selectedObject)
    {
        ImGui::Dummy(ImVec2(0, 15));
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "INSPECTOR: %s", scene->selectedObject->name.c_str());

        ImGui::Text("Transform");
        ImGui::DragFloat3("Pos", (float *)&scene->selectedObject->position, 0.05f);
        ImGui::DragFloat3("Rot", (float *)&scene->selectedObject->rotation, 1.0f);

        if (scene->selectedObject->name.find("Ground") != std::string::npos)
        {
            ImGui::DragFloat("Size X", &scene->selectedObject->scale.x, 0.1f, 0.1f, 100.0f);
            ImGui::DragFloat("Size Z", &scene->selectedObject->scale.z, 0.1f, 0.1f, 100.0f);
        }
        else
        {
            ImGui::DragFloat3("Scale", (float *)&scene->selectedObject->scale, 0.05f, 0.01f, 100.0f);
        }

        ImGui::Dummy(ImVec2(0, 5));
        ImGui::Text("Material & Texture");
        ImGui::ColorEdit3("Color", (float *)&scene->selectedObject->color);

        // [新增] 材质 Shininess 控制
        // 注意：目前 Shininess 是在 Shader 中统一设置的，为了支持单个物体，我们需要修改 Shader 和 Renderer
        // 这里暂时演示全局 Shininess 或者预留接口
        static float shininess = 32.0f;
        if (ImGui::DragFloat("Shininess", &shininess, 1.0f, 2.0f, 256.0f))
        {
            // TODO: 将此值传递给 Shader (目前 Renderer::SetupLights 中是硬编码的)
            // 我们可以临时通过 uniform 传递，或者将其作为 SceneObject 的属性
        }

        // [新增] 纹理 UI
        ImGui::Text("Texture Path (Absolute)");
        static char texBuf[256] = "";
        ImGui::InputText("##texPath", texBuf, sizeof(texBuf));
        ImGui::SameLine();
        if (ImGui::Button("Apply Tex"))
        {
            if (scene->selectedObject->mesh)
            {
                // 1. 清除旧纹理
                scene->selectedObject->mesh->textures.clear();

                // 2. 加载新纹理 (Part C 功能)
                std::string path = texBuf;
                Texture diffuseMap(path.c_str(), "diffuse");
                Texture specularMap(path.c_str(), "specular"); // 暂时复用同一张图

                // 3. 应用到 Mesh
                if (diffuseMap.id != 0)
                {
                    scene->selectedObject->mesh->textures.push_back(diffuseMap);
                    scene->selectedObject->mesh->textures.push_back(specularMap);
                    scene->selectedObject->texturePath = path;
                    std::cout << "Successfully loaded texture: " << path << std::endl;
                }
                else
                {
                    std::cerr << "Failed to load texture: " << path << std::endl;
                }
            }
        }

        ImGui::Dummy(ImVec2(0, 15));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        if (ImGui::Button("DELETE OBJECT", ImVec2(-1, 30)))
        {
            DeleteSelectedObject();
        }
        ImGui::PopStyleColor(2);
    }
    else
    {
        ImGui::Dummy(ImVec2(0, 20));
        ImGui::TextDisabled("Select an object to edit attributes");
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::MouseCallback(GLFWwindow *window, double xpos, double ypos)
{
    Application *app = (Application *)glfwGetWindowUserPointer(window);
    if (!app)
        return;

    bool isAltDown = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
    bool isRightMouse = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    bool isLeftMouse = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    // [新增] 拖拽逻辑：如果在拖拽状态且按住左键
    if (app->isDragging && isLeftMouse)
    {
        app->ProcessDrag(xpos, ypos);
        return; // 拖拽时不处理旋转逻辑
    }

    if (!isAltDown && !isRightMouse)
    {
        app->firstMouse = true;
        return;
    }

    if (app->firstMouse)
    {
        app->lastX = xpos;
        app->lastY = ypos;
        app->firstMouse = false;
    }

    float xoffset = xpos - app->lastX;
    float yoffset = app->lastY - ypos;
    app->lastX = xpos;
    app->lastY = ypos;

    if (isAltDown && app->scene && app->scene->selectedObject)
    {
        app->scene->selectedObject->rotation.y += xoffset * 0.5f;
        app->scene->selectedObject->rotation.x -= yoffset * 0.5f;
    }
    else if (isRightMouse && app->camera)
    {
        app->camera->ProcessMouseMovement(xoffset, yoffset);
    }
}

void Application::ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    Application *app = (Application *)glfwGetWindowUserPointer(window);
    if (!app || ImGui::GetIO().WantCaptureMouse)
        return;

    if (app->scene && app->scene->selectedObject)
    {
        float scaleFactor = 1.0f + (float)yoffset * 0.1f;
        if (scaleFactor < 0.1f)
            scaleFactor = 0.1f;
        app->scene->selectedObject->scale *= scaleFactor;
    }
    else if (app->camera)
    {
        app->camera->ProcessMouseScroll(yoffset);
    }
}

void Application::MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    Application *app = (Application *)glfwGetWindowUserPointer(window);
    if (!app || ImGui::GetIO().WantCaptureMouse)
        return;

    if (action == GLFW_PRESS)
    {
        if (button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            app->scene->selectedObject = nullptr;
            app->isMousePressed = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            app->SelectObjectFromMouse(xpos, ypos);

            // [新增] 如果点中了物体，进入拖拽状态
            if (app->scene->selectedObject)
            {
                app->isDragging = true;
            }
        }
    }
    else if (action == GLFW_RELEASE)
    {
        if (button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            app->isMousePressed = false;
            app->firstMouse = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            app->isDragging = false; // 释放拖拽
        }
    }
}

void Application::FramebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    Application *app = (Application *)glfwGetWindowUserPointer(window);
    if (app)
    {
        app->scrWidth = width;
        app->scrHeight = height;
    }
}