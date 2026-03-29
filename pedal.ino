#include "MIDIUSB.h"

#define MATRIX_OUT_1 A5
#define MATRIX_OUT_2 A4
#define MATRIX_OUT_3 A3
#define MATRIX_OUT_C 3

#define MATRIX_IN_START 2
#define MATRIX_IN_STOP 13

#define SWELL_PIN A0
#define SWELL_CONTROL 20

#define MIDI_CHANNEL 4
#define TRANSPOSE 36

byte matrix_pins_out[] = {MATRIX_OUT_1, MATRIX_OUT_2, MATRIX_OUT_3};

byte keys[MATRIX_OUT_C * 12];

byte swell_value = 0;

void setup() {
  pinMode(MATRIX_OUT_1, OUTPUT);
  pinMode(MATRIX_OUT_2, OUTPUT);
  pinMode(MATRIX_OUT_3, OUTPUT);

  for(int i = MATRIX_IN_START; i <= MATRIX_IN_STOP; ++i) {
    pinMode(i, INPUT);
  }

  pinMode(SWELL_PIN, INPUT);
}

void loop() {
  for(int out = 0; out < MATRIX_OUT_C; out++) {
    digitalWrite(matrix_pins_out[out], HIGH);
    
    for(int in = MATRIX_IN_START; in <= MATRIX_IN_STOP; ++in) {
      byte val = read_wasteful(in);
      byte index = out * 12 + (in - MATRIX_IN_START);
      
      if(val != keys[index]) {
        if(val) note_on(4, TRANSPOSE + index, 127);
        else note_off(4, TRANSPOSE + index, 0);
        keys[index] = val;
      }
    }

    digitalWrite(matrix_pins_out[out], LOW);
  }

  byte sv = map(analogRead(SWELL_PIN), 0, 1023, 0, 127);
  if(abs(sv - swell_value) > 2) {
    control_change(MIDI_CHANNEL, SWELL_CONTROL, 127 - sv);
    swell_value = sv;
  }

  MidiUSB.flush();
}

byte read_wasteful(byte pin) {
  byte val_c = 0;
  for(int x = 0; x < 20; ++x) if(digitalRead(pin)) val_c++;
  if(val_c > 17) return 1;
  return 0;
}

void note_on(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void note_off(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void control_change(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}
