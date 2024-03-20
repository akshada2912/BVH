#pragma once

#include "common.h"
#include "texture.h"

struct Surface
{
    std::vector<Vector3f> vertices, normals;
    std::vector<Vector3i> indices;
    std::vector<Vector2f> uvs;

    bool isLight;
    uint32_t shapeIdx;

    Vector3f diffuse;
    float alpha;

    Texture diffuseTexture, alphaTexture;

    Interaction rayPlaneIntersect(Ray ray, Vector3f p, Vector3f n);
    Interaction rayTriangleIntersect(Ray ray, Vector3f v1, Vector3f v2, Vector3f v3, Vector3f n);
    Interaction rayIntersect(Ray ray);
    // bool aabb_intersect(const Ray&ray, const AABB&aabb);

    AABB computeAABB() const
    {
        if (vertices.empty())
        {
            // Handle case where there are no vertices (error or default AABB)
            return AABB();
        }

        Vector3f minBound = vertices[0];
        Vector3f maxBound = vertices[0];

        for (const Vector3<float> &vertex : vertices)
        {
            minBound.x = std::min(minBound.x, vertex.x);
            minBound.y = std::min(minBound.y, vertex.y);
            minBound.z = std::min(minBound.z, vertex.z);

            maxBound.x = std::max(maxBound.x, vertex.x);
            maxBound.y = std::max(maxBound.y, vertex.y);
            maxBound.z = std::max(maxBound.z, vertex.z);
        }

        AABB aabb;
        aabb.min = minBound;
        aabb.max = maxBound;

        return aabb;
    }

    AABB computeTriangleAABB(const Vector3<float> &v1, const Vector3<float> &v2, const Vector3<float> &v3) const
    {
        AABB aabb;
        aabb.min.x = std::min({v1.x, v2.x, v3.x});
        aabb.min.y = std::min({v1.y, v2.y, v3.y});
        aabb.min.z = std::min({v1.z, v2.z, v3.z});

        aabb.max.x = std::max({v1.x, v2.x, v3.x});
        aabb.max.y = std::max({v1.y, v2.y, v3.y});
        aabb.max.z = std::max({v1.z, v2.z, v3.z});

        return aabb;
    }

private:
    bool hasDiffuseTexture();
    bool hasAlphaTexture();
};

std::vector<Surface> createSurfaces(std::string pathToObj, bool isLight, uint32_t shapeIdx);