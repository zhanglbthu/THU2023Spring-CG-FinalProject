#define STB_IMAGE_IMPLEMENTATION
# include<texture.hpp>
# include<stb_image.h>
# include<iostream>
using namespace std;
//实现texture mapping
Texture::Texture(const char *filename)
{
    if(strlen(filename) == 0)
    {
        return;
    }
    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data)
    {
        this->width = width;
        this->height = height;
        this->nrChannels = nrChannels;
        this->data = data;
        //打印长宽
        printf("Texture loaded: %s\n", filename);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
}
Texture::~Texture()
{
    stbi_image_free(data);
}
Vector3f Texture::getColor(const Vector2f &uv) const
{
    //双线性插值
    float u = uv.x();
    float v = uv.y();
    float x = u * width;
    float y = v * height;
    int x1 = floor(x);
    int x2 = ceil(x);
    int y1 = floor(y);
    int y2 = ceil(y);
    float s = x - x1;
    float t = y - y1;
    Vector3f color1 = getColor(x1, y1);
    Vector3f color2 = getColor(x2, y1);
    Vector3f color3 = getColor(x1, y2);
    Vector3f color4 = getColor(x2, y2);
    Vector3f color = (1 - s) * (1 - t) * color1 + s * (1 - t) * color2 + (1 - s) * t * color3 + s * t * color4;
    return color;
}
Vector3f Texture::getColor(const Vector3f &point) const
{
    float u = point.x();
    float v = point.y();
    int x = u * width;
    int y = v * height;
    return getColor(x, y);
}
Vector3f Texture::getColor(int x, int y) const
{
    if (x < 0)
        x = 0;
    if (x >= width)
        x = width - 1;
    if (y < 0)
        y = 0;
    if (y >= height)
        y = height - 1;
    int index = (x + y * width) * nrChannels;
    float r = data[index] / 255.0f;
    float g = data[index + 1] / 255.0f;
    float b = data[index + 2] / 255.0f;
    return Vector3f(r, g, b);
}
