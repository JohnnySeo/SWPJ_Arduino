#include <SPI.h>
#include <EEPROM.h>
#include <boards.h>
#include <RBL_nRF8001.h>
#include <Servo.h> 

 
#define DIGITAL_IN_PIN     A4
#define PWM_PIN            3
#define SERVO_PIN          5
#define ANALOG_IN_PIN      A5

Servo myservo;
const int DIGITAL_OUT_PIN = 2;
const int buzzerPin = 4;
const int songLength = 18;
char notes[] = "cdfda ag cdfdg gf "; // a space represents a rest
int beats[] = {1,1,1,1,1,1,4,4,2,1,1,1,1,1,1,4,4,2};
int tempo = 150;



void setup()
{
  // Default pins set to 9 and 8 for REQN and RDYN
  // Set your REQN and RDYN here before ble_begin() if you need
  //ble_set_pins(3, 2);
  
  // Set your BLE Shield name here, max. length 10
  ble_set_name("SWPJ_BLE");
  
  // Init. and start BLE library.
  ble_begin();
  
  // Enable serial debug
  Serial.begin(9600);
  
  pinMode(DIGITAL_OUT_PIN, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(DIGITAL_IN_PIN, INPUT);
  
  // Default to internally pull high, change it if you need
  digitalWrite(DIGITAL_IN_PIN, HIGH);
  //digitalWrite(DIGITAL_IN_PIN, LOW);
  
  myservo.attach(SERVO_PIN);
}

void loop()
{
  static boolean analog_enabled = false;
  static byte old_state = LOW;
  
  // If data is ready
  while(ble_available())
  {
    // read out command and data
    byte data0 = ble_read();
    byte data1 = ble_read();
    byte data2 = ble_read();
    
    if (data0 == 0x01)  // Command is to control digital out pin
    {
      if (data1 == 0x01){
        digitalWrite(DIGITAL_OUT_PIN, HIGH);
        delay(1000);
        digitalWrite(DIGITAL_OUT_PIN, LOW);
        delay(1000);
        digitalWrite(DIGITAL_OUT_PIN, HIGH);
        delay(1000);
        digitalWrite(DIGITAL_OUT_PIN, LOW);
        delay(1000);
        
        int i, duration;  
        for (i = 0; i < songLength; i++) {
          duration = beats[i] * tempo;  // length of note/rest in ms
          
          if (notes[i] == ' ') {
            delay(duration);            // then pause for a moment
          }
          else {
            tone(buzzerPin, frequency(notes[i]), duration);
            delay(duration);            // wait for tone to finish
          }
          delay(tempo/10);              // brief pause between notes
        }
       
        }
      else
        digitalWrite(DIGITAL_OUT_PIN, LOW);
    }
    
    else if (data0 == 0xA0) // Command is to enable analog in reading
    {
      if (data1 == 0x01)
        analog_enabled = true;
      else
        analog_enabled = false;
    }
    else if (data0 == 0x02) // Command is to control PWM pin
    {
      analogWrite(PWM_PIN, data1);
    }
    else if (data0 == 0x03)  // Command is to control Servo pin
    {
      myservo.write(data1);
    }
    else if (data0 == 0x04)
    {
      analog_enabled = false;
      myservo.write(0);
      analogWrite(PWM_PIN, 0);
      digitalWrite(DIGITAL_OUT_PIN, LOW);
    }
  }
  
  if (analog_enabled)  // if analog reading enabled
  {
    // Read and send out
    uint16_t value = analogRead(ANALOG_IN_PIN); 
    ble_write(0x0B);
    ble_write(value >> 8);
    ble_write(value);
  }
  
  // If digital in changes, report the state
  if (digitalRead(DIGITAL_IN_PIN) != old_state)
  {
    old_state = digitalRead(DIGITAL_IN_PIN);
    
    if (digitalRead(DIGITAL_IN_PIN) == HIGH)
    {
      ble_write(0x0A);
      ble_write(0x01);
      ble_write(0x00);    
    }
    else
    {
      ble_write(0x0A);
      ble_write(0x00);
      ble_write(0x00);
    }
  }
  
  if (!ble_connected())
  {
    analog_enabled = false;
    digitalWrite(DIGITAL_OUT_PIN, LOW);
  }
  
  // Allow BLE Shield to send/receive data
  ble_do_events();  
}

int frequency(char note) {  
  int i;
  const int numNotes = 8; 
  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  int frequencies[] = {262, 294, 330, 349, 392, 440, 494, 523};
  
  for (i = 0; i < numNotes; i++) {
    if (names[i] == note) {
      return(frequencies[i]);     // Yes! Return the frequency
    }
  }
  return(0);  
}

