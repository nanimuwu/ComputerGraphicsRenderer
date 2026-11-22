#include "RayTracedRenderer.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

const float pi = 3.1415926535897932384626;

enum rendererType currentRendererType = RAYTRACED;

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
            (possibleSolution[0] > 0.001f)){
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

Colour updateColourIntensity(Colour initialColour, float distanceToLight, glm::vec3 cameraDirection, glm::vec3 lightDirection, glm::vec3 normalVector) {
    float distMult = 1.0f/(pi*distanceToLight*distanceToLight);
    float aoi = std::max(0.0f, glm::dot(lightDirection, normalVector));
    float specularSpread = 0.0f;

    if (aoi > 0.0f) {
        glm::vec3 outgoingLight = lightDirection - 2.0f*normalVector*(glm::dot(lightDirection, normalVector));
        float specularPower = 64;
        specularSpread = std::pow(std::max(glm::dot(cameraDirection, outgoingLight), 0.0f), specularPower);
    }

    float lightScalar = 5.0f;


    float mult = std::min(1.0f, lightScalar*distMult*aoi);
    float newRed = std::min(255.0f, mult*initialColour.red+(255*specularSpread*mult));
    float newGreen = std::min(255.0f, mult*initialColour.green+(255*specularSpread*mult));
    float newBlue = std::min(255.0f, mult*initialColour.blue+(255*specularSpread*mult));
    return Colour(newRed, newGreen, newBlue);
}

void RayTracedRenderer::setRenderType(rendererType renderType) {
    currentRendererType = renderType;
}

void RayTracedRenderer::draw(DrawingWindow &window) {
    window.clearPixels();

    switch (currentRendererType) {
        case (RAYTRACED):
            for (int y = 0; y < window.height; y++) {
                for (int x = 0; x < window.width; x++) {
                    float u = (x - window.width  / 2.0f) / imagePlaneScaling;
                    float v = (window.height / 2.0f - y) / imagePlaneScaling;

                    glm::vec3 camDir(u, v, -focalLength);
                    glm::vec3 rayDirection = glm::normalize(cameraOrientation * camDir);

                    RayTriangleIntersection targetTriangle = getClosestIntersection(cameraPosition, rayDirection);
                    
                    //if hit triangle
                    if (targetTriangle.triangleIndex != -1) {
                        glm::vec3 pointToLight = lightPosition - targetTriangle.intersectionPoint;
                        RayTriangleIntersection lightIntersection = getClosestIntersection(targetTriangle.intersectionPoint, glm::normalize(pointToLight));
                       
                        Colour c = updateColourIntensity(targetTriangle.intersectedTriangle.colour, glm::length(pointToLight), glm::normalize(rayDirection), glm::normalize(pointToLight), targetTriangle.intersectedTriangle.normal);
                        Colour minColour = Colour(
                                0.1 * targetTriangle.intersectedTriangle.colour.red,
                                0.1 * targetTriangle.intersectedTriangle.colour.green,
                                0.1 * targetTriangle.intersectedTriangle.colour.blue);


                        //if in shadow
                        if ((lightIntersection.triangleIndex != -1) && (lightIntersection.distanceFromCamera < glm::length(pointToLight))) {
                            c = minColour;
                        }
                       
                        //if too far away from the light source 
                        if (glm::length(glm::vec3(c.red, c.green, c.blue)) < glm::length(glm::vec3(minColour.red, minColour.green, minColour.blue))) {
                            c = minColour;
                        }
                        

                        uint32_t colour = (255 << 24) + (c.red << 16) + (c.green << 8) + c.blue;
                        window.setPixelColour(x, y, colour);
                    }
                }
            }
            break;
        case (WIREFRAME):
            drawRasterisedScene(window, currentRendererType);
            break;
        case (RASTERISED):
            drawRasterisedScene(window, currentRendererType);
            break;
    }
}

