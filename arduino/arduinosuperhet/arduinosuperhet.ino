/*********************************************************************************
Copyright (c) 2021 Dilshan R Jayakody.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*********************************************************************************/

#include "Si5351Arduino/src/si5351.h"

#define ROTARY_ENCODER_PIN_1      2
#define ROTARY_ENCODER_PIN_2      3
#define ROTARY_ENCODER_BUTTON_PIN 4

#define RECEIVER_START_FREQUENCY  700000000ULL
#define INTERMEDIATE_FREQUENCY    45500000ULL
#define STEP_FREQUENCY            100000ULL

#define TUNER_LOW_FREQUENCY       475000000ULL
#define TUNER_HIGH_FREQUENCY      1583000000ULL

typedef enum tunerDirection
{
  TUNERDIR_IDLE,
  TUNERDIR_UP,
  TUNERDIR_DOWN
} tunerDirection;

tunerDirection tunerState;
Si5351 si5351;

unsigned long long freq = RECEIVER_START_FREQUENCY;
unsigned char buttonState;
char printBuffer[50];

void updateTunerFrequency()
{
  // Update CLK0 output frequency.
  si5351.set_freq(freq + INTERMEDIATE_FREQUENCY, SI5351_CLK0);

  // Print current frequency on serial console.
  sprintf(printBuffer, "%ld", (unsigned long long)freq);
  Serial.println(printBuffer);
}

void setup()
{
  bool si5351Status;
  tunerState = TUNERDIR_IDLE;

  // Initialize UART with baud rate of 57600 with 8N1 configuration.
  Serial.begin(57600);

  // Configure rotary encorder pins and ISR.
  pinMode(ROTARY_ENCODER_PIN_1, INPUT_PULLUP);
  pinMode(ROTARY_ENCODER_PIN_2, INPUT_PULLUP);
  pinMode(ROTARY_ENCODER_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(0, onChangeISR, CHANGE);

  // Initialize Si5351 module as local oscillator.
  si5351Status = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  if(!si5351Status)
  {
    Serial.println("SI5351 module is not attached to the Arduino!");
  }  

  delay(100);

  // Enable local oscillator on CLK0.
  updateTunerFrequency();  

  // Get rotary encoder button state.
  buttonState = digitalRead(ROTARY_ENCODER_BUTTON_PIN);
}

void loop() 
{
  // Rotary encoder rotation related updates.
  if(tunerState == TUNERDIR_UP)
  {
    // Increase tuner frequency within the specified frequency range.
    if(freq < TUNER_HIGH_FREQUENCY)
    {
      freq += STEP_FREQUENCY;
      updateTunerFrequency();
    }
  }
  else if(tunerState == TUNERDIR_DOWN)
  {
    // Decrease tuner frequency within the specified frequency range.
    if(freq > TUNER_LOW_FREQUENCY)
    {
      freq -= STEP_FREQUENCY;
      updateTunerFrequency();
    }
  }

  // Rotation encoder button press event.
  if((digitalRead(ROTARY_ENCODER_BUTTON_PIN) == HIGH) && (buttonState == LOW))
  {
    // switch to next available meter band (frequency range).
    if((freq <= 506000000ULL))
    {
      // 49m band.
      freq = 590000000ULL;
    }
    else if(freq <= 620000000ULL)
    {
      // 41m band.
      freq = 720000000ULL;
    }
    else if(freq <= 760000000ULL)
    {
      // 31m band.
      freq = 940000000ULL;
    }
    else if(freq <= 990000000ULL)
    {
      // 25m band.
      freq = 1160000000ULL;
    }
    else if(freq <= 1210000000ULL)
    {
      // 22m band.
      freq = 1357000000ULL;
    }
    else if(freq <= 1387000000ULL)
    {
      // 19m band.
      freq = 1510000000ULL;
    }
    else if(freq <= TUNER_HIGH_FREQUENCY)
    {
      // Reset to lowest meter band (60m).
      freq = TUNER_LOW_FREQUENCY;
    }

    // Tune local oscillator to the newest meter band.
    updateTunerFrequency();
  }

  buttonState = digitalRead(ROTARY_ENCODER_BUTTON_PIN);
  
  // Reset rotary encorder tuner state.
  tunerState = TUNERDIR_IDLE;
  delay(50);
}

void onChangeISR()
{
  // Handle rotary encorder up/down events.
  if(digitalRead(ROTARY_ENCODER_PIN_1) == digitalRead(ROTARY_ENCODER_PIN_2))
  {
    tunerState = TUNERDIR_UP;
  }
  else
  {
    tunerState = TUNERDIR_DOWN;
  }
}
