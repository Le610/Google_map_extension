#include <Arduino.h>
#include "functions.h"
#include <Wire.h>
#include "SPI.h"

//#include <Adafruit_GFX.h>
//#include <Adafruit_ST7735.h>
#include <algorithm>  // For std::count
#include <vector>
#include <string>

enum direction
{
  left =-1, straight, right
};

// 定義狀態
enum driving_mode
{
  ontheway=1 , touchdown
};

bool isFetchingData = false;
bool arrived = false;
int currentStep = 0;
int flag=ontheway; //1:ontheway,2: touchdown next step
int total_dirvalue = 0;
//TFT_eSPI tft;
TFT_eSPI tft = TFT_eSPI(); 
U8g2_for_TFT_eSPI u8f;

void drawText(const char* text, uint16_t color, uint8_t size, bool wrap) {
tft.setTextColor(color);
tft.setTextSize(size);
tft.setTextWrap(wrap);
tft.print(text);
}

String htmlEntitiesDecode(String input) {
  String output = input;
  output.replace("&quot;", "\"");
  output.replace("&amp;", "&");
  output.replace("&lt;", "<");
  output.replace("&gt;", ">");
  return output;
}

String removeHTMLTags(String input) {
  int start = input.indexOf('<');
  int end = input.indexOf('>', start);
  while (start >= 0 && end >= 0) {
    input.remove(start, end - start + 1);
    start = input.indexOf('<');
    end = input.indexOf('>', start);
  }
  return input;
}

int sim_position(int flag, int currentStep) {
if (flag==ontheway){
flag=touchdown;
Serial.println("行進中");
Serial.print("準備切換為flag:");
Serial.println(flag);
}
else if (flag==touchdown){
Serial.print("抵達路線,currenstep=");
Serial.println(currentStep);
//currentStep++;
flag=ontheway;
Serial.print("準備切換為flag:");
Serial.println(flag);
}
return flag;
}


void LCDupdate(){
  tft.begin(); // Initialize TFT
  tft.setRotation(1); // Set screen rotation 90
  tft.fillScreen(TFT_BLACK); // Clear the screen
  u8f.begin(tft); // Connect u8g2 to TFT_eSPI
  u8f.setFontMode(0);
  u8f.setFontDirection(0);
  u8f.setFont(u8g2_font_unifont_t_chinese1);  
}

