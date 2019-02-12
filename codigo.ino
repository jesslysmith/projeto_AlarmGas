#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "Led.h"
#include "Buzzer.h"
#include <LiquidCrystal.h>
const int sensor = A0;
Led led(D1);
Buzzer buzzer(D2);
const char* SSID = "Net Virtua 38";
const char* PASS = "kramer32350551";
HTTPClient http;
const String LOCATION = "http://alarmgas.herokuapp.com/";
const String UPLOAD = "upload";
const String DOWNLOAD = "download";
const String LED = "\"LED\":";
const String BUZZER = "\"BUZZER\":";
const String T_OFF = "\"T_OFF\":";
const String SENSOR = "\"SENSOR\":";
const int gatilhosensor = 380; //valor de gatilho
const int rs = D3, en = D0, d4 = D4, d5 = D5, d6 = D6, d7 = D7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

String JSON_INDICATORS(){

   return "{" +
      LED + String(led.status()) + "," +
      BUZZER + String(buzzer.status()) +
      "}";
 }
 
String JSON_SENSOR(){
 int sensor_value = analogRead(sensor);
 
    return "{" +
      SENSOR + String(sensor_value) +
      "}";
}

void desligarAlarme(String comando){
   if(comando.startsWith("{") && comando.endsWith("}")){
      if(comando.indexOf(T_OFF + " 0") > 0){
          led.apagar();
          buzzer.desligar();
      }
   }
}

void uploadINDICATORS(){

   http.begin(LOCATION+UPLOAD);
   http.addHeader("Content-Type", "application/json");
   http.POST(JSON_INDICATORS());
   http.end();
}

void uploadSENSOR(){

   http.begin(LOCATION+UPLOAD);
   http.addHeader("Content-Type", "application/json");
   http.POST(JSON_SENSOR());
   http.end();
}

String downloadWEB(){

   http.begin(LOCATION+DOWNLOAD);
   http.GET();
   String payload = http.getString();
   http.end();
   return payload;

}

void setup() {
  pinMode(sensor, INPUT);
  Serial.begin(115200);
  lcd.begin(16,2);
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Aguardando conexão");
  }
  delay(2000);
}

void loop() {
  int analogSensor = analogRead(sensor);
  if(Serial.available() > 0){
        String comando = Serial.readString();
        desligarAlarme(comando);
        Serial.print(JSON_SENSOR());
        uploadSENSOR();
        delay(10);
        Serial.print(JSON_INDICATORS());
        uploadINDICATORS();
  }
  lcd.clear();
  //flag=0;
  lcd.print("Em funcionamento!");
  lcd.setCursor(0, 1);
  delay (200);
  Serial.print(JSON_SENSOR());
  uploadSENSOR();
  delay(10);
  Serial.print(JSON_INDICATORS());
  uploadINDICATORS();
  
  while(analogSensor > gatilhosensor) // confronto entre valor de gatilho e o valor recebido pelo sensor
  {
    led.acender();
    buzzer.acionar();
    Serial.print(JSON_SENSOR());
    uploadSENSOR();
    delay(10);
    Serial.print(JSON_INDICATORS());
    uploadINDICATORS();
    delay(10);
    while(led.status() == 1){
      lcd.clear();
      lcd.print("PERIGO! NIVEL ");
      lcd.setCursor(0, 1);
      lcd.print ("ALARMANTE!");
      Serial.println(JSON_SENSOR());
      uploadSENSOR();
      delay(10);
      Serial.print(JSON_INDICATORS());
      uploadINDICATORS();
      delay(1000);
      if(Serial.available() > 0){
        String comando = Serial.readString();
        desligarAlarme(comando);
        Serial.print(JSON_SENSOR());
        uploadSENSOR();
        delay(10);
        Serial.print(JSON_INDICATORS());
        uploadINDICATORS();
      }
    }
    analogSensor = analogRead(sensor);
    delay (2000);
  }
   desligarAlarme(downloadWEB());
   uploadSENSOR();
   delay(10);
   uploadINDICATORS();
   
   delay (1000);
}
