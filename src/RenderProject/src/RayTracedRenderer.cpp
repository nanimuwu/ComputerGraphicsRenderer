#include "RayTracedRenderer.h"
#include <glm/glm.hpp>

RayTriangleIntersection RayTracedRenderer::getClosestIntersection(glm::vec3 startingPoint, glm::vec3 rayDirection){
    
    RayTriangleIntersection output; output.distanceFromCamera = std::numeric_limits<float>::infinity();
    bool found = false;

    for (int i = 0; i<triangles.size(); i++) {
        glm::vec3 e0 = triangles[i].vertices[1] - triangles[i].vertices[0];
        glm::vec3 e1 = triangles[i].vertices[2] - triangles[i].vertices[0];
        glm::vec3 SPVector = startingPoint - triangles[i].vertices[0];
        glm::mat3 DEMatrix(-rayDirection, e0, e1);
        glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
        
        if (((possibleSolution[1] >= 0.0) && (possibleSolution[1]  <= 1.0)) &&
            ((possibleSolution[2]  >= 0.0) && (possibleSolution[2] <= 1.0)) &&
            ((possibleSolution[1]  + possibleSolution[2]) <= 1.0) && 
            (possibleSolution[0] < output.distanceFromCamera) &&
            (possibleSolution[0] > 0.0001f)){
                output = RayTriangleIntersection(
                            (startingPoint + possibleSolution[0]*rayDirection), 
                            possibleSolution[0],
                            triangles[i],
                            i);

                            found = true;
        }
    }

    if (!found) {
        return RayTriangleIntersection(glm::vec3(0), -1, ModelTriangle(glm::vec3(0), glm::vec3(0), glm::vec3(0), Colour(0,0,0)), -1);
    } else {
        return output;
    }
}

void RayTracedRenderer::draw(DrawingWindow &window) {
    for (int y = 0; y < window.height; y++) {
        for (int x = 0; x < window.width; x++) {
            float u = ((x + 0.5f) - window.width  / 2.0f) / imagePlaneScaling;
            float v = (window.height / 2.0f - (y + 0.5f)) / imagePlaneScaling;

            glm::vec3 camDir(u, v, -focalLength);
            glm::vec3 rayDirection = glm::normalize(cameraOrientation * camDir);

            RayTriangleIntersection targetTriangle = getClosestIntersection(cameraPosition, rayDirection);
            if (targetTriangle.triangleIndex != -1) {
                glm::vec3 pointToLight = lightPosition - targetTriangle.intersectionPoint;
                RayTriangleIntersection lightIntersection = getClosestIntersection(targetTriangle.intersectionPoint, glm::normalize(pointToLight));
                Colour c = targetTriangle.intersectedTriangle.colour;
                if ((lightIntersection.triangleIndex != -1) && (lightIntersection.distanceFromCamera < pointToLight.length())) {
                    c = Colour(0,0,0);
                }

                uint32_t colour = (255 << 24) + (c.red << 16) + (c.green << 8) + c.blue;
                window.setPixelColour(x, y, colour);
            }
        }
    }
}

