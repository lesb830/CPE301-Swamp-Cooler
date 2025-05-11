//
// Leslie Becerra, Kj Moreno, Khoa Minh Do
//CPE 301: Final Project
//
#include <LiquidCrystal.h>
#include <RTClib.h>
#include <DHT.h>
#include <Stepper.h>

// === Pins ===
#define STEPPER_PIN1 10
#define STEPPER_PIN2 11
#define STEPPER_PIN3 12
#define STEPPER_PIN4 13

#define BUTTON_ON_OFF 18
#define BUTTON_RESET 19
#define BUTTON_LIMIT_TOP 2
#define BUTTON_LIMIT_BOTTOM 3
#define BUTTON_STEPPER_UP 4
#define BUTTON_STEPPER_DOWN 5

#define MOTOR_PIN 6     // PH3
#define DHT_PIN 7       // PH4
#define DHT_TYPE DHT11
#define WATER_LEVEL 5   // A5

#define LED_PINR 0  // PA0
#define LED_PINY 1  // PA1
#define LED_PING 2  // PA2
#define LED_PINB 3  // PA3

// === Constants ===
#define WATER_THRESHOLD 700
#define TEMP_THRESHOLD 10
#define STEPS_PER_REV 2048

// === Components ===
LiquidCrystal lcd(30, 31, 32, 33, 34, 35);
RTC_DS1307 RTC;
DHT dht(DHT_PIN, DHT_TYPE);
Stepper stepper(STEPS_PER_REV, STEPPER_PIN1, STEPPER_PIN3, STEPPER_PIN2, STEPPER_PIN4);

// === State ===
enum States { IDLE, DISABLED, RUNNING, ERROR, START };
States currentState = DISABLED;
States prevState = START;

// === Globals ===
int lastTempPrint = 0;
float temp = 0;
float hum = 0;
int stepperRate = 2048;
bool fanOn = false;
int ledC = -1;
bool displayTempHum = false;
bool stepperAllowed = false;
bool monitorWater = false;
bool buttonOnState = true;

void setup() {
  // Configure test LED on Pin 22 (PA0)
  DDRA |= (1 << PA0);  // Set PA0 (Pin 22) as output
  PORTD |= (1 << PD0) | (1 << PD1);
  RTC.begin();
  RTC.adjust(DateTime(2022, 12, 9, 0, 0, 0));

  DDRA |= (1 << LED_PINR) | (1 << LED_PINY) | (1 << LED_PING) | (1 << LED_PINB);

  DDRE &= ~((1 << PE3) | (1 << PE4));
  PORTE |= (1 << PE3) | (1 << PE4);
  DDRD &= ~((1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5));

  DDRH |= (1 << PH3);

  dht.begin();
  lcd.begin(16, 2);
  lcd.print("System Starting");
  stepper.setSpeed(10);

  adc_init();
  U0init(9600);
}

void loop() {
  // Blink test LED on Pin 22
  PORTA |= (1 << PA0);  // Turn LED on
  delay_custom(200000);
  PORTA &= ~(1 << PA0); // Turn LED off
  delay_custom(200000);
  DateTime now = RTC.now();

  if(displayTempHum){
    temp = dht.readTemperature();
    hum = dht.readHumidity();
    // Check if DHT failed to read
    if (isnan(temp) || isnan(hum)) {
      lcd.clear();
      lcd.print("DHT Fail");
      PORTA |= (1 << PA0);  // Blink test LED as error signal
      delay_custom(100000);
      PORTA &= ~(1 << PA0);
    }
  }

  currentState = decideState(temp, adc_read(WATER_LEVEL), currentState);

  static bool lastButtonState = 0;
  bool currentButtonState = pinRead(BUTTON_ON_OFF);
  if (currentButtonState && !lastButtonState) {
    delay_custom(50000); // Debounce delay
    if (pinRead(BUTTON_ON_OFF)) {
      handleOnOff();
    }
  }
  lastButtonState = currentButtonState;

  if(currentState != prevState){
    writeTimeStampTransition(now, prevState, currentState);
    updateStateFlags();
  }

  if(stepperAllowed){
    int stepperDirection = stepperRate * (pinRead(BUTTON_STEPPER_UP) ? 1 :
                         pinRead(BUTTON_STEPPER_DOWN) ? -1 : 0);
    stepperDirection = (pinRead(BUTTON_LIMIT_TOP) ? min(stepperDirection, 0) :
                      (pinRead(BUTTON_LIMIT_BOTTOM) ? max(stepperDirection,0) :
                      stepperDirection));
    if(stepperDirection != 0){
      writeStepperPos(now, prevState, currentState);
      stepper.step(stepperDirection);
    }
  }

  setFanMotor(fanOn);
  turnLEDOn(ledC);

  if(displayTempHum && abs(lastTempPrint - now.minute()) >= 1){
    lcd.clear();
    lastTempPrint = now.minute();
    temp = dht.readTemperature();
    hum = dht.readHumidity();
    lcd.print("Temp, Humidity");
    delay_custom(500000);
    lcd.clear();
    lcd.print(temp);
    lcd.print(hum);
  }

  if(monitorWater){
    if(adc_read(WATER_LEVEL) <= WATER_THRESHOLD){
      currentState = ERROR;
    }
  }

  prevState = currentState;
}

// === Utility and Support Functions ===

