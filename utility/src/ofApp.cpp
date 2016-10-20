#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetWindowShape(WIDTH, HEIGHT);
	ofSetWindowPosition((ofGetScreenWidth() - ofGetWidth()) / 2, (ofGetScreenHeight() - ofGetHeight()) / 2);
	ofDisableArbTex();
	ofSetFrameRate(60);
	//ofSetVerticalSync(true);

#ifdef SHIPPING
	ofLogToFile(ofVAArgsToString("logs/%s_log.txt", ofGetTimestampString("%Y-%m-%d-%H-%M-%S").c_str()));
	ofHideCursor();
	ofToggleFullscreen();
	is_debug_visible = false;
#endif

	{
		ofLog(OF_LOG_NOTICE, "application start with resolution: %u x %u", ofGetWidth(), ofGetHeight());

		pixel_saver = PixelSaver::create();
	}

	// setup gui
	{
		gui.setup("gui");
		gui.setName("gui");

		ofParameterGroup g_settings;
		g_settings.setName("settings");
		g_settings.add(time_step.set("time_step", 1.0f / 120.0f, 0.0f, 1.0f / 30.0f));
		time_step.setSerializable(false);
		g_settings.add(elapsed_time.set("elapsed_time", ofGetElapsedTimef()));
		elapsed_time.setSerializable(false);
		g_settings.add(time_value.set("time_value", 0, 0, 1));
		time_value.setSerializable(false);
		g_settings.add(g_threshold.set("threshold", 128, 0, 255));
		gui.add(g_settings);

		gui.loadFromFile(gui_filename);
	}

	reset();

	{
		vidGrabber.initGrabber(AR_WIDTH, AR_HEIGHT);

		artk.setup(AR_WIDTH, AR_HEIGHT);
		artk.setThreshold(g_threshold);

		ar_fbo_setting.width = AR_WIDTH;
		ar_fbo_setting.height = AR_HEIGHT;
		ar_fbo_setting.useDepth = false;
		ar_fbo_setting.colorFormats = { GL_R16 };

		ar_fbo.allocate(ar_fbo_setting);

		input_fbo.allocate(AR_WIDTH, AR_HEIGHT, GL_RGBA8);
		output_fbo.allocate(AR_WIDTH, AR_HEIGHT, GL_RGBA8);
	}

	{
		vector<string> source_filenames;
		source_filenames.push_back("images/origin/c_sheet_baloon.jpg");
		source_filenames.push_back("images/origin/c_sheet_clay.jpg");
		source_filenames.push_back("images/origin/c_sheet_frog.jpg");
		source_filenames.push_back("images/origin/c_sheet_pig.jpg");

		vector<string> mask_filenames;
		mask_filenames.push_back("images/mask/c_sheet_baloon.png");
		mask_filenames.push_back("images/mask/c_sheet_clay.png");
		mask_filenames.push_back("images/mask/c_sheet_frog.png");
		mask_filenames.push_back("images/mask/c_sheet_pig.png");
		
		size_t max_num_data = MIN(source_filenames.size(), mask_filenames.size());
		for (size_t i = 0; i < max_num_data; i++)
		{
			OriginData data;
			if (createOriginData(data, source_filenames[i], mask_filenames[i]))
			{
				origin_data.push_back(std::make_shared<OriginData>(data));
			}
			
		}
	}

	// allocate fbo
    {
        ofFbo::Settings s;
        s.width = FBO_WIDTH;
		s.height = FBO_HEIGHT;
        s.useDepth = true;
		s.colorFormats = { GL_RGBA8 };
        
        final_fbo.allocate(s);
    }
    
	
	
	
}

