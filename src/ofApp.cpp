#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(true);

    //svg
	svg.load("svg/001-fox.svg");
    
    sHeight = ofGetHeight()*0.7; //イラストの大きさ（たて）
    float scaleY = sHeight/svg.getHeight();
    sWidth = svg.getWidth()*scaleY; //イラストの大きさ（よこ）
    
	for (ofPath p: svg.getPaths()){
		// svg defaults to non zero winding which doesn't look so good as contours
		p.setPolyWindingMode(OF_POLY_WINDING_ODD);
		const vector<ofPolyline>& lines = p.getOutline();
		for(const ofPolyline & line: lines){
			outlines.push_back(line.getResampledBySpacing(1));
		}
	}
    scale = 1/svg.getHeight()*ofGetHeight()*0.7;
    sPos = glm::vec2(ofGetWidth()/2*0.94, ofGetHeight()/2*1.2); //イラストの位置（中心値）
    
    //font
    font.loadFont("font/funwari-round.otf", 140, true, true);
    font.setLetterSpacing(1.2);
    string words = "ぬりえ";
    paths = font.getStringAsPoints(words);
    nurieRect = font.getStringBoundingBox(words, 0, 0);
    tPos = glm::vec2(ofGetWidth()/2, ofGetHeight()*0.14); //テキストの位置（中心値）
    
    //colors
    setColor();
    
    //ofxNnoVG
    canvas.allocate(ofGetWidth(), ofGetHeight());
    canvas2.allocate(ofGetWidth(), ofGetHeight());
    assert(canvas2.loadFont("font/rounded-l-mplus-1m-bold.ttf", "round"));
    setNanoVG();
    
    //fbo
    setFbo();
    
    //saveSvg, clearSvg
    setSvg();
    
    //pattern
    setPattern();
}

