#include <SoftwareSerial.h>
#include "timer_handler.h"
#include"buzzer_handler.h"
#include "ultrasonic_handler.h"
#define DEBUG true
  #if !DEBUG
  #define log(...)
  #else
  #define log(fmt, ...) consoleLog(fmt, ##__VA_ARGS__)
#endif


// CONSTS
#define MAXSPEED 433
#define STEP 24

  // Only use port 0 to 7;
#define dirLeft 3
#define stepLeft 2
#define dirRight 5
#define stepRight 4

#define TXpin 7
#define RXpin 8

#define TriggerPin 9
#define EchoPin 10

#define BUZZER_PIN 11
//------------------------------------------------
#define dirRightBackward 0b00000000 | 1 << dirRight     // 00100000
#define dirRightForward (0b11111111 & ~(1 << dirRight)) // 11011111
#define stepRightHIGH 0b00000000 | 1 << stepRight       // 00010000
#define stepRightLOW (0b11111111 & ~(1 << stepRight))   // 11101111
#define dirLeftForward 0b00000000 | 1 << dirLeft        // 00001000
#define dirLeftBackward (0b11111111 & ~(1 << dirLeft))  // 11110111
#define stepLeftHIGH 0b00000000 | 1 << stepLeft         // 00000100
#define stepLeftLOW (0b11111111 & ~(1 << stepLeft))     // 11111011
#define GYRO_ADDRESS 0x68
//------------------------------------------------
SoftwareSerial HC05(TXpin, RXpin);   // TX | RX of hc05
//------------------------------------------------


char received_byte;

int  throttle_left_motor, throttle_counter_left_motor, throttle_left_motor_memory;
int  throttle_right_motor, throttle_counter_right_motor, throttle_right_motor_memory;

int receive_counter;
int gyro_pitch_data_raw, gyro_yaw_data_raw, accelerometer_data_raw;


float speed;
float turn;
bool stop = false;

bool want_send_distance = true;


void recieveData(){
  if(HC05.available()){                                                   
    received_byte = HC05.read();
    //log("recieved : ", received_byte);
    switch(received_byte){
      case 'w':
        turn = 0;
        stop=false;
        break;
      case 'd':
        if(turn + STEP/2 + speed < MAXSPEED){
        stop=false;
          turn-=STEP/2;
        }
        break;
      case 'a':
        if(turn + STEP/2 + speed < MAXSPEED){
        stop=false;
          turn += STEP/2;
        }
        break;
      case 'i':
        if(speed-STEP-turn>=-MAXSPEED){
        stop=false;
          speed+=STEP;
        }
        break;
      case 'o':
        if(speed+STEP+turn<=MAXSPEED){
          stop=false;
          speed-=STEP;
        }
        break;
      case ' ':
        //speed = 0;
        stop = true;
        break;
      case 'q':
        //speed = 0;
        turn = -turn;
        break;
      case 'b':
        digitalWrite(BUZZER_PIN,!digitalRead(BUZZER_PIN));
        log("buzzer: ", digitalRead(BUZZER_PIN));
        break;
      case 'm':
        want_send_distance = !want_send_distance;
        break;
    }
  }
}

int convertToLinear(float to_convert){
  to_convert = (to_convert > 0) ? 405 - (1/(to_convert + 9)) * 5500 : ((to_convert < 0)?-405 - (1/(to_convert - 9)) * 5500:to_convert);
  //Calculate the needed pulse time for the left and right stepper motor controllers
  return (to_convert > 0) ?  400 - to_convert : ((to_convert < 0) ? -400 - to_convert : 0);
}


void setup(){
  Serial.begin(115200);
    //3h wana kan9lb 3la error flkhr l9it bli 9600 is too slow, 
    //fach katprinti biha shi haja it slows down the loop, try, 
    //ghaybanlk valeur d gyro scope(which depend on the loop time) ghaltin
  HC05.begin(9600);
  log("~ Setup Started ~");
  timer::setup();
  ultraSonic::setup(TriggerPin, EchoPin);
  buzzer::setup(BUZZER_PIN);
  pinMode(stepLeft, OUTPUT);
  pinMode(dirLeft, OUTPUT);
  pinMode(stepRight, OUTPUT);
  pinMode(dirRight, OUTPUT);
  pinMode(13, OUTPUT);
  log("~ Setup Done ~");
  buzzer::beep(2, 40);
}


unsigned long previousTimeBuzzer; 
unsigned long currentTimeBuzzer;
unsigned long previousTimeUltrasonic; 
unsigned long currentTimeUltrasonic;
void loop(){
  buzzer::HandleBuzzer();
  if(want_send_distance){
    currentTimeUltrasonic = millis();
    if (currentTimeUltrasonic - previousTimeUltrasonic >= 500){
      previousTimeUltrasonic = currentTimeUltrasonic;
      HC05.println(ultraSonic::calculateDistance());
    }
  }
  recieveData();
  if(stop){
    currentTimeBuzzer = millis();
    if (currentTimeBuzzer - previousTimeBuzzer >= 20){
      
    previousTimeBuzzer = currentTimeBuzzer;
    currentTimeBuzzer =millis();
    turn = 0;
    if(speed > 0){
      speed-=STEP/2;
    }
    else if (speed<0){
      speed+=STEP/2;
    }
    }
  }
  
  throttle_left_motor = convertToLinear(speed + turn);
  throttle_right_motor = convertToLinear(speed - turn);
  
  //log("throttle_left_motor:",throttle_left_motor,",throttle_right_motor:",throttle_right_motor,",speed:",speed,",turn:",turn);
}

