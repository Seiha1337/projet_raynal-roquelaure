// ╔══════════════════════════════════════════════════════════╗
// ║       AUTOMATE FINDER OPTA - ESCLAVE MODBUS TCP          ║
// ╚══════════════════════════════════════════════════════════╝

#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoModbus.h>

// --- Configuration Réseau de l'OPTA ---
byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x76, 0x05 }; // MAC arbitraire
IPAddress ip(192, 168, 50, 210);

// --- Objets Serveur Ethernet et Modbus ---
EthernetServer ethServer(502);
ModbusTCPServer modbusTCPServer; // <-- LA LIGNE MANQUANTE ETAIT ICI !

// Variables pour simuler une évolution des données
unsigned long lastUpdate = 0;
int fausseTemperature = 200; // 20.0 °C

void setup() {
  Serial.begin(115200);
  delay(2000); // Laisse le temps d'ouvrir le moniteur série
  
  Serial.println("\n=== DEMARRAGE OPTA (ESCLAVE/SERVEUR) ===");

  // 1. Initialisation du réseau
  Ethernet.begin(mac, ip);
  Serial.print("Reseau OK ! IP OPTA : ");
  Serial.println(Ethernet.localIP());

  // 2. Démarrage du serveur Ethernet
  ethServer.begin();

  // 3. Démarrage du serveur Modbus (avec le 'm' minuscule !)
  if (!modbusTCPServer.begin()) {
    Serial.println("Erreur : Echec de l'initialisation Modbus !");
    while (1); // Bloque le programme en cas d'erreur
  }

  // 4. Création des "Tiroirs" mémoire (Holding Registers)
  // On crée 3 registres à partir de l'adresse 0x00
  modbusTCPServer.configureHoldingRegisters(0x00, 3);
  
  // Valeurs par défaut au démarrage
  modbusTCPServer.holdingRegisterWrite(0x00, fausseTemperature); // Température
  modbusTCPServer.holdingRegisterWrite(0x01, 1);                 // État fictif
  modbusTCPServer.holdingRegisterWrite(0x02, 1100);              // Consigne (110.0 °C)

  Serial.println("Serveur Modbus TCP pret sur le port 502.");
}

void loop() {
  // 1. On écoute si un Maître (comme qModMaster) essaie de se connecter
  EthernetClient client = ethServer.available();

  if (client) {
    Serial.println("\n>>> NOUVEAU CLIENT CONNECTE ! <<<");
    
    // On accepte la connexion Modbus
    modbusTCPServer.accept(client);

    // Tant que le client (PC) reste connecté
    while (client.connected()) {
      // On répond à ses requêtes (Scan)
      modbusTCPServer.poll();

      // 2. Mise à jour de nos données internes (toutes les secondes)
      if (millis() - lastUpdate > 1000) {
        lastUpdate = millis();
        
        // On simule une température qui monte doucement
        fausseTemperature += 5; // +0.5 °C
        if (fausseTemperature > 1100) fausseTemperature = 200;

        // On met à jour le registre Modbus pour que le client puisse le lire
        modbusTCPServer.holdingRegisterWrite(0x00, fausseTemperature);

        // On affiche dans le moniteur série de l'OPTA ce qui se passe
        Serial.print("[OPTA] Temp mise a jour : ");
        Serial.print(fausseTemperature / 10.0);
        Serial.println(" C");
      }
    }
    Serial.println(">>> CLIENT DECONNECTE <<<");
  }
}