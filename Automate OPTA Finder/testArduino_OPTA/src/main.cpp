/*
!! L'automate génère 2 ports COM
VsCode prend par défaut le mauvais
imposez le bon avant de transférer le code
*/

#include <Arduino.h>
#include "WiFi.h"

// ========== CONFIGURATION WiFi ==========
const char ssid[] = "Wifi_Iot215"; 
const char password[] = "CIEL1234#+";


// Serveur web sur le port 80
WiFiServer server(80);

// Variables d'état
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// ========== PAGE HTML/CSS EMBARQUÉE ==========
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Arduino OPTA</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 800px;
            margin: 0 auto;
        }
        
        h1 {
            color: white;
            text-align: center;
            margin-bottom: 10px;
            font-size: 2.5em;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        
        .subtitle {
            text-align: center;
            color: rgba(255,255,255,0.9);
            margin-bottom: 30px;
            font-size: 1.1em;
        }
        
        .info-card {
            background: white;
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 20px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
        }
        
        .info-card p {
            color: #666;
            line-height: 1.8;
            margin-bottom: 8px;
        }
        
        .info-card strong {
            color: #333;
        }
        
        .relays-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
            gap: 15px;
            margin-bottom: 20px;
        }
        
        .relay-card {
            background: white;
            border-radius: 15px;
            padding: 20px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
            text-align: center;
        }
        
        .relay-card h3 {
            color: #333;
            margin-bottom: 15px;
            font-size: 1.3em;
        }
        
        .relay-buttons {
            display: flex;
            gap: 10px;
            margin-bottom: 15px;
        }
        
        .relay-state {
            margin-top: 10px;
            padding: 8px;
            border-radius: 5px;
            font-weight: bold;
            font-size: 1.1em;
        }
        
        .btn {
            flex: 1;
            padding: 12px;
            font-size: 1em;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.3s ease;
            font-weight: bold;
            text-transform: uppercase;
            text-decoration: none;
            display: inline-block;
            color: white;
        }
        
        .btn-on {
            background: #4CAF50;
        }
        
        .btn-on:hover {
            background: #45a049;
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(76, 175, 80, 0.4);
        }
        
        .btn-off {
            background: #f44336;
        }
        
        .btn-off:hover {
            background: #da190b;
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(244, 67, 54, 0.4);
        }
        
        .inputs-section {
            background: white;
            border-radius: 15px;
            padding: 20px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
        }
        
        .inputs-section h3 {
            color: #333;
            margin-bottom: 20px;
            text-align: center;
        }
        
        .inputs-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(80px, 1fr));
            gap: 10px;
        }
        
        .input-item {
            text-align: center;
            padding: 10px;
            border-radius: 8px;
            background: #f5f5f5;
        }
        
        .input-label {
            font-weight: bold;
            color: #333;
            margin-bottom: 5px;
        }
        
        .input-state {
            font-size: 1.5em;
            margin-top: 5px;
        }
        
        .state-high {
            color: #4CAF50;
        }
        
        .state-low {
            color: #999;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>⚡ Arduino OPTA</h1>
        <p class="subtitle">Supervision Automate OPTA</p>
        
        <div class="info-card">
            <p><strong>📡 Connexion:</strong> WiFi</p>
            <p><strong>🏷️ Hostname:</strong> HOSTNAME_PLACEHOLDER</p>
            <p><strong>🌐 SSID:</strong> SSID_PLACEHOLDER</p>
            <p><strong>📶 Signal:</strong> RSSI_PLACEHOLDER dBm</p>
            <p><strong>🔌 IP:</strong> IP_PLACEHOLDER</p>
            <p><strong>🆔 MAC:</strong> MAC_PLACEHOLDER</p>
            <p><strong>⚙️ Plateforme:</strong> Arduino OPTA WiFi RS485</p>
            <p><strong>🖥️ MCU:</strong> STM32H747XI Dual Core</p>
        </div>
        
        <div class="relays-grid">
            <div class="relay-card">
                <h3>🔌 Relais 1</h3>
                <div class="relay-buttons">
                    <a href="/RELAY1=ON" class="btn btn-on">ON</a>
                    <a href="/RELAY1=OFF" class="btn btn-off">OFF</a>
                </div>
                <div class="relay-state RELAY1_CLASS">État: RELAY1_STATE</div>
            </div>
            
            <div class="relay-card">
                <h3>🔌 Relais 2</h3>
                <div class="relay-buttons">
                    <a href="/RELAY2=ON" class="btn btn-on">ON</a>
                    <a href="/RELAY2=OFF" class="btn btn-off">OFF</a>
                </div>
                <div class="relay-state RELAY2_CLASS">État: RELAY2_STATE</div>
            </div>
            
            <div class="relay-card">
                <h3>🔌 Relais 3</h3>
                <div class="relay-buttons">
                    <a href="/RELAY3=ON" class="btn btn-on">ON</a>
                    <a href="/RELAY3=OFF" class="btn btn-off">OFF</a>
                </div>
                <div class="relay-state RELAY3_CLASS">État: RELAY3_STATE</div>
            </div>
            
            <div class="relay-card">
                <h3>🔌 Relais 4</h3>
                <div class="relay-buttons">
                    <a href="/RELAY4=ON" class="btn btn-on">ON</a>
                    <a href="/RELAY4=OFF" class="btn btn-off">OFF</a>
                </div>
                <div class="relay-state RELAY4_CLASS">État: RELAY4_STATE</div>
            </div>
        </div>
        
        <div class="inputs-section">
            <h3>📥 État des Entrées</h3>
            <div class="inputs-grid">
                <div class="input-item">
                    <div class="input-label">I1</div>
                    <div class="input-state INPUT1_CLASS">INPUT1_STATE</div>
                </div>
                <div class="input-item">
                    <div class="input-label">I2</div>
                    <div class="input-state INPUT2_CLASS">INPUT2_STATE</div>
                </div>
                <div class="input-item">
                    <div class="input-label">I3</div>
                    <div class="input-state INPUT3_CLASS">INPUT3_STATE</div>
                </div>
                <div class="input-item">
                    <div class="input-label">I4</div>
                    <div class="input-state INPUT4_CLASS">INPUT4_STATE</div>
                </div>
                <div class="input-item">
                    <div class="input-label">I5</div>
                    <div class="input-state INPUT5_CLASS">INPUT5_STATE</div>
                </div>
                <div class="input-item">
                    <div class="input-label">I6</div>
                    <div class="input-state INPUT6_CLASS">INPUT6_STATE</div>
                </div>
                <div class="input-item">
                    <div class="input-label">I7</div>
                    <div class="input-state INPUT7_CLASS">INPUT7_STATE</div>
                </div>
                <div class="input-item">
                    <div class="input-label">I8</div>
                    <div class="input-state INPUT8_CLASS">INPUT8_STATE</div>
                </div>
            </div>
        </div>
    </div>
</body>
</html>
)rawliteral";

