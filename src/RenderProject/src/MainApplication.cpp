#include "Renderer.h"
#include "Object3DRenderer.h"
#include "RayTracedRenderer.h"
#include <fstream>
#include <vector>
#include <string>

// Define globals for WIDTH and HEIGHT of the window (can be accessed from any renderer)
extern const int WIDTH = 320;
extern const int HEIGHT = 240;

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT);
RayTracedRenderer rtRenderer = RayTracedRenderer();

//change depending on renderer
RayTracedRenderer* currentRenderer = &rtRenderer;
bool savingFrames = false;
int frameCounter = 0;


//Below moved in to renderer code

// Core 3D rendering data structures (for when we eventually get around to working in 3D ;o)
// std::vector<ModelTriangle> triangles;
// std::vector<Colour> palette; //unused 
// glm::vec3 cameraPosition(0.0, 0.0, 4.0);
// glm::mat3 cameraOrientation;
// glm::vec3 lightPosition(0.0, 0.8, 0.0);

enum triangleType {STROKED, FILLED, TEXTURE};

void loadObj(RayTracedRenderer* currentRenderer, std::string objPath, float scale) {
	currentRenderer->readOBJfile(objPath, scale);
}

void transformCamera(RayTracedRenderer* currentRenderer, direction d){
	if (!currentRenderer->getLookAt()){
		if (d < 10) {
			currentRenderer->translateCamera(d);
		} else {
			currentRenderer->rotateCamera(d);
		}
	}
}

void toggleLookAt() {
	currentRenderer->setLookAt(!(currentRenderer->getLookAt()), glm::vec3(0.0, 0.0, 0.0));
}

void changeRenderer(rendererType renderType) {
	currentRenderer->setRenderType(renderType);
}

void changeGouraud() {
	currentRenderer->toggleGouraud();
}

void changePhone() {
	currentRenderer->togglePhong();
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) transformCamera(currentRenderer, LEFT);
		else if (event.key.keysym.sym == SDLK_RIGHT) transformCamera(currentRenderer, RIGHT);
		else if (event.key.keysym.sym == SDLK_EQUALS) transformCamera(currentRenderer, FORWARDS);
		else if (event.key.keysym.sym == SDLK_MINUS) transformCamera(currentRenderer, BACKWARDS);
		else if (event.key.keysym.sym == SDLK_DOWN) transformCamera(currentRenderer, UP); //reversed
		else if (event.key.keysym.sym == SDLK_UP) transformCamera(currentRenderer, DOWN); //reversed
		else if (event.key.keysym.sym == SDLK_w) transformCamera(currentRenderer, PLEFT);
		else if (event.key.keysym.sym == SDLK_s) transformCamera(currentRenderer, PRIGHT);
		else if (event.key.keysym.sym == SDLK_a) transformCamera(currentRenderer, PUP);
		else if (event.key.keysym.sym == SDLK_d) transformCamera(currentRenderer, PDOWN);
		else if (event.key.keysym.sym == SDLK_PERIOD) transformCamera(currentRenderer, RYP);
		else if (event.key.keysym.sym == SDLK_COMMA) transformCamera(currentRenderer, RYN);
		else if (event.key.keysym.sym == SDLK_1) changeRenderer(WIREFRAME);
		else if (event.key.keysym.sym == SDLK_2) changeRenderer(RASTERISED);
		else if (event.key.keysym.sym == SDLK_3) changeRenderer(RAYTRACED);		

		else if (event.key.keysym.sym == SDLK_l) toggleLookAt();
		else if (event.key.keysym.sym == SDLK_TAB) savingFrames = !savingFrames;
		else if (event.key.keysym.sym == SDLK_RETURN) window.toggleFullscreen();
	}
}

int main(int argc, char *argv[]) {
	SDL_Event event;
	changeRenderer(RASTERISED);
	loadObj(currentRenderer, "./src/models/cornell-box/cornell-box.obj",0.35);
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		// In C++ we can't use "." notation for dynamic dispatch, so we have to use -> instead :o(
		currentRenderer->draw(window);
		// Need to ask the SDL window to render the frame, otherwise nothing actually gets shown on the screen !
		window.renderFrame();
		if(savingFrames) {
			std::string paddedCounter = std::to_string(frameCounter);
			while(paddedCounter.length() < 6) paddedCounter = "0" + paddedCounter;
			// Use whichever image file format works best on your platform (comment out the one you don't need)
			window.savePPM("saved-frames", paddedCounter + ".ppm");
			window.saveBMP("saved-frames", paddedCounter + ".bmp");
			frameCounter++;
		}
	}
}
