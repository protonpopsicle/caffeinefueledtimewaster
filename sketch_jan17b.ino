// constants (used by the compiler)
#define numAnalog 25
#define numDigital 16
#define usbBaudRate 38400
#define midiBaudRate 31250

#define DEBUG false

#define OP_CHANGE_PARAM 176 // MIDI op code for cc
#define MAX_ANALOG_VALUE 1023

// Six-Trak parameters
typedef struct param {
  byte number;
  int maxDecimalValue;
} param;

param NULL_PARAM            = { NULL, NULL };
param MOD_WHEEL	            = { 1, 124 };
param OSC_COARSE_FREQUENCY  = { 2, 96 };
param OSC_FINE_FREQUENCY    = { 3, 124 };
param OSC_GLIDE_RATE        = { 4, 120 };
param OSC_LFO               = { 5, 64 };
param OSC_ENVELOPE_AMOUNT   = { 6, 120 };
param OSC_ENV_INVERT 	    = { 7, 64 };
param OSC_ENV_ATTACK        = { 8, 120 };
param OSC_ENV_DECAY         = { 9, 120 };
param OSC_ENV_SUSTAIN       = { 10, 120 };
param OSC_ENV_RELEASE       = { 11, 120 };
param OSC_SAWTOOTH_WAVE     = { 12, 64 };
param OSC_TRIANGLE_WAVE     = { 13, 64 };
param OSC_PULSE_WAVE        = { 14, 64 };
param OSC_PULSE_WIDTH       = { 15, 126 };
param OSC_PULSE_LFO_MOD     = { 16, 64 };
param LFO_FREQUENCY         = { 17, 120 };
param LFO_PROG_AMOUNT       = { 18, 124 };
param LFO_TRI_SQUARE_WAVE   = { 19, 64 };
param OSC_NOISE_MIXER       = { 20, 124 };
param FILT_CUTOFF_FREQUENCY = { 21, 127 };
param FILT_RESONANCE 	    = { 22, 126 };
param FILT_ENVELOPE_AMOUNT  = { 23, 120 };
param FILT_ENV_INVERT       = { 24, 64 };
param FILT_ENV_ATTACK       = { 25, 120 };
param FILT_ENV_DECAY        = { 26, 120 };
param FILT_ENV_SUSTAIN      = { 27, 120 };
param FILT_ENV_RELEASE      = { 28, 120 };
param FILT_LFO_MOD          = { 29, 64 };
param FILT_KEYABORD_AMOUNT  = { 30, 64 };
param FILT_OSC_TRI_MOD_AMT  = { 31, 126 };
param AMP_ATTACK            = { 32, 120 };
param AMP_DECAY             = { 33, 120 };
param AMP_SUSTAIN           = { 34, 120 };
param AMP_RELEASE           = { 35, 120 };
param VOICE_VOLUME          = { 36, 120 };
param UNISON                = { 37, 64 };

// noise thresholds
const byte activateThreshold = 3; // noise threshold required to "activate" a knob
const byte changeThreshold = 2; // noise threshold for changing an active knob's value
const byte persistence = 20; // standard persistence of an active knob

// arrays for the knobs
int analogValues[numAnalog]; // holds current analog values
int oldAnalogValues[numAnalog]; // holds most recent analog values sent over MIDI
byte analogActiveFlag[numAnalog]; // number of rounds each knob will remain active

// arrays for the switches
boolean digitalValues[numDigital];

// arrays for associating params to kobs/switches
param analogParams[numAnalog];
param digitalParams[numDigital];

byte oldDigitalValues[numDigital] = {2,2,2,1,2,1,2,1,2,2,2,1,2,1,2,1};
// note about initialization:
// 1) initializing to 2 will trigger sending the switch state at startup
// 2) unconnected inputs on the MUX float to 1, therefore these are initialized to 1 to prevent
// them from being sent

int diff = 0;
boolean changed;
boolean activated;

