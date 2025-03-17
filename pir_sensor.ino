const int PIR_SENSOR_OUTPUT_PIN = 4;	/* PIR sensor O/P pin */
int warm_up;

void setup() {
  pinMode(PIR_SENSOR_OUTPUT_PIN, INPUT);
  Serial.begin(9600);	/* Define baud rate for serial communication */
  delay(20000);	/* Power On Warm Up Delay */
}

void loop() {
  int sensor_output;
  sensor_output = digitalRead(PIR_SENSOR_OUTPUT_PIN);
  if( sensor_output == LOW )
  {
    if( warm_up == 1 )
     {
      Serial.print("Warming Up\n\n");
      warm_up = 0;
      delay(2000);
    }
    Serial.print("No object in sight\n\n");
    delay(1000);
  }
  else
  {
    Serial.print("Object detected\n\n");    
    warm_up = 1;
    delay(1000);
  }  
  Serial.print(sensor_output);  
  
}
