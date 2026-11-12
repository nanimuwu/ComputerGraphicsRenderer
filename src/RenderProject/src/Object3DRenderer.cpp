#include "Object3DRenderer.h"
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include <Colour.h>
#include <CanvasTriangle.h>


Colour getColour(std::ifstream &mtlFile, std::string targetColour) {

	float r, g, b;
	bool found = false;
	std::string line;
	std::vector<std::string> splitOutput;

	while (!found && !mtlFile.eof()) {
		getline(mtlFile, line);
		if (line.size()>0){
			splitOutput = split(line, ' ');
			if (splitOutput[1] == targetColour){
				getline(mtlFile, line);
				splitOutput = split(line, ' ');
				r = std::stof(splitOutput[1]) * 255;
				g = std::stof(splitOutput[2]) * 255;
				b = std::stof(splitOutput[3]) * 255;
				found = true;
			}
		}
	}
	
	if (!found) {r = 0; g = 0; b = 0;};
	mtlFile.clear();
	mtlFile.seekg(0);

	return Colour(targetColour,r,g,b);
}

void Object3DRenderer::readOBJfile(std::string objPath, float scale) {
	triangles.clear();
	std::ifstream obj(objPath);
	std::ifstream mtl;
	std::string lineRead;

	float x, y, z;
	int v0, v1, v2;

	std::vector<glm::vec3> vertices;
	Colour currentColour;

	while (getline(obj, lineRead)) {

		if (lineRead.size() > 0){

			std::vector<std::string> splitOutput = split(lineRead, ' ');

			if (splitOutput[0] == "mtllib") {
				int pos = objPath.find_last_of('/');
				std::string pathOrigin = objPath.substr(0, pos+1);
				mtl = std::ifstream(pathOrigin + splitOutput[1]);

			} else if (splitOutput[0] == "usemtl") {
				currentColour = getColour(mtl, splitOutput[1]);

			} else if (splitOutput[0] == "v") {
				x = std::stof(splitOutput[1]) * scale;
				y = std::stof(splitOutput[2]) * scale;
				z = std::stof(splitOutput[3]) * scale;
				vertices.push_back(glm::vec3(x,y,z));

			} else if (splitOutput[0] == "f") { // assumes no matching texture vertex
				//obj vertex index starts at 1
				v0 = std::stoi(split(splitOutput[1], '/')[0]) - 1;
				v1 = std::stoi(split(splitOutput[2], '/')[0]) - 1;
				v2 = std::stoi(split(splitOutput[3], '/')[0]) - 1;
				
				triangles.push_back(ModelTriangle(vertices[v0], vertices[v1], vertices[v2], currentColour));

            } // ignores object identifier 'o', not needed in my renderer
		}
	}

	obj.close();
	mtl.close();
}

CanvasPoint Object3DRenderer::projectVertexOntoCanvasPoint(glm::vec3 cameraPosition, float focalLength, glm::vec3 vertexPosition) {
    glm::vec3 diffVector = vertexPosition - cameraPosition;
    diffVector = diffVector * cameraOrientation;
    float u = (focalLength*(diffVector.x)/(-diffVector.z));
    float v = (focalLength*(diffVector.y)/(-diffVector.z));
    return CanvasPoint(u,v,1.0/(-diffVector.z));
}

void Object3DRenderer::translateCamera(direction d) {
    glm::mat3 moveMatrix;
    switch (d) {
        case LEFT:
            cameraPosition.x -= 0.1;
            break;
        case RIGHT:
            cameraPosition.x += 0.1;
            break;
        case UP:
            cameraPosition.y -= 0.1;
            break;
        case DOWN:
            cameraPosition.y += 0.1;
            break;
        case FORWARDS:
            cameraPosition.z -= 0.1;
            break;
        case BACKWARDS:
            cameraPosition.z += 0.1;
            break;
        case RXP:
            moveMatrix = glm::mat3({1,0,0}, {0, glm::cos(glm::radians(2.5f)), glm::sin(glm::radians(2.5f))}, {0, -glm::sin(glm::radians(2.5f)), glm::cos(glm::radians(2.5f))});
            cameraPosition = moveMatrix*cameraPosition;
            break;
        case RYP:
            moveMatrix = glm::mat3({glm::cos(glm::radians(2.5f)),0,-glm::sin(glm::radians(2.5f))}, {0, 1, 0}, {glm::sin(glm::radians(2.5f)), 0, glm::cos(glm::radians(2.5f))});
            cameraPosition = moveMatrix*cameraPosition;
            break;
        case RXN:
            moveMatrix = glm::mat3({1,0,0}, {0, glm::cos(glm::radians(-2.5f)), glm::sin(glm::radians(-2.5f))}, {0, -glm::sin(glm::radians(-2.5f)), glm::cos(glm::radians(-2.5f))});
            cameraPosition = moveMatrix*cameraPosition;
            break;
        case RYN:
            moveMatrix = glm::mat3({glm::cos(glm::radians(-2.5f)),0,-glm::sin(glm::radians(-2.5f))}, {0, 1, 0}, {glm::sin(glm::radians(-2.5f)), 0, glm::cos(glm::radians(-2.5f))});
            cameraPosition = moveMatrix*cameraPosition;
            break;
        default:
            break;
    }
}