void setup() {
  // start serial object
  if (DEBUG) {
    Serial.begin(usbBaudRate);
  } else {
    Serial.begin(midiBaudRate);
  }
  
  // enable output pins (MUX 3-bit address)
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  
  // set up input pins (digital inputs from 2 switch MUX's)
  pinMode(5,INPUT);
  pinMode(6,INPUT);
  digitalWrite(5,HIGH); //enable 20k pullup resistor
  digitalWrite(6,HIGH); //enable 20k pullup resistor
  
  // initialize variables
  for(byte idx=0; idx < numAnalog; idx++){
     analogActiveFlag[idx] = 0;
     oldAnalogValues[idx] = -10000; // large negative value so that first read will activate all knobs
  }

  // assign params --> knobs
  analogParams[0] = OSC_NOISE_MIXER;
  analogParams[1] = OSC_COARSE_FREQUENCY;
  analogParams[2] = OSC_ENVELOPE_AMOUNT;
  analogParams[3] = OSC_GLIDE_RATE;
  analogParams[4] = OSC_PULSE_WIDTH;
  analogParams[5] = LFO_FREQUENCY;
  analogParams[6] = OSC_ENV_ATTACK;
  analogParams[7] = LFO_PROG_AMOUNT;
  analogParams[8] = FILT_OSC_TRI_MOD_AMT;
  analogParams[9] = OSC_ENV_DECAY;
  analogParams[10] = OSC_ENV_RELEASE;
  analogParams[11] = OSC_ENV_SUSTAIN;
  analogParams[12] = FILT_KEYABORD_AMOUNT;
  analogParams[13] = FILT_CUTOFF_FREQUENCY;
  analogParams[14] = FILT_ENVELOPE_AMOUNT;
  analogParams[15] = FILT_RESONANCE;
  analogParams[16] = AMP_SUSTAIN;
  analogParams[17] = FILT_ENV_ATTACK;
  analogParams[18] = AMP_ATTACK;
  analogParams[19] = FILT_ENV_DECAY;
  analogParams[20] = AMP_DECAY;
  analogParams[21] = FILT_ENV_RELEASE;
  analogParams[22] = AMP_RELEASE;
  analogParams[23] = FILT_ENV_SUSTAIN;
  analogParams[24] = NULL_PARAM;
  analogParams[25] = VOICE_VOLUME;

  // assign params --> switches
  digitalParams[0] = FILT_LFO_MOD;
  digitalParams[1] = LFO_TRI_SQUARE_WAVE;
  digitalParams[2] = OSC_ENV_INVERT;
  digitalParams[3] = NULL_PARAM;
  digitalParams[4] = FILT_ENV_INVERT;
  digitalParams[5] = NULL_PARAM;
  digitalParams[6] = UNISON;
  digitalParams[7] = NULL_PARAM;
  digitalParams[8] = OSC_PULSE_WAVE;
  digitalParams[9] = OSC_LFO;
  digitalParams[10] = OSC_SAWTOOTH_WAVE;
  digitalParams[11] = NULL_PARAM;
  digitalParams[12] = OSC_TRIANGLE_WAVE;
  digitalParams[13] = NULL_PARAM;
  digitalParams[14] = OSC_PULSE_LFO_MOD;
  digitalParams[15] = NULL_PARAM;
} // end setup()

void loop() {
  
  // READ ALL KNOBS AND SWITCHES
  // for loop takes about 2.7 ms
  for(byte addr=0; addr<8; addr++){
    
    // write 3 address bits to pins 2, 3 and 4
    PORTD = PORTD & B11100011;
    PORTD = PORTD | (addr << 2);
    
    // read analog values from three multiplexers
    analogValues[addr] = analogRead(A1);
    analogValues[addr+8] = analogRead(A2);
    analogValues[addr+16] = analogRead(A3);
    
    // read digital values from two multiplexers
    digitalValues[addr] = digitalRead(5);
    digitalValues[addr+8] = digitalRead(6);
    
  } // end for
  
  // read last analog pot on channel A0 (direct, no MUX)
  analogValues[24] = analogRead(A0);
  
  // PROCESS DATA AND SEND CHANGES OVER SERIAL
  
  // print digital values to serial if changed
  for (byte idx = 0; idx<16; idx++) {
    if (digitalValues[idx] != oldDigitalValues[idx]) { // if different
      // send digital value over serial
      oldDigitalValues[idx] = sendDigitalValue(idx, digitalValues[idx]);
    }
  }
  
  // print analog values to serial if changed (subject to noise filtering)
  for (byte idx = 0; idx<numAnalog; idx++) {
    
    // compute the magnitude of the change in the analog value
    diff = abs(analogValues[idx] - oldAnalogValues[idx]);
    
    // is the difference above either noise threshold?
    changed = diff >= changeThreshold; // difference constitues a change: active knobs will send
    activated = diff >= activateThreshold; // change is significant enough to activate the knob
    
    // a value will be sent if the knob value changes and the knob is active or about to be activated
    if(changed && (analogActiveFlag[idx] || activated)){
      
        // send analog value over serial
        oldAnalogValues[idx] = sendAnalogValue(idx, analogValues[idx]);
        
        // activate for X cycles
        analogActiveFlag[idx] = persistence; 
    }
    else if(analogActiveFlag[idx]) { // if didn't send and knob is active, decrement
        analogActiveFlag[idx]--;
    }
  } // end for
} // end loop()


void sendChangeParam(int paramNumber, int val) {
  sendMidiMessage(OP_CHANGE_PARAM, paramNumber, val);
}

void sendMidiMessage(int opNumber, int valOne, int valTwo) {
  if (DEBUG) {
    Serial.print(opNumber);
    Serial.print(", ");
    Serial.print(valOne);
    Serial.print(", ");
    Serial.print(valTwo);
    Serial.println();
  } else {
    Serial.write(opNumber);
    Serial.write(valOne);
    Serial.write(valTwo);
  }
}

int sendAnalogValue(byte idx, int val) {
//printAnalogValue(5,20); sends "A5: 20" and returns 20
  int scaledVal = ((double)val * (double)analogParams[idx].maxDecimalValue) / MAX_ANALOG_VALUE;

  if (DEBUG) {
    Serial.print("A");
    Serial.print(idx);
    Serial.print(": ");
    Serial.print(val);
    Serial.println();
  }
  
  sendChangeParam(analogParams[idx].number, scaledVal);      
  return val;
}

boolean sendDigitalValue(byte idx, boolean val) {
//printDigitalValue(5,TRUE); sends "D5: 1" and returns TRUE
  int scaledVal = 0;
  if (val) {
    scaledVal = digitalParams[idx].maxDecimalValue;
  }

  if (DEBUG) {
    Serial.print("D");
    Serial.print(idx);
    Serial.print(": ");
    Serial.print(val);
    Serial.println();
  }
  
  sendChangeParam(digitalParams[idx].number, scaledVal);
  return val;
}
