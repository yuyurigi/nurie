#pragma once

#include "ofMain.h"

#define MAX_STACK_COUNT 100000

struct Segment {
    int iY;
    int iXL;
    int iXR;
    int iDeltaY;
};

class Fill {
private:
    ofPixels pixels, pixels2, patternPixels;
    ofImage image;
    ofFbo pFbo;
    int x, y, patternNum;
    ofColor newColor, newColor_copy, pixels2Color, strokeColor, oldColor, oldColor2;
    vector<ofFbo> patterns;
    void makePattern();
    void pushSegment(Segment* aSegmentStack, int& riCountSegment, int iY, int iXL, int iXR, int iDeltaY);
    void popSegment(Segment* aSegmentStack, int& riCountSegment, int& riY, int& riXL, int& riXR, int& riDeltaY);
    void scanLineSeedFill();
    void cellFill(int cx, int cy);
public:
    Fill();
    void setup(ofFbo fbo, ofColor _strokeColor, ofColor backColor);
    void setPattern(vector<ofFbo> _patterns);
    void setPos(int _x, int _y, ofColor _newColor, int _patternNum);
    void draw();
};
