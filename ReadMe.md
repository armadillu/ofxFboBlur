# ofxFboBlur

[![Build Status](https://travis-ci.org/armadillu/ofxFboBlur.svg?branch=master)](https://travis-ci.org/armadillu/ofxFboBlur)

blur things easily! Render your scene into the ofxFboBlur object; it will keep your original scene, and a blurred copy of it. This way, you can draw your "clean" scene and overlay any amount of blur you want on top of it.

<img src="http://f.cl.ly/items/0A133I2e030w0L0B1c3M/Screen%20Shot%202013-08-23%20at%2013.39.58.PNG"/>

```
	//choose your fbo settings
	ofFbo::Settings s;
	s.width = ofGetWidth();
	s.height = ofGetHeight();
	s.internalformat = GL_RGBA;
	s.maxFilter = GL_LINEAR; GL_NEAREST;
	s.numSamples = 4;
	s.numColorbuffers = 3;
	s.useDepth = true;
	s.useStencil = false;

	//setup thing
	gpuBlur.setup(s);

	//tweak your blurring	
	gpuBlur.blurOffset = 5 * ofMap(mouseY, 0, ofGetHeight(), 1, 0, true);
	gpuBlur.blurPasses = 10. * ofMap(mouseX, 0, ofGetWidth(), 0, 1, true);
	gpuBlur.numBlurOverlays = 1;
	gpuBlur.blurOverlayGain = 255;

	//draw your scene in the fbo, no drawing on screen yet
	gpuBlur.beginDrawScene();
		//YOUR STUFF HERE!
	gpuBlur.endDrawScene();

	//calc the fbo blurring, no drawing on screen yet
	gpuBlur.performBlur();

	//draw the "clean" scene on screen
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	gpuBlur.drawSceneFBO();

	//overlay the blurred fbo on top of the previously drawn clean scene
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	gpuBlur.drawBlurFbo();

	
	


```

