#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <string>

class Texture
{
public:
    unsigned int id;
    std::string type; // diffuse, specular
    std::string path;

    Texture();
    Texture(const char *path, const std::string &type);

    void Bind(int unit) const;
};

#endif
