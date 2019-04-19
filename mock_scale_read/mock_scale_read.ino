
#include "HX711.h"

// HX711 circuit wiring
#define LOADCELL_DOUT D2
#define LOADCELL_SCK D3
#define LOAD_A 32
#define LOAD_B 64
HX711 scale;

int WEIGHT_INIT (HX711 scale_to_init, float calibration_factor)
{
  scale_to_init.begin(LOADCELL_DOUT, LOADCELL_SCK);
  if (scale_to_init.is_ready())
  {
    scale_to_init.set_scale();
    scale_to_init.tare();
    scale_to_init.set_scale(calibration_factor);
    return 0;
  }
  else
  {
    return 1;
  }
}

float WEIGHT_read (HX711 scale_to_read, int load_to_read)
{
  // TODO: once mounted, apply unit conversion
  scale_to_read.set_gain(load_to_read);
  float measured_weight = scale_to_read.read();
  return measured_weight;
}

void setup() {
  // TODO: fix calibration to match mounted cells
  float calibration = -7000;
  Serial.begin(9600);
  int weight_error = WEIGHT_INIT(scale, calibration);
  if (weight_error)
  {
    Serial.println("HX711 not found.");
  }
  else
  {
    Serial.println("HX711 ready.");
  }
}

void loop() {
  float reading = WEIGHT_read(scale, LOAD_A);
  Serial.print("HX711 reading: ");
  Serial.println(reading);
  delay(1000);
}
