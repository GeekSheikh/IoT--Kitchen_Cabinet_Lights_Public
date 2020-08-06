#define ALEXA_HOLD_MILLIS (60 * 30 * 1000)

#include <helpers.h>
#include <particleLog.h>

int analogPin = A0;
int lightPin = A0;

// This #include statement was automatically added by the Particle IDE.
// #include <HttpClient.h>

// This #include statement was automatically added by the Particle IDE.
/*#include "HttpClient/HttpClient.h"*/

/*HttpClient http;*/


/*http_header_t headers[] = {*/
//    { "Host", "ec2-35-166-193-108.us-west-2.compute.amazonaws.com" },
//    { "Content-Type", "application/json" },
    /*{ NULL, NULL }*/
/*};*/

/*http_request_t request;*/
/*http_response_t response;*/

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

const char DEVICE_ID[] = "thingy.photon.cabLights";
unsigned int localUDPPort = 2005;
unsigned int remoteUDPPort = 2005;
IPAddress remoteIP(10, 10, 80, 181);

void setup() {
  //pinMode(A0, INPUT);
  //request.path = "/api/put";
  pinMode(ledPin, OUTPUT);
  /*request.hostname = "10.10.81.211/";*/
  /*request.port = 4444;*/
  /*request.path = "/api/put";*/
  Serial.begin(9600);
  udp.begin(localUDPPort);

  // Particle.function("getLocalIP", getLocalIP);
  Particle.function("setvalue", setPercentage);
  Particle.function("onoff", onoffLight);
  Particle.function("deltachange", deltachange);
  // Particle.variable("analog", reading);
  // Particle.variable("volts", volts);
  // Particle.variable("currts", currts);

  Serial.printf("Current Time is: %s", Time.timeStr(Time.local()));
}

void loop() {

  unsigned long ts = Time.local();

  char lightReading[40];

  ambientReading = analogRead(lightPin);
  registerVal(ambientReading);
  volts = ambientReading * 3.3 / 4096;

  sprintf(lightReading, "%s.light.ambient %d %lu", DEVICE_ID, ambientReading, ts);
  sendLog(lightReading, remoteIP, remoteUDPPort);

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
    if (millis() - autoModeDisabled > ALEXA_HOLD_MILLIS){
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