void Object3DRenderer::rotateCamera(direction d) {
    glm::mat3 moveMatrix;
    if (!lookAt) {
        switch (d) {
            case PLEFT:
                moveMatrix = glm::mat3({1,0,0}, {0, glm::cos(glm::radians(2.5f)), glm::sin(glm::radians(2.5f))}, {0, -glm::sin(glm::radians(2.5f)), glm::cos(glm::radians(2.5f))});
                cameraOrientation = moveMatrix*cameraOrientation;
                break;
            case PUP:
                moveMatrix = glm::mat3({glm::cos(glm::radians(2.5f)),0,-glm::sin(glm::radians(2.5f))}, {0, 1, 0}, {glm::sin(glm::radians(2.5f)), 0, glm::cos(glm::radians(2.5f))});
                cameraOrientation = moveMatrix*cameraOrientation;
                break;
            case PRIGHT:
                moveMatrix = glm::mat3({1,0,0}, {0, glm::cos(glm::radians(-2.5f)), glm::sin(glm::radians(-2.5f))}, {0, -glm::sin(glm::radians(-2.5f)), glm::cos(glm::radians(-2.5f))});
                cameraOrientation = moveMatrix*cameraOrientation;
                break;
            case PDOWN:
                moveMatrix = glm::mat3({glm::cos(glm::radians(-2.5f)),0,-glm::sin(glm::radians(-2.5f))}, {0, 1, 0}, {glm::sin(glm::radians(-2.5f)), 0, glm::cos(glm::radians(-2.5f))});
                cameraOrientation = moveMatrix*cameraOrientation;
                break;
            default:
                break;
        }
        cameraOrientation[0] = glm::normalize(cameraOrientation[0]);
        cameraOrientation[1] = glm::normalize(cameraOrientation[1]);
        cameraOrientation[2] = glm::normalize(cameraOrientation[2]);
    }    
}

void Object3DRenderer::setLookAt(bool value, glm::vec3 targetCentre) {
    lookAt = value;
    lookAtCentre = targetCentre;
}

bool Object3DRenderer::getLookAt() {
    return lookAt;
}

void Object3DRenderer::setOrientationCentre() {
    //rotate about y axis
    glm::mat3 moveMatrix = glm::mat3({glm::cos(glm::radians(2.5f)),0,-glm::sin(glm::radians(2.5f))}, {0, 1, 0}, {glm::sin(glm::radians(2.5f)), 0, glm::cos(glm::radians(2.5f))});
    cameraPosition = moveMatrix*cameraPosition;

    //gave up trying to make it work for rotating in all axes because its just 
    //too much work for no marks and like i could get it to work but like nah
    glm::vec3 forward = glm::normalize(cameraPosition - lookAtCentre);
    glm::vec3 right = glm::normalize(glm::cross(vertical, forward));
    glm::vec3 up = glm::normalize(glm::cross(forward, right));
    cameraOrientation = glm::mat3(right, up , forward);
}

void Object3DRenderer::draw(DrawingWindow &window) {
    window.clearPixels();
    int xCameraOffset = round(window.width/2);
    int yCameraOffset = round(window.height/2);
    std::vector<std::vector<float>> zDepths (window.width, std::vector<float>(window.height,0));

    if (triangles.size() > 0) {
        if (lookAt) setOrientationCentre();
        for (int i = 0; i < triangles.size();i++) {
            CanvasTriangle targetTriangle;
            for (int j = 0; j <= 2; j++) {
                CanvasPoint projectedVertex = projectVertexOntoCanvasPoint(cameraPosition, focalLength, triangles[i].vertices[j] * cameraOrientation);
                glm::vec3 cameraVertex(projectedVertex.x*imagePlaneScaling+xCameraOffset, window.height-(projectedVertex.y*imagePlaneScaling+yCameraOffset), projectedVertex.depth);
                glm::vec3 targetVertex(cameraVertex);
                targetTriangle[j] = CanvasPoint(targetVertex.x, targetVertex.y, targetVertex.z);
            }
            drawFilledTriangle(window, targetTriangle, triangles[i].colour, false, zDepths);
            // drawStrokedTriangle(window, targetTriangle, triangles[i].colour);
        }
    }
}
