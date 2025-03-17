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

#define TRIG_PIN 6  // Ultrasonic sensor trigger pin
#define ECHO_PIN 7  // Ultrasonic sensor echo pin
#define BUZZER_PIN 8 // Buzzer pin

float LPGCurve[3] = {2.3, 0.21, -0.47};
float COCurve[3] = {2.3, 0.72, -0.34};
float SmokeCurve[3] = {2.3, 0.53, -0.44};
float Ro = 10;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    Serial.begin(9600);
    Serial.println("Calibrating...");
    Ro = MQCalibration(MQ_PIN);
    lcd.begin(16, 2);
    Serial.println("Calibration done...");
    Serial.print("Ro=");
    Serial.print(Ro);
    Serial.println("kohm");
}

void loop() {
    int lpgLevel = MQGetGasPercentage(MQRead(MQ_PIN) / Ro, GAS_LPG);
    int coLevel = MQGetGasPercentage(MQRead(MQ_PIN) / Ro, GAS_CO);
    int smokeLevel = MQGetGasPercentage(MQRead(MQ_PIN) / Ro, GAS_SMOKE);
    int distance = getDistance();

    Serial.print("LPG:"); Serial.print(lpgLevel); Serial.print(" ppm    ");
    Serial.print("CO:"); Serial.print(coLevel); Serial.print(" ppm    ");
    Serial.print("SMOKE:"); Serial.print(smokeLevel); Serial.print(" ppm    ");
    Serial.print("Distance: "); Serial.print(distance); Serial.println(" cm");

    if (lpgLevel >= 1 || coLevel >= 1 || smokeLevel >= 1 || distance < 100) {
        digitalWrite(BUZZER_PIN, HIGH);
    } else {
        digitalWrite(BUZZER_PIN, LOW);
    }
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

int getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH);
    return duration * 0.034 / 2;
}
