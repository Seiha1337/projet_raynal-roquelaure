#include <Arduino.h>
#include <ArduinoModbus.h>
#include <ArduinoRS485.h>

// ============================================================
// Communication ASCON KM3 - Modbus RTU (8N1)
// Adresse : 4 | Vitesse : 9600 bauds
// ============================================================

constexpr int KM3_ADDRESS = 4;    // Adresse du régulateur mise à 4
constexpr auto baudrate { 9600 }; // Vitesse de communication

constexpr auto bitduration { 1.f / baudrate }; // Delais de pré et post transmission (en microsecondes) pour le format 8N1
constexpr auto preDelayBR { bitduration * 9.6f * 3.5f * 1e6 }; // 9.6 bits de silence avant et après la transmission, multiplié par 3.5 pour être sûr d'être au delà du minimum requis par le standard Modbus RTU
constexpr auto postDelayBR { bitduration * 9.6f * 3.5f * 1e6 }; // 9.6 bits de silence avant et après la transmission, multiplié par 3.5 pour être sûr d'être au delà du minimum requis par le standard Modbus RTU

bool essayerLecture(const char* label, int adresse, uint16_t reg) {
  // Le KM3 ne supporte QUE la fonction FC03 pour la lecture
  long val = ModbusRTUClient.holdingRegisterRead(adresse, reg);
  if (val != -1) {
    Serial.print("  >>> REPONSE FC03 ");
    Serial.print(label);
    Serial.print(" = ");
    Serial.println(val);
    return true;
  } else {
    Serial.print("  >>> ERREUR Lecture ");
    Serial.println(label);
  }
  return false;
}

void testConfig(const char* nom, uint16_t format) {
  Serial.print("=== Configuration : ");
  Serial.print(nom);
  Serial.println(" ===");

  ModbusRTUClient.end();
  delay(100);
  RS485.setDelays(preDelayBR, postDelayBR);

  if (!ModbusRTUClient.begin(baudrate, format)) {
    Serial.println("  Erreur demarrage Modbus RTU");
    return;
  }
  delay(100);

  // Test registre 1 : Valeur lue (PV - Process Value)
  Serial.println("  Lecture Registre 1 (PV)...");
  essayerLecture("PV", KM3_ADDRESS, 0x0001);

  // Test registre 3 : Consigne opérative en cours (SP actif)
  Serial.println("  Lecture Registre 3 (SP actif)...");
  essayerLecture("SP Actif", KM3_ADDRESS, 0x0003);

  // Test registre 6 : Consigne SP1 (Lecture/Écriture)
  Serial.println("  Lecture Registre 6 (SP 1)...");
  essayerLecture("SP1", KM3_ADDRESS, 0x0006);

  Serial.println();
}

void setup() {
  pinMode(LED_D0, OUTPUT);
  pinMode(LED_D1, OUTPUT);
  digitalWrite(LED_D0, HIGH);

  Serial.begin(9600);
  delay(2000); // Attente pour l'ouverture du moniteur série

  Serial.println("==========================================");
  Serial.println(" Test OPTA <-> ASCON KM3 (Adresse 4)");
  Serial.println(" Si aucune réponse : INVERSER A et B !");
  Serial.println("==========================================");
  Serial.println();

  // On teste uniquement en 8N1 car c'est le seul format supporté par le KM3
  testConfig("9600 bauds - 8N1", SERIAL_8N1);

  Serial.println("==========================================");
  Serial.println("TESTS TERMINES");
  Serial.println("==========================================");
}

unsigned long previousMillis = 0;
const unsigned long INTERVALLE_AFFICHAGE = 2000; // 2 secondes

void loop() {
  // Fait clignoter la LED pour montrer que le programme tourne
  digitalWrite(LED_D1, !digitalRead(LED_D1));

  // Modification de la consigne SP1 par saisie sur le moniteur série
  if (Serial.available() > 0) {
    String saisie = Serial.readStringUntil('\n');
    saisie.trim();
    int nouvelleConsigne = saisie.toInt();

    // Vérification que la saisie est bien un nombre valide
    if (saisie.length() > 0 && (nouvelleConsigne != 0 || saisie == "0")) {
      Serial.print("Ecriture nouvelle consigne SP1 = ");
      Serial.println(nouvelleConsigne);

      // Écriture dans le registre 6 (SP1) via FC06
      if (ModbusRTUClient.holdingRegisterWrite(KM3_ADDRESS, 0x0006, nouvelleConsigne)) {
        Serial.println("  >>> Consigne SP1 modifiee avec succes !");
        // Relecture pour confirmation
        essayerLecture("SP1", KM3_ADDRESS, 0x0006);
      } else {
        Serial.println("  >>> ERREUR ecriture consigne SP1");
      }
    } else {
      Serial.println("Saisie invalide. Entrez une valeur entiere.");
    }
  }

  // Affichage périodique de la température (PV) et de la consigne (SP1) toutes les 2 secondes
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= INTERVALLE_AFFICHAGE) {
    previousMillis = currentMillis;

    long pv = ModbusRTUClient.holdingRegisterRead(KM3_ADDRESS, 0x0001);
    long sp1 = ModbusRTUClient.holdingRegisterRead(KM3_ADDRESS, 0x0006);

    Serial.print("Temperature (PV) = ");
    if (pv != -1) { Serial.print(pv); Serial.print(" ("); Serial.print(pv / 10.0, 1); Serial.print(" C)"); } else Serial.print("ERREUR");
    Serial.print("  |  Consigne (SP1) = ");
    if (sp1 != -1) { Serial.print(sp1); Serial.print(" ("); Serial.print(sp1 / 10.0, 1); Serial.println(" C)"); } else Serial.println("ERREUR");
  }

  delay(100);
}