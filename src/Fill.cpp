#include "Fill.h"
//--------------------------------------------------------------
Fill::Fill(){
    
}
//--------------------------------------------------------------
void Fill::setup(ofFbo fbo, ofColor _strokeColor, ofColor backColor){
    fbo.readToPixels(pixels); //パターンなしピクセル
    strokeColor = _strokeColor;
    
    //pixels から strokeColor と backColor 以外の色（線のふち部分）をbackColorに置き換える
    for (int i = 0; i < pixels.getWidth(); i++) {
        for (int j = 0; j < pixels.getHeight(); j++) {
            if (strokeColor != pixels.getColor(i, j) && backColor != pixels.getColor(i, j)) {
                pixels.setColor(i, j, backColor);
            }
        }
    }
    pixels2 = pixels; //結果用ピクセル（パターンを描画する）
}

//--------------------------------------------------------------
void Fill::setPattern(vector<ofFbo> _patterns){
    patterns = _patterns;
}
//--------------------------------------------------------------
void Fill::setPos(int _x, int _y, ofColor _newColor, int _patternNum){
    x = _x;
    y = _y;
    oldColor = pixels.getColor(x, y); //現在の色
    oldColor2.set(oldColor.r+1, oldColor.g, oldColor.b);
    //oldColor2 = ofColor(255, 0, 0);
        //↑パターン描写のときに使う色
        //元の色と少し変えることで塗りつぶしがスムーズに行われるようにする
    newColor = _newColor; //新しい色
    patternNum = _patternNum; //パターン番号
    
    if(oldColor == strokeColor){ //クリックした場所が線の上のときは塗りつぶし不要
        return;
    }
    
    if(newColor.a <255){ //透明度がある場合は、元の色に新しい色を重ねたときの色を生成する
        ofFbo fbo2;
        fbo2.allocate(1, 1, GL_RGB32F_ARB);
        fbo2.begin();
        ofBackground(oldColor);
        ofSetColor(newColor);
        ofDrawRectangle(0, 0, 1, 1);
        fbo2.end();
        ofPixels pixels2;
        fbo2.readToPixels(pixels2);
        newColor =  pixels2.getColor(0, 0);
    }
    
    newColor_copy = newColor; //新しい色のコピー
    
    if (patternNum > 0) { //パターン描画のとき、pixelのほうで塗りつぶす色（元の色からちょっとだけ変えた色にする）
        newColor = oldColor2;
    }
    
    pixels2Color = newColor;
    
    //パターンを生成
    makePattern();
    
    //塗りつぶし
    scanLineSeedFill();
}

//--------------------------------------------------------------
void Fill::makePattern() {
    pFbo.allocate(ofGetWidth(), ofGetHeight());
    pFbo.begin();
    if (patternNum == 1) { //ストライプ
        ofBackground(oldColor2);
        ofSetColor(newColor_copy);
        patterns[1].draw(0, 0);
    } else if (patternNum == 2) { //水玉
        ofBackground(oldColor2);
        ofSetColor(newColor_copy);
        patterns[2].draw(0, 0);
    } else if (patternNum == 3) { //水玉
        ofBackground(newColor_copy);
        ofSetColor(oldColor2);
        patterns[3].draw(0, 0);
    } else if (patternNum == 4) { //チェック
        ofBackground(oldColor2);
        ofSetColor(newColor_copy);
        patterns[4].draw(0, 0);
    }
    pFbo.end();
    
    pFbo.readToPixels(patternPixels);
}
//--------------------------------------------------------------
void Fill::draw(){
    image.setFromPixels(pixels2);
    image.draw(0, 0);
}
//--------------------------------------------------------------
void Fill::pushSegment(Segment* aSegmentStack, int& riCountSegment, int iY, int iXL, int iXR ,int iDeltaY) {
    assert(riCountSegment < MAX_STACK_COUNT);
    if (iY < 0 || ofGetHeight()-1 < iY) {
        //グリッド範囲外
        return;
    }
    aSegmentStack[riCountSegment].iY = iY;
    aSegmentStack[riCountSegment].iXL = iXL;
    aSegmentStack[riCountSegment].iXR = iXR;
    aSegmentStack[riCountSegment].iDeltaY = iDeltaY;
    //セグメント数を一つ増やす
    riCountSegment++;
}

//--------------------------------------------------------------
//スタックからセグメントの取り出し
void Fill::popSegment(Segment* aSegmentStack, int& riCountSegment, int& riY, int& riXL, int& riXR, int& riDeltaY) {
    //セグメント数を一つ減らす
    riCountSegment--;
    riY = aSegmentStack[riCountSegment].iY;
    riXL = aSegmentStack[riCountSegment].iXL;
    riXR = aSegmentStack[riCountSegment].iXR;
    riDeltaY = aSegmentStack[riCountSegment].iDeltaY;
}