//--------------------------------------------------------------
void ofApp::setColor(){ //色の設定
    backColor = ofColor(236);
    backFColor = ofFloatColor(backColor.r/255.0f, backColor.g/255.0f, backColor.b/255.0f); //floatColor
    strokeColor = ofColor(79, 86, 95); //イラストの線の色
    strokeFColor = ofFloatColor(strokeColor.r/255.0f, strokeColor.g/255.0f, strokeColor.b/255.0f); //floatColor
    white = ofColor(255, 255, 255);
    whiteF = ofFloatColor(1, 1, 1);
    deepPink = ofColor(239, 106, 144);
    deepPinkF = ofFloatColor(deepPink.r/255.0f, deepPink.g/255.0f, deepPink.b/255.0f);
    gray = ofColor(226);
    grayF = ofFloatColor(gray.r/255.0f, gray.g/255.0f, gray.b/255.0f);
    ofColor deepPink2 = ofColor(248, 58, 127);
    deepPinkF2 = ofFloatColor(deepPink2 .r/255.0f, deepPink2 .g/255.0f, deepPink2 .b/255.0f);
    windowBackColor = ofFloatColor(200/255.0f, 200/255.0f, 200/255.0f);
    colors.push_back(ofColor(244, 129, 112)); //赤
    colors.push_back(ofColor(249, 166, 86)); //オレンジ
    colors.push_back(ofColor(254, 206, 121)); //うすいオレンジ
    colors.push_back(ofColor(247, 228, 154)); //黄
    colors.push_back(ofColor(202, 224, 137)); //黄緑
    colors.push_back(ofColor(151, 209, 169)); //緑
    colors.push_back(ofColor(160, 217, 217)); //水色
    colors.push_back(ofColor(148, 202, 227)); //青
    colors.push_back(ofColor(139, 139, 195)); //むらさき
    colors.push_back(ofColor(228, 183, 213)); //うすい紫
    colors.push_back(ofColor(243, 145, 186)); //ピンク
    colors.push_back(ofColor(243, 129, 147)); //濃いピンク
    colors.push_back(backColor); //背景色
    
    currentColor = 0; //現在の色
    currentAlpha = 1; //現在の透明度（0~1）
    space = ofGetWidth()*0.013; //色円と色円の感覚
    colSize = (ofGetHeight() - space*(colors.size()+1))/colors.size(); //色円のサイズ（直径）
    colRect.resize(colors.size());
    for (int i = 0; i < colors.size(); i++) {
        colRect[i].set(space*3, space + space*i + colSize*i, colSize, colSize);
    }
    alphaRect.set(space, space+10, colSize*0.3, ofGetHeight()*0.4); //透明度のバー
    eY = alphaRect.y; //透明度バーの円の位置
    
    
}
//--------------------------------------------------------------
void ofApp::setSvg(){ //保存ボタン・全消しボタンのsvgをロード、設定
    
    //buttonフォルダに入ってる.svgファイルをロードする
    //0 : 全消しアイコン
    //1 : 保存アイコン
    //2 : 注意アイコン
    //3 : 注意アイコンの三角形だけ
    
    vector<ofxSVG> svgs;
    ofDirectory dir;
    dir.listDir("button/");
    dir.allowExt("svg");
    dir.sort();
    if (dir.size()) {
        svgs.assign(dir.size(), ofxSVG());
    }
    for (int i = 0; i < (int)dir.size(); i++) {
        svgs[i].load(dir.getPath(i));
    }
    
    button.resize(svgs.size());
    int iconSize = ofGetWidth()*0.07; //ボタンのサイズ
    
    //svgをofPathの配列（button）に代入
    int num = 0;
    for (int i = 0; i < svgs.size(); i++) {
        for (ofPath p: svgs[i].getPaths()){
            p.setPolyWindingMode(OF_POLY_WINDING_ODD);
            const vector<ofPolyline>& lines = p.getOutline();
            vector<ofPolyline> outlines;
            for(const ofPolyline & line: lines){
                outlines.push_back(line.getResampledBySpacing(1));
            }
            for (int j = 0; j < outlines.size(); j++) {
                ofPolyline & line = outlines[j];
                for (int k = 0; k < line.size(); k++) {
                       float x = ofMap(line[k].x, 0, svgs[0].getWidth(), 0, iconSize);
                       float y = ofMap(line[k].y, 0, svgs[0].getHeight(), 0, iconSize);
                    if (j==0 && k==0){
                        button[num].lineTo(x, y);
                    } else if(j>0 && k==0){
                        button[num].moveTo(x, y);
                    }else if (k > 0) {
                        button[num].lineTo(x, y);
                    }
                }
                button[num].close();
            }
        }
        num += 1;
    }
    
    //保存ボタンの位置を決定。rect.set(x, y, width, height)
    saveRect.set(ofGetWidth()-iconSize-space, ofGetHeight()-iconSize-space, iconSize, iconSize);
    saveFlag = false; //保存ボタン用のフラッグの初期値はfalse
    //保存ボタンを押したときに出るテキスト
    ofxNanoVG::Canvas & c2 = canvas2;
    textSize = ofGetWidth()*0.025;
    c2.textSize(textSize);
    ofRectangle stext = c2.textBounds("data/にスクリーンショットを保存しました", 0, 0);
    //保存テキストの位置
    saveTextRect.set(ofGetWidth()-stext.width-10, space, stext.width, stext.height);
    saveTextFlag = false; //保存ボタンを押したときに出るテキストんのフラッグ
    
    //全消しボタンの位置を決定。rect.set(x, y, width, height)
    clearRect.set(ofGetWidth()-iconSize*2-space*2, ofGetHeight()-iconSize-space, iconSize*0.9, iconSize*0.9);
    clearWindowFlag = false; //全消しボタンを押したとき出るウィンドウ用のフラッグ　初期値はfalse
    
    //全消しボタンを押したときに出るウィンドウ内のテキスト
    textRect = c2.textBounds("最初の", 0, 0);
    int yesNoHeight = textRect.height*1.2; //はい、いいえボタンの高さ
    
    //全消しボタンを押したときに出るウィンドウの位置、サイズを決定
    int padding = space*2;
    int windowHeight = padding*2+space+iconSize+textRect.height*3+textRect.height*1.2;
    int windowWidth = windowHeight*0.9;
    clearWindowRect.set(ofGetWidth()/2-windowWidth/2, ofGetHeight()/2-windowHeight/2, windowWidth, windowHeight);
    
    //注意ボタンの位置
    alertRect.set(ofGetWidth()/2-iconSize/2, ofGetHeight()/2-clearWindowRect.height/2+padding, iconSize, iconSize);
    //注意アイコンの色
    button[2].setColor(deepPink);
    
    //テキスト（説明文）の位置
    textRect.setX(ofGetWidth()/2-clearWindowRect.width/2+padding);
    textRect.setY(ofGetHeight()/2-clearWindowRect.height/2+padding+space/2+iconSize);
    
    //テキスト（はい、いいえ）の四角の位置
    yesRect.set(ofGetWidth()/2-clearWindowRect.width/2+padding, ofGetHeight()/2+clearWindowRect.height/2-padding-yesNoHeight, (clearWindowRect.width-padding*2-space)/2, yesNoHeight);
    noRect.set(ofGetWidth()/2+space/2, ofGetHeight()/2+clearWindowRect.height/2-padding-yesNoHeight, (clearWindowRect.width-padding*2-space)/2, yesNoHeight);
    
    //テキスト（はい、いいえ）の位置（x:センター、y:トップ）
    yesTextPos = glm::vec2(yesRect.x+yesRect.width/2, yesRect.y+yesRect.height/2-textRect.height/2);
    noTextPos = glm::vec2(noRect.x + noRect.width/2, noRect.y + noRect.height/2-textRect.height/2);
}

