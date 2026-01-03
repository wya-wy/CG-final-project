#ifndef COMMON_H
#define COMMON_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

// 定义顶点结构
struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

// Texture definition moved to Texture.h

#endif