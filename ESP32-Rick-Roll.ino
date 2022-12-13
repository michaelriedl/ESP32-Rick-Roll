#include <WiFi.h>
#include "esp_wifi.h"

esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);

// Create the default config
wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

// Set the delay for transmitting packets
uint32_t currentTime = 0;
uint32_t attackTime = 0;
uint32_t attackDelay = 10;

// Set the SSIDs to broadcast
int currentID = 0;
const int numIDs = 8;
const char* ssids[numIDs] = {
  "01 Never gonna give you up",
  "02 Never gonna let you down",
  "03 Never gonna run around",
  "04 and desert you",
  "05 Never gonna make you cry",
  "06 Never gonna say goodbye",
  "07 Never gonna tell a lie",
  "08 and hurt you"
};

// Create a bare packet
uint8_t packet[128] = { 0x80, 0x00, 0x00, 0x00, //Frame Control, Duration
                /*4*/   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, //Destination address 
                /*10*/  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //Source address - overwritten later
                /*16*/  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //BSSID - overwritten to the same as the source address
                /*22*/  0xc0, 0x6c, //Seq-ctl
                /*24*/  0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, //timestamp - the number of microseconds the AP has been active
                /*32*/  0x64, 0x00, //Beacon interval
                /*34*/  0x01, 0x04, //Capability info
                /* SSID */
                /*36*/  0x00
};

// Function to broadcast the SSID
void broadcastSetSSID(const char* ESSID) {

  // Set a random channel
  int set_channel = random(1,12); 
  esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
  delay(1);

  // Randomize SRC MAC
  packet[10] = packet[16] = random(256);
  packet[11] = packet[17] = random(256);
  packet[12] = packet[18] = random(256);
  packet[13] = packet[19] = random(256);
  packet[14] = packet[20] = random(256);
  packet[15] = packet[21] = random(256);

  // Get the length of the SSID
  int ssidLen = strlen(ESSID);
  int fullLen = ssidLen;
  packet[37] = fullLen;

  // Insert my tag
  for(int i = 0; i < ssidLen; i++) {
    packet[38 + i] = ESSID[i];
  }

  /////////////////////////////

  packet[50 + fullLen] = set_channel;

  uint8_t postSSID[13] = {0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, //supported rate
                      0x03, 0x01, 0x04 /*DSSS (Current Channel)*/ };

  // Add everything that goes after the SSID
  for(int i = 0; i < 12; i++) {
    packet[38 + fullLen + i] = postSSID[i];
  }

  // Transmit the packet
  esp_wifi_80211_tx(WIFI_IF_STA, packet, sizeof(packet), false);

}

void setup() {

  // Configure the ESP32
  esp_wifi_start();
  WiFi.mode(WIFI_MODE_STA);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_max_tx_power(82);

}

void loop() {

  // Get the current time
  currentTime = millis();

  // Send out SSIDs if the delay has been long enough
  if (currentTime - attackTime > attackDelay) {

    // Send the SSID
    broadcastSetSSID(ssids[currentID]);

    // Iterate over the SSIDs
    if (currentID >= numIDs - 1) {
      currentID = 0;
    } else {
      currentID++;
    }

    // Update the time
    attackTime = currentTime;

  }

}
