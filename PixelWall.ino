#include <FastSPI_LED2.h>
#include <SPI.h>
#include <UIPEthernet.h>

#define NUM_LEDS 128
#define ARR_WIDTH 16

#define DATA_PIN 3
#define CLOCK_PIN 2

CRGB leds[NUM_LEDS];
EthernetUDP Udp;

void setup() {
  Serial.begin(9600);
  uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};
  Ethernet.begin(mac,IPAddress(10,0,0,27));
  Udp.begin(7000);

  LEDS.addLeds<WS2801, DATA_PIN, CLOCK_PIN>(leds, NUM_LEDS);

  showInitImage();
}

void loop() {
  int packetSize = Udp.parsePacket();
  uint16_t currentPixel = 0;
  if (packetSize) {
    //Serial.println("DAT");
    bool synced = false;
    while (Udp.available()) {
      if (synced && Udp.available() >= 3) {
        //Serial.println("px");
        leds[currentPixel] = CRGB(Udp.read(), Udp.read(), Udp.read());
        currentPixel = (currentPixel + 1) % NUM_LEDS;
      } else if (!synced) {
        uint8_t check = Udp.read();
        if (check == 0xA0) {
          synced = true;
          currentPixel = 0;
        }
        //Serial.println("sync?");
      } else {
        // got extra data at the end, but dont know what to do with it...
        // Eat it up.
        Udp.read();
        //Serial.println("wat");
      }
    }
    LEDS.show();
    Udp.stop();
    Udp.begin(7000);
  }
}

void showInitImage() {
  for (int i = 0 ; i < NUM_LEDS; i++ ) {
    leds[i].setHue((i * 256 / NUM_LEDS) % 256);
  }
  LEDS.show();
}