void setup() {
  // Initialisation de la communication série
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n=== Arduino OPTA - Serveur Web ===");
  
  // Configuration LED
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_D0, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LED_D0, LOW);
  digitalWrite(LEDR, LOW);   // Éteint (test logique normale)
  digitalWrite(LEDG, LOW);   // Éteint (test logique normale)
  
  // Configuration bouton USER
  pinMode(BTN_USER, INPUT_PULLUP);
  
  // Configuration Relais
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);
  digitalWrite(RELAY3, LOW);
  digitalWrite(RELAY4, LOW);
  
  // Configuration Entrées I1-I8
  pinMode(I1, INPUT);
  pinMode(I2, INPUT);
  pinMode(I3, INPUT);
  pinMode(I4, INPUT);
  pinMode(I5, INPUT);
  pinMode(I6, INPUT);
  pinMode(I7, INPUT);
  pinMode(I8, INPUT);
  
  // Clignotement rouge/vert avant connexion WiFi (1 seconde)
  for(int i = 0; i < 2; i++) {
    digitalWrite(LEDR, LOW);   // Rouge ON
    delay(250);
    digitalWrite(LEDR, HIGH);  // Rouge OFF
    digitalWrite(LEDG, LOW);   // Vert ON
    delay(250);
    digitalWrite(LEDG, HIGH);  // Vert OFF
  }
  
  // Connexion au WiFi
  Serial.print("Connexion au WiFi: ");
  Serial.println(ssid);
  
  WiFi.setHostname("Arduino-OPTA");
  WiFi.begin(ssid, password);
  
  int timeout = 0;
  bool isRed = true;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    // Clignotement alternant rouge/vert pendant la tentative de connexion
    if (isRed) {
      digitalWrite(LEDR, HIGH);  // Rouge ON (logique normale)
      digitalWrite(LEDG, LOW);   // Vert OFF
    } else {
      digitalWrite(LEDR, LOW);   // Rouge OFF
      digitalWrite(LEDG, HIGH);  // Vert ON (logique normale)
    }
    isRed = !isRed;
    delay(250);
    Serial.print(".");
    timeout++;
  }
  
  // Éteindre les LEDs avant de signaler le résultat
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi connecté!");
    Serial.print("Adresse IP: ");
    Serial.println(WiFi.localIP());
    
    // Démarrage du serveur
    server.begin();
    Serial.println("✓ Serveur web démarré");
    Serial.println("=============================\n");
    
    // Indication visuelle de connexion réussie - LED verte fixe UNIQUEMENT
    digitalWrite(LEDR, LOW);   // Rouge OFF
    digitalWrite(LEDG, HIGH);  // Vert ON (logique normale)
    
  } else {
    Serial.println("\n✗ Échec de connexion WiFi");
    Serial.println("Vérifiez vos identifiants WiFi dans le code");
    
    // Indication visuelle d'échec - LED rouge fixe
    digitalWrite(LEDR, HIGH);  // Rouge ON (logique normale)
    digitalWrite(LEDG, LOW);   // Vert OFF
  }

}

