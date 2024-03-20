#include "camera.h"

Camera::Camera(Vector3f from, Vector3f to, Vector3f up, float fieldOfView, Vector2i imageResolution)
    : from(from),
      to(to),
      up(up),
      fieldOfView(fieldOfView),
      imageResolution(imageResolution)
{
    this->aspect = imageResolution.x / float(imageResolution.y);

    // Determine viewport dimensions in 3D
    float fovRadians = fieldOfView * M_PI / 180.f;
    float h = std::tan(fovRadians / 2.f);
    float viewportHeight = 2.f * h * this->focusDistance;
    float viewportWidth = viewportHeight * this->aspect;

    // Calculate basis vectors of the camera for the given transform
    this->w = Normalize(this->from - this->to);
    this->u = Normalize(Cross(up, this->w));
    this->v = Normalize(Cross(this->w, this->u));

    // Pixel delta vectors
    Vector3f viewportU = viewportWidth * this->u;
    Vector3f viewportV = viewportHeight * (-this->v);

    this->pixelDeltaU = viewportU / float(imageResolution.x);
    this->pixelDeltaV = viewportV / float(imageResolution.y);

    // Upper left
    this->upperLeft = from - this->w * this->focusDistance - viewportU / 2.f - viewportV / 2.f;
}

Ray Camera::generateRay(int x, int y)
{

    //CORRECT CODE:
        Vector3f pixelCenter = (this->upperLeft - this->from + this->pixelDeltaU*(x+0.5f) + this->pixelDeltaV*(y+0.5f));

        Vector3f direction = Normalize((pixelCenter));

    //INCORRECT CODE:
    // Vector3f pixelCenter = (this->upperLeft  + this->pixelDeltaU*(x+0.5f) + this->pixelDeltaV*(y+0.5f));

    // Vector3f direction = Normalize((pixelCenter- this->from));
    //direction-=this->from;
    
    return Ray(this->from,direction);






    //  Vector3f pixelCenter = (this->upperLeft + this->pixelDeltaU*(x+0.5f) + this->pixelDeltaV*(y+0.5f));
    // // pixelCenter += Vector3f(1000000.0f, 1000000.0f, 1000000.0f);

    // Vector3f direction = Normalize(pixelCenter - this->from);
    
    // return Ray(this->from,direction);

    // Calculate the ray direction in camera space

    // Transform the ray to world space

    // Transform the ray to world coordinates
    // return transformRayToWorld(cameraRay);
}

// 1
Ray Camera::transformRayToWorld(const Ray &ray) const
{
    return {transformPointToWorld(ray.o), transformVectorToWorld(ray.d), ray.t, ray.tmax};
}

Vector3f Camera::transformPointToWorld(const Vector3f &point) const
{
    // Transform the point from camera local coordinates to world coordinates
    return this->from + this->u * point.x + this->v * point.y + this->w * point.z;
}

Vector3f Camera::transformVectorToWorld(const Vector3f &vector) const
{
    // Transform the vector from camera local coordinates to world coordinates
    return this->u * vector.x + this->v * vector.y + this->w * vector.z;
}