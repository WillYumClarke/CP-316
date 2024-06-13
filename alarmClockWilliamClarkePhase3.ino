/*
CP316 Phase 2 Project
Mar 21 2024
William Clarke
clar5248@mylaurier.ca

Alarm Clock

*/

#include <TimeLib.h>

#define TIme_MSG_LEN 11
#define TIME_HEADER 255

#include <Servo.h>
#include <LiquidCrystal.h>

//*************************************************//
// Servo Code
Servo myServo;
int servoPos = 80;
const int MAX = 160;
const int servoPin = 10;
//*************************************************//
// distance sensor code
const int echoPin = 12;
const int trigPin = 11;
long duration;
long starttime, endtime;
//*************************************************//
// lcd screen code
const byte backlight = 13;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
char msgs[5][16] = {
  "Select Key OK",
  "Right  Key OK",
  "Left   Key OK",
  "Down   Key OK",
  "Up     Key OK"
};
int adc_key_val[5] = { 630, 820, 870, 910, 950 };
int NUM_KEYS = 5;
int adc_key_in;
int key = -1;
int oldkey = -1;

//*************************************************//
const int timeSize = 3;    // size of alarmTime array
int alarmTime[timeSize];   // array that stores the time the alarm will go off
bool alarmStatus = false;  // if the alarm is currently enabled
int position = 0;          // position variable tells me if im in hours (0), minutes (1), and seconds (2) for changeTime function

//*************************************************//
//functions
void displayTime();               // prints the time to the lcd screen (top row)
void displayAlarm();              // prints alarm status and alarm time to lcd screen (bottom row)
void key1();                      // used for reading the first keypress (menu = select; daylights savings time = left, right (spring forward, fall back); backlight = up,down (on off))
void key2();                      // used for the menu after the first keypress (changeTime = right, setAlarm = left)
int get_key(unsigned int input);  // translates keypresses, from lab
void changeTime();                // used for setting the time
void setAlarm();                  // used for setting the alarm
void alarm();                     // activates the alarm
int distance();                   // returns the current distance from the distance sensor

void setup() {
  Serial.begin(9600);
  myServo.attach(servoPin);
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(backlight, OUTPUT);
  digitalWrite(backlight, HIGH);

  alarmTime[0] = 0;
  alarmTime[1] = 0;
  alarmTime[2] = 0;

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
}

void loop() {
  while (analogRead(0) > 1000 && analogRead(0) < 1100) {                              // while there is no keypresses
    displayTime();                                                                    // print time
    displayAlarm();                                                                   // print alarm
    delay(100);                                                                       // not sure if i even need this delay
    if (alarmStatus == true && alarmTime[0] == hour() && alarmTime[1] == minute()) {  // if the alarm is enabled and the hour and minute match, alarm goes off
      alarm();
    }
  }
  key1();  // read keypress
}

void displayTime() {
  //int baseDistance = distance();
  //int newDistance;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(hour());
  lcd.print(":");
  lcd.print(minute());
  lcd.print(":");
  lcd.print(second());
  if (hour() < 12) {
    lcd.print("am");
    digitalWrite(backlight, LOW);
    //Serial.println(baseDistance);
    // newDistance = distance();
    // if (newDistance != baseDistance){
    //   Serial.println("5");
    //   digitalWrite(backlight, HIGH);
    //   delay(10000 - newDistance);
    // }
  } else {
    lcd.print("pm");
    digitalWrite(backlight, HIGH);
  }
}

void displayAlarm() {
  // display alarm
  lcd.setCursor(0, 1);
  if (alarmStatus == false) {
    lcd.print("A OFF ");
  } else {
    lcd.print("A ON ");
  }
  lcd.print(alarmTime[0]);
  lcd.print(":");
  lcd.print(alarmTime[1]);
  lcd.print(":");
  lcd.print(alarmTime[2]);
  if (alarmTime[1] < 12) {
    lcd.print("am");
  } else {
    lcd.print("pm");
  }
}

