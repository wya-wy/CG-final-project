#include "Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace PartC
{
    // 初始化静态成员
    LightSettings Renderer::mainLight;
    unsigned int Renderer::shadowMapFBO;
    unsigned int Renderer::shadowMap;
    Shader *Renderer::depthShader = nullptr;
    glm::mat4 Renderer::lightSpaceMatrix;

    void Renderer::InitShadowMap()
    {
        glGenFramebuffers(1, &shadowMapFBO);

        glGenTextures(1, &shadowMap);
        glBindTexture(GL_TEXTURE_2D, shadowMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = {1.0, 1.0, 1.0, 1.0};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        depthShader = new Shader("assets/shaders/shadow_depth.vert", "assets/shaders/shadow_depth.frag");
    }

    void Renderer::BeginShadowMap()
    {
        glm::mat4 lightProjection, lightView;
        float near_plane = 1.0f, far_plane = 20.0f;
        // Orthographic projection for directional light
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

        // Look from light direction
        // Note: direction is usually pointing FROM light TO object, so we negate it to get position
        // But here we assume direction is direction vector.
        // Let's assume light is far away.
        glm::vec3 lightPos = -mainLight.direction * 10.0f;
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        depthShader->use();
        depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::EndShadowMap(int scrWidth, int scrHeight)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, scrWidth, scrHeight);
    }

    void Renderer::RenderMesh(Mesh *mesh, Shader &shader, const glm::mat4 &modelMatrix)
    {
        shader.use();
        shader.setMat4("model", modelMatrix);

        // Calculate Normal Matrix: transpose(inverse(mat3(model)))
        // Note: Inverting a matrix is costly, so in a real engine we would cache this.
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));
        shader.setMat3("normalMatrix", normalMatrix);

        if (mesh)
        {
            mesh->Draw(shader);
        }
    }

    void Renderer::SetupLights(Shader &shader, const glm::vec3 &camPos)
    {
        shader.use();
        shader.setVec3("viewPos", camPos);

        // Use static mainLight settings
        shader.setVec3("dirLight.direction", mainLight.direction);
        shader.setVec3("dirLight.ambient", mainLight.ambient);
        shader.setVec3("dirLight.diffuse", mainLight.diffuse);
        shader.setVec3("dirLight.specular", mainLight.specular);

        // Material defaults
        shader.setFloat("material.shininess", 32.0f);

        // Shadow Map
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glActiveTexture(GL_TEXTURE15); // Use a high slot for shadow map
        glBindTexture(GL_TEXTURE_2D, shadowMap);
        shader.setInt("shadowMap", 15);
    }

    Mesh *GeometryGenerator::CreateSphere(float radius, int segments)
    {
        // Placeholder: Part B should implement this in GeometryUtils or similar.
        return nullptr;
    }
}
