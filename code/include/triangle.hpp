#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>
using namespace std;

// TODO: implement this class and add more fields as necessary,
class Triangle : public Object3D
{

public:
	Triangle() = delete;

	// a b c are three vertex positions of the triangle
	Triangle(const Vector3f &a, const Vector3f &b, const Vector3f &c, Material *m) : Object3D(m), vertices{a, b, c}
	{
		normal = Vector3f::cross(b - a, c - a).normalized();
		at = Vector2f::ZERO;
		bt = Vector2f::ZERO;
		ct = Vector2f::ZERO;
		an = Vector3f::ZERO;
		bn = Vector3f::ZERO;
		cn = Vector3f::ZERO;
	}

	bool intersect(const Ray &ray, Hit &hit, float tmin) override
	{
		const float EPSILON = 0.0000001;
		Vector3f edge1 = vertices[1] - vertices[0];				 // E1
		Vector3f edge2 = vertices[2] - vertices[0];				 // E2
		Vector3f h = Vector3f::cross(ray.getDirection(), edge2); // S1
		float a = Vector3f::dot(edge1, h);						 // S1 dot E1
		if (a > -EPSILON && a < EPSILON)
		{
			return false;
		}
		float f = 1.0 / a;							// 1/(S1 dot E1)
		Vector3f s = ray.getOrigin() - vertices[0]; // S
		float u = f * Vector3f::dot(s, h);			// S dot S1 / (S1 dot E1)=b1
		if (u < 0.0 || u > 1.0)
		{
			return false;
		}
		Vector3f q = Vector3f::cross(s, edge1);				// S cross E1=S2
		float v = f * Vector3f::dot(ray.getDirection(), q); // S2 dot D / (S1 dot E1)=b2
		if (v < 0.0 || u + v > 1.0)
		{
			return false;
		}
		float t = f * Vector3f::dot(edge2, q); // S2 dot E2 / (S1 dot E1)=t
		if (t > EPSILON && t < hit.getT())
		{
			//如果an，bn，cn不为0，说明有法线信息
			if (an != Vector3f::ZERO && bn != Vector3f::ZERO && cn != Vector3f::ZERO)
			{
				normal = get_normal(ray.pointAtParameter(t));
			}
			hit.set(t, material, normal, ray.pointAtParameter(t));
			set_hit_color(hit, ray.pointAtParameter(t));
			return true;
		}
		return false;
	}
	void set_vt(Vector2f a, Vector2f b, Vector2f c)
	{
		at = a;
		bt = b;
		ct = c;
		// printf("at: (%f, %f), bt: (%f, %f), ct: (%f, %f)\n", at.x(), at.y(), bt.x(), bt.y(), ct.x(), ct.y());
	}
	void set_vn(Vector3f a, Vector3f b, Vector3f c)
	{
		an = a;
		bn = b;
		cn = c;
	}
	// 计算包围盒
	AABB getAABB() override
	{
		float minx = min(vertices[0].x(), min(vertices[1].x(), vertices[2].x()));
		float miny = min(vertices[0].y(), min(vertices[1].y(), vertices[2].y()));
		float minz = min(vertices[0].z(), min(vertices[1].z(), vertices[2].z()));
		float maxx = max(vertices[0].x(), max(vertices[1].x(), vertices[2].x()));
		float maxy = max(vertices[0].y(), max(vertices[1].y(), vertices[2].y()));
		float maxz = max(vertices[0].z(), max(vertices[1].z(), vertices[2].z()));
		Vector3f min(minx, miny, minz);
		Vector3f max(maxx, maxy, maxz);

		return AABB(min, max);
	}
	Vector2f get_uv(const Vector3f &position)
	{
		Vector3f va = (vertices[0] - position), vb = (vertices[1] - position), vc = (vertices[2] - position);
		float ra = Vector3f::cross(vb, vc).length();
		float rb = Vector3f::cross(vc, va).length();
		float rc = Vector3f::cross(va, vb).length();
		Vector2f uv = (ra * at + rb * bt + rc * ct) / (ra + rb + rc); // 加权平均
		return uv;
	}
	Vector3f get_normal(const Vector3f &position)
	{
		Vector3f va = (vertices[0] - position), vb = (vertices[1] - position), vc = (vertices[2] - position);
		float ra = Vector3f::cross(vb, vc).length();
		float rb = Vector3f::cross(vc, va).length();
		float rc = Vector3f::cross(va, vb).length();
		return (ra * an + rb * bn + rc * cn).normalized();
	}
	void set_hit_color(Hit &h, const Vector3f &position) override
	{
		if (material->getTexture())
		{
			Vector2f uv = get_uv(position);
			h.setColor(material->getTextureColor(uv));
		}
		else
		{
			h.setColor(material->getColor());
		}
	}
	Vector3f normal;
	Vector3f vertices[3];

	Vector2f at, bt, ct; // 纹理
	Vector3f an, bn, cn; // 法线
protected:
};

#endif // TRIANGLE_H
