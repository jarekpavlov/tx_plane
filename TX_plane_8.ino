// 8 Channel Transmitter (No Trim)
// Input pin A5
  #include <SPI.h>
  #include <nRF24L01.h>
  #include <RF24.h>
  #define trimbut_1 2                       // Trim button 1 / Pin D2
  #define trimbut_2 3                      // Trim button 2 / Pin D3
  #define trimbut_3 4                      // Trim button 3 / Pin D4
  #define trimbut_4 5                      // Trim button 4 / Pin D5
  #define autopilot_7 7                      // autopilot / Pin D7
  #define autopilot_led 8                 // autopilot_led / Pin D8
  bool buttonWasPressed = false;
  const byte pipe[][10] = {"channel","channel2"};         // NOTE: The same as in the receiver 000322
  RF24 radio(9, 10);                       // select CE,CSN pin
  //int pitchTrimMiddle = EEPROM.read(1) * 4;        // Reading trim values from Eprom
  //int rollTrimMiddle = EEPROM.read(3) * 4;        
  int pitchTrimMiddle = 512;
  int rollTrimMiddle = 512;
struct Signal {
  byte throttle;
  byte pitch;
  byte roll;
  bool autopilot;
};
Signal data;
struct ResponseSignal {
  byte voltage;
  bool ledOn;
};
ResponseSignal responseData;
  void ResetData() 
  {
    data.throttle = 0;
    data.pitch = 127;
    data.roll = 127;
    data.autopilot = false;
  }
  void setup()
  {
                                       //Configure the NRF24 module
    radio.begin();
    radio.openWritingPipe(pipe[0]);
    radio.openReadingPipe(1,pipe[1]);
    radio.setChannel(100);
    radio.setAutoAck(false);
    radio.setDataRate(RF24_250KBPS);    // The lowest data rate value for more stable communication
    radio.setPALevel(RF24_PA_MAX);      // Output power is set for maximum

    radio.stopListening();              // Start the radio comunication for Transmitter
    ResetData();
    pinMode(trimbut_1, INPUT_PULLUP); 
    pinMode(trimbut_2, INPUT_PULLUP);
    pinMode(trimbut_3, INPUT_PULLUP); 
    pinMode(trimbut_4, INPUT_PULLUP);
    pinMode(autopilot_7, INPUT_PULLUP);
    pinMode(autopilot_led, OUTPUT);
    //pitchTrimMiddle = EEPROM.read(1) * 4;
    //rollTrimMiddle = EEPROM.read(3) * 4;

  }
                                      // Joystick center and its borders
  int Border_Map(int val, int lower, int middle, int upper, bool reverse)
  {
    val = constrain(val, lower, upper);
    if ( val < middle )
    val = map(val, lower, middle, 0, 128);
    else
    val = map(val, middle, upper, 128, 255);
    return ( reverse ? 255 - val : val );
  }
  void loop()
  {
    // setting button to turn on autopilot to switch mode on taking pressure off
    if (digitalRead(autopilot_7)==LOW) {
      buttonWasPressed = true;
    }
    if (digitalRead(autopilot_7)==HIGH && buttonWasPressed) {
      data.autopilot = !data.autopilot;
      buttonWasPressed = false;
    }
    //
    if (data.autopilot) {
     // digitalWrite(autopilot_led, HIGH);
    } else {
    //  digitalWrite(autopilot_led, LOW);
    }

  if(digitalRead(trimbut_1)==LOW && rollTrimMiddle < 630) {
    rollTrimMiddle=rollTrimMiddle+15;
    //EEPROM.write(1,rollTrimMiddle/4); 
    delay(130);
  }   
  if(digitalRead(trimbut_2)==LOW && rollTrimMiddle > 280){
    rollTrimMiddle=rollTrimMiddle-15;
    //EEPROM.write(1,rollTrimMiddle/4);
    delay(130);
  }
 
  if(digitalRead(trimbut_3)==LOW && pitchTrimMiddle < 630) {
    pitchTrimMiddle = pitchTrimMiddle + 15;
    //EEPROM.write(3,pitchTrimMiddle/4);
    delay(130);
  }   
  if(digitalRead(trimbut_4)==LOW && pitchTrimMiddle > 280){
    pitchTrimMiddle = pitchTrimMiddle - 15;
    //EEPROM.write(3,pitchTrimMiddle/4);
    delay(130);
  }  

                                     // Control Stick Calibration for channels
  data.roll = Border_Map(1023 - analogRead(A1), 0, rollTrimMiddle, 1023, true);        // "true" or "false" for signal direction | "true" veya "false" sinyal yönünü belirler

  data.pitch = Border_Map(analogRead(A0), 0, pitchTrimMiddle, 1023, true);      
  data.throttle = Border_Map(analogRead(A6),0, 800, 1023, false);  // For Single side ESC
  // data.throttle = Border_Map( analogRead(A1),0, 512, 1023, false ); // For Bidirectional ESC
  radio.write(&data, sizeof(Signal));
  radio.startListening();
  delay(2);
    while (radio.available()) {
    radio.read(&responseData, sizeof(ResponseSignal));                                   // Receive the data
  }
  radio.stopListening();
  if (responseData.ledOn) {
    digitalWrite(autopilot_led, HIGH);
  } else {
     digitalWrite(autopilot_led, LOW);
  }
}