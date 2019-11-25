#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include "SSD1306.h" 
#include <axp20x.h>
#include "EspMQTTClient.h"

// Configuração Power chip
AXP20X_Class axp;

// Configuração chip LoRa
#define SCK     5     // GPIO5  -- SX1278's SCK
#define MISO    19    // GPIO19 -- SX1278's MISO
#define MOSI    27    // GPIO27 -- SX1278's MOSI
#define SS      18    // GPIO18 -- SX1278's CS
#define RST     14    // GPIO14 -- SX1278's RESET
#define DI0     26    // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND    915E6 // BANDA BRASIL

// Configuração MQTT
EspMQTTClient client(
  "SSID", // SSID Wifi
  "PASS", // Senha Wifi
  "IP",   // MQTT Broker IP
  "USER", // Usuário MQTT
  "PASS", // Senha MQTT
  "NAME", // Nome do dispositivo
  1883    // Porta MQTT
);

// Configuração Display
SSD1306 display(0x3c, 21, 22);
String rssi = "RSSI --";
String packSize = "--";
String packet ;

// Procedimento responsável pela escrita do Display e envio ao Broker MQTT
void loraData(){
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0,  "Valor rec. "+ String(packet));
  display.drawString(0, 10, "Recebido "+ packSize + " bytes");
  display.drawString(0, 20, String(rssi));
  display.drawString(0, 44, "Bateria "+ String(axp.getBattVoltage()/1000) + " V");
  display.drawString(0, 54, "ESP "+ String(axp.getSysIPSOUTVoltage()/1000) + " V");
  client.publish("latinoware/temperatura", String(packet));
  display.display();
  Serial.println(rssi);
}

// Procedimento responsável pelo recebimento do pacote LoRa
void cbk(int packetSize) {
  packet ="";
  packSize = String(packetSize,DEC);
  for (int i = 0; i < packetSize; i++) { packet += (char) LoRa.read(); }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
  loraData();
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Habilita o debug do MQTT via porta Serial
  client.enableDebuggingMessages(); 
  
  // Configuração Power chip
  Wire.begin(21, 22);
  if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS)) {
    Serial.println("AXP192 Begin PASS");
  } else {
    Serial.println("AXP192 Begin FAIL");
  }
  axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);
  axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
  axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);
  
  // Config chip LoRa;
  Serial.println("LoRa Receiver Callback");
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);  
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.receive();
  Serial.println("init ok");

  // Configuração Display
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);
  delay(50); 
  digitalWrite(16, HIGH);
  display.init();
  display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_10);

  delay(1500);
}

// Esse procedimento é executado toda vez que a conexão com o Broker MQTT é estabelecida
void onConnectionEstablished()
{
  // Se inscreve no topico (Interessande para fins de Debug)
  client.subscribe("latinoware/temperatura", [](const String & payload) {
    Serial.println(payload);
  });
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) { cbk(packetSize);  }
  delay(10);
  client.loop();
}
