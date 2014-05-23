#include <FastSPI_LED2.h>
#include <SPI.h>
#include <UIPEthernet.h>

#define NUM_LEDS 128
#define ARR_WIDTH 16

#define DATA_PIN 3
#define CLOCK_PIN 2


#define TPM2NET_LISTENING_PORT 65506
#define TPM2NET_HEADER_SIZE 5
#define TPM2NET_HEADER_IDENT 0x9c
#define TPM2NET_CMD_DATAFRAME 0xda
#define TPM2NET_CMD_COMMAND 0xc0
#define TPM2NET_CMD_ANSWER 0xaa
#define TPM2NET_FOOTER_IDENT 0x36

CRGB leds[NUM_LEDS];
EthernetUDP Udp;

void setup() {
  Serial.begin(115200);
  uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};
  Ethernet.begin(mac,IPAddress(10,0,0,27));
  Udp.begin(TPM2NET_LISTENING_PORT);

  LEDS.addLeds<WS2801, DATA_PIN, CLOCK_PIN>(leds, NUM_LEDS);

  showInitImage();
  Serial.println(F("Start"));
}

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize > 0) {
    handlePacket();
    Udp.stop();
    Udp.begin(TPM2NET_LISTENING_PORT);
  }
}

void handlePacket() {
  if (Udp.available() < TPM2NET_HEADER_SIZE) {
    Serial.println(F("Packet too small"));
    return;
  }

  //check header byte
  uint8_t header = Udp.read();
  if (header!=TPM2NET_HEADER_IDENT) {
    Serial.print(F("Invalid header ident "));
    Serial.println(header, HEX);
    return;
  }
  
  //check command
  uint8_t cmd = Udp.read();
  if (cmd!=TPM2NET_CMD_DATAFRAME) {
    Serial.print(F("Invalid block type "));
    Serial.println(cmd, HEX);
    return;
  }

  uint16_t frameSize = Udp.read();
  frameSize = (frameSize << 8) + Udp.read();

  //use packetNumber to calculate offset
  uint8_t packetNumber = Udp.read();
  if (packetNumber != 0) {
    Serial.println(F("No multi-panel"));
    return;
  }

  // Some extra data (seems to be just 1)
  Udp.read();

  //check footer
  if (frameSize > Udp.available()) {
    Serial.print(F("Frame size was larger than data "));
    Serial.print(frameSize);
    Serial.print('>');
    Serial.println(Udp.available());
    return;
  }
  if (frameSize / 3 > NUM_LEDS) {
    Serial.print(F("Frame size was larger my number of LEDs "));
    Serial.print(frameSize / 3);
    Serial.print('>');
    Serial.println(NUM_LEDS);
    return;
  }

  //update LEDs
  for (uint16_t i=0; i < frameSize / 3; i++) {
    leds[i] = CRGB(Udp.read(), Udp.read(), Udp.read());
  }

  // Check footer
  if (Udp.available() > 0) {
    uint8_t footer = Udp.read();
    if (footer != TPM2NET_FOOTER_IDENT) {
      Serial.print(F("Invalid footer ident "));
      Serial.println(footer, HEX);
    }
  }

  LEDS.show();
}

void showInitImage() {
  for (int i = 0 ; i < NUM_LEDS; i++ ) {
    leds[i].setHue((i * 256 / NUM_LEDS) % 256);
  }
  LEDS.show();
}

