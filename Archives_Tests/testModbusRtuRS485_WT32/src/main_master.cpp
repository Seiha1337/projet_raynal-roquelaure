// ╔══════════════════════════════════════════════════════════╗
// ║                    MODBUS RTU MAITRE                     ║
// ║                    (MASTER MODE)                         ║
// ╚══════════════════════════════════════════════════════════╝

#include <Arduino.h>
#include <ModbusRTU.h>
#include <TFT_eSPI.h>

// Configuration RS485
#define RXD2 17         // GPIO17 - RX du RS485 (INVERSÉ POUR TEST)
#define TXD2 5          // GPIO5 - TX du RS485 (INVERSÉ POUR TEST)
// Pas de broche DE/RE : le module a une gestion automatique de direction

// Configuration Modbus
#define SLAVE_ID 1      // Adresse de l'esclave à interroger
#define FIRST_REG 0     // Première adresse de registre
#define NUM_REGS 10     // Nombre de registres à lire

// Objet Modbus
ModbusRTU mb;

// Objet écran TFT
TFT_eSPI tft = TFT_eSPI();

// Variables globales pour l'affichage
uint16_t lastReadRegs[NUM_REGS];
int successCount = 0;
int errorCount = 0;

// Variables pour le callback
bool cbRead(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event != Modbus::EX_SUCCESS) {
    Serial.print("Erreur de requête: 0x");
    Serial.print(event, HEX);
    
    // Afficher le nom de l'erreur
    switch(event) {
      case 0xE0: Serial.println(" (ILLEGAL_FUNCTION)"); break;
      case 0xE1: Serial.println(" (ILLEGAL_DATA_ADDRESS)"); break;
      case 0xE2: Serial.println(" (ILLEGAL_DATA_VALUE)"); break;
      case 0xE3: Serial.println(" (SLAVE_DEVICE_FAILURE)"); break;
      case 0xE4: Serial.println(" (TIMEOUT - Pas de reponse!)"); break;
      default: Serial.println();
    }
    
    errorCount++;
    
    // Afficher l'erreur sur l'écran
    tft.fillRect(0, 220, 240, 20, TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.setCursor(10, 225);
    if (event == 0xE4) {
      tft.printf("TIMEOUT (0x%02X)", event);
    } else {
      tft.printf("Erreur: 0x%02X", event);
    }
    
    return false;
  }
  
  if (event == Modbus::EX_SUCCESS) {
    successCount++;
    Serial.println("\n=== Lecture réussie ===");
    
    // Affichage des registres lus sur le moniteur série
    for (uint8_t i = 0; i < NUM_REGS; i++) {
      Serial.printf("Reg[%d] = %d\n", i, lastReadRegs[i]);
    }
    Serial.println();
    
    // Affichage sur l'écran TFT
    tft.fillRect(0, 130, 240, 80, TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(10, 130);
    tft.println("Registres lus:");
    
    // Afficher les registres
    for (int i = 0; i < 5; i++) {
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setCursor(10, 150 + i * 10);
      tft.printf("R%d: %5d", i, lastReadRegs[i]);
      tft.setCursor(130, 150 + i * 10);
      if (i + 5 < NUM_REGS) {
        tft.printf("R%d: %5d", i + 5, lastReadRegs[i + 5]);
      }
    }
    
    // Afficher les statistiques
    tft.fillRect(0, 220, 240, 20, TFT_BLACK);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setCursor(10, 225);
    tft.printf("OK:%d ERR:%d", successCount, errorCount);
  }
  return true;
}

bool cbWrite(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event != Modbus::EX_SUCCESS) {
    Serial.print("Erreur d'écriture: 0x");
    Serial.println(event, HEX);
    errorCount++;
  } else {
    Serial.println("Écriture réussie!");
    successCount++;
  }
  return true;
}

void setup() {
  // Initialisation du port série pour le debug
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Modbus RTU Master ===");
  Serial.println("Carte: WT32-ETH01");
  Serial.println("Mode: MAÎTRE (Master)");
  
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
  tft.println("Mode: MAITRE");
  tft.setCursor(10, 60);
  tft.printf("Esclave: %d", SLAVE_ID);
  tft.setCursor(10, 80);
  tft.println("Vitesse: 9600 bauds");
  tft.setCursor(10, 100);
  tft.println("Format: 8N1");
  
  // Configuration du Serial2 pour RS485
  Serial.println("\nConfiguration Serial2...");
  Serial.printf("RX=%d, TX=%d\n", RXD2, TXD2);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(100);
  
  // Configuration du Modbus RTU sur Serial2 (pas de contrôle DE/RE)
  Serial.println("Configuration Modbus...");
  mb.begin(&Serial2);
  mb.master();
  
  Serial.println("Maître Modbus démarré");
  Serial.print("Esclave cible: ");
  Serial.println(SLAVE_ID);
  Serial.println("Vitesse: 9600 bauds, Format: 8N1");
  Serial.println("\nATTENTION: Verifiez le cablage:");
  Serial.println("  WT32 TX (GPIO17) --> RXD du module RS485");
  Serial.println("  WT32 RX (GPIO5)  --> TXD du module RS485");
  Serial.println("  Module RS485: A+ <--> A+, B- <--> B-");
  Serial.println("\nDémarrage des tests...\n");
}

void loop() {
  // Traitement des requêtes Modbus
  mb.task();
  
  static unsigned long lastRead = 0;
  static unsigned long lastWrite = 0;
  static uint16_t writeValue = 1000;
  
  // Lecture des registres toutes les 3 secondes
  if (millis() - lastRead > 3000) {
    lastRead = millis();
    
    Serial.println("\n>>> Envoi requête de LECTURE <<<");
    Serial.printf("Lecture de %d registres à partir de l'adresse %d\n", NUM_REGS, FIRST_REG);
    
    if (!mb.readHreg(SLAVE_ID, FIRST_REG, lastReadRegs, NUM_REGS, cbRead)) {
      Serial.println("Erreur: Échec de l'envoi de la requête de lecture");
    }
  }
  
  // Écriture dans un registre toutes les 5 secondes
  if (millis() - lastWrite > 5000) {
    lastWrite = millis();
    
    Serial.println("\n>>> Envoi requête d'ÉCRITURE <<<");
    Serial.printf("Écriture de la valeur %d dans le registre %d\n", writeValue, 5);
    
    if (!mb.writeHreg(SLAVE_ID, 5, writeValue, cbWrite)) {
      Serial.println("Erreur: Échec de l'envoi de la requête d'écriture");
    }
    
    writeValue += 10;  // Incrément pour la prochaine écriture
  }
  
  delay(10);
}
