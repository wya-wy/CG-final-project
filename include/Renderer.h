#ifndef RENDERER_H
#define RENDERER_H

#include "Mesh.h"
#include "Shader.h"
#include <glm/glm.hpp>

namespace PartC
{
    class GeometryGenerator
    {
    public:
        // [接口] 生成球体
        static Mesh *CreateSphere(float radius, int segments);
    };

    // [新增] 光照参数结构体
    struct LightSettings
    {
        glm::vec3 direction = glm::vec3(-0.2f, -1.0f, -0.3f);
        glm::vec3 ambient = glm::vec3(0.2f);
        glm::vec3 diffuse = glm::vec3(0.5f);
        glm::vec3 specular = glm::vec3(1.0f);
    };

    class Renderer
    {
    public:
        // [新增] 全局光照设置实例
        static LightSettings mainLight;

        // [Shadow Mapping]
        static unsigned int shadowMapFBO;
        static unsigned int shadowMap;
        static const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
        static Shader *depthShader;
        static glm::mat4 lightSpaceMatrix;

        static void InitShadowMap();
        static void BeginShadowMap();
        static void EndShadowMap(int scrWidth, int scrHeight);

        // [接口] 统一渲染入口
        static void RenderMesh(Mesh *mesh, Shader &shader, const glm::mat4 &modelMatrix);

        // [接口] 设置光照参数
        static void SetupLights(Shader &shader, const glm::vec3 &camPos);
    };
}

#endif