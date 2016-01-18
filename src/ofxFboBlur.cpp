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
	gain = 1.0f;
}

void ofxFboBlur::setup(ofFbo::Settings s, bool additive, float scaleDownPercent){

	scaleDown = scaleDownPercent;
	this->additive = additive;

	if( (!shaderV.isLoaded() && !additive) || (additive && !shaderVadd.isLoaded()) ){
		string fragV = "#extension GL_ARB_texture_rectangle : enable\n" + (string)
		STRINGIFY(
			 uniform float blurLevel;
			 uniform float gain;
			 uniform sampler2DRect src;

			 void main(void){

				 float blurSize = blurLevel;
				 vec4 sum = vec4(0.0);

				 sum += texture2DRect(src, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - 4.0 * blurSize)) * 0.049049;
				 sum += texture2DRect(src, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - 3.0 * blurSize)) * 0.0882;
				 sum += texture2DRect(src, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - 2.0 * blurSize)) * 0.1176;
				 sum += texture2DRect(src, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - blurSize)) * 0.147;
				 sum += texture2DRect(src, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y)) * 0.196;
				 sum += texture2DRect(src, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + blurSize)) * 0.147;
				 sum += texture2DRect(src, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + 2.0 * blurSize)) * 0.1176;
				 sum += texture2DRect(src, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + 3.0 * blurSize)) * 0.0882;
				 sum += texture2DRect(src, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + 4.0 * blurSize)) * 0.049049;
		);

		if(additive) fragV += STRINGIFY( if (sum.a > 0.0) sum.a = 1.0; );

		fragV += STRINGIFY(
				 gl_FragColor = vec4(sum.r * gain, sum.g * gain, sum.b * gain, sum.a );
				//gl_FragColor = vec4(sum.a, sum.a, sum.a, 1.0);
			 }
		);

		string fragH = "#extension GL_ARB_texture_rectangle : enable\n" + (string)
		STRINGIFY(
				  uniform float blurLevel;
				  uniform float gain;
				  uniform sampler2DRect src;

				  void main(void){

					  float blurSize = blurLevel;
					  vec4 sum = vec4(0.0);

					  sum += texture2DRect(src, vec2(gl_TexCoord[0].x - 4.0 * blurSize, gl_TexCoord[0].y)) * 0.049049;
					  sum += texture2DRect(src, vec2(gl_TexCoord[0].x - 3.0 * blurSize, gl_TexCoord[0].y)) * 0.0882;
					  sum += texture2DRect(src, vec2(gl_TexCoord[0].x - 2.0 * blurSize, gl_TexCoord[0].y)) * 0.1176;
					  sum += texture2DRect(src, vec2(gl_TexCoord[0].x - blurSize, gl_TexCoord[0].y)) * 0.147;
					  sum += texture2DRect(src, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y)) * 0.196;
					  sum += texture2DRect(src, vec2(gl_TexCoord[0].x + blurSize, gl_TexCoord[0].y)) * 0.147;
					  sum += texture2DRect(src, vec2(gl_TexCoord[0].x + 2.0 * blurSize, gl_TexCoord[0].y)) * 0.1176;
					  sum += texture2DRect(src, vec2(gl_TexCoord[0].x + 3.0 * blurSize, gl_TexCoord[0].y)) * 0.0882;
					  sum += texture2DRect(src, vec2(gl_TexCoord[0].x + 4.0 * blurSize, gl_TexCoord[0].y)) * 0.049049;
					  
					);

		if(additive) fragH += STRINGIFY( if (sum.a > 0.0) sum.a = 1.0; );

		fragH += STRINGIFY(
					  gl_FragColor = vec4(sum.r * gain, sum.g * gain, sum.b * gain, sum.a );
					  //gl_FragColor = vec4(sum.a, sum.a, sum.a, 1.0);
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

  	if(s.textureTarget != GL_TEXTURE_RECTANGLE_ARB){
		ofLogError("ofxFboBlur") << "ofFbo::Settings textureTarget must be set to GL_TEXTURE_RECTANGLE_ARB for the shaders to work!";
	}

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
	blur(&cleanImgFBO, &blurOutputFBO, &blurTempFBO, &blurTempFBO2, blurPasses, blurOffset, gain);
}

void ofxFboBlur::drawSceneFBO(){
#if (OF_VERSION_MINOR >= 8)
	cleanImgFBO.getTextureReference().draw(0, 0, cleanImgFBO.getWidth(), cleanImgFBO.getHeight());
#else
	cleanImgFBO.getTextureReference().draw(0, cleanImgFBO.getHeight(), cleanImgFBO.getWidth(), -cleanImgFBO.getHeight());
#endif
}

void ofxFboBlur::drawBlurFbo(bool useCurrentColor){
	ofPushStyle();
	if(!useCurrentColor) ofSetColor(blurOverlayGain);
	for(int i = 0; i < numBlurOverlays; i++){
		#if (OF_VERSION_MINOR >= 8)
		blurOutputFBO.getTextureReference().draw(0, 0, blurOutputFBO.getWidth() / scaleDown, blurOutputFBO.getHeight() / scaleDown);
		#else
		blurOutputFBO.getTextureReference().draw(0, blurOutputFBO.getHeight(), blurOutputFBO.getWidth() / scaleDown, -blurOutputFBO.getHeight() / scaleDown);
		#endif
	}
	ofPopStyle();
}


void ofxFboBlur::blur( ofFbo * input, ofFbo * output, ofFbo * buffer, ofFbo * buffer2, int iterations, float blurOffset, float gain){

	ofPushStyle();
	ofDisableAlphaBlending();

	if( iterations > 0 ){

		ofShader * sv = additive ? &shaderVadd : &shaderV;
		ofShader * sh = additive ? &shaderHadd : &shaderH;

		buffer2->begin();
		input->draw(0,0, buffer2->getWidth(), buffer2->getHeight());
		buffer2->end();

		for (int i = 0; i < iterations; i++) {

			float blurLevel = blurOffset * (i + 1) / ( iterations * iterations + 1);

			buffer->begin();
				sv->begin();
				//sv->setUniformTexture( "texture", buffer2->getTextureReference(), 0 ); //wtf how does this work?
				sv->setUniform1f("blurLevel", blurLevel);
				sv->setUniform1f("gain", gain);
				buffer2->draw(0,0);
				sv->end();
			buffer->end();

			buffer2->begin();
				sh->begin();
				//sh->setUniformTexture( "texture", buffer->getTextureReference(), 0 ); //wtf how does this work?
				sh->setUniform1f("blurLevel", blurLevel);
				sh->setUniform1f("gain", gain);
				buffer->draw(0,0);
				sh->end();
			buffer2->end();
		}

		output->begin();
		buffer2->draw(0, 0);
		output->end();

	}else{
		output->begin();
		input->draw(0,0, output->getWidth(), output->getHeight());
		output->end();
	}
	ofPopStyle();
}
