#define NUM_NODES 5 // Arduino uno cant handle many due to low memory
#define BUF_SIZE 80

#define RX_PIN 7
#define TX_PIN 3
// Include RadioHead Amplitude Shift Keying Library
#include <RH_ASK.h>


// Create Amplitude Shift Keying Object
RH_ASK rf_driver(1000,RX_PIN,TX_PIN);

void setup(){
  // set power to radio module
    pinMode (6, OUTPUT);
    digitalWrite (6, LOW); 
    pinMode (8, OUTPUT);
    digitalWrite (8, HIGH);
    
    Serial.begin(115200);
    setupNodes();
    setupRadio();
    setupEthernet();
    // Setup Serial Monitor
    //debugCreate();
    
}
 
void loop(){
    // Set buffer to size of expected message
    uint8_t buf[BUF_SIZE];
    uint8_t buflen = sizeof(buf);
    // Check if received packet is correct size
    if (rf_driver.recv(buf, &buflen)) {
      // Message received with valid checksum
      Serial.print("Message Received: ");
      Serial.println((char*)buf);   
      
      handleBuffer(buf);
      
    }
    webServerLoop();
}
