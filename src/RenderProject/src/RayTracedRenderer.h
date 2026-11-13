#pragma once

#include "Object3DRenderer.h"
#include "RayTriangleIntersection.h"


class RayTracedRenderer: public Object3DRenderer {
    public:
        void draw(DrawingWindow &window) override;
    private:
        RayTriangleIntersection getClosestIntersection(glm::vec3 startingPoint, glm::vec3 rayDirection);
};
