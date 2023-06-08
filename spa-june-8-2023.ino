// I2C LCD with Arduino  
// 120 v circ pump runs all the time that main breaker is on
// Lynnwood spa - right washer pull is for big pump
// left washer pull is for heater allowed. Heater high temp is
// controlled by ARDUINO NANO
// IMPORTANT - do NOT pull both washers at once or breaker may blow.*/

// https://makeabilitylab.github.io/physcomp/arduino/debouncing.html#debouncing-solution-1-using-delays

// #############################################################
#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // Library for LCD

// Wiring: LCD SDA pin is connected to A4 and LCD SCL pin to A5.
// Connect to LCD via I2C, default address 0x27 (A0-A2 not jumpered)
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3F, 20, 4); 
//##################################################################

const int pump            = 3;             // pin that the pump RELAY is attached to
const int PUMP_WASHER_PIN = 5;
int _savedpump_washer     = LOW;           //starts low because using pull-down resistor
//###############################

float SETPOINT            = 96.0;                     // change as needed

//#######################################################33int heater_washer_call= 0;

const int heater            = 12;                    // pin that the heater RELAY is attached to
const int HEATER_WASHER_PIN = 4;
int heater_status           = 0;
int _savedheater_washer     = LOW;            //starts low because using pull-down resistor

//####################################

int Thermistor1 = 1;                     // AO-1 hwr to heater
int Vo;
float logR2, R2, Temp_to_heater; 

int Thermistor2 = 2;                     // AO-2 hws from heater
int Vo2;
float logR2z2, R2z2, Temp_from_heater;

//###############################################################

//thermistor divider resistor ohms to match thermistor
float R_thermistor_a1 = 10030;  
float R_thermistor_a2 = 9960;    
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

//###################################################################

const int DEBOUNCE_WINDOW = 2000; // in milliseconds

//################################################################

void setup() {
  pinMode (pump,   OUTPUT);
  pinMode (heater, OUTPUT);
  pinMode(PUMP_WASHER_PIN,   INPUT);
  pinMode(HEATER_WASHER_PIN, INPUT);
 
  // Initiate the LCD:
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
}

//######################################################################

void loop() {
  
  //PUMP PUMP PUMP PUMP PUMP PUMP PUMP PUMP PUMP PUMP PUMP PUMP PUMP PUMP PUMP PUMP
  // Read the PUMP washer value. A pull-down resistor configuration so
  // will be HIGH when pulled and LOW when not pulled
  int pump_washer = digitalRead(PUMP_WASHER_PIN);

  // Wait to check the washer state again
  delay(DEBOUNCE_WINDOW);

  // read the washer value again
  int pump_washer2 = digitalRead(PUMP_WASHER_PIN);

  // If washer and washer2 are the same, then we are in steady state
  // If this stead state value does not match our _last_washer, then
  // a transition has occurred and we should save the new _washer
  // This works both for open-to-close transitions and close-to-open transitions
  if(pump_washer == pump_washer2 && _savedpump_washer != pump_washer){
    _savedpump_washer = pump_washer;
  }

  // Write out HIGH or LOW to start or stop pump
  digitalWrite(pump, _savedpump_washer);
  // END PUMP

//####################################################################################

//HEATER HEATER HEATER HEATER HEATER HEATER HEATER HEATER HEATER HEATER HEATER HEATER
// Read the HEATER washer value. A pull-down resistor configuration so
 // will be HIGH when pulled and LOW when not pulled

  int heater_washer = digitalRead(HEATER_WASHER_PIN);

  // Wait to check the washer state again
  delay(DEBOUNCE_WINDOW);

  // read the washer value again
  int heater_washer2 = digitalRead(HEATER_WASHER_PIN);      

  // If _washer and _washer2 are the same, then we are in steady state
  // If this stead state value does not match our _last_washer, then
  // a transition has occurred and we should save the new _washer
  // This works both for open-to-close transitions and close-to-open transitions
  if((heater_washer == heater_washer2) && (_savedheater_washer != heater_washer)){
    _savedheater_washer = heater_washer;
  }
  //but we do not start heater until temperature call is made.
  // END HEATER
//#######################################################################################

  // display starts at 0.00
  lcd.setCursor(4, 0);       // Set the cursor on the third column and first row.
  lcd.print(Temp_to_heater);
  lcd.print("  Temp In");
  lcd.setCursor(4, 2);       //Set the cursor on the third column and the second row (counting starts at 0!).
  lcd.print(Temp_from_heater);
  lcd.print("  Temp Out");
//#########################################################################################
 
  Vo = analogRead(Thermistor1);
  R2 = R_thermistor_a1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  Temp_to_heater = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  Temp_to_heater = Temp_to_heater - 273.15;
  Temp_to_heater = ((Temp_to_heater * 9.0)/ 5.0) + 32.0;
  
   
  Vo2 = analogRead(Thermistor2);
  R2z2 = R_thermistor_a2 * (1023.0 / (float)Vo2 - 1.0);
  logR2z2 = log(R2z2);
  Temp_from_heater = (1.0 / (c1 + c2*logR2z2 + c3*logR2z2*logR2z2*logR2z2));
  Temp_from_heater = Temp_from_heater - 273.15;
  Temp_from_heater = ((Temp_from_heater * 9.0)/ 5.0) + 32.0; 
//###################################################################################

  // if the SPA temp is less than setpoint, and washer pulled, turn on the HEATER:
  if ((Temp_to_heater < (SETPOINT)) && ( _savedheater_washer == 1)){
    digitalWrite(heater, HIGH);
    heater_status = 1;
  } 
  // Keep heater on until 1 degree above setpoint 
  //  example spa is 70 deg F, heater comes on until 97
  // then goes off and then comes back below 96.
  // but in Lynnwood Spa the heater manual washer ring must be pulled.
  
  if ((Temp_to_heater > (SETPOINT+1)) || (_savedheater_washer  == 0)){
    digitalWrite(heater, LOW);
    heater_status = 0;
  }
 //##########################################################################################
 
  Serial.print("Temp in: "); 
  Serial.print(Temp_to_heater);
  Serial.print("F  :  Temp out  :"); 
  Serial.print(Temp_from_heater);
  Serial.print("F  : ");
  Serial.print(" setpoint = ");
  Serial.print(SETPOINT);
  Serial.print("    pump  ");
  Serial.print(_savedpump_washer);
  Serial.print("    heater washer: ");
  Serial.print(_savedheater_washer);
  Serial.print("    heater: ");
  Serial.println(heater_status);
  
  delay(4000);
  
}
