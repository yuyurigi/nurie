#pragma once

#include "ofMain.h"
#include "ofxSvg.h"
#include "ofxNanoVG.h"
#include "ofxTrueTypeFontUC.h"
#include "ofxEasing.h"
#include "Fill.h"

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed  (int key);
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
    
    void setColor();
    void setSvg();
    void setNanoVG();
    void setFbo();
    void setPattern();
    void drawPattern();
    void changeAlphaEllipse(int y);
    
    
    //font
    ofxTrueTypeFontUC font;
    ofRectangle nurieRect;
    vector < ofPath > paths;
    glm::vec2 tPos;
    
    //svg
    ofxSVG svg;
    float scale, sHeight, sWidth;
    vector<ofPolyline> outlines;
    vector<ofPath> button;
    glm::vec2 sPos;
    ofRectangle saveRect, clearRect, clearWindowRect, alertRect;
    bool saveFlag; //保存ボタンを押したかどうかを判別するフラッグ
    bool clearWindowFlag; //全消しボタンを押したかどうかを判別するフラッグ、全消しボタンを押したとき出るウィンドウ用のフラッグ
    bool saveTextFlag;
    
    //text
    float textSize;
    ofRectangle textRect, yesRect, noRect, saveTextRect;
    glm::vec2 yesTextPos, noTextPos;
    float initTime, initTime2;
    float textWindowY, windowAlpha, saveTextX, saveTextAlpha;
    
    //ofxNanoVG
    ofxNanoVG::Canvas canvas, canvas2;
    
    //fbo
    ofFbo fbo;
     
    //colors
    ofFloatColor backFColor, strokeFColor, whiteF, deepPinkF, grayF, deepPinkF2, windowBackColor;
    ofColor backColor, strokeColor, white, deepPink, gray;
    vector<ofColor> colors;
    vector<ofRectangle> colRect;
    int space, currentColor;
    float currentAlpha;
    float colSize, colRad, eY;
    ofRectangle alphaRect, colPalRect;
    
    //pattern
    int currentPattern;
    vector<ofFbo> patterns;
    vector<ofFbo> pTumbnail;
    vector<ofRectangle> patternRect;
    
    //クラス　Fill
    Fill myFill;
};