//--------------------------------------------------------------
void Fill::scanLineSeedFill() {
    //スタック
    Segment aSegmentStack[MAX_STACK_COUNT];
    int iCountSegment = 0;
    
    //シードセルを、セグメントスタックに登録する
    //「逆方向について、『セグメント範囲は、塗りつぶし済み』、『セグメント範囲の左右の隣接する１セルは、塗りつぶし対象でなかった』」を前提としているが、
    //シードセルのセグメントスタック登録においては、この前提は成り立たない。
    //「シードセルのセグメントスタック登録は、２方向分行う」ことと、
    //「シードセルのセグメントスタック登録の１回目は、シードセルの左右を含めた３セル分で行う」ことで対処。
    //if (oldColor == pixels.getColor(x, y + 1) {
    if (newColor != pixels.getColor(x, y+1) || strokeColor != pixels.getColor(x, y+1)) {
        //+Y方向は、Y+1のセルが塗りつぶし対象の場合のみ登録
        pushSegment(aSegmentStack, iCountSegment, y+1, x-1>=0 ? x-1 : x , x+1 < ofGetWidth() ? x+1 : x, 1);
    }
    pushSegment(aSegmentStack, iCountSegment, y, x, x, -1);
    
    while (iCountSegment > 0) {
        //セグメントを一つ取り出す
        int iSegmentY;
        int iSegmentXL;
        int iSegmentXR;
        int iSegmentDeltaY;
        popSegment(aSegmentStack, iCountSegment, iSegmentY, iSegmentXL, iSegmentXR, iSegmentDeltaY);
        
        //iSegmentXLから左方向塗りつぶし
        int xl = iSegmentXL;
        while (xl >= 0) {
            //if (oldColor != pixels.getColor(xl, iSegmentY)) {
            if (newColor == pixels.getColor(xl, iSegmentY) || strokeColor == pixels.getColor(xl, iSegmentY)){
                //塗りつぶし対象以外が来たら左探索塗りつぶし終了
                break;
            }
            cellFill(xl, iSegmentY);
            xl--;
        }
    
        
        int iStartX; //塗りつぶしが行われた左端位置
        if (xl == iSegmentXL) {
            //塗りつぶしが行われていないので、左端位置として、無効な値を設定しておく
            iStartX = -1;
        } else {
            //while文で、インデックスが一つ行き過ぎている（塗り対象以外を指している）ので、一つ戻す。
            iStartX = xl + 1;
            
            if( iStartX < iSegmentXL - 1){
                //セグメント範囲外の塗りつぶしが行われた場合は、逆方向でセグメント登録
                pushSegment(aSegmentStack, iCountSegment, iSegmentY - iSegmentDeltaY, iStartX, iSegmentXL-2, -iSegmentDeltaY);
            }
            
            // iSegmentXLから左方向塗りつぶしに続いて、iSegmentXL + 1から右方向塗りつぶし
            xl = iSegmentXL + 1;
        }
        
        //右方向塗りつぶし
        do {
            if (-1 != iStartX) {
                while (xl <= ofGetWidth()) {
                    //if(oldColor != pixels.getColor(xl, iSegmentY)) {
                    if (newColor == pixels.getColor(xl, iSegmentY) || strokeColor == pixels.getColor(xl, iSegmentY)){
                        //塗りつぶし対象以外に辿り着いたら抜ける
                        break;
                    }
                    cellFill(xl, iSegmentY);
                    xl++;
                }
                
                //塗りつぶし範囲を、順方向でセグメント登録
                pushSegment(aSegmentStack, iCountSegment, iSegmentY + iSegmentDeltaY, iStartX, xl-1, iSegmentDeltaY);
                if( xl - 1 > iSegmentXR + 1) {
                    pushSegment(aSegmentStack, iCountSegment, iSegmentY - iSegmentDeltaY, iSegmentXR + 2, xl-1, -iSegmentDeltaY);
                }
                iStartX = -1; //左端位置として、無効な値を設定
            }
            
            //新たな左端位置探し
            xl++;
            while (xl <= iSegmentXR) {
                //if (oldColor == pixels.getColor(xl, iSegmentY)) {
                if (newColor != pixels.getColor(xl, iSegmentY) || strokeColor != pixels.getColor(xl, iSegmentY)){
                    //塗りつぶし対象が見つかったら抜ける
                    cellFill(xl, iSegmentY);
                    iStartX = xl;
                    xl++;
                    break;
                }
                xl++;
            }
        } while(-1 != iStartX);
    }
         
}
//--------------------------------------------------------------
void Fill::cellFill(int cx, int cy){
    pixels.setColor(cx, cy, newColor);
    
    if (patternNum>0) pixels2Color = patternPixels.getColor(cx, cy); //パターンの描画
    pixels2.setColor(cx, cy, pixels2Color);
}
//--------------------------------------------------------------
