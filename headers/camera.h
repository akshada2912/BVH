#pragma once

#include "common.h"

struct Camera
{
    Vector3f from, to, up;
    float fieldOfView;
    Vector2i imageResolution;

    float focusDistance = 1.f;
    float aspect;

    Vector3f u, v, w;
    Vector3f pixelDeltaU, pixelDeltaV;
    Vector3f upperLeft;

    Camera(){};
    Camera(Vector3f from, Vector3f to, Vector3f up, float fieldOfView, Vector2i imageResolution);
    

    Ray generateRay(int x, int y);

    Ray transformRayToWorld(const Ray &ray) const;
    Vector3f transformPointToWorld(const Vector3f &point) const;
    Vector3f transformVectorToWorld(const Vector3f &vector) const;
};