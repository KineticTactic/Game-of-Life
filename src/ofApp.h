#pragma once

#include "ofMain.h"
#include "ofxImGui.h"

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

public:
	void setup();
	void update();
	void draw();

	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseScrolled(int x, int y, float scrollX, float scrollY);

private:
	int SIM_WIDTH, SIM_HEIGHT;

	ofFbo renderFBO;

	ofImage startFrame;

	ofTexture frame;
	ofTexture bufferFrame;

	ofShader compute;

	ofPlanePrimitive plane;

	ofCamera cam;
	
	ofVec2f prevMousePos;

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
};
