// Main script by:
// hirez2006 @youtube
// https://www.youtube.com/watch?v=LojGHyBFE8Q
//
// Modified by:
// Jacek Radoszewski @youtube
// https://drive.google.com/open?id=1R8YxhRvAX0JNEOv4NZ_PxmUHkazco7aB
//
// My modifications (Randy Rasmussen)
// Updated default vars and options for different types of segments
// i.e. 3x7 and 4x7 digits
// Created variables for easier modification of code
// Turn off led animation/transition each minute
//
#include <DS3232RTC.h>
#include <TimeLib.h>
#include <Time.h>
#include <Wire.h>
#include <FastLED.h>

// Configure based on your setup
// Your total number of leds
#define NUM_LEDS 114 // 4x7 segment, 28 x 4 digits + 2 dots
//#define NUM_LEDS 86 // 3x7 segment, 21 x 4 digits + 2 dots
#define LED_TYPE WS2811 // Led type, i.e. WS2812B. See more: FastLED @github
#define COLOR_ORDER RGB // Define color order for your strip
#define BRIGHTNESS_HIGH 100 // Define your max brightness
#define BRIGHTNESS_LOW 70 // Define your lowest brightness
// Default brightness, i.e. BRIGHTNESS_LOW or BRIGHTNESS_HIGH
int BRIGHTNESS = BRIGHTNESS_HIGH;
// Change if you need to use a different ping for your led data connection
#define LED_PIN 6 // Data pin for led comunication
#define LIGHT_PIN 3 // Light Sensor pin
bool Dot = true;
bool DST = true; //Daylight Savings Time
long ledColor = CRGB::DarkRed; // Default clock color, can be any valid color
long DotledColor = CRGB::DarkRed; // Can be any valid color
bool UseRandomColor = false; // true or false
bool AnimateLeds = false; // Animate leds every minute
bool Use12Hour = true; // Use 12 hour format instead of 24 hour format
int BAUD_RATE = 9600;
int DAY_BRIGHT = 1; // bright at night = 1, bright during day = 0
// Random Color Table
// To use this make sure the number of colors are the same as the
// number of the ColorTable array. Default is set to 16 for 16 colors.
// Also, you need ensure that UseRandomColor is set to true (above)
// To disable change UseRandomColor to false.
// default is to not use this
// feel free to change any of these colors or add/delete
long ColorTable[16] = {
  CRGB::Amethyst,
  CRGB::Aqua,
  CRGB::Blue,
  CRGB::Chartreuse,
  CRGB::DarkGreen,
  CRGB::DarkMagenta,
  CRGB::DarkOrange,
  CRGB::DeepPink,
  CRGB::Fuchsia,
  CRGB::Gold,
  CRGB::GreenYellow,
  CRGB::LightCoral,
  CRGB::Tomato,
  CRGB::Salmon,
  CRGB::Red,
  CRGB::Orchid
};

