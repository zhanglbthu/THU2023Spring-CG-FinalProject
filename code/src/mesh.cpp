#include "mesh.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <utility>
#include <sstream>
#include <cfloat>

Mesh::Mesh(const char *filename, Material *material) : Object3D(material)
{

    // Optional: Use tiny obj loader to replace this simple one.
    std::ifstream f;
    f.open(filename);
    if (!f.is_open())
    {
        std::cout << "Cannot open " << filename << "\n";
        return;
    }
    std::string line;
    std::string vTok("v");
    std::string fTok("f");
    std::string texTok("vt");
    std::string vnTok("vn");
    char bslash = '/', space = ' ';
    std::string tok;
    int texID;
    while (true)
    {
        std::getline(f, line);
        if (f.eof()) // End of file
        {
            break;
        }
        if (line.size() < 3)
        {
            continue;
        }
        if (line.at(0) == '#') // Comment
        {
            continue;
        }
        std::stringstream ss(line);
        ss >> tok; // Get first token
        if (tok == vTok)
        {
            Vector3f vec;
            ss >> vec[0] >> vec[1] >> vec[2]; // Read vertex coordinates
            v.push_back(vec);
        }
        else if (tok == fTok)
        {
            //TODO: 支持顶点索引和纹理索引
            if (line.find(bslash) != std::string::npos)//包含斜杠
            {
                has_texture_and_normal = true;
                std::replace(line.begin(), line.end(), bslash, space); //将斜杠替换为空格
                std::stringstream facess(line); 
                TriangleIndex v, vt, vn;
                facess >> tok;//读取f
                for (int ii = 0; ii < 3; ii++)
                {
                    facess >> v[ii] >> vt[ii] >> vn[ii];
                    v[ii]--;
                    vt[ii]--;
                    vn[ii]--;
                }
                vIndex.push_back(v);
                vtIndex.push_back(vt);
                vnIndex.push_back(vn);
            }
            else
            {
                TriangleIndex v;
                for (int ii = 0; ii < 3; ii++)
                {
                    ss >> v[ii];
                    v[ii]--;
                }
                vIndex.push_back(v);
            }
        }
        else if (tok == texTok)
        {
            Vector2f texcoord;
            ss >> texcoord[0];
            ss >> texcoord[1];
            vt.push_back(texcoord);
        }
        else if (tok == vnTok)
        {
            Vector3f norm;
            ss >> norm[0];
            ss >> norm[1];
            ss >> norm[2];
            vn.push_back(norm);
        }
    }

    f.close();
    if(has_texture_and_normal)
    {
        set_texture_and_normal();
    }
    else
    {
        computeNormal();
    }
    bvh = new BVH(triangles, 0, triangles.size());
}

void Mesh::computeNormal()
{
    n.resize(vIndex.size());
    for (int triId = 0; triId < (int)vIndex.size(); ++triId)
    {
        TriangleIndex &triIndex = vIndex[triId]; // 第triId个三角形的三个顶点索引
        Vector3f a = v[triIndex[1]] - v[triIndex[0]];
        Vector3f b = v[triIndex[2]] - v[triIndex[0]];
        b = Vector3f::cross(a, b);
        n[triId] = b / b.length(); 

        Triangle *triangle = new Triangle(v[triIndex[0]], v[triIndex[1]], v[triIndex[2]], material);
        triangle->normal = n[triId];
        triangles.push_back(triangle);
    }
}
void Mesh::set_texture_and_normal()
{
    for (int triId = 0; triId < (int)vIndex.size(); ++triId)
    {
        //设置顶点坐标
        TriangleIndex &vId = vIndex[triId];
        Triangle *triangle = new Triangle(v[vId[0]], v[vId[1]], v[vId[2]], material);
        //设置纹理坐标
        if(!vt.empty())
        {
            TriangleIndex &vtId = vtIndex[triId];
            triangle->set_vt(vt[vtId[0]], vt[vtId[1]], vt[vtId[2]]);
        }
        //设置法线
        if(!vn.empty())
        {
            TriangleIndex &vnId = vnIndex[triId];
            triangle->set_vn(vn[vnId[0]], vn[vnId[1]], vn[vnId[2]]);
        }
        triangles.push_back(triangle);
    }
}
