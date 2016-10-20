#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
#if 1
	ofGLFWWindowSettings settings;
	settings.multiMonitorFullScreen = true;
	//ofGLWindowSettings settings;
	settings.setGLVersion(4, 3);
	settings.windowMode = OF_WINDOW;
	shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);
	shared_ptr<ofApp> mainApp(new ofApp);

	ofRunApp(mainWindow, mainApp);
	ofRunMainLoop();
#else
	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context
	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());
#endif
}
