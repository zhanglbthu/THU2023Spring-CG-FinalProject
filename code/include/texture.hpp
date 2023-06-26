#ifndef TEXTURE_H
#define TEXTURE_H

#include <vecmath.h>
#include <cstring>
//texture mapping类
class Texture
{
public:
    Texture() = default;
    Texture(const char *filename);
    ~Texture();
    Vector3f getColor(const Vector2f &uv) const;
    Vector3f getColor(const Vector3f &point) const;
    Vector3f getColor(int x, int y) const;
private:
    int width;
    int height;
    int nrChannels;
    unsigned char *data;
};

#endif // TEXTURE_H