//--------------------------------------------------------------
void ofApp::setNanoVG(){ //"ぬりえ"の文字とイラストを描く
    
    //ofxNanoVG
    ofxNanoVG::Canvas& c = canvas;
    
    c.background(1, 1, 1, 0); //背景を透明にする
    c.begin();
    
    c.fillColor(backFColor);
    c.strokeColor(strokeFColor);
    
    c.lineCap(ofxNanoVG::LineCap::ROUND);
    c.lineJoin(ofxNanoVG::LineCap::ROUND);
    c.lineWidth(10);
    
    //text（ぬりえ）
    c.push();
    c.translate(tPos.x, tPos.y);
    //c.scale(scale, scale);
    c.translate(-nurieRect.width/2, nurieRect.height/2);
    for (int i = 0; i < paths.size(); i++){
        
        vector <ofPolyline> polylines = paths[i].getOutline();
        c.beginPath();
        for (int j = 0; j < polylines.size(); j++){
            for (int k = 0; k < polylines[j].size(); k++){
                if(k == 0) c.moveTo(polylines[j][k].x, polylines[j][k].y);
                else c.lineTo(polylines[j][k].x, polylines[j][k].y);
            }
        }
        c.strokePath();
    }
    c.pop();
     
    //illust
    c.push();
    c.translate(sPos.x, sPos.y);
    c.translate(-sWidth/2, -sHeight/2);
    for (ofPolyline & line: outlines){
        c.beginPath();
        for (int j = 0; j < line.size(); j++){
            float x = ofMap(line[j].x, 0, svg.getWidth(), 0, sWidth);
            float y = ofMap(line[j].y, 0, svg.getHeight(), 0, sHeight);
            if(j == 0)c.moveTo(x, y);
            else c.lineTo(x, y);
        }
        c.fillPath();
        
        c.beginPath();
        for (int j = 0; j < line.size(); j++){
            float x = ofMap(line[j].x, 0, svg.getWidth(), 0, sWidth);
            float y = ofMap(line[j].y, 0, svg.getHeight(), 0, sHeight);
            if(j == 0)c.moveTo(x, y);
            else c.lineTo(x, y);
        }
        c.strokePath();
    }
    c.pop();
    
    c.end();

}

//--------------------------------------------------------------
void ofApp::setFbo(){
    fbo.allocate(ofGetWidth(), ofGetHeight());
    fbo.begin();
    ofPushMatrix();
    ofBackground(backColor);
    ofSetColor(255);
    canvas.draw(0, 0);
    ofPopMatrix();
    fbo.end();
    
    //FillクラスにbackColorとfboとstrokeColorを送る
    myFill.setup(fbo, strokeColor, backColor);
    
}