void road_drawing_st7735(String dir, String road, String roadinfor, int i, int total_dirvalue) {
  static int start_lx,start_rx,start_y;
  static int dir_guide = straight; 
  uint32_t TFT_color; 
  static int k;
  //if ( float(i)/3.0== (float)currentStep ){
  if(i == currentStep){ //first dir will always be straight 
    LCDupdate();
    Serial.print("i == currentStep:(i,currentStep)");
    Serial.print(i);
    Serial.print(",");
    Serial.println(currentStep);
    switch(total_dirvalue){
      case -2://2 lefts
        start_lx=63,start_rx=69,start_y=110;//lx+rx/2=66
        break;
      case 2://2 rights
        start_lx=5,start_rx=11,start_y=110;//lx+rx/2=8
        break;
      default://else
        start_lx=34,start_rx=40,start_y=110;//lx+rx/2=37
    }
  tft.fillTriangle(start_lx - 3, start_y + 10, start_rx + 3, start_y + 10, (start_lx + start_rx) / 2, start_y, TFT_GREEN);// 60 120 72 153 66 110 
  for (int x = start_lx; x <= start_rx; x++) {
    tft.drawFastVLine(x, start_y-35, 35, TFT_BLUE); // 63 75 110
  }
    start_y=start_y-35;//75
    //myDirection1 = straight;
    dir_guide = straight;
    Serial.println("begin");
    Serial.print("start_y:");
    Serial.println(start_y);
    Serial.print("start_lx:");
    Serial.println(start_lx);
    Serial.print("start_rx:");
    Serial.println(start_rx);
    TFT_color=TFT_BLUE;
    k=0;
    Serial.print("k 歸0後,k=");
    Serial.println(k);
  } else{ // i!=currentStep
    k++;
    Serial.print("k++後,k=");
    Serial.println(k);
    //uint32_t TFT_color;
    if(i == currentStep+1)  
      TFT_color=TFT_ORANGE;
    else  
      TFT_color=TFT_CYAN;

  //dir judge
  
    if (strcmp(dir.c_str(), "keep-left") == 0) { //draw V + right slash
     if (dir_guide == straight){
      for (int x=start_lx;x<=start_rx;x++){
        tft.drawFastVLine(x,start_y-35,35, TFT_color); // start_lx 40 75
        tft.drawLine(x,start_y,x+29,start_y-35, TFT_WHITE);// start_lx,75,start_lx+29,40
      }      
      tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  66 75 102 92
      start_y=start_y-35;//40
      dir_guide = straight;
    }
    else if (dir_guide == right){
      //start_lx=start_rx;
        for (int y=-3;y<=3;y++){
          tft.drawFastHLine(start_lx,start_y+y,29,TFT_color); // 5 75 63
        }
        for (int x=start_lx;x<=start_rx;x++){
        tft.drawLine(x,start_y,x+29,start_y+35, TFT_WHITE);//start_lx,75,start_lx+29,40
        }
        tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80
        dir_guide =left;
        start_lx=start_lx+29;//63
        start_rx=start_lx+6;//69
        start_y=start_y;//75
    }
    else {
      //start_lx=start_rx;
        for (int y=-3;y<=3;y++){
        tft.drawFastHLine(start_lx-29,start_y+y,29,TFT_color); // 5 75 63
        }
        for (int x=start_lx;x<=start_rx;x++){
        tft.drawLine(x,start_y,x-29,start_y-35, TFT_WHITE);//start_lx,75,start_lx+29,40
        }
        tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80
        dir_guide=left;
        start_lx=start_lx-29;
        start_rx=start_lx+6;
        start_y=start_y;
    }
    Serial.println("keep-left");
    Serial.print("start_y:");
    Serial.println(start_y);
    Serial.print("start_lx:");
    Serial.println(start_lx);
    Serial.print("start_rx:");
    Serial.println(start_rx);
  }
  else if (strcmp(dir.c_str(), "keep-right") == 0) { //draw V + left slash
  if (dir_guide ==straight){
    for (int x=start_lx;x<=start_rx;x++){
      tft.drawFastVLine(x,start_y-35,35,TFT_color); // start_lx 40 75
      tft.drawLine(x,start_y,x-29,start_y-35,TFT_WHITE);// start_lx,75,start_lx-29,40
    }
    tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  66 75 102 92
    start_y=start_y-35;//40
    dir_guide = straight;
    }
    else if (dir_guide == right){
      //start_lx=start_rx;
        for (int y=-3;y<=3;y++){
          tft.drawFastHLine(start_lx,start_y+y,29,TFT_color); // 34 75 63
        }
        for (int x=start_lx;x<=start_rx;x++){
        tft.drawLine(x,start_y,x+29,start_y-35, TFT_WHITE);//start_lx,75,start_lx+29,40
        }
        tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 34 29 39
        dir_guide =left;
        start_lx=start_lx+29;//63
        start_rx=start_lx+6;//69
        start_y=start_y;//75
    }
    else {
      //start_lx=start_rx;
        for (int y=-3;y<=3;y++){
        tft.drawFastHLine(start_lx-29,start_y+y,29,TFT_color); // 34 75 63
        }
        for (int x=start_lx;x<=start_rx;x++){
        tft.drawLine(x,start_y,x-29,start_y+35, TFT_WHITE);//start_lx,75,start_lx+29,110
        }
        tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80
        dir_guide=left;
        start_lx=start_lx-29;
        start_rx=start_lx+6;
        start_y=start_y;
    }
    Serial.println("keep-right");
    Serial.print("start_y:");
    Serial.println(start_y);
    Serial.print("start_lx:");
    Serial.println(start_lx);
    Serial.print("start_rx:");
    Serial.println(start_rx);
  }
  else if (strcmp(dir.c_str(), "ramp-right") == 0) {
    if (dir_guide ==straight ){
    for (int x=start_lx;x<=start_rx;x++){
        tft.drawLine(x,start_y,x+29,start_y-35, TFT_color);//start_lx,75,start_lx-29,40
        }
        tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80
        dir_guide=straight;
        start_lx=start_lx+29;
        start_rx=start_lx+6;
        start_y=start_y-35;
    }else if (dir_guide == right){
      for (int x=start_lx;x<=start_rx;x++){
        tft.drawLine(x,start_y,x+29,start_y+35, TFT_color);//start_lx,75,start_lx+29,120
        }
        tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80
        dir_guide=straight;
        start_lx=start_lx+29;
        start_rx=start_lx+6;
        start_y=start_y+35;

    }
    else{
      for (int x=start_lx;x<=start_rx;x++){
        tft.drawLine(x,start_y,x-29,start_y-35, TFT_color);//start_lx,75,start_lx-29,40
        }
        tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80
        dir_guide=straight;
        start_lx=start_lx-29;
        start_rx=start_lx+6;
        start_y=start_y-35;
    }
    Serial.println("ramp-right");
    Serial.print("start_y:");
    Serial.println(start_y);
    Serial.print("start_lx:");
    Serial.println(start_lx);
    Serial.print("start_rx:");
    Serial.println(start_rx);
  }
  else if (strcmp(dir.c_str(), "turn-left") == 0) {
    if (dir_guide == straight){
      for (int y=-3;y<=3;y++){
        tft.drawFastHLine(start_lx-29,start_y+y,29,TFT_color); // 34 75 63
      }   
      tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  66 75 70 80
      start_lx=start_lx-29;//34
      start_rx=start_lx+6;
      dir_guide = left;
    }
    else if (dir_guide == right){
      for (int x=start_lx;x<=start_rx;x++){
        tft.drawFastVLine(x,start_y-35,35,TFT_color); // start_lx 40 75
        }
      tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80   tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80
      dir_guide =straight;
      start_y=start_y-35;//40
    }
    else {
      for (int x=start_lx;x<=start_rx;x++){
        tft.drawFastVLine(x,start_y,35,TFT_color); // start_lx 75 110
        }
      tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80   tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80
      dir_guide =straight;
      start_y=start_y+35;//40
    }
    Serial.println("turn-left");
    Serial.print("start_y:");
    Serial.println(start_y);
    Serial.print("start_lx:");
    Serial.println(start_lx);
    Serial.print("start_rx:");
    Serial.println(start_rx);
  }
  else if (strcmp(dir.c_str(), "turn-right") == 0){
    if (dir_guide == straight){
      for (int y=-3;y<=3;y++){
        tft.drawFastHLine(start_lx,start_y+y,29,TFT_color); // 34 75 63
      }   
      tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  66 75 70 80
      start_lx=start_lx+29;//63
      start_rx=start_lx+6;
      dir_guide = right;
    }
    else if (dir_guide == right){
      for (int x=start_lx;x<=start_rx;x++){
        tft.drawFastVLine(x,start_y,35,TFT_color); // start_lx 75 110
        }
      tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80   tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80
      dir_guide =straight;
      start_y=start_y+35;//110
    }
    else {
      for (int x=start_lx;x<=start_rx;x++){
        tft.drawFastVLine(x,start_y-35,35,TFT_color); // start_lx  40 75
        }
      tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80   tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80
      dir_guide =straight;
      start_y=start_y-35;//40
    }
    Serial.println("turn-right");
    Serial.print("start_y:");
    Serial.println(start_y);
    Serial.print("start_lx:");
    Serial.println(start_lx);
    Serial.print("start_rx:");
    Serial.println(start_rx);
  }
  else if (strcmp(dir.c_str(), "fork-left") == 0) { //draw V + left slash
  if (dir_guide ==straight){
    for (int x=start_lx;x<=start_rx;x++){
      tft.drawFastVLine(x,start_y-35,35,TFT_WHITE); // start_lx 40 75
      tft.drawLine(x,start_y,x-29,start_y-35,TFT_color);// start_lx,75,start_lx-29,40
    }
    tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  66 75 102 92
    start_y=start_y-35;//40
    start_lx=start_lx-29;//5
    start_rx=start_lx+6;//11
    dir_guide = straight;
    }
    else if (dir_guide == right){
      //start_lx=start_rx;
        for (int y=-3;y<=3;y++){
          tft.drawFastHLine(start_lx,start_y+y,29,TFT_WHITE); // 34 75 63
        }
        for (int x=start_lx;x<=start_rx;x++){
        tft.drawLine(x,start_y,x+29,start_y-35, TFT_color);//start_lx,75,start_lx+29,40
        }
        tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 34 75 29 39
        dir_guide =straight;
        start_lx=start_lx+29;//63
        start_rx=start_lx+6;//69
        start_y=start_y-35;//40
    }
    else {
      //start_lx=start_rx;
        for (int y=-3;y<=3;y++){
        tft.drawFastHLine(start_lx-29,start_y+y,29,TFT_WHITE); //5 34 75 
        }
        for (int x=start_lx;x<=start_rx;x++){
        tft.drawLine(x,start_y,x-29,start_y+35, TFT_color);//start_lx,75,start_lx+29,110
        }
        tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80
        dir_guide=straight;
        start_lx=start_lx-29;
        start_rx=start_lx+6;
        start_y=start_y+35;
    }
    Serial.println("fork-left");
    Serial.print("start_y:");
    Serial.println(start_y);
    Serial.print("start_lx:");
    Serial.println(start_lx);
    Serial.print("start_rx:");
    Serial.println(start_rx);
   }
  else if (strcmp(dir.c_str(), "fork-right") == 0) { //draw V + left slash
  if (dir_guide ==straight){
    for (int x=start_lx;x<=start_rx;x++){
      tft.drawFastVLine(x,start_y-35,35,TFT_WHITE); // start_lx 40 75
      tft.drawLine(x,start_y,x+29,start_y-35,TFT_color);// 34,75,63,40
    }
    tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  66 75 102 92
    start_y=start_y-35;//40
    start_lx=start_lx+29;//63
    start_rx=start_lx+6;//69
    dir_guide = straight;
    }
    else if (dir_guide == right){
      //start_lx=start_rx;
        for (int y=-3;y<=3;y++){
          tft.drawFastHLine(start_lx,start_y+y,29,TFT_WHITE); // 34 75 63
        }
        for (int x=start_lx;x<=start_rx;x++){
        tft.drawLine(x,start_y,x+29,start_y-35, TFT_color);//start_lx,75,start_lx+29,40
        }
        tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 34 75 29 39
        dir_guide =straight;
        start_lx=start_lx+29;//63
        start_rx=start_lx+6;//69
        start_y=start_y-35;//40
    }
    else {
      //start_lx=start_rx;
        for (int y=-3;y<=3;y++){
        tft.drawFastHLine(start_lx-29,start_y+y,29,TFT_WHITE); //5 34 75 
        }
        for (int x=start_lx;x<=start_rx;x++){
        tft.drawLine(x,start_y,x-29,start_y-35, TFT_color);//start_lx,75,start_lx+29,40
        }
        tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80
        dir_guide=straight;
        start_lx=start_lx-29;
        start_rx=start_lx+6;
        start_y=start_y-35;
    }
    Serial.println("fork-right");
    Serial.print("start_y:");
    Serial.println(start_y);
    Serial.print("start_lx:");
    Serial.println(start_lx);
    Serial.print("start_rx:");
    Serial.println(start_rx);
    }
    else{
    if (dir_guide ==straight){
    for (int x=start_lx;x<=start_rx;x++){
        tft.drawFastVLine(x,start_y-35,35, TFT_color);//start_lx,75,40
        }
        tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80
        dir_guide=straight;
        start_y=start_y-35;
    }
    else if (dir_guide == right){
      for (int y=-3;y<=3;y++){
        tft.drawFastHLine(start_lx,start_y+y,29,TFT_color); // 34 75 63
      }
      tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 34 75 29 39
      dir_guide =right;
      start_lx=start_lx+29;//63
      start_rx=start_lx+6;//69
      start_y=start_y;//40
    }
    else {
      //start_lx=start_rx;
        for (int y=-3;y<=3;y++){
        tft.drawFastHLine(start_lx-29,start_y+y,29,TFT_WHITE); //5 34 75 
        }
        tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color); //  start_lx 75 70 80
        dir_guide=left;
        start_lx=start_lx-29;
        start_rx=start_lx+6;
        start_y=start_y;
    }
    Serial.println("else");
    Serial.print("start_y:");
    Serial.println(start_y);
    Serial.print("start_lx:");
    Serial.println(start_lx);
    Serial.print("start_rx:");
    Serial.println(start_rx);
  
  
  }//else string
  tft.fillCircle((start_lx + start_rx)/2, start_y, 5, TFT_color);
  }// i !=currentStep
  
  //road judge
  tft.setCursor(75, 115);
  char*C="Start here";
  drawText(C,TFT_GREEN,1.6,true);
  tft.setCursor(72, 85-40*k);//show roadinfor
  //Serial.print("roadinfor座標:");
  //Serial.println(85-40*k);
  //String road_infor = roadinfor; // 使用 Arduino 的 String
  drawText(roadinfor.c_str(), TFT_color, 1.5, false);
  //Serial.print("roadinfor:drawing");
  //Serial.println(roadinfor); // 转换为 const char*
  u8f.setCursor(72, 108-38*k);//show roadname
  //Serial.print("road座標:");
  //Serial.println(108-38*k);
  u8f.setForegroundColor(TFT_color);
  int len = road.length();
  if (len <= 15){
  //Serial.print("road:drawing<=15");
  //Serial.println(road);
  u8f.print(road.c_str());
  } else{
    char truncatedString[16];
    for (int k=0;k<15;k++)
    truncatedString[k] = road[k];
    truncatedString[15] = '\0';
    //Serial.print("road:drawing>16");
    //Serial.println(truncatedString);
    u8f.print(truncatedString);
    }//else direction
} //end road_drawing

