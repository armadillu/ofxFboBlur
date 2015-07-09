/*
 *  ofxFboBlur.cpp
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 26/03/12.
 *  Copyright 2012 uri.cat. All rights reserved.
 *
 */

#include "ofxFboBlur.h"

ofShader ofxFboBlur::shaderV = ofShader();
ofShader ofxFboBlur::shaderH = ofShader();
ofShader ofxFboBlur::shaderVadd = ofShader();
ofShader ofxFboBlur::shaderHadd = ofShader();

ofxFboBlur::ofxFboBlur(){
	blurOffset = 0.5;
	blurPasses = 1;
	numBlurOverlays = 1;
	blurOverlayGain = 128;
}

void ofxFboBlur::setup(ofFbo::Settings s, bool additive, float scaleDownPercent){

	scaleDown = scaleDownPercent;
	this->additive = additive;
	backgroundColor = ofColor(0,0,0,0);

	if( (!shaderV.isLoaded() && !additive) || (additive && !shaderVadd.isLoaded()) ){
		string fragV = "#extension GL_ARB_texture_rectangle : enable\n" + (string)
		STRINGIFY(
			 uniform float blurLevel;
			 uniform sampler2DRect texture;

			 void main(void){

				 float blurSize = blurLevel;
				 vec4 sum = vec4(0.0);

				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - 4.0 * blurSize)) * 0.049049;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - 3.0 * blurSize)) * 0.0882;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - 2.0 * blurSize)) * 0.1176;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - blurSize)) * 0.147;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y)) * 0.196;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + blurSize)) * 0.147;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + 2.0 * blurSize)) * 0.1176;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + 3.0 * blurSize)) * 0.0882;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + 4.0 * blurSize)) * 0.049049;
		);

		if(additive) fragV += STRINGIFY( if (sum.a > 0.0) sum.a = 1.0; );

		fragV += STRINGIFY(
				 gl_FragColor = sum;
			 }
		);

		string fragH = "#extension GL_ARB_texture_rectangle : enable\n" + (string)
		STRINGIFY(
				  uniform float blurLevel;
				  uniform sampler2DRect texture;

				  void main(void){

					  float blurSize = blurLevel;
					  vec4 sum = vec4(0.0);

					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x - 4.0 * blurSize, gl_TexCoord[0].y)) * 0.049049;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x - 3.0 * blurSize, gl_TexCoord[0].y)) * 0.0882;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x - 2.0 * blurSize, gl_TexCoord[0].y)) * 0.1176;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x - blurSize, gl_TexCoord[0].y)) * 0.147;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y)) * 0.196;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x + blurSize, gl_TexCoord[0].y)) * 0.147;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x + 2.0 * blurSize, gl_TexCoord[0].y)) * 0.1176;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x + 3.0 * blurSize, gl_TexCoord[0].y)) * 0.0882;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x + 4.0 * blurSize, gl_TexCoord[0].y)) * 0.049049;
					  
					);

		if(additive) fragH += STRINGIFY( if (sum.a > 0.0) sum.a = 1.0; );

		fragH += STRINGIFY(
					  gl_FragColor = sum;
				  }
		);

		string vertex =
		STRINGIFY(
			   void main() {
				   gl_TexCoord[0] = gl_MultiTexCoord0;
				   gl_Position = ftransform();
			   }
		);

		if(!additive){
			shaderV.setupShaderFromSource(GL_VERTEX_SHADER, vertex);
			shaderV.setupShaderFromSource(GL_FRAGMENT_SHADER, fragV);
			bool ok = shaderV.linkProgram();
			
			shaderH.setupShaderFromSource(GL_VERTEX_SHADER, vertex);
			shaderH.setupShaderFromSource(GL_FRAGMENT_SHADER, fragH);
			ok = shaderH.linkProgram();
		}else{
			shaderVadd.setupShaderFromSource(GL_VERTEX_SHADER, vertex);
			shaderVadd.setupShaderFromSource(GL_FRAGMENT_SHADER, fragV);
			bool ok = shaderVadd.linkProgram();

			shaderHadd.setupShaderFromSource(GL_VERTEX_SHADER, vertex);
			shaderHadd.setupShaderFromSource(GL_FRAGMENT_SHADER, fragH);
			ok = shaderHadd.linkProgram();
		}
	}

	ofLogLevel l = ofGetLogLevel();
	ofSetLogLevel(OF_LOG_WARNING);
	cleanImgFBO.allocate( s );

	s.width *= scaleDown;
	s.height *= scaleDown;

	blurOutputFBO.allocate( s );
	blurTempFBO.allocate( s );
	blurTempFBO2.allocate( s );
	ofSetLogLevel(l);
}

