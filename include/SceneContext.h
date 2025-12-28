#ifndef SCENE_CONTEXT_H
#define SCENE_CONTEXT_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Shader.h"

struct SceneObject {
    std::string name;
    Mesh* mesh;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::vec3 color;
    
    // [新增] 纹理路径，用于 UI 显示和 Part C 加载
    std::string texturePath; 
    // [新增] 纹理ID (如果加载成功)
    unsigned int textureId = 0; 

    SceneObject(std::string n, Mesh* m) 
        : name(n), mesh(m), position(0.0f), rotation(0.0f), scale(1.0f), color(1.0f), texturePath("") {}
};

class SceneContext {
public:
    std::vector<SceneObject*> objects;
    SceneObject* selectedObject = nullptr;

    SceneContext();
    ~SceneContext();

    void AddObject(SceneObject* obj);
    void DrawAll(Shader& shader);
};

#endif