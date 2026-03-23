// ╔══════════════════════════════════════════════════════════╗
// ║                    MODBUS RTU ESCLAVE                    ║
// ║                    (SLAVE MODE)                          ║
// ╚══════════════════════════════════════════════════════════╝

#include <Arduino.h>
#include <ModbusRTU.h>
#include <TFT_eSPI.h>

// Configuration RS485
#define RXD2 17         // GPIO17 - RX du RS485 (INVERSÉ POUR TEST)
#define TXD2 5          // GPIO5 - TX du RS485 (INVERSÉ POUR TEST)
// Pas de broche DE/RE : le module a une gestion automatique de direction

// Configuration Modbus
#define SLAVE_ID 1      // Adresse de l'esclave Modbus
#define REGN 10         // Nombre de registres

// Objet Modbus
ModbusRTU mb;

// Objet écran TFT
TFT_eSPI tft = TFT_eSPI();

// Tableau de registres Holding (lecture/écriture)
uint16_t holdingRegs[REGN];

void setup() {
  // Initialisation du port série pour le debug
  Serial.begin(115200);
  Serial.println("\n=== Modbus RTU Slave ===");
  Serial.println("Carte: WT32-ETH01");
  Serial.println("Mode: ESCLAVE (Slave)");
  
  // Initialisation de l'écran TFT
  tft.init();
  tft.setRotation(0); // Orientation portrait
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // Affichage écran de démarrage
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Modbus RTU");
  tft.setTextSize(1);
  tft.setCursor(10, 40);
  tft.println("Mode: ESCLAVE");
  tft.setCursor(10, 60);
  tft.printf("Adresse: %d", SLAVE_ID);
  tft.setCursor(10, 80);
  tft.println("Vitesse: 9600 bauds");
  tft.setCursor(10, 100);
  tft.println("Format: 8N1");
  tft.setCursor(10, 130);
  tft.println("En attente du maitre...");
  
  // Configuration du Serial2 pour RS485
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  
  // Configuration du Modbus RTU sur Serial2 (pas de contrôle DE/RE)
  mb.begin(&Serial2);
  mb.slave(SLAVE_ID);
  
  // Configuration des registres Holding (adresse 0 à 9)
  for (int i = 0; i < REGN; i++) {
    holdingRegs[i] = i * 100;  // Valeurs initiales: 0, 100, 200, ...
    mb.addHreg(i, holdingRegs[i]);
  }
  
  Serial.println("Esclave Modbus démarré");
  Serial.print("Adresse esclave: ");
  Serial.println(SLAVE_ID);
  Serial.print("Vitesse: 9600 bauds, Format: 8N1");
  Serial.println("\nEn attente de requêtes du maître...\n");
}

void loop() {
  // Traitement des requêtes Modbus
  mb.task();
  
  // Mise à jour des valeurs des registres (simulation de capteurs)
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 2000) {  // Mise à jour toutes les 2 secondes
    lastUpdate = millis();
    
    // Mise à jour du registre 0 avec une valeur incrémentale
    static uint16_t counter = 0;
    holdingRegs[0] = counter++;
    mb.Hreg(0, holdingRegs[0]);
    
    // Affichage sur le moniteur série
    Serial.println("--- État des registres ---");
    for (int i = 0; i < REGN; i++) {
      holdingRegs[i] = mb.Hreg(i);
      Serial.printf("Reg[%d] = %d\n", i, holdingRegs[i]);
    }
    Serial.println();
    
    // Affichage sur l'écran TFT
    tft.fillRect(0, 160, 240, 80, TFT_BLACK); // Effacer la zone des registres
    tft.setTextSize(1);
    tft.setCursor(10, 160);
    tft.println("Registres:");
    
    // Afficher les 5 premiers registres
    for (int i = 0; i < 5; i++) {
      tft.setCursor(10, 180 + i * 10);
      tft.printf("R%d: %5d", i, holdingRegs[i]);
      tft.setCursor(130, 180 + i * 10);
      if (i + 5 < REGN) {
        tft.printf("R%d: %5d", i + 5, holdingRegs[i + 5]);
      }
    }
  }
  
  delay(10);
}