void ofxFboBlur::beginDrawScene(){
	cleanImgFBO.begin();
}

void ofxFboBlur::endDrawScene(){
	cleanImgFBO.end();
}

void ofxFboBlur::performBlur(){
	blur(&cleanImgFBO, &blurOutputFBO, &blurTempFBO, &blurTempFBO2, blurPasses, blurOffset);
}

void ofxFboBlur::drawSceneFBO(){
#if (OF_VERSION_MINOR >= 8)
	cleanImgFBO.getTextureReference().draw(0, 0, cleanImgFBO.getWidth(), cleanImgFBO.getHeight());
#else
	cleanImgFBO.getTextureReference().draw(0, cleanImgFBO.getHeight(), cleanImgFBO.getWidth(), -cleanImgFBO.getHeight());
#endif
}

void ofxFboBlur::drawBlurFbo(bool useCurrentColor){
	if(!useCurrentColor) ofSetColor(blurOverlayGain);
	for(int i = 0; i < numBlurOverlays; i++){
		#if (OF_VERSION_MINOR >= 8)
		blurOutputFBO.getTextureReference().draw(0, 0, blurOutputFBO.getWidth() / scaleDown, blurOutputFBO.getHeight() / scaleDown);
		#else
		blurOutputFBO.getTextureReference().draw(0, blurOutputFBO.getHeight(), blurOutputFBO.getWidth() / scaleDown, -blurOutputFBO.getHeight() / scaleDown);
		#endif
	}
}

void ofxFboBlur::setBackgroundColor(ofColor c){
	backgroundColor = c;
}

void ofxFboBlur::blur( ofFbo * input, ofFbo * output, ofFbo * buffer, ofFbo * buffer2, int iterations, float blurOffset){

	if( iterations > 0 ){

		ofShader * sv = additive ? &shaderVadd : &shaderV;
		ofShader * sh = additive ? &shaderHadd : &shaderH;

		buffer->begin();
		ofClear(backgroundColor);
		buffer->end();

		buffer2->begin();
		ofClear(backgroundColor);
		buffer2->end();

		ofEnableAlphaBlending();
		for (int i = 0; i < iterations; i++) {

			buffer->begin();
				sv->begin();
				if (i == 0){ //for first pass, we use input as src; after that, we retro-feed the output of the 1st pass
					sv->setUniformTexture( "texture", input->getTextureReference(), 0 );
				}else{
					sv->setUniformTexture( "texture", buffer2->getTextureReference(), 0 );
				}
				sv->setUniform1f("blurLevel", blurOffset * (i + 1) / ( iterations * iterations + 1));
				if (i == 0){
					input->draw(0,0, buffer->getWidth(), buffer->getHeight());
				}else{
					buffer2->draw(0,0);
				}
				sv->end();
			buffer->end();

			buffer2->begin();
				sh->begin();
				sh->setUniformTexture( "texture", buffer->getTextureReference(), 0 );
				sh->setUniform1f("blurLevel", blurOffset * (i + 1) / ( iterations * iterations + 1));
				buffer->draw(0,0);
				sh->end();
			buffer2->end();
		}
		//draw back into original fbo

		output->begin();
		ofClear(backgroundColor);
		buffer2->draw(0, 0);
		output->end();

	}else{
		output->begin();
		ofClear(backgroundColor);
		input->draw(0,0, buffer->getWidth(), buffer->getHeight());
		output->end();
	}
}