void key1() {                  // handles the first keypress
  adc_key_in = analogRead(0);  // read the value from the sensor
  key = get_key(adc_key_in);   // convert into key press
  if (key != oldkey)           // if keypress is detected
  {
    oldkey = key;
    if (key >= 0) {
      Serial.println(msgs[key]);
      switch (key) {
        case 0:  // select
          oldkey = -1;
          delay(100);
          key2();  // move to the menu function
          break;
        case 1:  // right, spring forward, increase hour by 1
          if (hour() == 23) {
            setTime(0, minute(), second(), day(), month(), year());
          } else {
            setTime(hour() + 1, minute(), second(), day(), month(), year());
          }
          oldkey = -1 delay(100);
          break;
        case 2:  // left, fall back, decrease hour by 1
          if (hour() == 0) {
            setTime(23, minute(), second(), day(), month(), year());
          } else {
            setTime(hour() - 1, minute(), second(), day(), month(), year());
          }
          oldkey = -1 delay(100);
          break;
        case 3:  // down
          digitalWrite(backlight, LOW);
          oldkey = -1;
          delay(100);
          break;
        case 4:  // up
          digitalWrite(backlight, HIGH);
          oldkey = -1;
          delay(100);
          break;
      }
    }
  }
  oldkey = -1;
  return;
}

void key2() {                                             //handles all subsequenct keypresses
  while (analogRead(0) > 1000 && analogRead(0) < 1050) {  // wait for keypress
    //do nothing
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set Time >");  // menu options
    lcd.setCursor(0, 1);
    lcd.print("Set Alarm <");
    delay(5);
  }
  adc_key_in = analogRead(0);  // read the value from the sensor
  key = get_key(adc_key_in);   // convert into key press
  delay(100);
  if (key != oldkey)  // if keypress is detected
  {
    lcd.clear();
    lcd.setCursor(0, 1);
    oldkey = key;
    if (key > 0) {
      Serial.println(msgs[key]);
      switch (key) {
        case 0:  // select
          oldkey = -1;
          delay(100);
          break;
        case 1:  // right
          oldkey = -1;
          delay(100);
          position = 0;
          changeTime();  // call the change time fucntion to change the time
          break;
        case 2:                        // left
          if (alarmStatus == false) {  // enable alarm if off, disable if on
            alarmStatus = true;
          } else {
            alarmStatus = false;
          }
          oldkey = -1;
          delay(100);
          position = 0;
          setAlarm();  // call the set alarm function to set the alarm
          break;
        case 3:  // down
          oldkey = -1;
          delay(100);
          break;
        case 4:  // up
          oldkey = -1;
          delay(100);
          break;
      }
    }
  }
  oldkey = -1;
  return;
}


int get_key(unsigned int input) {
  int k;
  for (k = 0; k < NUM_KEYS; k++) {
    if (input < adc_key_val[k]) {
      return k;
    }
  }
  if (k >= NUM_KEYS) k = -1;  // No valid key pressed
  return k;
}


