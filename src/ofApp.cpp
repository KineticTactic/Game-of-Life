#include "ofApp.h"
#include "ofxImGui.h"
#include "imgui.h"
#include "imgui_internal.h"

ImTextureID loadImageForImGui(const char* filename) {
	ofImage img;
	img.setUseTexture(false);
	img.load(filename);

	ofPixels& pixels = img.getPixels();

	ofTextureData texData;
	texData.width = img.getWidth();
	texData.height = img.getHeight();
	texData.textureTarget = GL_TEXTURE_2D;
	texData.glInternalFormat = GL_RGBA;

	ofTexture* tex = new ofTexture();
	tex->allocate(texData);
	tex->loadData(pixels);

	return (ImTextureID)tex->getTextureData().textureID;
}

ofVec2f screenSpaceToWorldSpace(ofVec2f pos, ofCamera& cam) {
	float worldX = ((float)pos.x - ofGetWidth() / 2 + cam.getPosition().x / cam.getScale().x) * cam.getScale().x;
	float worldY = ((float)pos.y - ofGetHeight() / 2 + cam.getPosition().y / cam.getScale().y) * cam.getScale().y;
	return { worldX, worldY };
}


//--------------------------------------------------------------
void ofApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetFrameRate(60);

	SIM_SIZE = 128;
	MAP_SIZE = 1600.f;

	initComputeShader();
	initRenderFBO();
	initTextures();
	initCamera();
	initGui();
	initImGuiStyle();

	playIcon = loadImageForImGui("images/play.png");
	pauseIcon = loadImageForImGui("images/pause.png");

	plane.set(100, 100);
}

//--------------------------------------------------------------
void ofApp::update() {
	if (isBeingDrawn) {
		int x = ofGetMouseX();
		int y = ofGetMouseY();

		ofVec2f worldSpaceCoords = screenSpaceToWorldSpace({ (float)x, (float)y }, cam);
		ofVec2f mapCoords = worldSpaceCoords / 1600.f * SIM_SIZE;

		if (brushSize == 1) {
			addBuffer.push_back(vec2i{ (int)floor(mapCoords.x) , (int)floor(mapCoords.y) });
		}
		else {
			for (int offX = -brushSize / 2; offX < brushSize / 2; offX++) {
				for (int offY = -brushSize / 2; offY < brushSize / 2; offY++) {
					if (roundBrush) {
						float distSq = offX * offX + offY * offY;
						if (distSq > brushSize * brushSize / 4)
							continue;
					}
					addBuffer.push_back(vec2i{ (int) mapCoords.x + offX , (int)mapCoords.y + offY });
				}
			}
		}
	}

	bool updated = false;
	ofPixels pix;

	if (simulationRunning && (ofGetFrameNum() % (101 - simulationSpeed) == 0)) {
		frame.bindAsImage(0, GL_READ_ONLY);
		bufferFrame.bindAsImage(1, GL_WRITE_ONLY);

		compute.begin();
		compute.setUniform2i("simSize", SIM_SIZE, SIM_SIZE);
		compute.dispatchCompute(SIM_SIZE / 32, SIM_SIZE / 32, 1);
		compute.end();


		bufferFrame.readToPixels(pix);	
		//frame.loadData(pix);
		updated = true;
	}
	else if(addBuffer.size() > 0) {
		frame.readToPixels(pix);
		updated = true;
	}

	if (updated) {
		for (vec2i& a : addBuffer) {
			pix.setColor(a.x, a.y, { 255.f });
		}
		addBuffer.clear();
		frame.loadData(pix);
	}

}

//--------------------------------------------------------------
void ofApp::draw() {
	renderFBO.begin();
	renderFBO.clearColorBuffer(ofFloatColor(0.f, 0.f, 0.f));

	cam.begin();

	frame.draw(glm::vec3(0.f), 1600, 1600);

	cam.end();

	renderFBO.end();


	renderGui();
}

//--------------------------------------------------------------
void ofApp::renderGui() {
	gui.begin();

	beginImGuiDockSpace();

	ImGui::Begin("Controls");
	ImGui::Text("Hello dear ImGui!");

	if(simulationRunning) {
		if (ImGui::ImageButton(pauseIcon, {60.f, 60.f}))
			simulationRunning = false;
	} else {
		if (ImGui::ImageButton(playIcon, {60.f, 60.f}))
			simulationRunning = true;
	}

	ImGui::SliderInt("Speed", &simulationSpeed, 1, 100);
	ImGui::SliderInt("Brush Size", &brushSize, 1, 20);
	ImGui::Checkbox("Round Brush", &roundBrush);
	ImGui::End();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.f, 0.f });
	ImGui::Begin("Output");
	isRenderWindowHovered = ImGui::IsWindowHovered();
	ImGui::Image(GetImTextureID(renderFBO.getTexture().getTextureData().textureID), { (float)ofGetWidth(), (float)ofGetHeight() });
	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::ShowDemoWindow();

	gui.end();
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	if (isRenderWindowHovered && button == OF_MOUSE_BUTTON_MIDDLE) {
		cam.move(ofVec3f{ (prevMousePos.x - x) * cam.getScale().x, (prevMousePos.y - y) * cam.getScale().y, 0.f });
	}
	prevMousePos = { (float)x, (float)y };
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	prevMousePos = { (float)x, (float)y };

	if (isRenderWindowHovered && button == OF_MOUSE_BUTTON_LEFT) {
		isBeingDrawn = true;
	}
}

void ofApp::mouseReleased(int x, int y, int button) {
	isBeingDrawn = false;
}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY) {
	cam.setScale(cam.getScale() + scrollY * -0.1f);
}

void ofApp::beginImGuiDockSpace() {
	static bool dockspaceOpen = true;
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking; // ImGuiWindowFlags_MenuBar 
	if (opt_fullscreen) {
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->GetWorkPos());
		ImGui::SetNextWindowSize(viewport->GetWorkSize());
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	else {
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}

	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace", &dockspaceOpen, window_flags);
	if (!opt_padding)
		ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// DockSpace
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
	float minWinSizeX = style.WindowMinSize.x;
	style.WindowMinSize.x = 370.0f;
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	ImGui::End();
}

void ofApp::addCellAt(int x, int y)
{
}