// Use for 4 x 7 segments +2 for dots (114 leds)
// 7 segments of 4 leds = 28 total leds per digit for array below
// 1 2 3 4 5 6 7 8 9 10111213141516171819202122232425262728
byte digits[10][28] = {
  {0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // Digit 0
  {0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1}, // Digit 1
  {1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0}, // Digit 2
  {1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1}, // Digit 3
  {1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1}, // Digit 4
  {1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1}, // Digit 5
  {1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // Digit 6
  {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1}, // Digit 7
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // Digit 8
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1}  // Digit 9
}; // 2D Array for numbers on 7 segment

// Use for 3 x 7 segments +2 for dots (86 leds)
// 7 segments of 3 leds = 21 total leds per digit for array below
// 1 2 3 4 5 6 7 8 9 101112131415161718192021
//byte digits[10][21] = {
  //{0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // Digit 0
  //{0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1}, // Digit 1
  //{1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,0,0,0}, // Digit 2
  //{1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1}, // Digit 3
  //{1,1,1,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,1,1,1}, // Digit 4
  //{1,1,1,0,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1}, // Digit 5
  //{1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // Digit 6
  //{0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1}, // Digit 7
  //{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // Digit 8
  //{1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1}  // Digit 9
//}; //| 2D Array for numbers on 7 segment

// Shouldn't need to change anything below this line
int LAST_SECTION = NUM_LEDS - 4;
int LEDS_PER_DIGIT = (NUM_LEDS - 2) / 4;
int DIGIT_LOOP = LEDS_PER_DIGIT - 1;
int DOT_ONE = LEDS_PER_DIGIT * 2;
int DOT_TWO = DOT_ONE + 1;
int FIRST_DIGIT4 = NUM_LEDS - LEDS_PER_DIGIT;
int FIRST_DIGIT3 = FIRST_DIGIT4 - LEDS_PER_DIGIT;
int FIRST_DIGIT2 = FIRST_DIGIT3 - LEDS_PER_DIGIT - 2; // subtract 2 for dots
int FIRST_DIGIT1 = 0;
int last_digit = 0;
CRGB leds[NUM_LEDS]; // Define LEDs strip

void setup() {
  Serial.begin(BAUD_RATE);
  Wire.begin();
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness( BRIGHTNESS );
}

// Check Light sensor and set brightness accordingly
void BrightnessCheck() {
  const byte sensorPin = LIGHT_PIN; // light sensor pin
  const byte brightnessLow = BRIGHTNESS_LOW; // Low brightness value
  const byte brightnessHigh = BRIGHTNESS_HIGH; // High brightness value
  int sensorValue = digitalRead(sensorPin); // Read sensor
  if (sensorValue == DAY_BRIGHT) {
    Serial.println(" Brightness High");
    LEDS.setBrightness(brightnessHigh);
  }
  else { // Night = Dimmer
    Serial.println(" Brightness Low");
    LEDS.setBrightness(brightnessLow);
  }
};

// Get time in a single number
int GetTime() {
  tmElements_t Now;
  RTC.read(Now);
  int hour = Now.Hour;
  Serial.print("HOUR is: ");
  Serial.println(hour);

  if ( hour > 12 ) {
    hour = Now.Hour - 12;
  }
  else {
    hour = Now.Hour;
  }
    Serial.print("NEW_HOUR is: ");
    Serial.println(hour);
  int minutes = Now.Minute;
  int second = Now.Second;
  if (second % 2 == 0) {
    Dot = false;
  }
  else {
    Dot = true;
  };
  return (hour * 100 + minutes);
};

// Convert time to array needed for display
void TimeToArray() {
  int Now = GetTime();
  int digit_location =  0;
  // Debug output
  Serial.print("DST is: ");
  Serial.println(DST);
  Serial.print("Time is: ");
  Serial.println(Now);
  if (Dot){
    leds[DOT_ONE]=DotledColor;
    leds[DOT_TWO]=DotledColor;
  }
  else  {
    leds[DOT_ONE]=0x000000;
    leds[DOT_TWO]=0x000000;
  };

  for (int i = 1; i <= 4; i++) {
    int digit = Now % 10; // get last digit in time
    if (i == 1) {
      digit_location = FIRST_DIGIT4;
      Serial.print("Digit 4 is : ");
      Serial.print(digit);
      Serial.print(", the array is : ");
      for (int k = 0; k <= DIGIT_LOOP; k++) {
        Serial.print(digits[digit][k]);
        if (digits[digit][k] == 1) {
          leds[digit_location] = ledColor;
        }
        else if (digits[digit][k] == 0) {
          leds[digit_location] = 0x000000;
        };
        digit_location ++;
      };
      Serial.println();
      if (digit != last_digit && AnimateLeds) {
        fadefunction();
      }
      last_digit = digit;
    }
    else if (i == 2) {
      digit_location = FIRST_DIGIT3;
      Serial.print("Digit 3 is : ");
      Serial.print(digit);
      Serial.print(", the array is : ");
      for (int k = 0; k <= DIGIT_LOOP; k++) {
        Serial.print(digits[digit][k]);
        if (digits[digit][k] == 1) {
          leds[digit_location] = ledColor;
        }
        else if (digits[digit][k] == 0) {
          leds[digit_location] = 0x000000;
        };
        digit_location ++;
      };
      Serial.println();
    }
    else if (i == 3) {
      digit_location = FIRST_DIGIT2;
      Serial.print("Digit 2 is : ");
      Serial.print(digit);
      Serial.print(", the array is : ");
      for (int k = 0; k <= DIGIT_LOOP; k++) {
        Serial.print(digits[digit][k]);
        if (digits[digit][k] == 1) {
          leds[digit_location] = ledColor;
        }
        else if (digits[digit][k] == 0) {
          leds[digit_location] = 0x000000;
        };
        digit_location ++;
      };
      Serial.println();
    }
    else if (i == 4) {
      digit_location = FIRST_DIGIT1;
      Serial.print("Digit 1 is : ");
      Serial.print(digit);
      Serial.print(", the array is : ");
      for (int k = 0; k <= DIGIT_LOOP; k++) {
        Serial.print(digits[digit][k]);
        if (digits[digit][k] == 1) {
          leds[digit_location] = ledColor;
        }
        else if (digits[digit][k] == 0) {
          leds[digit_location] = 0x000000;
        };
        digit_location ++;
      };
    };
      Now /= 10;
  };
};

void fadeall() {
  for (int m = 0; m < NUM_LEDS; m++) {
    leds[m].nscale8(250);
  }
}

void dofade(int number, int myhue) {
  leds[number] = CHSV(myhue++, 255, 255); // Set the i'th led to red
  FastLED.show(); // Show the leds
  fadeall();
  delay(10); // Wait a little bit before we loop around and do it again
}
void fadefunction() {
  static uint8_t hue = 0;
  // First slide the led in one direction
  for (int i = 0; i < NUM_LEDS; i++) {
    dofade(i,hue++);
  }
  // Now go in the other direction.
  for (int i = (NUM_LEDS) - 1; i >= 0; i--) {
    dofade(i,hue++);
  }
  if (UseRandomColor) {
    ledColor = ColorTable[random(16)];
  }
}

// Main loop
void loop() {
  BrightnessCheck(); // Check brightness
  TimeToArray(); // Get leds array with required configuration
  FastLED.show(); // Display leds array
}
