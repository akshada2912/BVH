#include "render.h"

Integrator::Integrator(Scene &scene)
{
    this->scene = scene;
    this->outputImage.allocate(TextureType::UNSIGNED_INTEGER_ALPHA, this->scene.imageResolution);
}

long long Integrator::render()
{
    auto startTime = std::chrono::high_resolution_clock::now();
    for (int x = 0; x < this->scene.imageResolution.x; x++) {
        for (int y = 0; y < this->scene.imageResolution.y; y++) {
            Ray cameraRay = this->scene.camera.generateRay(x, y);
            Interaction si = this->scene.rayIntersect(cameraRay);

            if (si.didIntersect)
                this->outputImage.writePixelColor(0.5f * (si.n + Vector3f(1.f, 1.f, 1.f)), x, y);
            else
                this->outputImage.writePixelColor(Vector3f(0.f, 0.f, 0.f), x, y);
        }
    }
    auto finishTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
}

long long Integrator::render2()
{
    auto startTime = std::chrono::high_resolution_clock::now();
    for (int x = 0; x < this->scene.imageResolution.x; x++) {
        for (int y = 0; y < this->scene.imageResolution.y; y++) {
            Ray cameraRay = this->scene.camera.generateRay(x, y);
            
            Interaction si = this->scene.rayIntersect2(cameraRay);

            if (si.didIntersect)
                this->outputImage.writePixelColor(0.5f * (si.n + Vector3f(1.f, 1.f, 1.f)), x, y);
            else
                this->outputImage.writePixelColor(Vector3f(0.f, 0.f, 0.f), x, y);
        }
    }
    auto finishTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
}

long long Integrator::render3()
{
    BVHNode* bvhRoot = scene.buildBVHRecursive(this->scene.surfaces, 0, this->scene.surfaces.size());
    auto startTime = std::chrono::high_resolution_clock::now();
    //auto startTime2 = std::chrono::high_resolution_clock::now();
    for (int x = 0; x < this->scene.imageResolution.x; x++) {
        for (int y = 0; y < this->scene.imageResolution.y; y++) {
            Ray cameraRay = this->scene.camera.generateRay(x, y);
            Interaction si = this->scene.rayIntersectBVH(cameraRay,bvhRoot);


            if (si.didIntersect)
                this->outputImage.writePixelColor(0.5f * (si.n + Vector3f(1.f, 1.f, 1.f)), x, y);
            else
                this->outputImage.writePixelColor(Vector3f(0.f, 0.f, 0.f), x, y);
        }
    }
    auto finishTime = std::chrono::high_resolution_clock::now();
    // auto renderTime=std::chrono::duration_cast<std::chrono::microseconds>(startTime2 - startTime).count();
    // std::cout << "Render Time: " << std::to_string(renderTime / 1000.f) << " ms" << std::endl;
    return std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
}

long long Integrator::render4()
{
    BVHNode* bvhRoot = scene.buildBVHRecursive(this->scene.surfaces, 0, this->scene.surfaces.size());
    auto startTime = std::chrono::high_resolution_clock::now();
    //auto startTime2 = std::chrono::high_resolution_clock::now();
    for (int x = 0; x < this->scene.imageResolution.x; x++) {
        for (int y = 0; y < this->scene.imageResolution.y; y++) {
            Ray cameraRay = this->scene.camera.generateRay(x, y);
            Interaction si = this->scene.rayIntersectBVHLeaf(cameraRay,bvhRoot);


            if (si.didIntersect)
                this->outputImage.writePixelColor(0.5f * (si.n + Vector3f(1.f, 1.f, 1.f)), x, y);
            else
                this->outputImage.writePixelColor(Vector3f(0.f, 0.f, 0.f), x, y);
        }
    }
    auto finishTime = std::chrono::high_resolution_clock::now();
    // auto renderTime=std::chrono::duration_cast<std::chrono::microseconds>(startTime2 - startTime).count();
    // std::cout << "Render Time: " << std::to_string(renderTime / 1000.f) << " ms" << std::endl;
    return std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        std::cerr << "Usage: ./render <scene_config> <out_path> <intersection_variant>";
        return 1;
    }
    Scene scene(argv[1]);
    int arg3 = std::stoi(argv[3]); 
    Integrator rayTracer(scene);
    auto renderTime=0;
    if(arg3==0)
        renderTime = rayTracer.render();
    if(arg3==1)
        renderTime = rayTracer.render2();
    if(arg3==2)
        renderTime = rayTracer.render3();
    if(arg3==3)
        renderTime = rayTracer.render4();

    std::cout << "Render Time: " << std::to_string(renderTime / 1000.f) << " ms" << std::endl;
    rayTracer.outputImage.save(argv[2]);

    return 0;
}