int precheck_dirvalue(JsonObject step, int *countR, int *countL,int *countRR) {
  String dir = htmlEntitiesDecode(step["maneuver"].as<String>());
  String showR[] = {"turn-right", "fork-left", "fork-right", "ramp-right", "fork-right", "keep-left", "keep-right"};
  String showRR[] = { "ramp-right"};
  String showL[] = {"turn-left", "fork-left", "fork-right", "ramp-right", "fork-right", "keep-left", "keep-right"};
  *countR += std::count(std::begin(showR), std::end(showR), dir);
  *countL += std::count(std::begin(showL), std::end(showL), dir);
  *countRR += std::count(std::begin(showRR), std::end(showRR), dir);
  Serial.print("countR: ");
  Serial.println(*countR);
  Serial.print("countL: ");
  Serial.println(*countL);
  int total_dirvalue;
    if (*countRR >= 2 ) {
        total_dirvalue = 2;     
    }else if (*countR >= 2 && *countL >= 2) {
        total_dirvalue = 0;     
    }  else if (*countL >= 2) {
        total_dirvalue = -2;
    } else if (*countR >= 2) {
        total_dirvalue = 2;
    } else {
        total_dirvalue = 0;
    }
    Serial.print("total_dirvalue: ");
    Serial.println(total_dirvalue);
    return total_dirvalue;
}// end precheck_dirvalue