//--------------------------------------------------------------
void ofApp::setPattern() {
    //パターン
    currentPattern = 0; //現在のパターン
    
    patterns.resize(5);
    for (int i = 0; i < patterns.size(); i++) {
        patterns[i].allocate(ofGetWidth(), ofGetHeight(), GL_RGBA); //GL_RGBA:透明の背景にできる
    }
    
    //ノーマル
    patterns[0].begin();
    ofClear(255, 255, 255, 0); //透明の背景
    patterns[0].end();
    
    //ストライプ
    patterns[1].begin();
    ofClear(255, 255, 255, 0); //透明の背景
    int sx = 30;
    ofSetLineWidth(10);
    ofSetColor(white);
    while (sx <= ofGetWidth()*1.7) {
        ofDrawLine(sx, -30, -30, sx);
        sx += 30;
    }
    patterns[1].end();
    
    //水玉
    patterns[2].begin();
    ofClear(255, 255, 255, 0); //透明の背景
    ofSetColor(white);
    int dSize = 10; //水玉ひとつのサイズ
    int dnum = ofGetWidth()/dSize;
    for (int i = 0; i < dnum+1; i++) {
        for (int j = 0; j < dnum+1; j++) {
            if ((i + j) % 2 == 0) {
                ofDrawEllipse((dSize+5)*j, (dSize+5)*i, dSize, dSize);
            }
        }
    }
    patterns[2].end();
    
    //水玉２
    patterns[3].begin();
    ofClear(255, 255, 255, 0); //透明の背景
    patterns[2].draw(0, 0);
    patterns[3].end();
    
    //チェック
    patterns[4].begin();
    ofClear(255, 255, 255, 0); //透明の背景
    int csize = 17;
    ofSetColor(255);
    ofSetLineWidth(5);
    for (int i = 0; i <= ofGetWidth(); i+=csize) {
        ofDrawLine(i, 0, i, ofGetHeight());
    }
    for (int i = 0; i <= ofGetHeight(); i+=csize) {
        ofDrawLine(0, i, ofGetWidth(), i);
    }
    patterns[4].end();
    
    //Fillクラスにパターンのfboを送る
    myFill.setPattern(patterns);
    
    //サムネール用Fbo
    pTumbnail.resize(patterns.size());
    for (int i = 0; i < pTumbnail.size(); i++) {
        pTumbnail[i].allocate(colSize, colSize, GL_RGBA);
    }
    
    //画面に表示するサムネール用パターンのRectangle
    patternRect.resize(patterns.size());
    for (int i = 0; i < patternRect.size(); i++) {
        patternRect[i].set(colRect[0].x+colSize+space, space + space*i + colSize*i, colSize, colSize);
    }
    
    //透明度バーとカラーパレットとパターンを合わせたrectangle
    colPalRect.set(0, 0, patternRect[0].x+colSize, ofGetHeight());
}

