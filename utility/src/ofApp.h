#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "Utilities.h"

#include "ofxARToolkitPlus.h"

//#define SHIPPING

struct Positioning
{
	enum PointIndex
	{
		LeftTop = 0,
		RightTop,
		RightBottom,
		LeftBottom,
		MaxPointIndex
	};

	struct PointInfo
	{
		int id_marker;
		ofPoint position;
	};
	
	std::map<int, ofPoint> point_infos;
	
	bool setPoint(unsigned int idx, int id, ofPoint pos)
	{
		if (idx >= MaxPointIndex) 
			return false;
		point_infos[id] = pos;
		return true;
	}

	std::vector<ofPoint> getNormalizedPoints(int width, int height)
	{
		std::vector<ofPoint> pts;
		for (auto& info : point_infos)
			pts.push_back(info.second);
		for (auto& p : pts)
			p /= ofPoint(width, height, 1.0f);
		return pts;
	}

	bool isSameType(std::map<int, ofPoint>& ref_infos)
	{
		for (auto& i : ref_infos)
		{
			if (point_infos.find(i.first) == point_infos.end())
			{
				return false;
			}
		}
		return true;
	}
};

using OriginDataRef = std::shared_ptr<struct OriginData>;

struct OriginData
{
	string name;
	ofFbo source;
	ofFbo mask;
	Positioning pos;
};

class ofApp : public ofBaseApp{

public:
	void setup();
	void update();
	void draw();
	void exit();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	void reset();
	void updateParameters();

	void apply(const ofTexture& tex);
	
	void drawLuminanceFbo(const ofTexture& tex, ofFbo& fbo);
	bool createPositioning(Positioning& pos, ofPixels& pix);
	bool createOriginData(OriginData& data, const string& source_filename, const string& mask_filename);
	

private:
	enum {
		WIDTH = 1280,
		HEIGHT = 720,
		FBO_WIDTH = 2048,
		FBO_HEIGHT = 2048,
		WORK_GROUP_SIZE = 1 << 10,
		AR_WIDTH = 2048,
		AR_HEIGHT = 2048
	};
	
	// gl
	ofFbo final_fbo;
	


	// utilities
	PixelSaverRef pixel_saver;

	bool is_debug_visible = true;
	const string gui_filename = "settings/gui_setting.xml";
	ofxGuiGroup gui;
	
	// settings
	ofParameter<float> time_step;
	ofParameter<float> elapsed_time;
	ofParameter<float> time_value;
	ofParameter<int> g_threshold;

	ofFbo input_fbo;
	ofFbo output_fbo;

	ofFbo::Settings ar_fbo_setting;
	ofFbo ar_fbo;
	ofxARToolkitPlus artk;
	ofVideoGrabber vidGrabber;
	ofShader luminance_shader;
	ofShader homo_shader;

	OriginDataRef matched_data = nullptr;
	std::vector<OriginDataRef> origin_data;

	

};



