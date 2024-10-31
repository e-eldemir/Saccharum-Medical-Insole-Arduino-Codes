#include "driver/dac.h"

void setup() {
  // Set pin 25 to DAC
  dac_output_enable(DAC_CHANNEL_1); // DAC_CHANNEL_1 corresponds to GPIO25

  // Set pin 26 to PWM
  const int pwmFreq = 5000;  // PWM frequency
  const int pwmChannel = 0;  // PWM channel
  const int pwmResolution = 8;  // PWM resolution in bits

  ledcSetup(pwmChannel, pwmFreq, pwmResolution);  // Configure PWM
  ledcAttachPin(33, pwmChannel);  // Attach GPIO26 to the PWM channel
}

void loop() {
  // Generate a DAC output on pin 25
  for (int i = 0; i <= 210; i++) {
    dac_output_voltage(DAC_CHANNEL_1, i);  // Set DAC output voltage
    delay(50);  // Delay to observe the change
  }

  // Generate a PWM signal on pin 26
  for (int dutyCycle = 0; dutyCycle <= 210; dutyCycle++) {
    ledcWrite(0, dutyCycle);  // Set PWM duty cycle
    delay(50);  // Delay to observe the change
  }
}