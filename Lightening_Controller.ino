// include the library code:
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Time.h>
#include "RTClib.h"
RTC_DS3231 rtc;
#include <Thread.h>
#include <ThreadController.h>

/*
  LiquidCrystal Library - scrollDisplayLeft() and scrollDisplayRight()

  The circuit:
   LCD RS pin to digital pin 12
   LCD Enable pin to digital pin 11
   LCD D4 pin to digital pin 5
   LCD D5 pin to digital pin 4
   LCD D6 pin to digital pin 3
   LCD D7 pin to digital pin 2
   LCD R/W pin to ground
   10K resistor:
   ends to +5V and ground
   wiper to LCD VO pin (pin 3)
*/

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 11, en = 12, d4 = 2, d5 = 3, d6 = 4, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

////Buttons/////////////
const int buttonPinSelect = 24;
const int buttonPinLeft = 26;
const int buttonPinRight = 28;
int buttonSelect = 0;
int buttonLeft = 0;
int buttonRight = 0;

/////LedsPins/////
const int channelOnePin = 10;
const int channelTwoPin = 9;
const int channelThreePin = 8;

///channels////////
int channelOne = 100;
int channelTwo = 100;
int channelThree = 100;

int channelOneDay;
int channelTwoDay;
int channelThreeDay;

int channelOneNight;
int channelTwoNight;
int channelThreeNight;

////////Menus//////////
int menu = 0;


//////EEPROM/////////////////
byte addressChannelOne = 0;
byte addressChannelTwo = 1;
byte addressChannelThree = 2;
byte addressSunriseStartHour = 3;
byte addressSunriseStartMinute = 4;
byte addressSunsetStartHour = 5;
byte addressSunsetStartMinute = 6;
byte addressSunriseDuration = 7;
byte addressSunsetDuration = 8;

byte addressChannelOneDay = 9;
byte addressChannelTwoDay = 10;
byte addressChannelThreeDay = 11;
byte addressChannelOneNight = 12;
byte addressChannelTwoNight = 13;
byte addressChannelThreeNight = 14;
////////Set Time Variables//////////
int varHour;
int varMinute;
int varSecond;
boolean isTimeChanged = false;

////////////Threads////////////////
// ThreadController that will controll all threads
ThreadController controll = ThreadController();
Thread inputThread = Thread();
Thread updateDisplayThread = Thread();
Thread sunriseCircleThread = Thread();

Thread sunsetCircleThread = Thread();

int displayChangeCounter = 0;
int displayTimeChange = 10;

int sunriseStartHour = 0;
int sunriseStartMinute = 0;

int sunsetStartHour = 0;
int sunsetStartMinute = 0;

int sunriseDuration = 0;
int sunsetDuration = 0;

boolean isSunset;
boolean isSunrise;


void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }


  ///buttons setup/////
  pinMode(buttonPinSelect, INPUT);
  pinMode(buttonPinLeft, INPUT);
  pinMode(buttonPinRight, INPUT);
  pinMode(channelOnePin, OUTPUT);
  pinMode(channelTwoPin, OUTPUT);
  pinMode(channelThreePin, OUTPUT);



  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  ///////READ SETTINGS FROM EEPROM////////////
  channelOne = EEPROM.read(addressChannelOne);
  channelTwo = EEPROM.read(addressChannelTwo);
  channelThree = EEPROM.read(addressChannelThree);

  sunriseStartHour = EEPROM.read(addressSunriseStartHour);
  sunriseStartMinute = EEPROM.read(addressSunriseStartMinute);
  sunsetStartHour = EEPROM.read(addressSunsetStartHour);
  sunsetStartMinute = EEPROM.read(addressSunsetStartMinute);

  sunriseDuration = EEPROM.read(addressSunriseDuration);
  sunsetDuration = EEPROM.read(addressSunsetDuration);

  channelOneDay = EEPROM.read(addressChannelOneDay);
  channelTwoDay = EEPROM.read(addressChannelTwoDay);
  channelThreeDay = EEPROM.read(addressChannelThreeDay);
  
  channelOneNight = EEPROM.read(addressChannelOneNight);
  channelTwoNight = EEPROM.read(addressChannelTwoNight);
  channelThreeNight = EEPROM.read(addressChannelThreeNight);

  analogWrite(channelOnePin, channelOne);
  analogWrite(channelTwoPin, channelTwo);
  analogWrite(channelThreePin, channelThree);


  // Configure myThread
  inputThread.onRun(input);
  inputThread.setInterval(100);

  // Configure hisThread
  updateDisplayThread.onRun(updateDisplay);
  updateDisplayThread.setInterval(500);

  // Configure blinkLedThread
  sunriseCircleThread.onRun(sunriseCircle);
  sunriseCircleThread.setInterval((sunriseDuration * 60000) / 256);

  // Configure blinkLedThread
  sunsetCircleThread.onRun(sunsetCircle);
  sunsetCircleThread.setInterval((sunsetDuration * 60000) / 256);

  // Adds myThread to the controll
  controll.add(&inputThread);
  controll.add(&updateDisplayThread);
  controll.add(&sunriseCircleThread);
  controll.add(&sunsetCircleThread);
}

