#include <Arduino.h>
#include <ETH.h>
#include <ModbusIP_ESP8266.h>

// --- Configuration Spécifique de la puce WT32-ETH01 ---
#define ETH_PHY_ADDR  1
#define ETH_PHY_POWER 16
#define ETH_PHY_MDC   23
#define ETH_PHY_MDIO  18
#define ETH_PHY_TYPE  ETH_PHY_LAN8720
#define ETH_CLK_MODE  ETH_CLOCK_GPIO0_IN

// --- Paramètres Réseau Fixes ---
IPAddress local_ip(192, 168, 10, 51); 
IPAddress gateway(192, 168, 10, 1);
IPAddress subnet(255, 255, 255, 0);

// --- Modbus & Variables ---
ModbusIP mb;
float temperature = 20.0;
int cycleState = 0; 
int timerSterilisation = 0;
unsigned long lastUpdate = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- DEMARRAGE FORCE WT32-ETH01 ---");

  // 1. Allumage forcé de la puce réseau avec les bonnes broches
  ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_TYPE, ETH_CLK_MODE);
  ETH.config(local_ip, gateway, subnet);
  
  Serial.print("Puce reseau active ! IP : ");
  Serial.println(ETH.localIP());

  // 2. Démarrage Modbus
  mb.server();
  mb.addHreg(0x00, 200); 
  mb.addHreg(0x01, 0);  
}

void loop() {
  mb.task();

  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();

    switch (cycleState) {
      case 0: temperature = 20.0; cycleState = 1; break;
      case 1: temperature += 2.5; if (temperature >= 121.0) { temperature = 121.0; cycleState = 2; timerSterilisation = 0; } break;
      case 2: temperature = 121.0 + (random(-2, 3) / 10.0); timerSterilisation++; if (timerSterilisation > 15) cycleState = 3; break;
      case 3: temperature -= 3.0; if (temperature <= 20.0) cycleState = 0; break;
    }

    mb.Hreg(0x00, (uint16_t)(temperature * 10));
    mb.Hreg(0x01, cycleState);
  }
}