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

///channels////////
int channelOne = 100;
int channelTwo = 100;

int channelOneDay;
int channelTwoDay;

int channelOneNight;
int channelTwoNight;

////////Menus//////////
int menu = 0;


//////EEPROM/////////////////
byte addressChannelOne = 0;
byte addressChannelTwo = 1;
byte addressSunriceStartHour = 2;
byte addressSunriceStartMinute = 3;
byte addressSunsetStartHour = 4;
byte addressSunsetStartMinute = 5;
byte addressSunriceLongevity = 6;
byte addressSunsetLongevity = 7;

byte addressChannelOneDay = 8;
byte addressChannelTwoDay = 9;
byte addressChannelOneNight = 10;
byte addressChannelTwoNight = 11;


////////////Threads////////////////
// ThreadController that will controll all threads
ThreadController controll = ThreadController();
Thread inputThread = Thread();
Thread updateDisplayThread = Thread();
Thread sunriseCircleThread = Thread();

Thread sunsetCircleThread = Thread();

int displayChangeCounter = 0;
int displayTimeChange = 10;

int sunriceStartHour = 0;
int sunriceStartMinute = 0;

int sunsetStartHour = 0;
int sunsetStartMinute = 0;

int sunriceLongevity = 0;
int sunsetLongevity = 0;