void loop() {
  DateTime now = rtc.now();
  // put your main code here, to run repeatedly:

  // run ThreadController
  // this will check every thread inside ThreadController,
  // if it should run. If yes, he will run it;
  controll.run();
  // Rest of code

  if ((sunriseStartHour == now.hour()) && (sunriseStartMinute == now.minute()) && (now.second() == 0)) {
    isSunrise = true;
  }
  if ((sunsetStartHour == now.hour()) && (sunsetStartMinute == now.minute()) && (now.second() == 0)) {
    isSunset = true;
  }

}



// callback for input
void input() {
  buttonSelect = digitalRead(buttonPinSelect);
  buttonLeft = digitalRead(buttonPinLeft);
  buttonRight = digitalRead(buttonPinRight);


  if (buttonSelect == HIGH) {
    if (menu == 16) {
      menu = 0;
    } else {
      menu++;
    }
    delay(200);
  }

  if (menu == 1) {
    if (buttonRight == HIGH) {
      if (channelOne < 255) {
        channelOne++;
      }
    }
    if (buttonLeft == HIGH) {
      if (channelOne > 0) {
        channelOne--;
      }
    }
  }

  if (menu == 2) {
    if (buttonRight == HIGH) {
      if (channelTwo < 255) {
        channelTwo++;
      }
    }
    if (buttonLeft == HIGH) {
      if (channelTwo > 0) {
        channelTwo--;
      }
    }
  }

 if (menu == 3) {
    if (buttonRight == HIGH) {
      if (channelThree < 255) {
        channelThree++;
      }
    }
    if (buttonLeft == HIGH) {
      if (channelThree > 0) {
        channelThree--;
      }
    }
  }

  if (menu == 4) {
    if (buttonRight == HIGH) {
      if (sunriseStartMinute < 59) {
        sunriseStartMinute++;

      } else if (sunriseStartMinute == 59) {
        if (sunriseStartHour < 23) {
          sunriseStartMinute = 0;
          sunriseStartHour++;
        }
      }
    }
    if (buttonLeft == HIGH) {
      if (sunriseStartMinute > 0) {
        sunriseStartMinute--;
      } else if (sunriseStartMinute == 0) {
        if (sunriseStartHour > 0) {
          sunriseStartMinute = 59;
          sunriseStartHour--;
        }
      }
    }

  }

  if (menu == 5) {
    if (buttonRight == HIGH) {
      if (sunsetStartMinute < 59) {
        sunsetStartMinute++;

      } else if (sunsetStartMinute == 59) {
        if (sunsetStartHour < 23) {
          sunsetStartMinute = 0;
          sunsetStartHour++;
        }
      }
    }
    if (buttonLeft == HIGH) {
      if (sunsetStartMinute > 0) {
        sunsetStartMinute--;
      } else if (sunsetStartMinute == 0) {
        if (sunsetStartHour > 0) {
          sunsetStartMinute = 59;
          sunsetStartHour--;
        }
      }
    }

  }


  if (menu == 6) {
    if (buttonRight == HIGH) {
      if (sunriseDuration < 59) {
        sunriseDuration++;
      }
    }
    if (buttonLeft == HIGH) {
      if (sunriseDuration > 1) {
        sunriseDuration--;
      }
    }
  }

  if (menu == 7) {
    if (buttonRight == HIGH) {
      if (sunsetDuration < 59) {
        sunsetDuration++;
      }
    }
    if (buttonLeft == HIGH) {
      if (sunsetDuration > 1) {
        sunsetDuration--;
      }
    }
  }

  if (menu == 8) {
    if (buttonRight == HIGH) {
      if (channelOneDay < 255) {
        channelOneDay++;
      }
    }
    if (buttonLeft == HIGH) {
      if (channelOneDay > 0) {
        channelOneDay--;
      }
    }

  }

  if (menu == 9) {
    if (buttonRight == HIGH) {
      if (channelTwoDay < 255) {
        channelTwoDay++;
      }
    }
    if (buttonLeft == HIGH) {
      if (channelTwoDay > 0) {
        channelTwoDay--;
      }
    }

  }

  if (menu == 10) {
    if (buttonRight == HIGH) {
      if (channelThreeDay < 255) {
        channelThreeDay++;
      }
    }
    if (buttonLeft == HIGH) {
      if (channelThreeDay > 0) {
        channelThreeDay--;
      }
    }

  }

  if (menu == 11) {
    if (buttonRight == HIGH) {
      if (channelOneNight < 255) {
        channelOneNight++;
      }
    }
    if (buttonLeft == HIGH) {
      if (channelOneNight > 0) {
        channelOneNight--;
      }
    }
  }

  if (menu == 12) {
    if (buttonRight == HIGH) {
      if (channelTwoNight < 255) {
        channelTwoNight++;
      }
    }
    if (buttonLeft == HIGH) {
      if (channelTwoNight > 0) {
        channelTwoNight--;
      }
    }
  }

  if (menu == 13) {
    if (buttonRight == HIGH) {
      if (channelThreeNight < 255) {
        channelThreeNight++;
      }
    }
    if (buttonLeft == HIGH) {
      if (channelThreeNight > 0) {
        channelThreeNight--;
      }
    }
  }

  if (menu == 14) {
    
    if (buttonRight == HIGH) {
      isTimeChanged = true;
      if (varHour < 23) {
        if (varMinute == 59) {
          varHour++;
          varMinute = 0;
        } else {
          varMinute++;
        }
      }else if(varHour == 23){
          if (varMinute < 59){
            varMinute++;  
          }
      }
    }

    if (buttonLeft == HIGH) {
      isTimeChanged = true;
      if (varHour > 0){
        if (varMinute > 0){
          varMinute -= 1;
        }else if(varMinute == 0){
         varMinute = 59;
         varHour-=1;  
        }
      }else if (varHour == 0){
        if (varMinute > 0){
          varMinute -= 1;
        }
     }   
    }   
  }
}