//--------------------------------------------------------------
void ofApp::update(){
    //ofxNanoVG
    ofxNanoVG::Canvas& c2 = canvas2;
    c2.background(1, 1, 1, 0); //背景を透明にする
    c2.begin();
    
    
    //透明度バー
    c2.lineCap(ofxNanoVG::LineCap::ROUND);
    c2.lineJoin(ofxNanoVG::LineCap::ROUND);
    c2.lineWidth(alphaRect.width);
    c2.strokeColor(whiteF);
    c2.beginPath();
    c2.moveTo(alphaRect.x+space/2, alphaRect.y);
    c2.lineTo(alphaRect.x+space/2, alphaRect.y+alphaRect.height);
    c2.strokePath();
    c2.strokeColor(strokeFColor);
    c2.lineWidth(alphaRect.width-10);
    c2.beginPath();
    c2.moveTo(alphaRect.x+space/2, eY);
    c2.lineTo(alphaRect.x+space/2, alphaRect.y+alphaRect.height);
    c2.strokePath();
    //透明度バー（円）
    float eSize;
    if (alphaRect.inside(mouseX, mouseY)) { //マウスオーバー時の円の大きさ
        eSize = colSize*0.2;
    } else {
        eSize = colSize*0.16;
    }
    c2.fillColor(strokeFColor);
    c2.beginPath();
    c2.ellipse(alphaRect.x+space/2, eY, eSize ,eSize);
    c2.fillPath();
    c2.lineWidth(5);
    c2.strokeColor(whiteF);
    c2.beginPath();
    c2.ellipse(alphaRect.x+space/2, eY, eSize, eSize);
    c2.strokePath();
    
    //colors
    c2.push();
    c2.translate(colSize/2, colSize/2);
    for (int i = 0; i < colors.size(); i++) {
        //fill
        c2.fillColor(whiteF);
        c2.beginPath();
        c2.ellipse(colRect[i].x, colRect[i].y, colSize/2, colSize/2);
        c2.fillPath();
        
        c2.fillColor(ofFloatColor(colors[i].r/255.0f, colors[i].g/255.0f, colors[i].b/255.0f, currentAlpha));
        c2.beginPath();
        c2.ellipse(colRect[i].x, colRect[i].y, colSize/2, colSize/2);
        c2.fillPath();
        
        //stroke
        c2.lineWidth(5);
        c2.strokeColor(whiteF);
        c2.beginPath();
        c2.ellipse(colRect[i].x, colRect[i].y, colSize/2, colSize/2);
        c2.strokePath();
        
        //現在の色 & マウスオーバー
        if(i == currentColor || colRect[i].inside(mouseX, mouseY)){
            c2.fillColor(whiteF);
            c2.beginPath();
            c2.ellipse(colRect[i].x, colRect[i].y, colSize/6, colSize/6);
            c2.fillPath();
            
        }
    }
    c2.pop();
    
    //pattern
    for (int i = 0; i < patternRect.size(); i++) {
        c2.fillColor(backColor);
        
        //stroke
        c2.lineWidth(5);
        c2.strokeColor(whiteF);
        c2.beginPath();
        c2.rect(patternRect[i].x, patternRect[i].y, colSize, colSize);
        c2.strokePath();
        
        //現在のパターン & マウスオーバー
        if(i == currentPattern || patternRect[i].inside(mouseX, mouseY)){
            c2.fillColor(whiteF);
            c2.beginPath();
            c2.rect(patternRect[i].x+colSize/2-colSize/6, patternRect[i].y+colSize/2-colSize/6, colSize/3, colSize/3);
            c2.fillPath();
            
        }
    }
    
    //全消し用ウィンドウ
    if(clearWindowFlag){
        
        //上から下にウィンドウを出すための値
        auto duration = 0.1f; //早い 0.1f ~ 1.0f ゆっくり
        auto endTime = initTime + duration;
        auto now = ofGetElapsedTimef();
        //イージング関数早見表　https://easings.net/ja
        textWindowY = ofxeasing::map_clamp(now, initTime, endTime, -80, 0, &ofxeasing::quad::easeIn);
        windowAlpha = ofxeasing::map_clamp(now, initTime, endTime, 0, 1, &ofxeasing::quad::easeIn);
        
        //fill
        c2.push();
        c2.translate(0, textWindowY);
        c2.fillColor(ofFloatColor(windowBackColor, windowAlpha));
        c2.beginPath();
        c2.roundedRect(clearWindowRect.x, clearWindowRect.y, clearWindowRect.width, clearWindowRect.height, 10);
        c2.fillPath();
        
        //stroke
        c2.strokeColor(ofFloatColor(whiteF, windowAlpha));
        c2.lineWidth(10);
        c2.beginPath();
        c2.roundedRect(clearWindowRect.x, clearWindowRect.y, clearWindowRect.width, clearWindowRect.height, 10);
        c2.strokePath();
        
        //テキスト
        c2.textFont("round");
        c2.textAlign(ofxNanoVG::TextAlign::LEFT | ofxNanoVG::TextAlign::TOP);
        c2.push();
        c2.translate(textRect.x, textRect.y);
        c2.textSize(textSize);
        c2.fillColor(ofFloatColor(strokeFColor, windowAlpha));
        c2.text("最初の", 0, 0);
        c2.text("状態に", 0, textRect.height);
        c2.text("戻しますか？", 0, textRect.height*2);
        c2.pop();
        
        //はい、いいえの四角
        c2.lineWidth(10);
        if(yesRect.inside(mouseX, mouseY)){ //マウスオーバー
            c2.fillColor(strokeFColor);
        } else {
            c2.fillColor(ofFloatColor(whiteF, windowAlpha));
        }
        c2.beginPath();
        c2.roundedRect(yesRect.x, yesRect.y, yesRect.width, yesRect.height, 10);
        c2.fillPath();
        
        if(noRect.inside(mouseX, mouseY)){ //マウスオーバー
            c2.fillColor(strokeFColor);
        } else {
            c2.fillColor(ofFloatColor(deepPinkF, windowAlpha));
        }
        c2.beginPath();
        c2.roundedRect(noRect.x, noRect.y, noRect.width, noRect.height, 10);
        c2.fillPath();
        
        //はい、いいえのテキスト
        c2.textAlign(ofxNanoVG::TextAlign::CENTER | ofxNanoVG::TextAlign::TOP);
        c2.textSize(textSize);
        if(yesRect.inside(mouseX, mouseY)){ //マウスオーバー（はい）
            c2.fillColor(whiteF);
        } else {
            c2.fillColor(ofFloatColor(strokeFColor, windowAlpha));
        }
        c2.text("はい", yesTextPos.x, yesTextPos.y);
        c2.fillColor(ofFloatColor(whiteF, windowAlpha));
        c2.text("いいえ", noTextPos.x, noTextPos.y);
        
        c2.pop();
    }
    
    //保存ボタンを押したときに出るテキスト
    
    if (saveTextFlag) {
        auto now = ofGetElapsedTimef();
        auto duration = 0.2f; //早い 0.1 ~ 1.0 ゆっくり（fはfloat型を意味する、つけないとdoubleになる）
        auto endTime = initTime2 + duration;
        auto initTime3 = endTime+2.5f;
        auto endTime2 = initTime3+duration;
        //出るときのモーション
        saveTextX= ofxeasing::map_clamp(now, initTime2, endTime, ofGetWidth(), saveTextRect.x-20, &ofxeasing::sine::easeIn);
        
        //戻るときのモーション
        if(now > endTime) saveTextX = ofxeasing::map_clamp(now, initTime3, endTime2, saveTextRect.x-20, ofGetWidth(), &ofxeasing::linear::easeIn);
        
        if (now > endTime2) saveTextFlag = false;
        
        c2.fillColor(ofFloatColor(strokeColor, 1));
        c2.beginPath();
        c2.roundedRect(saveTextX, saveTextRect.y, saveTextRect.width+40, saveTextRect.height, 5);
        c2.fillPath();
        c2.textSize(textSize);
        c2.textAlign(ofxNanoVG::TextAlign::LEFT | ofxNanoVG::TextAlign::TOP);
        c2.fillColor(ofFloatColor(whiteF, 1));
        c2.text("data/にスクリーンショットを保存しました", saveTextX+20, saveTextRect.y);
    }
    
    c2.end();
    
}
//--------------------------------------------------------------
void ofApp::drawPattern() {
    //ノーマル
    pTumbnail[0].begin();
    ofBackground(colors[currentColor]);
    pTumbnail[0].end();
    
    //ストライプ
    pTumbnail[1].begin();
    ofBackground(gray);
    ofSetColor(colors[currentColor]);
    patterns[1].draw(0, 0);
    pTumbnail[1].end();
    
    //水玉
    pTumbnail[2].begin();
    ofBackground(gray);
    ofSetColor(colors[currentColor]);
    patterns[2].draw(0, 0);
    pTumbnail[2].end();
    
    //水玉２
    pTumbnail[3].begin();
    ofBackground(colors[currentColor]);
    ofSetColor(white);
    patterns[3].draw(0, 0);
    pTumbnail[3].end();
    
    //チェック
    pTumbnail[4].begin();
    ofBackground(gray);
    ofSetColor(colors[currentColor]);
    patterns[4].draw(0, 0);
    pTumbnail[4].end();
    
    for (int i = 0; i < patternRect.size(); i++) {
        pTumbnail[i].draw(patternRect[i].x, patternRect[i].y);
    }
    
}
//--------------------------------------------------------------
void ofApp::draw(){
    
    ofSetColor(255);
    
    myFill.draw(); //ぬりえ部分
    
    //もしsaveボタンを押したら、画面キャプチャをとって、saveFlagをfalseに戻す
    if (saveFlag) {
        ofImage myImage;
        myImage.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
        myImage.save(ofGetTimestampString("%Y%m%d%H%M%S")+"##.png");
        
        saveFlag = false;
    }
    
    //パターン
    drawPattern();
    canvas2.draw(0, 0); //色パレット、ウィンドウ
    
    //全消しボタンの色（マウスオーバーで色が変わる）
    if (clearRect.inside(mouseX, mouseY)) {
        button[0].setColor(strokeColor);
    } else {
        ofColor bColor = ofColor(strokeColor, 160);
        button[0].setColor(bColor);
    }
    //保存ボタンの色（マウスオーバーで色が変わる）
    if (saveRect.inside(mouseX, mouseY)) {
        button[1].setColor(strokeColor);
    } else {
        ofColor bColor = ofColor(strokeColor, 160);
        button[1].setColor(bColor);
    }
    
    //全消しボタン
    button[0].draw(clearRect.x, clearRect.y);
    //保存ボタン
    button[1].draw(saveRect.x, saveRect.y);
    
    //注意アイコン
    if(clearWindowFlag){
        ofPushMatrix();
        ofTranslate(0, textWindowY);
        button[2].setFillColor(ofColor(deepPink, windowAlpha*255));
        //button[3].draw(alertRect.x, alertRect.y);
        button[2].draw(alertRect.x, alertRect.y);
        ofPopMatrix();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    //スクリーンショット
    if (key == 'S' || key == 's') {
            ofImage myImage;
            myImage.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
            myImage.save(ofGetTimestampString("%Y%m%d%H%M%S")+"##.png");
        }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    if (alphaRect.inside(x, y)) {
        //透明度のバーの丸の位置を変える
        changeAlphaEllipse(y);
    }

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    if (clearRect.inside(x, y)) {
        if(!clearWindowFlag) initTime = ofGetElapsedTimef();
        clearWindowFlag = true; //全消しウィンドウのフラッグをtrueにする
    }
    
    if (clearWindowFlag == true){ //全消しウィンドウが出てるとき
        if (yesRect.inside(x, y)) { //はいボタンを押したとき
            //色を初期値に戻す
            myFill.setup(fbo, strokeColor, backColor);
            clearWindowFlag = false;
        } else if(noRect.inside(x, y)){ //いいえボタンを押したとき
            clearWindowFlag = false;
        }
        
    } else { //全消しウィンドウが出てないとき有効
        
        for(int i = 0; i < colRect.size(); i++){ //カラーパレット
            if (colRect[i].inside(x, y)) {
                currentColor = i;
            }
        }
        
        for(int i = 0; i < patternRect.size(); i++){ //パターン
            if (patternRect[i].inside(x, y)) {
                currentPattern = i;
            }
        }
        
        if (saveRect.inside(x, y)) {
            saveFlag = true; //saveフラッグをtrueにする
            saveTextFlag = true; //テキストフラッグをtrueにする
            initTime2 = ofGetElapsedTimef();
        }
        
        if (alphaRect.inside(x, y)) {
            //透明度のバーの丸の位置を変える
            changeAlphaEllipse(y);
        }
        
        if (!saveRect.inside(x, y) && !alphaRect.inside(x, y) && !clearRect.inside(x, y) && !colPalRect.inside(x, y)) { //色で塗りつぶす（アイコンのない場所を押したとき）
            ofColor newColor = ofColor(colors[currentColor], currentAlpha*255);
            myFill.setPos(x, y, newColor, currentPattern);
        }
    }

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::changeAlphaEllipse(int y){ //透明度バーの丸の位置を変える
    eY = y;
    //float _currentAlpha = ofMap(y, alphaRect.y+alphaRect.height, alphaRect.y, 1, 0);
    float _currentAlpha = ofMap(y, alphaRect.y+alphaRect.height, alphaRect.y, 0.4, 1);
    currentAlpha = round(_currentAlpha * 10) / 10; //小数点第２位で四捨五入
}
//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
