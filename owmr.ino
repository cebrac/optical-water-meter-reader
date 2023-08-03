#define PIN_LASER 3
#define PIN_LED LED_BUILTIN
#define PIN_SENSOR A6
#define PIN_PULSE_OUTPUT 10

// Qualitätsstandards
#define OFFSET_LIMIT 950 // Reading bei der Dunkelmessung, das nicht unterschritten werden darf
#define MIN_MEASUREMENT 25 // Mindestwert einer gültigen Vollmessung (Hellmessung abzüglich Dunkelmessung)
#define MAX_VARIANCE 30 // Max. tolerierte Fluktuation bei der Dunkelmessung
#define MIN_TRAINING_BANDWIDTH 30 // Mindest-Bandbreite zwischen höchster und niedrigster Reflektion im Zyklus
#define TRAINING_CYCLES 3 // Anzahl der Zyklen beim Training

#define STATE_UNTRAINED 0
#define STATE_TRAINING 1
#define STATE_TRAINED 2

#define CYCLE_UNKNOWN 0
#define CYCLE_RISING 1
#define CYCLE_FALLING 2

int state = STATE_UNTRAINED;
uint16_t cycleMax = 0;
uint16_t cycleMin = 1023;

uint8_t cyclePosition = CYCLE_UNKNOWN;
uint8_t trainedCycles = 0;

uint16_t measurementLoopCounter = 0;
float measurementAvgSum = 0;
uint16_t measurementAvgCount = 0;
float offsetAvgSum = 0;
uint16_t offsetMin = 1023;
uint16_t offsetMax = 0;

uint16_t ledBlink = 0;
uint16_t ledBlinkInterval = 0;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_LASER, OUTPUT);
  pinMode(PIN_PULSE_OUTPUT, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
}

void loop() {
  measurementLoopCounter++;
  if(measurementLoopCounter >= 500) {
    int offset = analogRead(PIN_SENSOR);
    if(offset > offsetMax) offsetMax = offset;
    if(offset < offsetMin) offsetMin = offset;
    offsetAvgSum += offset;
    digitalWrite(PIN_LASER, true);
    delay(10);
    int reading = offset - analogRead(PIN_SENSOR);
    if(reading < 0) reading = 0;
    digitalWrite(PIN_LASER, false);
    measurementLoopCounter = 0;
    measurementAvgSum += reading;
    measurementAvgCount++;
  }

  if(measurementAvgCount >= 10) {
    float measurementAvg = measurementAvgSum / measurementAvgCount;
    float offsetAvg = offsetAvgSum / measurementAvgCount;
    uint16_t variance = offsetMax - offsetMin;
    bool cycleCounterActive = false;
    measurementAvgCount = 0;
    measurementAvgSum = 0;
    offsetAvgSum = 0;
    offsetMin = 1023;
    offsetMax = 0;
    
    Serial.print(F("Messung: "));
    Serial.print(measurementAvg);
    Serial.print(F("\tOffset:"));
    Serial.print(offsetAvg);
    Serial.print(F("\tVariance:"));
    Serial.print(variance);
    Serial.print(F("\tStatus: "));
    if(offsetAvg < OFFSET_LIMIT || variance > MAX_VARIANCE) { // Signal der Dunkelmessung zu stark oder zu unstet
      if(measurementAvg < MIN_MEASUREMENT) {
        Serial.print(F("Reflektion zu schwach!"));
      } else {
        Serial.print(F("Zu viel Streulichteinfall!"));
      }
      state = STATE_UNTRAINED;
      ledBlinkInterval = 20000;
    } else if(state == STATE_UNTRAINED) {
      cycleMax = 0;
      cycleMin = 1023;
      trainedCycles = 0;
      cyclePosition = CYCLE_UNKNOWN;
      Serial.print(F("Nicht kalibriert!"));
      state = STATE_TRAINING;
      ledBlinkInterval = 15000;
    } else if(state == STATE_TRAINING) {
      if(measurementAvg < cycleMin) cycleMin = measurementAvg;
      if(measurementAvg > cycleMax) cycleMax = measurementAvg;
      Serial.print(F("Kalibriere... Q="));
      float quality = round(((float)(cycleMax - cycleMin) / MIN_TRAINING_BANDWIDTH) * 100);
      Serial.print(quality);
      Serial.print(F("%, C="));
      Serial.print(trainedCycles);
      Serial.print('/');
      Serial.print(TRAINING_CYCLES);
      if(cycleMax - cycleMin >= MIN_TRAINING_BANDWIDTH) {
        ledBlinkInterval = 5000;
        cycleCounterActive = true;
      } else {
        ledBlinkInterval = 10000;
      }
      if(trainedCycles >= TRAINING_CYCLES) state = STATE_TRAINED;
    } else if(state == STATE_TRAINED) {
      cycleCounterActive = true;
      ledBlinkInterval = 0;
      Serial.print(F("OK"));
    }

    if(cycleCounterActive) {
      uint16_t lowerThreshold = cycleMin + (MIN_TRAINING_BANDWIDTH / 3);
      uint16_t upperThreshold = cycleMax - (MIN_TRAINING_BANDWIDTH / 3);

      Serial.print('\t');
      Serial.print(lowerThreshold);
      Serial.print('\t');
      Serial.print(upperThreshold);

      if(cyclePosition == CYCLE_UNKNOWN && measurementAvg > upperThreshold) {
        cyclePosition = CYCLE_RISING;
      } else if(cyclePosition == CYCLE_FALLING && measurementAvg > upperThreshold) {
        cyclePosition = CYCLE_RISING;
        digitalWrite(PIN_PULSE_OUTPUT, false);
      } else if(cyclePosition == CYCLE_UNKNOWN && measurementAvg < lowerThreshold){
        cyclePosition = CYCLE_FALLING;
      } else if(cyclePosition == CYCLE_RISING && measurementAvg < lowerThreshold) {
        cyclePosition = CYCLE_FALLING;
        if(state == STATE_TRAINING) {
          trainedCycles++;
        }
        digitalWrite(PIN_PULSE_OUTPUT, true);
      }
      
      if(cyclePosition == CYCLE_RISING) {
        Serial.print(F(" /"));
      } else if(cyclePosition == CYCLE_FALLING) {
        Serial.print(F(" \\"));
      }
    }
    
    Serial.println("");
  }

  if(ledBlinkInterval == 0) {
      digitalWrite(PIN_LED, cyclePosition == CYCLE_FALLING);
      ledBlink = 0;
  } else {
    ledBlink++;
    if(ledBlink > ledBlinkInterval) {
      ledBlink = 0;
      digitalWrite(PIN_LED, false);
    } else if(ledBlink > (ledBlinkInterval / 2)) {
      digitalWrite(PIN_LED, true);
    }
  }
}