// callback for Display Update
void updateDisplay() {
  lcd.clear();
  if (menu == 0) {
    rootMenu();
  } else if (menu == 1) {
    SetChannelOne();
  } else if (menu == 2) {
    SetChannelTwo();
  } else if (menu == 3) {
    SetChannelThree();
  } else if (menu == 4) {
    setSunriseTime();
  } else if (menu == 5) {
    setSunsetTime();
  } else if (menu == 6) {
    setSunriseDuration();
  } else if (menu == 7) {
    setSunsetDuration();
  } else if (menu == 8) {
    SetChannelOneDay();
  } else if (menu == 9) {
    SetChannelTwoDay();
  }else if (menu == 10) {
    SetChannelThreeDay();
  } else if (menu == 11) {
    SetChannelOneNight();
  } else if (menu == 12) {
    SetChannelTwoNight();
  } else if (menu == 13) {
    SetChannelThreeNight();
  } else if (menu == 14) {
    setMyClock();
  } else if (menu == 15) {
    saveSettings();
  }
}

// callback for dayCircle
void sunriseCircle() {

  if (isSunrise) {
    if (channelOne < channelOneDay) {
      channelOne++;
      analogWrite(channelOnePin, channelOne);
    }
    if (channelTwo < channelTwoDay) {
      channelTwo++;
      analogWrite(channelTwoPin, channelTwo);
    }
    if (channelThree < channelThreeDay) {
      channelThree++;
      analogWrite(channelThreePin, channelThree);
    }
    
    if ((channelTwo == channelTwoDay) && (channelOne == channelOneDay)&& (channelThree == channelThreeDay)) {
      isSunrise = false;
    }
  }
}

void sunsetCircle() {
  if (isSunset) {
    if (channelOne > channelOneNight) {
      channelOne--;
      analogWrite(channelOnePin, channelOne);
    }
    if (channelTwo > channelTwoNight) {
      channelTwo--;
      analogWrite(channelTwoPin, channelTwo);
    }
    if (channelThree > channelThreeNight) {
      channelThree--;
      analogWrite(channelThreePin, channelThree);
    }
    if ((channelOne == channelOneNight) && (channelTwo == channelTwoNight) && (channelThree == channelThreeNight)) {
      isSunset = false;
    }
  }
}

void rootMenu() {

//  Serial.print(__DATE__);
//  Serial.print(" ");
//  Serial.println(__TIME__);
  
  DateTime now = rtc.now();
  lcd.print("    ");

  //show clock to lcd
  if (now.hour() < 10) {
    lcd.print('0');
  }
  lcd.print(now.hour());
  lcd.print(':');
  if (now.minute() < 10) {
    lcd.print('0');
  }
  lcd.print(now.minute());
  lcd.print(':');
  if (now.second() < 10) {
    lcd.print('0');
  }
  lcd.print(now.second());
  ///////////////////////////
  lcd.setCursor(0, 1);
  lcd.print(channelOne);
  lcd.print(" ");
  lcd.print(channelTwo);
  lcd.print(" ");
  lcd.print(channelThree);
  lcd.print(" ");
  
  if (isSunrise) {
    lcd.print("Sunrise");
  }
  if (isSunset) {
    lcd.print("Sunset");
  }
}

