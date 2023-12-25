#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "SPI.h"
#include "TFT_eSPI.h"
#include "U8g2_for_TFT_eSPI.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <algorithm>  // For std::count
#include <vector>
#include <string>
#include "functions.h"


// 定義 LCD 物件
//TFT_eSPI tft = TFT_eSPI(); 
//U8g2_for_TFT_eSPI u8f;

// 定義 Wi-Fi
const char* ssid = "";
const char* password = "";

// 定義 Google Maps API 金鑰和方向查詢參數
const char *apikey = "";
const String origin = "出發";
const String destination = "終點";
/*
// 定義方向指示
enum direction
{
  left =-1, straight, right
};

// 定義狀態
enum driving_mode
{
  ontheway=1 , touchdown
};
*/
//定義初始+布林值
/*
bool isFetchingData = false;
bool arrived = false;
int currentStep = 0;
int flag=ontheway; //1:ontheway,2: touchdown next step
int total_dirvalue = 0;
*/
// JSON 解析器
//String jsonStr = ""; // 保存完整的JSON字符串
//HTTPClient http; // 在这里声明 HTTPClient 对象
DynamicJsonDocument doc(16384); // JSON解析的缓冲区

// 函數原型
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
/*
void LCDupdate(){
  tft.begin(); // Initialize TFT
  tft.setRotation(1); // Set screen rotation 90
  tft.fillScreen(TFT_BLACK); // Clear the screen
  u8f.begin(tft); // Connect u8g2 to TFT_eSPI
  u8f.setFontMode(0);
  u8f.setFontDirection(0);
  u8f.setFont(u8g2_font_unifont_t_chinese1);  
}
*/
/*
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
*/
void setup() {
  LCDupdate();
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("已连接到WiFi");
}

void loop() {
  if (!isFetchingData) {
    String apiKeyParam = "&key=" + String(apikey);
    String languageParam = "&language=zh-TW";
    String url = "https://maps.googleapis.com/maps/api/directions/json?origin=" + origin + "&destination=" + destination + apiKeyParam + languageParam;
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode == 200) {
      String jsonStr = http.getString();
      DeserializationError err = deserializeJson(doc, jsonStr);
      if (err) {
        Serial.print("JSON解析错误：");
        Serial.println(err.c_str());
      } else {
        isFetchingData = true;
        currentStep = 0;
      } // deserialized success
    } else {
      Serial.print("HTTP错误代码：");
      Serial.println(httpCode);
      delay(3000);
    } // http correct
  } else {
    JsonArray legs = doc["routes"][0]["legs"][0]["steps"];
    int *dis = new int; // 分配記憶體空間
    *dis = 0;
    int total = legs.size();
    Serial.print("total:");
    Serial.println(total);
    if (currentStep < total) {
      flag = sim_position(flag, currentStep);
      currentStep = check_position(legs[currentStep], total, flag, legs, currentStep, dis);
    } else {
      Serial.println("已到达目的地！");
      arrived = true;
      Serial.begin(115200);
      LCDupdate();
      u8f.print("已到達目的地!");
      
      } //arrive
      delete dis;
    } // fetching complete
} 
