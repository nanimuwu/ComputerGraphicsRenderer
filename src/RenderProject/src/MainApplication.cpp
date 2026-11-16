#include "Renderer.h"
// #include "RedNoiseRenderer.h"
// #include "BlueNoiseRenderer.h"
// #include "ColourSpectrumRenderer.h"
// #include "RasterisedRenderer.h"
#include "Object3DRenderer.h"
#include "RayTracedRenderer.h"
#include <fstream>
#include <vector>
#include <string>

// Define globals for WIDTH and HEIGHT of the window (can be accessed from any renderer)
extern const int WIDTH = 320;
extern const int HEIGHT = 240;

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT);
// RedNoiseRenderer redNoise = RedNoiseRenderer();
// Renderer* currentRenderer = &redNoise;
// BlueNoiseRenderer blueNoise = BlueNoiseRenderer();
// ColourSpectrumRenderer spectrumRenderer = ColourSpectrumRenderer();
// RasterisedRenderer rasterisedRenderer = RasterisedRenderer();
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

// void drawRandomTriangle(RasterisedRenderer* currentRenderer, triangleType type){
// 	CanvasPoint v0(rand()%window.width, rand()%window.height);
// 	CanvasPoint v1(rand()%window.width, rand()%window.height);
// 	CanvasPoint v2(rand()%window.width, rand()%window.height);
// 	CanvasTriangle t(v0,v1,v2);
// 	Colour c(rand()%256, rand()%256, rand()%256);
// 	if (type == STROKED) {
// 		currentRenderer->drawStrokedTriangle(window,t,c);
// 	} else if (type == FILLED) {
// 		currentRenderer->drawFilledTriangle(window,t,c, true);
// 	} else if (type == TEXTURE) {
// 		v0 = CanvasPoint(160, 10); v0.texturePoint = TexturePoint(195,5);
// 		v1 = CanvasPoint(300, 230); v1.texturePoint = TexturePoint(395,380);
// 		v2 = CanvasPoint(10, 150); v2.texturePoint = TexturePoint(65,330);
// 		t = CanvasTriangle(v0, v1, v2);
// 		//unclear if still working
// 		TextureMap texture ("./src/models/texture.ppm");
// 		currentRenderer->drawTextureTriangle(window, t, texture);
// 	}
	
// }

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

void toggleLookAt(RayTracedRenderer* currentRenderer) {
	currentRenderer->setLookAt(!(currentRenderer->getLookAt()), glm::vec3(0.0, 0.0, 0.0));
}

void changeRenderer(rendererType renderType) {
	currentRenderer->setRenderType(renderType);
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) transformCamera(currentRenderer, LEFT);
		else if (event.key.keysym.sym == SDLK_RIGHT) transformCamera(currentRenderer, RIGHT);
		else if (event.key.keysym.sym == SDLK_UP) transformCamera(currentRenderer, FORWARDS);
		else if (event.key.keysym.sym == SDLK_DOWN) transformCamera(currentRenderer, BACKWARDS);
		else if (event.key.keysym.sym == SDLK_w) transformCamera(currentRenderer, UP);
		else if (event.key.keysym.sym == SDLK_s) transformCamera(currentRenderer, DOWN);
		else if (event.key.keysym.sym == SDLK_q) transformCamera(currentRenderer, PLEFT);
		else if (event.key.keysym.sym == SDLK_e) transformCamera(currentRenderer, PRIGHT);
		else if (event.key.keysym.sym == SDLK_2) transformCamera(currentRenderer, PUP);
		else if (event.key.keysym.sym == SDLK_x) transformCamera(currentRenderer, PDOWN);
		else if (event.key.keysym.sym == SDLK_3) transformCamera(currentRenderer, RXP);
		else if (event.key.keysym.sym == SDLK_4) transformCamera(currentRenderer, RYP);
		else if (event.key.keysym.sym == SDLK_5) transformCamera(currentRenderer, RXN);
		else if (event.key.keysym.sym == SDLK_6) transformCamera(currentRenderer, RYN);
		// else if (event.key.keysym.sym == SDLK_u) drawRandomTriangle(&rasterisedRenderer, STROKED);
		// else if (event.key.keysym.sym == SDLK_f) drawRandomTriangle(&rasterisedRenderer, FILLED);
		// else if (event.key.keysym.sym == SDLK_t) drawRandomTriangle(&rasterisedRenderer, TEXTURE);
		//TODO: change the keybinds to specification 
		else if (event.key.keysym.sym == SDLK_8) changeRenderer(WIREFRAME);
		else if (event.key.keysym.sym == SDLK_9) changeRenderer(RASTERISED);
		else if (event.key.keysym.sym == SDLK_0) changeRenderer(RAYTRACED);		
		else if (event.key.keysym.sym == SDLK_o) loadObj(currentRenderer, "./src/models/cornell-box.obj",0.35);
		else if (event.key.keysym.sym == SDLK_l) toggleLookAt(currentRenderer);
		else if (event.key.keysym.sym == SDLK_TAB) savingFrames = !savingFrames;
		else if (event.key.keysym.sym == SDLK_RETURN) window.toggleFullscreen();
	}
}

int main(int argc, char *argv[]) {
	SDL_Event event;
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