void SetChannelOne() {
  lcd.print("Set Ch 1 manual:");
  lcd.setCursor(0, 1);
  lcd.print(channelOne);
  analogWrite(channelOnePin, channelOne);

}

void SetChannelTwo() {
  lcd.print("Set Ch 2 manual:");
  lcd.setCursor(0, 1);
  lcd.print(channelTwo);
  analogWrite(channelTwoPin, channelTwo);
}

void SetChannelThree() {
  lcd.print("Set Ch 3 manual:");
  lcd.setCursor(0, 1);
  lcd.print(channelThree);
  analogWrite(channelThreePin, channelThree);
}

void saveSettings() {
  lcd.print("Save settings");
  lcd.setCursor(13, 1);
  lcd.print("Yes");

  if (buttonRight == HIGH) {
    EEPROM.update(addressChannelOne, channelOne);
    EEPROM.update(addressChannelTwo, channelTwo);
    EEPROM.update(addressChannelThree, channelThree);

  
    EEPROM.update(addressSunriseStartHour, sunriseStartHour);
    EEPROM.update(addressSunriseStartMinute, sunriseStartMinute);

    EEPROM.update(addressSunsetStartHour, sunsetStartHour);
    EEPROM.update(addressSunsetStartMinute, sunsetStartMinute);

    EEPROM.update(addressSunriseDuration, sunriseDuration);
    EEPROM.update(addressSunsetDuration, sunsetDuration);


    EEPROM.update(addressChannelOneDay, channelOneDay);
    EEPROM.update(addressChannelTwoDay, channelTwoDay);
    EEPROM.update(addressChannelThreeDay, channelThreeDay);
    
    EEPROM.update(addressChannelOneNight, channelOneNight);
    EEPROM.update(addressChannelTwoNight, channelTwoNight);
    EEPROM.update(addressChannelTwoNight, channelThreeNight);

    if(isTimeChanged){
      rtc.adjust(DateTime(2017, 12, 4, varHour, varMinute, 0));
      isTimeChanged = false;
      varHour = 0;
      varMinute = 0;
    }

    lcd.clear();
    lcd.print("Settings saved");
    delay(3000);
    menu = 0;
  }
}

void setSunsetTime() {
  lcd.print("Sunset start at");
  lcd.setCursor(0, 1);
  lcd.print("     ");
  if (sunsetStartHour < 10) {
    lcd.print("0");
  }
  lcd.print(sunsetStartHour);
  lcd.print(":");

  if (sunsetStartMinute < 10) {
    lcd.print("0");
  }
  lcd.print(sunsetStartMinute);
}

void setSunriseTime() {
  lcd.print("Sunrise start at");
  lcd.setCursor(0, 1);
  lcd.print("     ");
  if (sunriseStartHour < 10) {
    lcd.print("0");
  }
  lcd.print(sunriseStartHour);
  lcd.print(":");

  if (sunriseStartMinute < 10) {
    lcd.print("0");
  }
  lcd.print(sunriseStartMinute);
}

void setSunriseDuration() {
  lcd.print("Sunrise duration");
  lcd.setCursor(0, 1);
  lcd.print(sunriseDuration);
  lcd.print(" Min.");
}

void setSunsetDuration() {
  lcd.print("Sunset duration");
  lcd.setCursor(0, 1);
  lcd.print(sunsetDuration);
  lcd.print(" Min.");
}


void SetChannelOneDay() {
  lcd.print("Channel 1 Day: ");
  lcd.setCursor(0, 1);
  lcd.print(channelOneDay);

}

void SetChannelTwoDay() {
  lcd.print("Channel 2 Day: ");
  lcd.setCursor(0, 1);
  lcd.print(channelTwoDay);
}
void SetChannelThreeDay() {
  lcd.print("Channel 3 Day: ");
  lcd.setCursor(0, 1);
  lcd.print(channelThreeDay);
}

void SetChannelOneNight() {
  lcd.print("Channel 1 Night: ");
  lcd.setCursor(0, 1);
  lcd.print(channelOneNight);
}

void SetChannelTwoNight() {
  lcd.print("Channel 2 Night: ");
  lcd.setCursor(0, 1);
  lcd.print(channelTwoNight);
}

void SetChannelThreeNight() {
  lcd.print("Channel 3 Night: ");
  lcd.setCursor(0, 1);
  lcd.print(channelThreeNight);
}

void setMyClock() {

  varHour;
  varMinute;
  varSecond;

  lcd.print("    Set time");
  lcd.setCursor(4, 1);
  //show clock to lcd
  if (varHour < 10) {
    lcd.print('0');
  }
  lcd.print(varHour);
  lcd.print(':');
  if (varMinute < 10) {
    lcd.print('0');
  }
  lcd.print(varMinute);
  lcd.print(':');
  if (varSecond < 10) {
    lcd.print('0');
  }
  lcd.print(varSecond);
}
