#pragma once

#include <vector>

#include "ofMain.h"
#include "ofxImGui.h"

struct vec2i {
	int x;
	int y;
};


class ofApp : public ofBaseApp {
private:
	void initRenderFBO();
	void initComputeShader();
	void initTextures();
	void initCamera();
	void initGui();
	void initImGuiStyle();

	void renderGui();
	void beginImGuiDockSpace();

	void addCellAt(int x, int y);

public:
	void setup();
	void update();
	void draw();

	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseScrolled(int x, int y, float scrollX, float scrollY);

private:
	int SIM_SIZE;
	float MAP_SIZE;

	ofFbo renderFBO;
	ofImage startFrame;
	ofTexture frame;
	ofTexture bufferFrame;

	ofShader compute;

	ofPlanePrimitive plane;

	ofCamera cam;
	
	ofVec2f prevMousePos;
	bool isBeingDrawn = false;

	std::vector<vec2i> addBuffer;

	// GUI
	ofxImGui::Gui gui;
	ImFont* roboto;
	bool isRenderWindowHovered = false;

	// Assets
	ImTextureID playIcon;
	ImTextureID pauseIcon;

	// Control variables
	bool simulationRunning = false;
	int simulationSpeed = 100;
	int brushSize = 1;
	bool roundBrush = false;
};
