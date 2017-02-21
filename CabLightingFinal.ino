#define MAX_ARGS 64
#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)
#define CHANGE_FREQ_MILLIS (60 * 1000)
#define AMBIENT_HIGH 50
#define AMBIENT_LOW 5
#include <math.h>
// This #include statement was automatically added by the Particle IDE.
// #include <HttpClient.h>

// This #include statement was automatically added by the Particle IDE.
/*#include "HttpClient/HttpClient.h"*/

/*HttpClient http;*/
int analogPin = A0;
int currts = 0;
double volts = 0.0;
int rolling5Index = 0;
int rolling5[] = {
  0,0,0,0,0
};
int rollingAvg = 0;
int ledPin = D0;
int lightPin = A0;
int ledState = 0;
int ambientReading;
bool firstLoop = true;
bool automode = true;
unsigned long autoModeDisabled;
unsigned long ledLastUpdated;


/*http_header_t headers[] = {*/
//    { "Host", "ec2-35-166-193-108.us-west-2.compute.amazonaws.com" },
//    { "Content-Type", "application/json" },
    /*{ NULL, NULL }*/
/*};*/

/*http_request_t request;*/
/*http_response_t response;*/

void doStartupCycle(){
  modifyLEDState(50);
  delay(20);
  modifyLEDState(100);
  delay(20);
  modifyLEDState(150);
  delay(20);
  modifyLEDState(200);
  delay(20);
  modifyLEDState(240);
  delay(20);
  for (int j=0;j<5;j++){
    Serial.println("j: " + String(j));
    for (int i=240;i>=10;i=i-5) {
      Serial.println("i: " + String(i));
      modifyLEDState(i);
      delay(5);
    }
    for (int i=10;i<= 250;i=i+5){
      Serial.println("i: " + String(i));
      modifyLEDState(i);
      delay(5);
    }
  }
  for (int i=0;i<5;i++){
    modifyLEDState(240);
    delay(200);
    modifyLEDState(0);
    delay(200);
  }
  tryModifyState();
}

void enableAutoMode(){
  automode = true;
}

void disableAutoMode(){
  autoModeDisabled = millis();
  automode = false;
}

int onoffLight(String args){
    Serial.println("Alexa Called Me");
    int inVal = args.toInt();
    if (inVal == 1 ){
      modifyLEDState(240);
    } else if (inVal == 0){
      modifyLEDState(0);
    }

    Serial.println();
    Serial.print("On/Off");
    Serial.println();
    Serial.print("Arguments: ");
    Serial.print(args);
    Serial.println();
    Serial.print("LED State : ");
    Serial.print(ledState);
    Serial.println();

    //override auto mode
    disableAutoMode();

    return ledState;
}

int setPercentage(String args){
    int inVal = args.toInt();
    int newVal = (int)((float)inVal * 240.0) / 100.0;
    modifyLEDState(newVal);

    Serial.println();
    Serial.print("Set Percentage");
    Serial.println();
    Serial.print("Arguments: ");
    Serial.print(args);
    Serial.println();
    Serial.print("LED State : ");
    Serial.print(ledState);

    disableAutoMode();

    return ledState;
}

int deltachange(String args){
    int inVal = args.toInt();
    int newVal = ledState + (int)((float)inVal * 240.0) / 100.0;

    if (ledState + newVal < 0){
      modifyLEDState(0);
    } else if (ledState + newVal > 240){
      modifyLEDState(240);
    } else {
      modifyLEDState(newVal);
    }

    Serial.println();
    Serial.print("Simple Change");
    Serial.println();
    Serial.print("Arguments: ");
    Serial.print(args);
    Serial.println();
    Serial.print("LED State : ");
    Serial.print(ledState);

    disableAutoMode();

    return ledState;

}

bool tryModifyState(){
  double tmp1 = sqrt(pow(float(rollingAvg),2.0));
  double tmp2 = sqrt(pow(float(ambientReading),2.0));
  double math = abs(tmp1 - tmp2);
  Serial.println("1: " + String(tmp1) + "; 2: " + String(tmp2) + "; Final: " + String(math));
  if (abs(sqrt(pow(float(rollingAvg),2.0)) - sqrt(pow(float(ambientReading),2.0))) > 74){
    Serial.println("Detected Major Change; performing immediate update");
    sendNewLEDState(ambientReading);
    //Resetting RollingAvg
    for (int n=0;n<5;n++){
      rolling5[n] = ambientReading;
    }
    rollingAvg = ambientReading;
    return true;
  } else if (millis() - ledLastUpdated >= CHANGE_FREQ_MILLIS || firstLoop){
    Serial.println("Slow Roll Change Required");
    sendNewLEDState(rollingAvg);
    return true;
  } else return false;
}

