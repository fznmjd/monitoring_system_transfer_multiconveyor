#include <WiFi.h>  
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <AsyncHTTPSRequest_Generic.h>
#define ASYNC_HTTPS_DEBUG_PORT     Serial
#define _ASYNC_HTTPS_LOGLEVEL_     4
# define pin_proximity1 15
# define pin_proximity2 4
# define pin_proximity3 16
# define pin_proximity4 17
# define pin_tachometer1 5
# define pin_tachometer2 18
# define pin_ledWIFI 19
# define pin_ledserver 21
const char* ssid = "Al-Majid";      // Nama SSID (jaringan WiFi)
const char* password = "yangmulia";  // Kata sandi WiFi
const float pi = 3.14159265; 
int period = 1000;           
int radius = 22;              
int jml_celah = 18;          
int rotation1, speedbelt1, totalspeedbelt1A, totalspeedbelt1B, ratarataspeedbelt1A, ratarataspeedbelt1B;
int rotation2, speedbelt2, totalspeedbelt2A, totalspeedbelt2B, ratarataspeedbelt2A, ratarataspeedbelt2B;
int ratarataspeedbelt1, ratarataspeedbelt2, duration;
int proximity1, proximity2, proximity3, proximity4, indikator1, indikator2, kondisi, counter1, counter2;
long startMeasurementTime, a, A, stopA, durationA, TimerA, b, B, stopB, durationB, TimerB;
long startA = 0;
long startB = 0;
bool sendingwarning = false;
bool proxi2 = false;
bool proxi4 = false;
int table;
String status, statusA, statusB;
WiFiClientSecure client;
AsyncHTTPSRequest request;
void setup() {
  Serial.begin(115200);
  pinMode(pin_ledWIFI, OUTPUT);
  pinMode(pin_ledserver, OUTPUT);
  while (!Serial && millis() < 5000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to WiFi. IP Address: ");
  Serial.println(WiFi.localIP());
  client.setInsecure();
  pinMode(pin_proximity1, INPUT);
  pinMode(pin_proximity2, INPUT);
  pinMode(pin_proximity3, INPUT);
  pinMode(pin_proximity4, INPUT);
  pinMode(pin_tachometer1, INPUT);
  digitalWrite(pin_tachometer1, HIGH);
  pinMode(pin_tachometer2, INPUT);
  digitalWrite(pin_tachometer2, HIGH);
  request.onReadyStateChange(requestCB);
  sendRequest();
}
void requestCB(void *optParm, AsyncHTTPSRequest *request, int readyState){
  (void) optParm;
  if (readyState == readyStateDone){
    AHTTPS_LOGDEBUG(F("\n**************************************"));
    AHTTPS_LOGDEBUG1(F("Response Code = "), request->responseHTTPString());
    if (request->responseHTTPString() != "Created"){
      digitalWrite(pin_ledserver, LOW);
    }
    else if (request->responseHTTPString() == "Created"){
      digitalWrite(pin_ledserver, HIGH);
    }
    if (request->responseHTTPcode() == 200){
      Serial.println(F("\n**************************************"));
      Serial.println(request->responseText());
      Serial.println(F("**************************************"));
    }
    sendRequest();
  }
}
void sendRequest(){
  int stat = 0;
  if (status == "SUCCESS"){
    stat = 1;
  }
  if (status == "Enter Problem"){
    stat = 2;
  }
  if (status == "Material Fall"){
    stat = 3;
  }
  if (status == "Time Transfer is Too Long"){
    stat = 4;
  }
  if (status == "Time Transfer is Too Long: Enter Problem"){
    stat = 5;
  }
  if (status == "Time Transfer is Too Long: Material Fall"){
    stat = 6;
  }
  if (status == "WARNING"){
    stat = 7;
  }
  if (startA > 0){
        if (ratarataspeedbelt1A == 0 && ratarataspeedbelt2A == 0){
          ratarataspeedbelt1A = speedbelt1;
          ratarataspeedbelt2A = speedbelt2;
        }
        ratarataspeedbelt1A = ((speedbelt1 + ratarataspeedbelt1A) / 2);
        ratarataspeedbelt2A = ((speedbelt2 + ratarataspeedbelt2A) / 2);
      }
      if (startB > 0){
        if (ratarataspeedbelt1B == 0 && ratarataspeedbelt2B == 0){
          ratarataspeedbelt1B = speedbelt1;
          ratarataspeedbelt2B = speedbelt2;
        }
        ratarataspeedbelt1B = ((speedbelt1 + ratarataspeedbelt1B) / 2);
        ratarataspeedbelt2B = ((speedbelt2 + ratarataspeedbelt2B) / 2);
      }
  static bool requestOpenResult;
  if (request.readyState() == readyStateUnsent || request.readyState() == readyStateDone){
    if(table == 1){
      StaticJsonDocument<200> doc;
      doc["ind1"] = indikator1;
      doc["ind2"] = indikator2;
      doc["kond"] = kondisi;
      doc["kec1"] = speedbelt1;
      doc["kec2"] = speedbelt2;
      doc["startA"] = startA;
      doc["startB"] = startB;
      doc["dur"] = duration;
      doc["konv1"] = ratarataspeedbelt1;
      doc["konv2"] = ratarataspeedbelt2;
      doc["stat"] = stat;
      String requestBody;
      serializeJson(doc, requestBody);
      requestOpenResult = request.open("POST", "https://api-konveyor-gwuo7fgaxq-uc.a.run.app/api/sensor");
      if (requestOpenResult){
        request.setReqHeader("Content-Type", "application/json");
        request.send(requestBody.c_str());
      }
      else{
        Serial.println(F("Can't send bad request"));
      }
      table = 0;
    }
    else if(!sendingwarning && kondisi ==1){
      StaticJsonDocument<200> doc;
      doc["ind1"] = indikator1;
      doc["ind2"] = indikator2;
      doc["kond"] = kondisi;
      doc["kec1"] = speedbelt1;
      doc["kec2"] = speedbelt2;
      doc["startA"] = startA;
      doc["startB"] = startB;
      doc["dur"] = duration;
      doc["konv1"] = ratarataspeedbelt1;
      doc["konv2"] = ratarataspeedbelt2;
      doc["stat"] = 7;
      String requestBody;
      serializeJson(doc, requestBody);
      requestOpenResult = request.open("POST", "https://api-konveyor-gwuo7fgaxq-uc.a.run.app/api/sensor");
      if (requestOpenResult){
        request.setReqHeader("Content-Type", "application/json");
        request.send(requestBody.c_str());
      }
      else{
        Serial.println(F("Can't send bad request"));
      }
        sendingwarning = true;
    }
    else {
      StaticJsonDocument<200> doc;
      doc["ind1"] = indikator1;
      doc["ind2"] = indikator2;
      doc["kond"] = kondisi;
      doc["kec1"] = speedbelt1;
      doc["kec2"] = speedbelt2;
      doc["startA"] = startA;
      doc["startB"] = startB;
      String requestBody;
      serializeJson(doc, requestBody);
      requestOpenResult = request.open("POST", "https://api-konveyor-gwuo7fgaxq-uc.a.run.app/api/sensor");
      if (requestOpenResult){
        request.setReqHeader("Content-Type", "application/json");
        request.send(requestBody.c_str());
      }
      else{
        Serial.println(F("Can't send bad request"));
      }
    }
  }
  else{
    Serial.println(F("Can't send request"));
  }
}
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(pin_ledWIFI, HIGH);
  }
  else {
    digitalWrite(pin_ledWIFI, LOW);
    digitalWrite(pin_ledserver, LOW);
  }
  attachInterrupt(digitalPinToInterrupt(pin_tachometer1), addcount1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pin_tachometer2), addcount2, CHANGE);
  if (millis() - startMeasurementTime >= period){
    detachInterrupt(digitalPinToInterrupt(pin_tachometer1));
    detachInterrupt(digitalPinToInterrupt(pin_tachometer2));
    BeltSpeed();
    startMeasurementTime = millis();
    counter1 = 0;
    counter2 = 0;
  }
    proximity1 = digitalRead(pin_proximity1);
    proximity2 = digitalRead(pin_proximity2);
    proximity3 = digitalRead(pin_proximity3);
    proximity4 = digitalRead(pin_proximity4);
    if (proximity1 == 0){
      if (B == 0){
        startA = millis();
        A = 1;
        if(proximity2 == 0){
          statusA = "ENTER SUCCESS";
        }
      }
      if (B == 1){
        startB = millis();
        if(proximity2 == 0){
          statusB = "ENTER SUCCESS";
        }
      }
    }
    if (proximity1 == 1){
      if (A == 1){
       B = 1;
      }
    }
    if(startA > 0){
      TimerA = (millis() - startA)/1000;
      if (TimerA > 10){
        ratarataspeedbelt1 = ratarataspeedbelt1A;
        ratarataspeedbelt2 = ratarataspeedbelt2A;
        kondisi = 1;
        status = "WARNING";
      }
    }
    if(startB > 0){
      TimerB = (millis() - startB)/1000;
      if (TimerB > 10){
        ratarataspeedbelt1 = ratarataspeedbelt1B;
        ratarataspeedbelt2 = ratarataspeedbelt2B;
        kondisi = 1;
        status = "WARNING";
      }
    }
    if (proximity3 == 0){
      kondisi = 0;
      if (b == 0){
        stopA = millis();
        if (statusA == "ENTER SUCCESS"){
          if (proximity4 == 0){
            statusA = "SUCCESS";
          }
        }
        a = 1;
      }
      if (b == 1){
        stopB = millis();
        if (statusB == "ENTER SUCCESS"){
          if (proximity4 == 0){
            statusB = "SUCCESS";
          }
        }
      }
    }
    if (proximity3 == 1){
      if (a == 1){
        b = 1;
      }
      if (startA > 0 && startA < stopA){
        kondisi = 0;
        timerA();
      }
      if (startB > 0 && startB < stopB){
        kondisi = 0;
        timerB();
      }
    }
    if (proximity2 == 0){
      if (proximity1 == 0){
        indikator1 = 2;
      }
      proxi2 = true;
    }
    if (proximity1 == 1 && proximity2 == 1){
      indikator1 = 0;
      proxi2 = false;
    }
    if (proximity1 == 0 && proximity2 == 1){
      if (proxi2 == false){
        indikator1 = 1;
      }
    }
    if (proximity4 == 0){
      if (proximity3 == 0){
        indikator2 = 2;
      }
      proxi4 = true;
    }
    if (proximity3 == 1 && proximity4 == 1){
      indikator2 = 0;
      proxi4 = false;
    }
    if (proximity3 == 0 && proximity4 == 1){
      if (proxi4 == false){
        indikator2 = 1;
      }
    }
}
void timerA() {
  durationA = ((stopA - startA)/1000);
  if (durationA > 5) {
    if (statusA == "SUCCESS") {
      statusA = "Time Transfer is Too Long";
    } 
    else if (statusA == "ENTER SUCCESS") {
      statusA = "Time Transfer is Too Long: Material Fall";
    } 
    else {
      statusA = "Time Transfer is Too Long: Enter Problem";
    }
  }
  if (statusA == ""){
    statusA = "Enter Problem";
  }
  if (statusA == "ENTER SUCCESS"){
    statusA = "Material Fall";
  }
  status = statusA;
  ratarataspeedbelt1 = ratarataspeedbelt1A;
  ratarataspeedbelt2 = ratarataspeedbelt2A;
  duration = durationA;
  table = 1;
  startA = 0;
  stopA = 0;
  A = 0;
  a = 0;
  statusA = "";
  ratarataspeedbelt1A = 0;
  ratarataspeedbelt2A = 0;
  kondisi = 0;
  sendingwarning = false;
  TimerA = 0;
  TimerB = 0;
}
void timerB() {
  durationB = ((stopB - startB)/1000);
  if (durationB > 5) {
    if (statusB == "SUCCESS") {
      statusB = "Time Transfer is Too Long";
    } 
    else if (statusB == "ENTER SUCCESS") {
      statusB = "Time Transfer is Too Long: Material Fall";
    } 
    else {
      statusB = "Time Transfer is Too Long: Enter Problem";
    }
  }
  if (statusB == ""){
    statusB = "Enter Problem";
  }
  if (statusB == "ENTER SUCCESS"){
    statusB = "Material Fall";
  }
  status = statusB; 
  ratarataspeedbelt1 = ratarataspeedbelt1B;
  ratarataspeedbelt2 = ratarataspeedbelt2B;
  duration = durationB;
  table = 1;
  startB = 0;
  stopB = 0;
  B = 0;
  b = 0;
  statusB = "";
  ratarataspeedbelt1B = 0;
  ratarataspeedbelt2B = 0;
  kondisi = 0;
  sendingwarning = false;
  TimerA = 0;
  TimerB = 0;
}
void addcount1() {
  counter1++;
}
void addcount2() {
  counter2++;
}
void BeltSpeed() {
  speedbelt1 = 0;
  speedbelt2 = 0;
  if(counter1 != 0){
    rotation1 = (((counter1 / jml_celah) * 60) / (period / 1000)); // Calculate revolutions per minute (RPM)
    speedbelt1 = ((2 * pi * radius * rotation1 * 10) * 0.0001368) + 385.43;// Calculate belt speed in cm/min
  }
  if(counter2 != 0){
    rotation2 = ((counter2 / jml_celah) * 60) / (period / 1000); // Calculate revolutions per minute (RPM)
    speedbelt2 = ((2 * pi * radius * rotation2 * 10)* 0.00001306) + 667.75; // Calculate belt speed in cm/min
  }
}