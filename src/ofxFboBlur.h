/*
 *  ofxFboBlur.h
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 26/03/12.
 *  Copyright 2012 uri.cat. All rights reserved.
 *
 */

#include "ofMain.h"

#define STRINGIFY(A) #A

class ofxFboBlur{

public:
	
	void setup(ofFbo::Settings s){

		string fragV = "#extension GL_ARB_texture_rectangle : enable\n" + (string)
		STRINGIFY(
			 uniform float blurLevel;
			 uniform sampler2DRect texture;

			 void main(void){

				 float blurSize = blurLevel ;
				 vec4 sum = vec4(0.0);

				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - 4.0 * blurSize)) * 0.05;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - 3.0 * blurSize)) * 0.09;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - 2.0 * blurSize)) * 0.12;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - blurSize)) * 0.15;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y)) * 0.2;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + blurSize)) * 0.15;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + 2.0 * blurSize)) * 0.12;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + 3.0 * blurSize)) * 0.09;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + 4.0 * blurSize)) * 0.05;
				 
				 if (sum.a > 0.0) sum.a = 1.0;
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
					  vec2 st = gl_TexCoord[0].st;

					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x - 4.0 * blurSize, gl_TexCoord[0].y)) * 0.05;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x - 3.0 * blurSize, gl_TexCoord[0].y)) * 0.09;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x - 2.0 * blurSize, gl_TexCoord[0].y)) * 0.12;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x - blurSize, gl_TexCoord[0].y)) * 0.15;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y)) * 0.2;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x + blurSize, gl_TexCoord[0].y)) * 0.15;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x + 2.0 * blurSize, gl_TexCoord[0].y)) * 0.12;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x + 3.0 * blurSize, gl_TexCoord[0].y)) * 0.09;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x + 4.0 * blurSize, gl_TexCoord[0].y)) * 0.05;
					  
					  if (sum.a > 0.0) sum.a = 1.0;
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

		shaderV.setupShaderFromSource(GL_VERTEX_SHADER, vertex);
		shaderV.setupShaderFromSource(GL_FRAGMENT_SHADER, fragV);
		shaderV.linkProgram();
		
		shaderH.setupShaderFromSource(GL_VERTEX_SHADER, vertex);
		shaderH.setupShaderFromSource(GL_FRAGMENT_SHADER, fragH);
		shaderH.linkProgram();

		cleanImgFBO.allocate( s );
		blurOutputFBO.allocate( s );
		blurTempFBO.allocate( s );
		blurTempFBO2.allocate( s );

		//shaderV.load("shaders/blur.vert", "shaders/blurV.frag");
		//shaderH.load("shaders/blur.vert", "shaders/blurH.frag");
	}

	void beginDrawScene(){
		cleanImgFBO.begin();
	}

	void endDrawScene(){
		cleanImgFBO.end();
	}

	void performBlur(){
		blur(&cleanImgFBO, &blurOutputFBO, &blurTempFBO, &blurTempFBO2, blurPasses, blurOffset);
	}

	void drawSceneFBO(){
		cleanImgFBO.getTextureReference().draw(0, cleanImgFBO.getHeight(), cleanImgFBO.getWidth(), -cleanImgFBO.getHeight());
	}

	void drawBlurFbo(){
		ofSetColor(blurOverlayGain);
		for(int i = 0; i < numBlurOverlays; i++){
			blurOutputFBO.getTextureReference().draw(0, blurOutputFBO.getHeight(), blurOutputFBO.getWidth(), -blurOutputFBO.getHeight());
		}
	}

	//access direclty please!
	float blurOffset;
	int blurPasses;
	int numBlurOverlays;
	int blurOverlayGain;

private:

	void blur( ofFbo * input, ofFbo * output, ofFbo * buffer, ofFbo * buffer2, int iterations, float blurOffset  ){

		if( iterations > 0 ){

			buffer->begin();
			ofClear(0, 0, 0, 0);
			buffer->end();

			buffer2->begin();
			ofClear(0, 0, 0, 0);
			buffer2->end();

			ofEnableAlphaBlending();
			for (int i = 0; i < iterations; i++) {

				buffer->begin();
				shaderV.begin();
				if (i == 0){ //for first pass, we use input as src; after that, we retro-feed the output of the 1st pass
					shaderV.setUniformTexture( "texture", input->getTextureReference(), 0 );
				}else{
					shaderV.setUniformTexture( "texture", buffer2->getTextureReference(), 1 );
				}
				shaderV.setUniform1f("blurLevel", blurOffset * (i + 1) );
				input->draw(0,0);
				shaderV.end();
				buffer->end();

				buffer2->begin();
				shaderH.begin();
				shaderH.setUniformTexture( "texture", buffer->getTextureReference(), 2 );
				shaderH.setUniform1f("blurLevel", blurOffset * (i + 1) );
				buffer->draw(0,0);
				shaderH.end();
				buffer2->end();
			}
			//draw back into original fbo

			output->begin();
			ofClear(0, 0, 0, 0);
			buffer2->draw(0, 0);
			output->end();

		}else{
			output->begin();
			ofClear(0, 0, 0, 0);
			input->draw(0, 0);
			output->end();

		}
	}


	ofFbo	cleanImgFBO;
	ofFbo	blurOutputFBO;
	ofFbo	blurTempFBO;
	ofFbo	blurTempFBO2;

	ofShader shaderV;
	ofShader shaderH;
};