void sendNewLEDState(int inVal){
  Serial.println("Calculating New LED State");
  if (inVal <= 50) modifyLEDState(50);
  else if (inVal <= 125) modifyLEDState(100);
  else if (inVal <= 160) modifyLEDState(150);
  else if (inVal <= 230) modifyLEDState(200);
  else if (inVal > 280) modifyLEDState(240);
}

void modifyLEDState(int newLEDState){
  ledState = constrain(ledState, 50, 240);
  Serial.println("LED State: " + String(ledState));
  Serial.println("New LED State: " + String(newLEDState));
  if (ledState != newLEDState){
    //Graceful move to new Brightness
    if (firstLoop) {
      analogWrite(ledPin, newLEDState);
    } else if (ledState < newLEDState) {
      for (int a=ledState;a<newLEDState;a=a+1){
        a = constrain(a, 0, 240);
        Serial.println("a: " + String(a));
        analogWrite(ledPin, a);
        delay(20);
      }
    } else if (ledState > newLEDState){
        for (int b=ledState;b>newLEDState;b=b-1){
          b = constrain(b, 0, 240);
          Serial.println("b: " + String(b));
          analogWrite(ledPin, b);
          delay(20);
        }
    }
    ledState = newLEDState;
    ledLastUpdated = millis();
    /*sendToTSDB("lightVolts", "1", "ledUpdated");*/
  }
}

void registerVal(int newVal){
  rolling5[rolling5Index] = newVal;
  int x = 0;
  for (int i = 0; i <= 4; i++) {
    x += rolling5[i];
  }
  rollingAvg = x / 5;
  if (rolling5Index >= 4) rolling5Index = 0;
  else rolling5Index++;
}

//Used to send measurements to TSDB on HBASE via HTTP Rest call
/*void sendToTSDB(String _metric, String _value, String _measure) {
  String jsonString = "{";
  jsonString += "\"metric\":\"" + _metric + "\",";
  jsonString += "\"timestamp\":";
  jsonString += String(Time.now());
  jsonString += String(millis()).substring(2,5);
  jsonString += ",\"value\":";
  jsonString += String(_value);
  jsonString += ",\"tags\":{\"measure\":\"" + _measure + "\"}}";
  Serial.println("jsonString = " + jsonString);

  request.body = jsonString;
  http.post(request, response, headers);
}*/

void setup() {
    //pinMode(A0, INPUT);
    //request.path = "/api/put";
    pinMode(ledPin, OUTPUT);
    /*request.hostname = "10.10.81.211/";*/
    /*request.port = 4444;*/
    /*request.path = "/api/put";*/
    Serial.begin(9600);
    Particle.function("setvalue", setPercentage);
    Particle.function("onoff", onoffLight);
    Particle.function("deltachange", deltachange);
    // Particle.variable("analog", reading);
    // Particle.variable("volts", volts);
    // Particle.variable("currts", currts);
}

void loop() {

  ambientReading = analogRead(lightPin);
  registerVal(ambientReading);
  volts = ambientReading * 3.3 / 4096;
  /*sendToTSDB("lightVolts", String(ambientReading), "Ambient_Lighting");*/
  /*sendToTSDB("lightVolts", String(volts), "Volts_Reading");*/
  /*sendToTSDB("lightVolts", String(rollingAvg), "Rolling_Avg");*/
  /*sendToTSDB("lightVolts", String((millis() - ledLastUpdated)/10000), "Time_Since_Last_LED_Update");*/

  if (firstLoop) {
    delay(5000);
    Serial.println("Starting Startup Cycle");
    doStartupCycle();
    firstLoop = false;
  }

  if (automode) {
    ambientReading = analogRead(lightPin);
    volts = ambientReading * 3.3 / 4096;
    Serial.println("Ambient Reading: " + String(ambientReading));
    Serial.println("Volts: " + String(volts));
    Serial.println("Rolling Average: " + String(rollingAvg));
    Serial.println("Time Since Last Upate: " + String((millis() - ledLastUpdated) / 1000));

    tryModifyState();

    Serial.println("LED State: " + String(ledState));
    /*Particle.publish("LED State", String(ledState));*/
  } else {
    if (millis() - autoModeDisabled > CHANGE_FREQ_MILLIS){
      enableAutoMode();
    }
  }
  delay(1000);

  /*Serial.print("Application>\tResponse status: ");*/
  /*Serial.println(response.status);*/
  /*Serial.print("Application>\tHTTP Response Body: ");*/
  /*Serial.println(response.body);*/

  delay(2000);
}
