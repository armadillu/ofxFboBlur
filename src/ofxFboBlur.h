/*
 *  ofxFboBlur.h
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 26/03/12.
 *  Copyright 2012 uri.cat. All rights reserved.
 *
 */

#include "ofMain.h"
#pragma once

#define STRINGIFY(A) #A

class ofxFboBlur{

public:

	ofxFboBlur();
	
	void setup(ofFbo::Settings s, bool additive, float scaleDownPercent = 1.0f);
	void beginDrawScene();

	void endDrawScene();
	void performBlur();


	void drawSceneFBO();
	void drawBlurFbo(bool useCurrentColor = false);

	void setBackgroundColor(ofColor c);

	//access directly please!
	float blurOffset;
	int blurPasses;
	int numBlurOverlays;	
	int blurOverlayGain;	//[0..255]

	ofFbo &getSceneFbo(){return cleanImgFBO;};
	ofFbo &getBlurredSceneFbo(){return blurOutputFBO;};

private:

	float scaleDown;
	bool additive;

	void blur( ofFbo * input, ofFbo * output, ofFbo * buffer, ofFbo * buffer2, int iterations, float blurOffset);

	ofFbo	cleanImgFBO;
	ofFbo	blurOutputFBO;
	ofFbo	blurTempFBO;
	ofFbo	blurTempFBO2;

	static ofShader shaderV;
	static ofShader shaderH;
	static ofShader shaderVadd;
	static ofShader shaderHadd;

	ofColor backgroundColor;
};