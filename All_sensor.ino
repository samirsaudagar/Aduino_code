#include <Arduino.h>
#include <stdio.h>
#define ON 1
#define OFF 0

#define MQ_PIN (0) // MQ sensor analog pin
#define RL_VALUE (5) // Load resistance in kilo-ohms
#define RO_CLEAN_AIR_FACTOR (9.83) // Clean air factor

#define CALIBRATION_SAMPLE_TIMES (50) 
#define CALIBRATION_SAMPLE_INTERVAL (500) 
#define READ_SAMPLE_TIMES (5) 
#define READ_SAMPLE_INTERVAL (50) 

#define GAS_LPG (0)
#define GAS_CO (1)
#define GAS_SMOKE (2)

#include <LiquidCrystal.h>
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int BUZZER_PIN = 8; 
const int trigPin = 9;
const int echoPin = 10;

float duration, distance;

float LPGCurve[3] = {2.3, 0.21, -0.47};
float COCurve[3] = {2.3, 0.72, -0.34};
float SmokeCurve[3] = {2.3, 0.53, -0.44};
float Ro = 10;

int vibration_Sensor = A5;
int present_condition = 0;
int previous_condition = 0;

void send_sensor_data(int id, int status) {
    Serial.print(id);
    Serial.print(": ");
    Serial.println(status);
}

void setup() {
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(vibration_Sensor, INPUT);
    pinMode(12, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
    Serial.println("Calibrating...");
    Ro = MQCalibration(MQ_PIN);
    Serial.println("Calibration done...");
    noTone(BUZZER_PIN);
}

void loop() {
    int lpgLevel = MQGetGasPercentage(MQRead(MQ_PIN) / Ro, GAS_LPG);
    int coLevel = MQGetGasPercentage(MQRead(MQ_PIN) / Ro, GAS_CO);
    int smokeLevel = MQGetGasPercentage(MQRead(MQ_PIN) / Ro, GAS_SMOKE);
    
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    distance = (duration * 0.0343) / 2;
    
    previous_condition = present_condition;
    present_condition = digitalRead(vibration_Sensor);

    send_sensor_data(1, lpgLevel >= 500 ? 1 : 0);
    send_sensor_data(2, coLevel >= 500 ? 1 : 0);
    send_sensor_data(3, smokeLevel >= 500 ? 1 : 0);
    send_sensor_data(4, distance < 10 ? 1 : 0);
    if (digitalRead(12) == 0 ){
    send_sensor_data(5,1);
    }else{
    send_sensor_data(5,0);
    }
    send_sensor_data(6, previous_condition != present_condition ? 1 : 0);

    
    if (lpgLevel >= 500 || coLevel >= 500 || smokeLevel >= 500 || !digitalRead(12) || distance < 10 || previous_condition != present_condition) {
        tone(BUZZER_PIN, 85);
    } else {
        noTone(BUZZER_PIN);
    }
    
    delay(500);
    noTone(BUZZER_PIN);
    delay(500);
}

float MQResistanceCalculation(int raw_adc) {
    return ((float)RL_VALUE * (1023 - raw_adc) / raw_adc);
}

float MQCalibration(int mq_pin) {
    float val = 0;
    for (int i = 0; i < CALIBRATION_SAMPLE_TIMES; i++) {
        val += MQResistanceCalculation(analogRead(mq_pin));
        delay(CALIBRATION_SAMPLE_INTERVAL);
    }
    return (val / CALIBRATION_SAMPLE_TIMES) / RO_CLEAN_AIR_FACTOR;
}

float MQRead(int mq_pin) {
    float rs = 0;
    for (int i = 0; i < READ_SAMPLE_TIMES; i++) {
        rs += MQResistanceCalculation(analogRead(mq_pin));
        delay(READ_SAMPLE_INTERVAL);
    }
    return rs / READ_SAMPLE_TIMES;
}

int MQGetGasPercentage(float rs_ro_ratio, int gas_id) {
    if (gas_id == GAS_LPG) return MQGetPercentage(rs_ro_ratio, LPGCurve);
    else if (gas_id == GAS_CO) return MQGetPercentage(rs_ro_ratio, COCurve);
    else if (gas_id == GAS_SMOKE) return MQGetPercentage(rs_ro_ratio, SmokeCurve);
    return 0;
}

int MQGetPercentage(float rs_ro_ratio, float *pcurve) {
    return pow(10, ((log(rs_ro_ratio) - pcurve[1]) / pcurve[2]) + pcurve[0]);
}