//--------------------------------------------------------------
void ofApp::update(){
	ofSetWindowTitle("ofxARtoolkitPlusUtility: " + ofToString(ofGetFrameRate(), 1));

	// update params and ubo
	updateParameters();
	
	// update main fbo
	{
		final_fbo.begin();
		auto viewport = ofGetCurrentViewport();
		ofRectangle rect = viewport;
		rect.scale(0.5f);
		ofClear(0);

		rect.alignTo(viewport, ofAlignHorz::OF_ALIGN_HORZ_LEFT, ofAlignVert::OF_ALIGN_VERT_TOP);
		input_fbo.draw(rect);
		artk.draw(rect.x, rect.y, rect.width, rect.height);

		rect.alignTo(viewport, ofAlignHorz::OF_ALIGN_HORZ_RIGHT, ofAlignVert::OF_ALIGN_VERT_TOP);
		output_fbo.draw(rect);

		if (matched_data)
		{
			rect.alignTo(viewport, ofAlignHorz::OF_ALIGN_HORZ_LEFT, ofAlignVert::OF_ALIGN_VERT_BOTTOM);
			matched_data->source.draw(rect);

			rect.alignTo(viewport, ofAlignHorz::OF_ALIGN_HORZ_RIGHT, ofAlignVert::OF_ALIGN_VERT_BOTTOM);
			matched_data->mask.draw(rect);
		}
		


		final_fbo.end();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(0);
	auto viewport = ofGetCurrentViewport();
	
	{
		auto rect = getCenteredRect(final_fbo.getWidth(), final_fbo.getHeight(), viewport.width, viewport.height, false);
		auto& tex = final_fbo.getTexture();
		tex.bind();
		drawRectangle(rect);
		tex.unbind();
	}

	// draw debug things
	if (is_debug_visible)
	{
		gui.draw();
	}
}

//--------------------------------------------------------------
void ofApp::exit() {
	ofLog(OF_LOG_NOTICE, "application exit");
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){	
	switch (key)
	{
	case OF_KEY_F1:
		is_debug_visible = !is_debug_visible;
		break;
	case OF_KEY_F5:
		reset();
		break;
	case OF_KEY_F11:
		toggleFullscreen(WIDTH, HEIGHT);
		break;
	case 's':
		gui.saveToFile(gui_filename);
		break;
	case 'l':
		gui.loadFromFile(gui_filename);
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
	string filename = dragInfo.files[0];
	string ext = filename.substr(filename.find_last_of(".") + 1);
	if (ext == "jpg" || ext == "png")
	{
		ofTexture tex;
		if (ofLoadImage(tex, filename))
			apply(tex);		
	}
}

///////////////////////////////////////////////////////////////

void ofApp::reset()
{
	ofLog(OF_LOG_NOTICE, "%s reset", ofGetTimestampString("%H:%M:%S").c_str());
	
	luminance_shader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/pass.vert");
	luminance_shader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/luminance.frag");
	luminance_shader.linkProgram();

	homo_shader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/pass.vert");
	homo_shader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/homo_effect.frag");
	homo_shader.linkProgram();
}

void ofApp::updateParameters()
{
	float current_time = ofGetElapsedTimef();
	time_step = ofClamp(current_time - elapsed_time, time_step.getMin(), time_step.getMax());
	elapsed_time = current_time;
	int interval = 100000;
	float value = ofGetElapsedTimeMillis() % interval / float(interval);
	time_value = sin(value * TWO_PI) * 0.5f + 0.5f; // continuous sin value

}

void ofApp::apply(const ofTexture& tex)
{
	matched_data = nullptr;

	{
		input_fbo.begin();
		auto viewport = ofGetCurrentViewport();
		auto rect = getCenteredRect(tex.getWidth(), tex.getHeight(), viewport.width, viewport.height, false);
		ofClear(0);
		tex.draw(rect);
		input_fbo.end();
	}

	{
		ar_fbo.begin();
		auto viewport = ofGetCurrentViewport();
		auto rect = getCenteredRect(tex.getWidth(), tex.getHeight(), viewport.width, viewport.height, false);
		ofClear(0);
		tex.draw(rect);
		ar_fbo.end();
	}

	ofPixels pix;
	ar_fbo.readToPixels(pix);

	Positioning pos;
	if (createPositioning(pos, pix) == false)
	{
		return;
	}

	for (auto& data : origin_data)
	{
		if (data->pos.isSameType(pos.point_infos) == false)
			continue;
		matched_data = data;
	}

	if (matched_data)
	{
		vector<ofPoint> src = pos.getNormalizedPoints(AR_WIDTH, AR_HEIGHT);
		vector<ofPoint> dst = matched_data->pos.getNormalizedPoints(AR_WIDTH, AR_HEIGHT);
		ofMatrix4x4 homo_mat4 = findVectorHomographyForOpenGL(src.data(), dst.data());

		ofLogNotice("AR", "find data match %s", matched_data->name.c_str());

		output_fbo.begin();
		auto viewport = ofGetCurrentViewport();
		ofClear(0);
		homo_shader.begin();
		homo_shader.setUniformTexture("tex_input", input_fbo.getTexture(), 0);
		homo_shader.setUniformTexture("tex_mask", matched_data->mask.getTexture(), 1);
		homo_shader.setUniformMatrix4f("homo_mat4", homo_mat4);
		drawRectangle(viewport);
		homo_shader.end();
		output_fbo.end();

		pixel_saver->saveFbo(output_fbo);
	}
}

void ofApp::drawLuminanceFbo(const ofTexture& tex, ofFbo& fbo)
{
	fbo.begin();
	ofClear(0);
	auto rect = getCenteredRect(tex.getWidth(), tex.getHeight(), fbo.getWidth(), fbo.getHeight(), false);
	luminance_shader.begin();
	luminance_shader.setUniformTexture("tex", tex, 0);
	drawRectangle(rect);
	luminance_shader.end();
	fbo.end();
}

bool ofApp::createPositioning(Positioning& pos, ofPixels& pix)
{
	artk.update(pix.getData());

	int num = artk.getNumDetectedMarkers();
	if (num < Positioning::MaxPointIndex)
	{
		return false;
	}
	for (int i = 0; i < num; i++)
	{
		ofPoint p = artk.getDetectedMarkerCenter(i);
		int id = artk.getMarkerID(i);
		pos.setPoint(i, id, p);
	}
	return true;
}

bool ofApp::createOriginData(OriginData& data, const string& source_filename, const string& mask_filename)
{
	string name = source_filename.substr(source_filename.find_last_of('/') + 1);
	name = name.substr(0, name.find_last_of('.'));
	std::replace(name.begin(), name.end(), ' ', '+');
	data.name = name;

	ofTexture tex;
	if (ofLoadImage(tex, source_filename))
	{
		data.source.allocate(ar_fbo_setting);
		drawLuminanceFbo(tex, data.source);
	}
	else
	{
		return false;
	}
	if (ofLoadImage(tex, mask_filename))
	{
		data.mask.allocate(AR_WIDTH, AR_HEIGHT, GL_RGBA8);
		data.mask.begin();
		auto viewport = ofGetCurrentViewport();
		auto rect = getCenteredRect(tex.getWidth(), tex.getHeight(), viewport.getWidth(), viewport.getHeight(), false);
		ofClear(0);
		tex.bind();
		drawRectangle(rect);
		tex.unbind();
		data.mask.end();
	}
	else
	{
		return false;
	}

	
	ofPixels pix;
	data.source.readToPixels(pix);

	if (createPositioning(data.pos, pix) == false)
	{
		return false;
	}

	printf("[OriginData] %s\n", data.name.c_str());
	for (auto& info : data.pos.point_infos)
	{
		printf("id: %d, v(%f, %f, %f)\n", info.first, info.second.x, info.second.y, info.second.z);
	}

	return true;
}


