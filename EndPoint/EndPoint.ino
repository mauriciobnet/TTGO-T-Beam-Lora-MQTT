#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include <SSD1306.h>
#include <axp20x.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 4    // Digital pin connected to the DHT sensor
#define DHTTYPE    DHT22     // DHT 22 (AM2302) 
DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

// Configuração Power chip
AXP20X_Class axp;

// Config chip LoRa
#define SCK     5      // GPIO5  -- SX1278's SCK
#define MISO    19     // GPIO19 -- SX1278's MISnO
#define MOSI    27     // GPIO27 -- SX1278's MOSI
#define SS      18     // GPIO18 -- SX1278's CS
#define RST     14     // GPIO14 -- SX1278's RESET
#define DI0     26     // GPIO26 -- SX1278's IRQ
#define BAND    915E6  // BANDA BRASIL

// Configuração Display
SSD1306 display(0x3c, 21, 22);

// Contador de Pacotes LoRa
unsigned int counter = 0;

// Configuração Potenciômetro
const int analogIn = 36;
int sensor_value = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

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
   
  // Config chip LoRa
  Serial.println("LoRa Sender Test");
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
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

void loop() {
  // Leitura do Potenciômetro com map de 0 a 100(Regra de 3).
  sensor_value = map(analogRead(analogIn), 0, 2040, 0, 100);
  
  // Escreve o Display
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Env. pacote: ");
  display.drawString(70, 0, String(counter));
  display.drawString(0, 10, "Valor env. "+ String(sensor_value));
  display.drawString(0 , 44 , "Bateria "+ String(axp.getBattVoltage()/1000) + " V");
  display.drawString(0 , 54 , "ESP "+ String(axp.getSysIPSOUTVoltage()/1000) + " V");
  Serial.println(String(counter));
  display.display();

  // Envio do pacote LoRa a cada 5 segundos
  LoRa.beginPacket();
  LoRa.print(sensor_value);
  LoRa.endPacket();
  counter++;
  delay(5000);

}
