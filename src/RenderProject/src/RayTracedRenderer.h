#pragma once

#include "Object3DRenderer.h"
#include "RayTriangleIntersection.h"


class RayTracedRenderer: public Object3DRenderer {
    public:
        void draw(DrawingWindow &window) override;
        RayTriangleIntersection getClosestIntersection(glm::vec3 rayDirection);
};