void changeTime() {
  //Serial.println("ChangeTime called");
  displayTime();
  lcd.setCursor(0, 1);

  if (position == 0) {  // get the position of the cursor for changing the time
    lcd.print("Hours");
  } else if (position == 1) {
    lcd.print("Minutes");
  } else if (position == 2) {
    lcd.print("Seconds");
  } else {
    Serial.print("Position out of range");
    return;
  }
  delay(100);
  while (analogRead(0) > 1000 && analogRead(0) < 1100) {  // wait for keypress
    // do nothing
    //delay(100);
    //Serial.println(analogRead(0));
  }
  //Serial.println("no longer waiting");
  adc_key_in = analogRead(0);
  key = get_key(adc_key_in);
  delay(100);
  if (key != oldkey) {
    oldkey = key;
    if (key >= 0) {
      Serial.println(msgs[key]);
      switch (key) {
        case 0:  // select
          //return;
          oldkey = -1;
          delay(100);
          break;
        case 1:                      // right
          if (position == 0 || 1) {  // move to minutes/seconds
            position++;
          }
          oldkey = -1;
          delay(100);
          changeTime();  // recursively call the changeTime function, can only increment the time by 1 unit every call to changeTime
          break;
        case 2:                      // left
          if (position == 1 || 2) {  // move to hours/minutes
            position--;
          }
          oldkey = -1;
          delay(100);
          changeTime();  // recursively call the changeTime function
          break;
        case 3:                 // down
          if (position == 0) {  // hour
            if (hour() != 0) {
              setTime(hour() - 1, minute(), second(), day(), month(), year());  // remove hour
              delay(100);
              oldkey = -1;
              changeTime();  // recursively call the changeTime function
            } else {
              setTime(23, minute(), second(), day(), month(), year());
              delay(100);
              oldkey = -1;
              changeTime();  // recursively call the changeTime function
            }
          } else if (position == 1) {  // minute
            if (minute() != 0) {
              setTime(hour(), minute() - 1, second(), day(), month(), year());  // remove minute
              delay(100);
              oldkey = -1;
              changeTime();  // recursively call the changeTime function
            } else {
              setTime(hour(), 59, second(), day(), month(), year());
              delay(100);
              oldkey = -1;
              changeTime();  // recursively call the changeTime function
            }
          } else if (position == 2) {  // second
            if (second() != 0) {
              setTime(hour(), minute(), second() - 1, day(), month(), year());  // remove second
              delay(100);
              oldkey = -1;
              changeTime();  // recursively call the changeTime function
            } else {
              setTime(hour(), minute(), 59, day(), month(), year());
              delay(100);
              oldkey = -1;
              changeTime();  // recursively call the changeTime function
            }
          }
          break;
          //*************************************************//
        case 4:                 // up
          if (position == 0) {  // hour
            if (hour() != 23) {
              setTime(hour() + 1, minute(), second(), day(), month(), year());
              delay(100);
              oldkey = -1;
              changeTime();  // recursively call the changeTime function
            } else {
              setTime(0, minute(), second(), day(), month(), year());
              delay(100);
              oldkey = -1;
              changeTime();  // recursively call the changeTime function
            }
          } else if (position == 1) {  // minute
            if (minute() != 59) {
              setTime(hour(), minute() + 1, second(), day(), month(), year());
              delay(100);
              oldkey = -1;
              changeTime();  // recursively call the changeTime function
            } else {
              setTime(hour(), 0, second(), day(), month(), year());
              delay(100);
              oldkey = -1;
              changeTime();  // recursively call the changeTime function
            }
          } else if (position == 2) {  // second
            if (second() != 59) {
              setTime(hour(), minute(), second() + 1, day(), month(), year());
              delay(100);
              oldkey = -1;
              changeTime();  // recursively call the changeTime function
            } else {
              setTime(hour(), minute(), 0, day(), month(), year());
              delay(100);
              oldkey = -1;
              changeTime();  // recursively call the changeTime function
            }
          }
          break;
      }
    }
  }
  oldkey = -1;
  return;
}


