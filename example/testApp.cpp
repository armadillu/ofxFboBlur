#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){

	ofBackground(22);
	ofFbo::Settings s;
	s.width = ofGetWidth();
	s.height = ofGetHeight();
	s.internalformat = GL_RGBA;
	s.textureTarget = GL_TEXTURE_RECTANGLE_ARB;
	s.maxFilter = GL_LINEAR; GL_NEAREST;
	s.numSamples = 0;
	s.numColorbuffers = 1;
	s.useDepth = false;
	s.useStencil = false;

	gpuBlur.setup(s, false);
	ofLoadImage(tex, "zoidberg.png");

}

//--------------------------------------------------------------
void testApp::update(){

	gpuBlur.blurOffset = 100 * ofMap(mouseY, 0, ofGetHeight(), 1, 0, true);
	//gpuBlur.blurOffset = 15;
	gpuBlur.blurPasses = 50 * ofMap(mouseX, 0, ofGetWidth(), 0, 1, true);
	//gpuBlur.blurPasses = 1;
	gpuBlur.numBlurOverlays = 1;
	gpuBlur.blurOverlayGain = 255;

}

//--------------------------------------------------------------
void testApp::draw(){

	ofBackgroundGradient(ofColor::green, ofColor::red);
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	gpuBlur.beginDrawScene();
		ofClear(0, 0, 0, 0);
		ofSetColor(255);
		ofCircle(ofGetWidth()/2, ofGetHeight()/2,  100);
		tex.draw(0,0);
	gpuBlur.endDrawScene();

	//blur the fbo
	//blending will be disabled at this stage
	gpuBlur.performBlur();

	//draw the "clean" scene
	//ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	//gpuBlur.drawSceneFBO();

	//overlay the blur on top
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); //pre-multiplied alpha
	gpuBlur.drawBlurFbo();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);

	//draw info
	string info =	"blurOffset: " + ofToString(gpuBlur.blurOffset) + "\n" +
					"blurPasses: " + ofToString(gpuBlur.blurPasses) + "\n" +
					"numBlurOverlays: " + ofToString(gpuBlur.numBlurOverlays) + "\n" +
	"blurOverlayGain: " + ofToString(gpuBlur.blurOverlayGain);
	ofDrawBitmapStringHighlight(info, 20,20);

	ofDrawBitmapStringHighlight("MouseX to control Blur Passes\nMouseY to control blur Offset", 400,20);

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

	ofTexture shadowTexture = gpuBlur.getBlurredSceneFbo().getTexture();
	ofPixels pixels;
	shadowTexture.readToPixels(pixels);
	ofSaveImage(pixels, "shadow.tga");


}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}