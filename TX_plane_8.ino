// 8 Channel Transmitter (No Trim)
// Input pin A5
  #include <SPI.h>
  #include <nRF24L01.h>
  #include <RF24.h>
  #include <avr/wdt.h>
  #define trimbut_1 2                       // Trim button 1 / Pin D2
  #define trimbut_2 3                      // Trim button 2 / Pin D3
  #define trimbut_3 4                      // Trim button 3 / Pin D4
  #define trimbut_4 5                      // Trim button 4 / Pin D5
  const uint64_t pipeOut = 000322;         // NOTE: The same as in the receiver 000322
  RF24 radio(7, 8);                       // select CE,CSN pin
  //int pitchTrimMiddle = EEPROM.read(1) * 4;        // Reading trim values from Eprom
  //int rollTrimMiddle = EEPROM.read(3) * 4;        
  int pitchTrimMiddle = 512;
  int rollTrimMiddle = 512;
  struct Signal {
  byte throttle;
  byte pitch;
  byte roll;
};
  Signal data;
  void ResetData() 
{
  data.throttle = 0;
  data.pitch = 127;
  data.roll = 127;
}
  void setup()
{
                                       //Configure the NRF24 module
  radio.begin();
  radio.openWritingPipe(pipeOut);
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
  //pitchTrimMiddle = EEPROM.read(1) * 4;
  //rollTrimMiddle = EEPROM.read(3) * 4;
  wdt_enable(WDTO_120MS);

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
  if(digitalRead(trimbut_1)==LOW && rollTrimMiddle < 630) {
    rollTrimMiddle=rollTrimMiddle+15;
    //EEPROM.write(1,rollTrimMiddle/4); 
    delay (130);
  }   
  if(digitalRead(trimbut_2)==LOW && rollTrimMiddle > 280){
    rollTrimMiddle=rollTrimMiddle-15;
    //EEPROM.write(1,rollTrimMiddle/4);
    delay (130);
  }
 
  if(digitalRead(trimbut_4)==LOW && pitchTrimMiddle < 630) {
    pitchTrimMiddle = pitchTrimMiddle+  15;
    //EEPROM.write(3,pitchTrimMiddle/4);
    delay (130);
  }   
  if(digitalRead(trimbut_3)==LOW && pitchTrimMiddle > 280){
    pitchTrimMiddle = pitchTrimMiddle - 15;
    //EEPROM.write(3,pitchTrimMiddle/4);
    delay (130);
  }  

                                     // Control Stick Calibration for channels
  data.roll = Border_Map(1023 - analogRead(A1), 0, rollTrimMiddle, 1023, true);        // "true" or "false" for signal direction | "true" veya "false" sinyal yönünü belirler

  data.pitch = Border_Map(1023 - analogRead(A0), 0, pitchTrimMiddle, 1023, true);      
  data.throttle = Border_Map(analogRead(A6),0, 800, 1023, false);  // For Single side ESC
  // data.throttle = Border_Map( analogRead(A1),0, 512, 1023, false ); // For Bidirectional ESC
  radio.write(&data, sizeof(Signal));  
  wdt_reset();
}