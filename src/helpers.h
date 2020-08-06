#include <math.h>

#define MAX_ARGS 64
#define MAX_PWM 225.0
#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)
#define CHANGE_FREQ_MILLIS (60 * 1000)
#define AMBIENT_HIGH 50
#define AMBIENT_LOW 5

unsigned long autoModeDisabled;
bool automode = true;
int ledState = 0;
int currts = 0;
double volts = 0.0;
int rolling5Index = 0;
int rolling5[] = {
  0,0,0,0,0
};
int ledPin = D0;
int rollingAvg = 0;
int ambientReading;
bool firstLoop = true;
unsigned long ledLastUpdated;

void modifyLEDState(int newLEDState){
  ledState = constrain(ledState, MAX_PWM * 0.20, MAX_PWM);
  Serial.println("LED State: " + String(ledState));
  Serial.println("New LED State: " + String(newLEDState));
  if (ledState != newLEDState){
    //Graceful move to new Brightness
    if (firstLoop) {
      analogWrite(ledPin, newLEDState);
    } else if (ledState < newLEDState) {
      for (int a=ledState;a<newLEDState;a=a+1){
        a = constrain(a, 0, MAX_PWM);
        Serial.println("a: " + String(a));
        analogWrite(ledPin, a);
        delay(20);
      }
    } else if (ledState > newLEDState){
        for (int b=ledState;b>newLEDState;b=b-1){
          b = constrain(b, 0, MAX_PWM);
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

void sendNewLEDState(int inVal){
  Serial.println("Calculating New LED State");
  if (inVal <= 50) modifyLEDState(MAX_PWM * 0.20);
  else if (inVal <= 125) modifyLEDState(MAX_PWM * 0.40);
  else if (inVal <= 160) modifyLEDState(MAX_PWM * 0.60);
  else if (inVal <= 235) modifyLEDState(MAX_PWM * 0.80);
  else if (inVal > 280) modifyLEDState(MAX_PWM);
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

void doStartupCycle(){
  modifyLEDState(MAX_PWM * 0.20);
  delay(20);
  modifyLEDState(MAX_PWM * 0.40);
  delay(20);
  modifyLEDState(MAX_PWM * 0.60);
  delay(20);
  modifyLEDState(MAX_PWM * 0.80);
  delay(20);
  modifyLEDState(MAX_PWM);
  delay(20);
  for (int j=0;j<5;j++){
    Serial.println("j: " + String(j));
    for (int i=MAX_PWM;i>=10;i=i-5) {
      Serial.println("i: " + String(i));
      modifyLEDState(i);
      delay(5);
    }
    for (int i=10;i<=MAX_PWM;i=i+5){
      Serial.println("i: " + String(i));
      modifyLEDState(i);
      delay(5);
    }
  }
  for (int i=0;i<5;i++){
    modifyLEDState(MAX_PWM);
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
      modifyLEDState(MAX_PWM);
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
    enableAutoMode();

    return ledState;
}

int setPercentage(String args){
    int inVal = args.toInt();
    int newVal = (int)((float)inVal * MAX_PWM) / 100.0;
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
    int newVal = ledState + (int)((float)inVal * MAX_PWM) / 100.0;

    if (ledState + newVal < 0){
      modifyLEDState(0);
    } else if (ledState + newVal > MAX_PWM){
      modifyLEDState(MAX_PWM);
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