int pinRead(int pin) {
  switch(pin) {
    case 18: return (PINE & (1 << PE3)) ? 0 : 1;
    case 19: return (PINE & (1 << PE4)) ? 0 : 1;
    case 2:  return (PIND & (1 << 2)) ? 1 : 0;
    case 3:  return (PIND & (1 << 3)) ? 1 : 0;
    case 4:  return (PIND & (1 << 4)) ? 1 : 0;
    case 5:  return (PIND & (1 << 5)) ? 1 : 0;
    default: return 0;
  }
}

void handleOnOff() {
  prevState = currentState;
  if(buttonOnState){
    currentState = IDLE;
    buttonOnState = false;
  } else {
    currentState = DISABLED;
    buttonOnState = true;
  }
}

void updateStateFlags() {
  switch(currentState) {
    case DISABLED:
      fanOn = false; ledC = 3;
      displayTempHum = false; stepperAllowed = true; monitorWater = false;
      break;
    case IDLE:
      fanOn = false; ledC = 2;
      displayTempHum = true; stepperAllowed = true; monitorWater = true;
      break;
    case RUNNING:
      fanOn = true; ledC = 1;
      displayTempHum = true; stepperAllowed = true; monitorWater = true;
      break;
    case ERROR:
      lcd.clear(); lcd.print("Error, low water");
      fanOn = false; ledC = 0;
      displayTempHum = true; stepperAllowed = false; monitorWater = true;
      break;
    default: break;
  }
}

void setFanMotor(bool on) {
  if(on) PORTH |= (1 << PH3);
  else   PORTH &= ~(1 << PH3);
}

void turnLEDOn(int ledColor) {
  PORTA &= ~((1 << LED_PINR) | (1 << LED_PINY) | (1 << LED_PING) | (1 << LED_PINB));
  switch(ledColor) {
    case 0: PORTA |= (1 << LED_PINR); break;
    case 1: PORTA |= (1 << LED_PINB); break;
    case 2: PORTA |= (1 << LED_PING); break;
    case 3: PORTA |= (1 << LED_PINY); break;
  }
}

States decideState(float temp, int waterLvl, States currentState) {
  if(temp <= TEMP_THRESHOLD && currentState == RUNNING)
    return IDLE;
  else if(temp > TEMP_THRESHOLD && currentState == IDLE)
    return RUNNING;
  else if(currentState == ERROR && pinRead(BUTTON_RESET) && waterLvl > WATER_THRESHOLD)
    return IDLE;
  else
    return currentState;
}

void writeStepperPos(DateTime now, States prev, States curr) {
  U0putchar('S'); U0putchar('T'); U0putchar('E'); U0putchar('P'); U0putchar(' ');
  writeTimeStampTransition(now, prev, curr);
}

void writeTimeStampTransition(DateTime now, States prev, States curr) {
  U0putchar(prev == DISABLED ? 'd' : prev == IDLE ? 'i' : prev == RUNNING ? 'r' : prev == ERROR ? 'e' : 'u');
  U0putchar(':');
  U0putchar(curr == DISABLED ? 'd' : curr == IDLE ? 'i' : curr == RUNNING ? 'r' : curr == ERROR ? 'e' : 'u');
  U0putchar(' ');

  char n[] = "0123456789";
  int y = now.year(), m = now.month(), d = now.day();
  int h = now.hour(), mi = now.minute(), s = now.second();

  U0putchar(n[m / 10]); U0putchar(n[m % 10]); U0putchar(':');
  U0putchar(n[d / 10]); U0putchar(n[d % 10]); U0putchar(':');
  U0putchar(n[y / 1000]); U0putchar(n[(y % 1000) / 100]); U0putchar(' ');
  U0putchar(n[h / 10]); U0putchar(n[h % 10]); U0putchar(':');
  U0putchar(n[mi / 10]); U0putchar(n[mi % 10]); U0putchar(':');
  U0putchar(n[s / 10]); U0putchar(n[s % 10]); U0putchar(' ');
}

void adc_init() {
  ADCSRA = 0x80;
  ADCSRB = 0x00;
  ADMUX = 0x40;
}

unsigned int adc_read(unsigned char adc_channel) {
  ADCSRB = (ADCSRB & 0xF8) | ((adc_channel >> 3) & 0x07);
  ADMUX = (ADMUX & 0xF8) | (adc_channel & 0x07);
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC)) {}
  return ADC;
}

void delay_custom(unsigned long count) {
  for (unsigned long i = 0; i < count; i++) asm volatile("nop");
}

#define RDA 0x80
#define TBE 0x20

volatile unsigned char* myUCSR0A = (unsigned char*)0x00C0;
volatile unsigned char* myUCSR0B = (unsigned char*)0x00C1;
volatile unsigned char* myUCSR0C = (unsigned char*)0x00C2;
volatile unsigned int*  myUBRR0  = (unsigned int*) 0x00C4;
volatile unsigned char* myUDR0   = (unsigned char*)0x00C6;

void U0init(unsigned long baud) {
  unsigned long FCPU = 16000000;
  unsigned int tbaud = FCPU / 16 / baud - 1;
  *myUCSR0A = 0x20;
  *myUCSR0B = 0x18;
  *myUCSR0C = 0x06;
  *myUBRR0 = tbaud;
}

void U0putchar(unsigned char data) {
  while (!(*myUCSR0A & TBE)) {}
  *myUDR0 = data;
}

unsigned char U0getchar() {
  while (!(*myUCSR0A & RDA)) {}
  return *myUDR0;
}

unsigned char U0kbhit() {
  return (*myUCSR0A & RDA) ? 1 : 0;
}
