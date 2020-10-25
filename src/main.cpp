#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

#define STRIP_COUNT 40
#define STRIP_PIN 6
#define FAN_COUNT 40
#define FAN_PIN 5

#define BUTTON 4

#define MODE_SHUTDOWN 0
#define MODE_CHENILLE 1
#define MODE_DOUBLE_CHENILLE 2
#define MODE_PULSE 3
#define MODE_BLINK 4
#define MODE_THEATRE 5
#define MODE_RAINBOW 6
#define MODE_MAX 7

#define MODE_BUTTON_SAVE 0
#define MODE_BUTTON_HUE_1 1
#define MODE_BUTTON_SAT_1 2
#define MODE_BUTTON_VAL_1 3
#define MODE_BUTTON_HUE_2 4
#define MODE_BUTTON_SAT_2 5
#define MODE_BUTTON_VAL_2 6
#define MODE_BUTTON_MODE 7
#define MODE_BUTTON_MAX 8

Adafruit_NeoPixel strip(STRIP_COUNT, STRIP_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel fan(FAN_COUNT, FAN_PIN, NEO_GRB + NEO_KHZ800);

int buttonMode = MODE_BUTTON_SAVE;

uint8_t buttonState = 1;
uint8_t state = 0;

struct light_t
{
  uint8_t mode = MODE_CHENILLE;
  uint16_t h1 = 0, h2 = 0;
  uint8_t s1 = 255, s2 = 0;
  uint8_t v1 = 255, v2 = 255;
};

light_t light;

uint32_t color1;
uint32_t color2;

void setup()
{
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  strip.begin();
  strip.show();
  fan.begin();
  fan.show();
  Serial.begin(9600);
  EEPROM.get(0, light);
  color1 = strip.ColorHSV(light.h1, light.s1, light.v1);
  color2 = strip.ColorHSV(light.h2, light.s2, light.v2);
}

uint32_t color(uint8_t v, uint8_t s, uint8_t e, uint32_t cs, uint32_t ce)
{
  uint8_t r1 = (cs & 0xff0000) >> 16;
  uint8_t g1 = (cs & 0x00ff00) >> 8;
  uint8_t b1 = (cs & 0x0000ff);
  uint8_t r2 = (ce & 0xff0000) >> 16;
  uint8_t g2 = (ce & 0x00ff00) >> 8;
  uint8_t b2 = (ce & 0x0000ff);
  uint8_t r = map(v, s, e, r1, r2);
  uint8_t g = map(v, s, e, g1, g2);
  uint8_t b = map(v, s, e, b1, b2);
  return strip.Color(r, g, b);
}

uint32_t color(uint8_t v, uint8_t s, uint8_t e)
{
  return color(v, s, e, color1, color2);
}

void displayChenille()
{
  uint8_t base = map(7, 0, STRIP_COUNT, 0, 128);
  for (uint8_t led = 0; led < STRIP_COUNT; led++)
  {
    uint8_t step = (map(led, 0, STRIP_COUNT, 0, 128) + state) % 128;
    uint8_t value = min(step, base);
    strip.setPixelColor(led, color(value, 0, base));
  }
  base = map(7, 0, FAN_COUNT, 0, 128);
  for (uint8_t led = 0; led < FAN_COUNT; led++)
  {
    uint8_t step = (map(led, 0, FAN_COUNT, 0, 128) + state) % 128;
    uint8_t value = min(step, base);
    fan.setPixelColor(led, color(value, 0, base));
  }
}

void displayDoubleChenille()
{
  uint8_t base = map(7, 0, STRIP_COUNT, 0, 128);
  for (int8_t led = 0; led < STRIP_COUNT; led++)
  {
    uint8_t step = (map(led, 0, STRIP_COUNT, 0, 128) + state) % 64;
    uint8_t value = min(step, base);
    strip.setPixelColor(led, color(value, 0, base));
  }
  base = map(7, 0, FAN_COUNT, 0, 128);
  for (int8_t led = 0; led < FAN_COUNT; led++)
  {
    uint8_t step = (map(led, 0, FAN_COUNT, 0, 128) + state) % 64;
    uint8_t value = min(step, base);
    fan.setPixelColor(led, color(value, 0, base));
  }
}

void displayPulse()
{
  uint32_t c = color(abs(64 - state), 0, 64);
  for (int8_t led = 0; led < STRIP_COUNT; led++)
  {
    strip.setPixelColor(led, c);
  }
  for (int8_t led = 0; led < FAN_COUNT; led++)
  {
    fan.setPixelColor(led, c);
  }
}

void displayBlink()
{
  uint32_t c;
  if (state < 64)
  {
    c = color(abs(32 - state), 0, 32, color1, strip.Color(0, 0, 0));
  }
  else
  {
    c = color(abs(96 - state), 0, 32, color2, strip.Color(0, 0, 0));
  }
  for (int8_t led = 0; led < STRIP_COUNT; led++)
  {
    strip.setPixelColor(led, c);
  }
  for (int8_t led = 0; led < FAN_COUNT; led++)
  {
    fan.setPixelColor(led, c);
  }
}

void displayTheatre()
{
  for (int8_t led = 0; led < STRIP_COUNT; led++)
  {
    uint8_t s = (map(led, 0, STRIP_COUNT, 0, 128) + state) % 128;
    strip.setPixelColor(led, strip.ColorHSV(map(s, 0, 128, 0, 65535), 255, 255));
  }
  for (int8_t led = 0; led < FAN_COUNT; led++)
  {
    uint8_t s = (map(led, 0, FAN_COUNT, 0, 128) + state) % 128;
    fan.setPixelColor(led, fan.ColorHSV(map(s, 0, 128, 0, 65535), 255, 255));
  }
}

void displayRainbow()
{
  uint32_t c = strip.ColorHSV(map(state, 0, 128, 0, 65535), 255, 255);
  for (int8_t led = 0; led < STRIP_COUNT; led++)
  {
    strip.setPixelColor(led, c);
  }
  for (int8_t led = 0; led < FAN_COUNT; led++)
  {
    fan.setPixelColor(led, c);
  }
}

uint8_t speed()
{
  switch (light.mode)
  {
  case MODE_CHENILLE:
  case MODE_DOUBLE_CHENILLE:
    return 3;
  case MODE_BLINK:
  case MODE_PULSE:
    return 5;
  case MODE_THEATRE:
    return 2;
  case MODE_RAINBOW:
    return 8;
  default:
    return 10;
  }
}

void updateColors()
{
  switch (buttonMode)
  {
  case MODE_BUTTON_MODE:
    light.mode = map(analogRead(A0), 0, 1023, 1, MODE_MAX);
    break;
  case MODE_BUTTON_HUE_1:
    light.h1 = map(analogRead(A0), 0, 1023, 0, 65535);
    light.s1 = 255;
    light.v1 = 255;
    color1 = strip.ColorHSV(light.h1, light.s1, light.v1);
    break;
  case MODE_BUTTON_SAT_1:
    light.s1 = map(analogRead(A0), 0, 1023, 0, 255);
    color1 = strip.ColorHSV(light.h1, light.s1, light.v1);
    break;
  case MODE_BUTTON_VAL_1:
    light.v1 = map(analogRead(A0), 0, 1023, 0, 255);
    color1 = strip.ColorHSV(light.h1, light.s1, light.v1);
    break;
  case MODE_BUTTON_HUE_2:
    light.h2 = map(analogRead(A0), 0, 1023, 0, 65535);
    light.s2 = 255;
    light.v2 = 255;
    color2 = strip.ColorHSV(light.h2, light.s2, light.v2);
    break;
  case MODE_BUTTON_SAT_2:
    light.s2 = map(analogRead(A0), 0, 1023, 0, 255);
    color2 = strip.ColorHSV(light.h2, light.s2, light.v2);
    break;
  case MODE_BUTTON_VAL_2:
    light.v2 = map(analogRead(A0), 0, 1023, 0, 255);
    color2 = strip.ColorHSV(light.h2, light.s2, light.v2);
    break;
  }
}

void loop()
{
  switch (light.mode)
  {
  case MODE_CHENILLE:
    displayChenille();
    break;
  case MODE_DOUBLE_CHENILLE:
    displayDoubleChenille();
    break;
  case MODE_PULSE:
    displayPulse();
    break;
  case MODE_BLINK:
    displayBlink();
    break;
  case MODE_THEATRE:
    displayTheatre();
    break;
  case MODE_RAINBOW:
    displayRainbow();
    break;
  }
  strip.show();
  fan.show();

  updateColors();

  state++;
  if (state >= 128)
  {
    state = 0;
  }

  for (int i = 0; i < speed(); i++)
  {
    delay(10);
    if (digitalRead(BUTTON) == 0 && buttonState)
    {
      buttonMode++;
      if (buttonMode == MODE_BUTTON_MAX)
      {
        buttonMode = 0;
        EEPROM.put(0, light);
        for (int j = 0; j < 20; j++)
        {
          digitalWrite(LED_BUILTIN, 1);
          delay(20);
          digitalWrite(LED_BUILTIN, 0);
          delay(200);
        }
      }
      for (int j = 0; j < buttonMode; j++)
      {
        digitalWrite(LED_BUILTIN, 1);
        delay(20);
        digitalWrite(LED_BUILTIN, 0);
        delay(200);
      }
    }
    buttonState = digitalRead(BUTTON);
  }
}