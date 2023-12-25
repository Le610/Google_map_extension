#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <ArduinoJson.h>
#include "TFT_eSPI.h"
#include "U8g2_for_TFT_eSPI.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// 定義 LCD 物件
extern TFT_eSPI tft ; 
extern U8g2_for_TFT_eSPI u8f;

//定義初始+布林值
extern bool isFetchingData;
extern bool arrived;
extern int currentStep;
extern int flag; //1:ontheway,2: touchdown next step
extern int total_dirvalue ;

void drawText(const char* text, uint16_t color, uint8_t size, bool wrap);
void LCDupdate();
String htmlEntitiesDecode(String input);
String removeHTMLTags(String input);
int sim_position(int flag, int currentStep);
void road_drawing_st7735(String dir, String road, String roadinfor, int i, int total_dirvalue);
int precheck_dirvalue(JsonObject step, int *countR, int *countL,int *countRR);
String extractRoadName(JsonObject step);
void processStep(JsonObject step, int j, int total_dirvalue);
int check_position(JsonObject step, int total, int flag, JsonArray legs, int currentStep, int *dis);

#endif