void setAlarm() {
  Serial.println("set alarm called");
  //displayTime();
  displayAlarm();
  lcd.setCursor(0, 0);

  if (position == 0) {  // checks position of cursor
    lcd.print("Hours");
  } else if (position == 1) {
    lcd.print("Minutes");
  } else if (position == 2) {
    lcd.print("Seconds");
  } else {
    Serial.print("Position out of range");
    return;
  }
  delay(100);
  while (analogRead(0) > 1000 && analogRead(0) < 1100) {  // wait for keypress
    // do nothing
    //delay(100);
    //Serial.println(analogRead(0));
  }
  //Serial.println("ahhh");
  adc_key_in = analogRead(0);
  key = get_key(adc_key_in);
  delay(100);
  if (key != oldkey) {
    oldkey = key;
    if (key >= 0) {
      Serial.println(msgs[key]);
      switch (key) {
        case 0:  // select
          oldkey = -1;
          delay(100);
          break;
        case 1:  // right
          if (position == 0 || 1) {
            position++;
          }
          oldkey = -1;
          delay(100);
          setAlarm();  // recursively call setAlarm
          break;
        case 2:  // left
          if (position == 1 || 2) {
            position--;
          }
          oldkey = -1;
          delay(100);
          setAlarm();  // recursively call setAlarm
          break;
        case 3:  // down
          if (position == 0) { //hour
            if (alarmTime[0] != 0) {
              alarmTime[0]--;
              delay(100);
              oldkey = -1;
              setAlarm();  // recursively call setAlarm
            } else {
              alarmTime[0] = 23;
              delay(100);
              oldkey = -1;
              setAlarm();  // recursively call setAlarm
            }
          } else if (position == 1) { // minute
            if (alarmTime[1] != 0) {
              alarmTime[1]--;
              delay(100);
              oldkey = -1;
              setAlarm();  // recursively call setAlarm
            } else {
              alarmTime[1] = 59;
              delay(100);
              oldkey = -1;
              setAlarm();  // recursively call setAlarm
            }
          } else if (position == 2) { // second
            if (alarmTime[2] != 0) {
              alarmTime[2]--;
              delay(100);
              oldkey = -1;
              setAlarm();  // recursively call setAlarm
            } else {
              alarmTime[2] = 59;
              delay(100);
              oldkey = -1;
              setAlarm();  // recursively call setAlarm
            }
          }
          break;
        case 4:  // up ----------------------------------------------------------
          if (position == 0) { // hour
            if (alarmTime[0] != 23) {
              alarmTime[0]++;
              delay(100);
              oldkey = -1;
              setAlarm();  // recursively call setAlarm
            } else {
              alarmTime[0] = 0;
              delay(100);
              oldkey = -1;
              setAlarm();  // recursively call setAlarm
            }
          } else if (position == 1) { // minute
            if (alarmTime[1] != 59) {
              alarmTime[1]++;
              delay(100);
              oldkey = -1;
              setAlarm();  // recursively call setAlarm
            } else {
              alarmTime[1] = 0;
              delay(100);
              oldkey = -1;
              setAlarm();  // recursively call setAlarm
            }
          } else if (position == 2) { // second
            if (alarmTime[2] != 59) {
              alarmTime[2]++;
              delay(100);
              oldkey = -1;
              setAlarm();  // recursively call setAlarm
            } else {
              alarmTime[2] = 0;
              delay(100);
              oldkey = -1;
              setAlarm();  // recursively call setAlarm
            }
          }
          break;
      }
    }
  }
  oldkey = -1;
  return;
}

void alarm() {
  int baseDistance = distance();
  int newDistance;
  displayTime();
  Serial.println("alarm going off");
  digitalWrite(backlight, HIGH);

  while (analogRead(0) > 1000 && analogRead(0) < 1100) { // while no keypress
    newDistance = distance();
    if (baseDistance - newDistance > 100) { // if the difference in distances is significant enough to not be noise
      Serial.println("snoozin");
      displayTime();
      //Serial.println(baseDistance - newDistance);
      digitalWrite(backlight, LOW);
      delay(10000 - newDistance); // change snooze time based upon distance sensor
    }
    myServo.write(20); // move the motor as alarm goes off
    delay(250);
    myServo.write(160);
  }
  alarmStatus = false; // onc keypress, alarmStatus is disabled and alarm ends
  return;
}

int distance() {// from distance sensor lab
  digitalWrite(trigPin, HIGH);
  digitalWrite(trigPin, LOW);
  digitalWrite(trigPin, HIGH);
  while (digitalRead(echoPin) == LOW) {
  }
  starttime = micros();
  while (digitalRead(echoPin) == HIGH) {
  }
  endtime = micros();
  duration = endtime - starttime;

  digitalWrite(trigPin, LOW);

  delay(100);
  return duration;
}