boolean isSunset;
boolean isSunrice;


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



  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  ///////READ SETTINGS FROM EEPROM////////////
  channelOne = EEPROM.read(addressChannelOne);
  channelTwo = EEPROM.read(addressChannelTwo);

  sunriceStartHour = EEPROM.read(addressSunriceStartHour);
  sunriceStartMinute = EEPROM.read(addressSunriceStartMinute);
  sunsetStartHour = EEPROM.read(addressSunsetStartHour);
  sunsetStartMinute = EEPROM.read(addressSunsetStartMinute);

  sunriceLongevity = EEPROM.read(addressSunriceLongevity);
  sunsetLongevity = EEPROM.read(addressSunsetLongevity);

  channelOneDay = EEPROM.read(addressChannelOneDay);
  channelTwoDay = EEPROM.read(addressChannelTwoDay);

  channelOneNight = EEPROM.read(addressChannelOneNight);
  channelTwoNight = EEPROM.read(addressChannelTwoNight);

  analogWrite(channelOnePin, channelOne);
  analogWrite(channelTwoPin, channelTwo);


  // Configure myThread
  inputThread.onRun(input);
  inputThread.setInterval(100);

  // Configure hisThread
  updateDisplayThread.onRun(updateDisplay);
  updateDisplayThread.setInterval(500);

  // Configure blinkLedThread
  sunriseCircleThread.onRun(dayCircle);
  sunriseCircleThread.setInterval((sunriceLongevity * 60000) / 256);

  // Configure blinkLedThread
  sunsetCircleThread.onRun(sunsetCircle);
  sunsetCircleThread.setInterval((sunsetLongevity * 60000) / 256);

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

  if ((sunriceStartHour == now.hour()) && (sunriceStartMinute == now.minute()) && (now.second() == 0)) {
    isSunrice = true;
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
    if (menu == 11) {
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
      if (sunriceStartMinute < 59) {
        sunriceStartMinute++;

      } else if (sunriceStartMinute == 59) {
        if (sunriceStartHour < 23) {
          sunriceStartMinute = 0;
          sunriceStartHour++;
        }
      }
    }
    if (buttonLeft == HIGH) {
      if (sunriceStartMinute > 0) {
        sunriceStartMinute--;
      } else if (sunriceStartMinute == 0) {
        if (sunriceStartHour > 0) {
          sunriceStartMinute = 59;
          sunriceStartHour--;
        }
      }
    }

  }

  if (menu == 4) {
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


  if (menu == 5) {
    if (buttonRight == HIGH) {
      if (sunriceLongevity < 59) {
        sunriceLongevity++;
      }
    }
    if (buttonLeft == HIGH) {
      if (sunriceLongevity > 1) {
        sunriceLongevity--;
      }
    }
  }

  if (menu == 6) {
    if (buttonRight == HIGH) {
      if (sunsetLongevity < 59) {
        sunsetLongevity++;
      }
    }
    if (buttonLeft == HIGH) {
      if (sunsetLongevity > 1) {
        sunsetLongevity--;
      }
    }
  }

  if (menu == 7) {
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

  if (menu == 8) {
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

  if (menu == 9) {
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

  if (menu == 10) {
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
    setSunriceTime();
  } else if (menu == 4) {
    setSunsetTime();
  } else if (menu == 5) {
    setSunriceLongevity();
  } else if (menu == 6) {
    setSunsetLongevity();
  } else if (menu == 7) {
    SetChannelOneDay();
  } else if (menu == 8) {
    SetChannelTwoDay();
  } else if (menu == 9) {
    SetChannelOneNight();
  } else if (menu == 10) {
    SetChannelTwoNight();
  } else if (menu == 11) {
    saveSettings();
  }
}

// callback for dayCircle
void dayCircle() {

  if (isSunrice) {
    if (channelOne < channelOneDay) {
      channelOne++;
      analogWrite(channelOnePin, channelOne);
    }
    if (channelTwo < channelTwoDay) {
      channelTwo++;
      analogWrite(channelTwoPin, channelTwo);
    }
    if ((channelTwo == channelTwoDay) && (channelOne == channelOneDay)) {
      isSunrice = false;
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
    if ((channelOne == channelOneNight) && (channelTwo == channelTwoNight)) {
      isSunset = false;
    }
  }
}

void rootMenu() {

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

  if (isSunrice) {
    lcd.print("Sunrise");
  }
  if (isSunset) {
    lcd.print("Sunset");
  }

}

void SetChannelOne() {
  lcd.print("Set Ch 1 manual:");
  lcd.setCursor(0,1);
  lcd.print(channelOne);
  analogWrite(channelOnePin, channelOne);

}

void SetChannelTwo() {
  lcd.print("Set Ch 2 manual:");
  lcd.setCursor(0,1);
  lcd.print(channelTwo);
  analogWrite(channelTwoPin, channelTwo);

}

void saveSettings() {
  lcd.print("Save settings");
  lcd.setCursor(13, 1);
  lcd.print("Yes");

  if (buttonRight == HIGH) {
    EEPROM.update(addressChannelOne, channelOne);
    EEPROM.update(addressChannelTwo, channelTwo);

    EEPROM.update(addressSunriceStartHour, sunriceStartHour);
    EEPROM.update(addressSunriceStartMinute, sunriceStartMinute);

    EEPROM.update(addressSunsetStartHour, sunsetStartHour);
    EEPROM.update(addressSunsetStartMinute, sunsetStartMinute);

    EEPROM.update(addressSunriceLongevity, sunriceLongevity);
    EEPROM.update(addressSunsetLongevity, sunsetLongevity);


    EEPROM.update(addressChannelOneDay, channelOneDay);
    EEPROM.update(addressChannelTwoDay, channelTwoDay);
    EEPROM.update(addressChannelOneNight, channelOneNight);
    EEPROM.update(addressChannelTwoNight, channelTwoNight);


    lcd.clear();
    lcd.print("Settings saved");
    delay(3000);
    menu = 0;
  }
}

void setTime() {

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

void setSunriceTime() {
  lcd.print("Sunrise start at");
  lcd.setCursor(0, 1);
  lcd.print("     ");
  if (sunriceStartHour < 10) {
    lcd.print("0");
  }
  lcd.print(sunriceStartHour);
  lcd.print(":");

  if (sunriceStartMinute < 10) {
    lcd.print("0");
  }
  lcd.print(sunriceStartMinute);
}

void setSunriceLongevity() {
  lcd.print("Sunrise duration");
  lcd.setCursor(0, 1);
  lcd.print(sunriceLongevity);
  lcd.print(" Min.");
}

void setSunsetLongevity() {
  lcd.print("Sunset duration");
  lcd.setCursor(0, 1);
  lcd.print(sunsetLongevity);
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
