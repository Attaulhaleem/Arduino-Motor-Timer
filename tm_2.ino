// Timing Based Motor Operation (v2.0)
// getTime() function
#include <LiquidCrystal_I2C.h>
#include <Pushbutton.h>

const byte upPin = 2;
const byte downPin = 3;
const byte startPin = 4;
const byte motorIn1 = 6;
const byte motorIn2 = 9;
const byte lcdAddress = 0x27;

LiquidCrystal_I2C lcd(lcdAddress, 16, 2);
Pushbutton upButton(upPin);
Pushbutton downButton(downPin);
Pushbutton startButton(startPin);

byte dHours;  // change in hours due to up and down buttons
byte testHours;  // hours to run motor test for
unsigned long wastedMillis; // total milliseconds wasted when timer was not running

// motor functions
void motorOn()
{
  digitalWrite(motorIn1, HIGH);
  digitalWrite(motorIn2, LOW);
}

void motorOff()
{
  digitalWrite(motorIn1, LOW);
  digitalWrite(motorIn2, LOW);
}

// time functions
byte getElapsedSeconds()
{
  return (millis() - wastedMillis) / 1000;
}

byte getSeconds()
{
  return getElapsedSeconds() % 60;
}

byte getMinutes()
{
  return getElapsedSeconds() / 60 % 60;
}

byte getHours()
{
  return getElapsedSeconds() / 3600 + dHours;
}

String formatted(byte num)
{
  return (num < 10) ? "0" + String(num) : (num < 100) ? String(num) : "ER";
}

String getTime()
{
  return formatted(getHours()) + ":" + formatted(getMinutes()) + ":" + formatted(getSeconds());
}

// lcd functions
void setTestHours()
{
  lcd.clear();
  lcd.home();
  lcd.print("Set Testing Time");
  
  while (!startButton.isPress() || testHours == 0)
  {
    lcd.setCursor(0, 1);
    lcd.print(formatted(testHours) + " Hours");
    
    if (upButton.isPress() && testHours < 99)    testHours++;
    if (downButton.isPress() && testHours > 0)   testHours--;
  }
}

void displayStart()
{
  lcd.clear();
  lcd.home();
  lcd.print("Test time is set");
  lcd.setCursor(0, 1);
  lcd.print("for " + formatted(testHours) + " hours.");
  delay(1000);
  lcd.clear();
}

void displayRealTime()
{
  lcd.home();
  lcd.print("Test Time " + formatted(testHours) + " hrs");
  lcd.setCursor(0, 1);
  lcd.print("Elapsed " + getTime());
}

void displayStaticTime(String staticTime)
{
  lcd.home();
  lcd.print(" --- PAUSED --- ");
  lcd.setCursor(0, 1);
  lcd.print("Elapsed " + staticTime);
}

void displayEnd()
{
  lcd.clear();
  lcd.home();
  lcd.print("Test Finished in");
  lcd.setCursor(0, 1);
  lcd.print(formatted(testHours) + " Hours.");
}

void pause()
{
  motorOff();
  String staticTime = getTime();
  unsigned long pauseMillis = millis(); // to calculate wasted millis

  while (!startButton.isPress())
  {
    static unsigned long flashMillis = millis();
    unsigned long dt = millis() - flashMillis;
    flashMillis = (dt > 1500) ? millis() : flashMillis;

    static bool prevLcdClear = false;
    bool lcdClear = (dt < 500);
    
    if (lcdClear != prevLcdClear) // on transition
    {
      if (lcdClear)
        lcd.clear();
      else
        displayStaticTime(staticTime);
    }
    
    prevLcdClear = lcdClear;
  }

  motorOn();
  wastedMillis += (millis() - pauseMillis); // add paused millis to wasted millis
}

// main functions
void setup()
{
  pinMode(motorIn1, OUTPUT);
  pinMode(motorIn2, OUTPUT);
  lcd.init();
  lcd.backlight();
}

void loop() // multiple tests
{
  // resetting variables for new test
  dHours = 0;
  testHours = 0;
  setTestHours();
  displayStart();
  wastedMillis = millis();

  while (getHours() < testHours)  // loop till test complete
  {
    motorOn();
    displayRealTime();

    if (upButton.isPress() && getHours() < 99) dHours++;
    
    if (downButton.isPress())
    {
      if (getHours() == 0)  // reset timer
      {
        dHours = 0;
        wastedMillis = millis();
      }
      else
        dHours--;
    }

    if (startButton.isPress())
      pause();
  }
  
  motorOff();
  displayEnd();
  while (!startButton.isPress()); // wait for start button press
}