String extractRoadName(JsonObject step) {
  String road1 = removeHTMLTags(htmlEntitiesDecode(step["html_instructions"].as<String>()));
  String keywords[] = {"進入", "繼續走","接著走","繼續行駛","直行走","朝","循著","沿著","匝道上","繼續沿","走","轉"};
  // 遍歷關鍵字陣列
  for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
    // 查找關鍵字
    int keywordIndex = road1.indexOf(keywords[i]);
    if (keywordIndex != -1) {
      // 取得 "進入" 之後的字串
      road1 = road1.substring(keywordIndex + keywords[i].length());
      // 截取前 6 個字串
      if (road1.length() > 18){
      road1 = road1.substring(0,18);
      }
      // 返回路名
      return road1;
     }
  }
  // 未找到路名
  return road1;
}//end extractRoadName

void processStep(JsonObject step, int j, int total_dirvalue) {
  String Getroadname;
  String road_infor = "";
  Serial.println("步骤：");
  Serial.println("起始位置：" + step["start_location"]["lat"].as<String>() + ", " + step["start_location"]["lng"].as<String>());
  Serial.println("结束位置：" + step["end_location"]["lat"].as<String>() + ", " + step["end_location"]["lng"].as<String>());
  Serial.println("出行方式：" + step["travel_mode"].as<String>());
  String dir = htmlEntitiesDecode(step["maneuver"].as<String>());
  String road = removeHTMLTags(htmlEntitiesDecode(step["html_instructions"].as<String>()));
  Serial.println("方向指示：" + dir);
  Serial.println("路段：" + road);
  Serial.println("距離：" + step["distance"]["text"].as<String>());
  Serial.println("持续时间：" + step["duration"]["text"].as<String>());
  Serial.println("--------------");
  //String road_infor= step["distance"]["text"].as<String>()+step["duration"]["text"].as<String>();
  int distance=step["distance"]["value"].as<int>(); 
  int duration = step["duration"]["value"].as<int>();
  String englishdistance;
  String englishDuration;
  if (distance > 1000){
      float distance1=distance/1000.0;
      englishdistance = String(distance1) + "km";
      //Serial.println("路長：" + englishdistance);
    }
    else {
      englishdistance = String(distance) + "m";
      //Serial.println("路長：" + englishdistance);
    }
  
    if (duration > 60){
      float duration1=duration/60.0;
      englishDuration = String(duration1) + "mins";
      //Serial.println("持续时间：" + englishDuration);
    }
    else {
      englishDuration = String(duration) + "secs";
      //Serial.println("持续时间：" + englishDuration);
    }
    road_infor.reserve(50);  // 为 road_infor 分配足够的内存
    road_infor.concat(englishdistance);
    road_infor.concat(", ");
    road_infor.concat(englishDuration);
    //Serial.print("road_infor:");
    //Serial.println(road_infor);
    //Serial.println(road_infor);
    Getroadname=extractRoadName(step);
    //Serial.print("Getroadname:");
    //Serial.println(Getroadname);
    road_drawing_st7735(dir, Getroadname,road_infor,j, total_dirvalue);
    
}//end processStep

int check_position(JsonObject step, int total, int flag, JsonArray legs, int currentStep, int *dis) {
 int total_dirvalue = 0;
   if (flag == touchdown) {
     Serial.println("進入flag2:");
     int* countR = new int(0);
     int* countL = new int(0);
     int* countRR = new int(0);
     for (int k = currentStep+1; k < currentStep + 3; k++) 
       total_dirvalue +=precheck_dirvalue(legs[k], countR, countL,countRR);
     for (int j = currentStep; j < currentStep + 3; j++) 
        processStep(legs[j],j,total_dirvalue);
      //delay(3000);  // 添加延时
      delete countR;
      delete countL;         
    } else if (flag == ontheway) {
        Serial.println("進入flag1:");
        float n1;
        JsonObject distance = step["routes"][0]["legs"][0]["distance"];
        *dis = step["distance"]["value"].as<int>();
        Serial.print("距離");
        Serial.print(*dis);
        Serial.println("公尺");
        Serial.print("等待");
        n1 = *dis / 11.11;
        Serial.print(n1);
        Serial.println("秒");
        delay(n1 * 1000); // sec
        currentStep++;
        Serial.print("currentStep:");
        Serial.println(currentStep);
    }
   return currentStep;
}//end check_position