void loop() {

     digitalWrite(LED_D0, HIGH);
  delay(100);
  digitalWrite(LED_D0, LOW);
  delay(100);

  digitalWrite(LED_D1, HIGH);
  delay(100);
  digitalWrite(LED_D1, LOW);
  delay(100);

  digitalWrite(LED_D2, HIGH);
  delay(100);
  digitalWrite(LED_D2, LOW);
  delay(100);

  digitalWrite(LED_D3, HIGH);
  delay(100);
  digitalWrite(LED_D3, LOW);
  delay(500);

  // Détection appui bouton USER avec anti-rebond
  int buttonReading = digitalRead(BTN_USER);
  
  if (buttonReading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonReading == LOW) {  // Bouton pressé
      // Test des relais - activation 2 secondes chacun
      Serial.println("Test des relais déclenché...");
      
      digitalWrite(RELAY1, HIGH);
      delay(2000);
      digitalWrite(RELAY1, LOW);
      delay(100);

      digitalWrite(RELAY2, HIGH);
      delay(2000);
      digitalWrite(RELAY2, LOW);
      delay(100);

      digitalWrite(RELAY3, HIGH);
      delay(2000);
      digitalWrite(RELAY3, LOW);
      delay(100);

      digitalWrite(RELAY4, HIGH);
      delay(2000);
      digitalWrite(RELAY4, LOW);
      delay(500);
      
      Serial.println("Test des relais terminé");
      
      // Attendre que le bouton soit relâché
      while(digitalRead(BTN_USER) == LOW) {
        delay(10);
      }
    }
  }
  
  lastButtonState = buttonReading;

  // Vérifier si un client est connecté
  WiFiClient client = server.accept();
  
  if (client) {
    Serial.println("Nouveau client connecté");
    String currentLine = "";
    String request = "";
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Traitement de la requête
            // Commandes RELAY1
            if (request.indexOf("GET /RELAY1=ON") >= 0) {
              Serial.println("Commande: RELAY1 ON");
              digitalWrite(RELAY1, HIGH);
            }
            else if (request.indexOf("GET /RELAY1=OFF") >= 0) {
              Serial.println("Commande: RELAY1 OFF");
              digitalWrite(RELAY1, LOW);
            }
            // Commandes RELAY2
            else if (request.indexOf("GET /RELAY2=ON") >= 0) {
              Serial.println("Commande: RELAY2 ON");
              digitalWrite(RELAY2, HIGH);
            }
            else if (request.indexOf("GET /RELAY2=OFF") >= 0) {
              Serial.println("Commande: RELAY2 OFF");
              digitalWrite(RELAY2, LOW);
            }
            // Commandes RELAY3
            else if (request.indexOf("GET /RELAY3=ON") >= 0) {
              Serial.println("Commande: RELAY3 ON");
              digitalWrite(RELAY3, HIGH);
            }
            else if (request.indexOf("GET /RELAY3=OFF") >= 0) {
              Serial.println("Commande: RELAY3 OFF");
              digitalWrite(RELAY3, LOW);
            }
            // Commandes RELAY4
            else if (request.indexOf("GET /RELAY4=ON") >= 0) {
              Serial.println("Commande: RELAY4 ON");
              digitalWrite(RELAY4, HIGH);
            }
            else if (request.indexOf("GET /RELAY4=OFF") >= 0) {
              Serial.println("Commande: RELAY4 OFF");
              digitalWrite(RELAY4, LOW);
            }
            
            // Envoi de la réponse HTTP
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html; charset=utf-8");
            client.println("Connection: close");
            client.println();
            
            // Envoi de la page HTML avec remplacement des placeholders
            String page = webpage;
            
            // Informations WiFi
            page.replace("HOSTNAME_PLACEHOLDER", "Arduino-OPTA");
            page.replace("SSID_PLACEHOLDER", WiFi.SSID());
            page.replace("RSSI_PLACEHOLDER", String(WiFi.RSSI()));
            page.replace("IP_PLACEHOLDER", WiFi.localIP().toString());
            page.replace("MAC_PLACEHOLDER", WiFi.macAddress());
            
            // Lecture et remplacement des états des entrées I1-I8
            page.replace("INPUT1_STATE", digitalRead(I1) ? "●" : "○");
            page.replace("INPUT1_CLASS", digitalRead(I1) ? "state-high" : "state-low");
            
            page.replace("INPUT2_STATE", digitalRead(I2) ? "●" : "○");
            page.replace("INPUT2_CLASS", digitalRead(I2) ? "state-high" : "state-low");
            
            page.replace("INPUT3_STATE", digitalRead(I3) ? "●" : "○");
            page.replace("INPUT3_CLASS", digitalRead(I3) ? "state-high" : "state-low");
            
            page.replace("INPUT4_STATE", digitalRead(I4) ? "●" : "○");
            page.replace("INPUT4_CLASS", digitalRead(I4) ? "state-high" : "state-low");
            
            page.replace("INPUT5_STATE", digitalRead(I5) ? "●" : "○");
            page.replace("INPUT5_CLASS", digitalRead(I5) ? "state-high" : "state-low");
            
            page.replace("INPUT6_STATE", digitalRead(I6) ? "●" : "○");
            page.replace("INPUT6_CLASS", digitalRead(I6) ? "state-high" : "state-low");
            
            page.replace("INPUT7_STATE", digitalRead(I7) ? "●" : "○");
            page.replace("INPUT7_CLASS", digitalRead(I7) ? "state-high" : "state-low");
            
            page.replace("INPUT8_STATE", digitalRead(I8) ? "●" : "○");
            page.replace("INPUT8_CLASS", digitalRead(I8) ? "state-high" : "state-low");
            
            // Lecture et remplacement des états des relais RELAY1-4
            page.replace("RELAY1_STATE", digitalRead(RELAY1) ? "ON" : "OFF");
            page.replace("RELAY1_CLASS", digitalRead(RELAY1) ? "state-high" : "state-low");
            
            page.replace("RELAY2_STATE", digitalRead(RELAY2) ? "ON" : "OFF");
            page.replace("RELAY2_CLASS", digitalRead(RELAY2) ? "state-high" : "state-low");
            
            page.replace("RELAY3_STATE", digitalRead(RELAY3) ? "ON" : "OFF");
            page.replace("RELAY3_CLASS", digitalRead(RELAY3) ? "state-high" : "state-low");
            
            page.replace("RELAY4_STATE", digitalRead(RELAY4) ? "ON" : "OFF");
            page.replace("RELAY4_CLASS", digitalRead(RELAY4) ? "state-high" : "state-low");
            
            client.print(page);
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    
    client.stop();
    Serial.println("Client déconnecté\n");
  }
}
