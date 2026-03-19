/*
!! L'automate génère 2 ports COM. Imposez le bon !
*/
#include <Arduino.h>
#include <Ethernet.h>
#include <ArduinoModbus.h>

// ========== CONFIGURATION ETHERNET FILAIRE (RJ45 UNIQUEMENT) ==========
byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x76, 0x05 }; 
IPAddress ip(192, 168, 50, 50);

EthernetServer webServer(80);
EthernetClient ethClient;          
ModbusTCPClient modbusTCPClient(ethClient); 

// --- Paramètres de Scalabilité (Mise à l'échelle) ---
#define MAX_AUTOCLAVES 10
int nbAutoclaves = 5; // On commence avec les 5 de base à chaque redémarrage

// Tableaux de données dimensionnés pour le maximum
IPAddress ipAutoclaves[MAX_AUTOCLAVES] = {
  IPAddress(192, 168, 50, 51),
  IPAddress(192, 168, 50, 52),
  IPAddress(192, 168, 50, 53),
  IPAddress(192, 168, 50, 54),
  IPAddress(192, 168, 50, 55)
};

float consigneGlobale = 110.0;
int etatMachine[MAX_AUTOCLAVES] = {0};
float tempMachine[MAX_AUTOCLAVES] = {0.0};
bool enLigne[MAX_AUTOCLAVES] = {false};

// --- Fonction "Ping" et Synchronisation d'une machine ---
void pingAndSync(int id) {
  // Timeout très court (250ms) : agit comme un Ping.
  modbusTCPClient.setTimeout(250); 
  
  if (modbusTCPClient.begin(ipAutoclaves[id], 502)) {
    enLigne[id] = true;
    
    // 1. Envoi de la consigne globale
    modbusTCPClient.holdingRegisterWrite(0x02, (uint16_t)(consigneGlobale * 10));
    // 2. Envoi de l'état (Marche/Arrêt)
    modbusTCPClient.holdingRegisterWrite(0x01, etatMachine[id]);
    // 3. Lecture de la température
    long tempBrute = modbusTCPClient.holdingRegisterRead(0x00);
    if (tempBrute != -1) {
      tempMachine[id] = tempBrute / 10.0;
    }
    
    modbusTCPClient.stop();
  } else {
    enLigne[id] = false;
    tempMachine[id] = 0.0;
    modbusTCPClient.stop();
    ethClient.stop();
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n=== OPTA : SUPERVISION DA CARNUS / R&R ===");
  Ethernet.begin(mac, ip);
  delay(1000);
  Serial.print("IP OPTA: ");
  Serial.println(Ethernet.localIP());
  webServer.begin();
}

void loop() {
  EthernetClient webClient = webServer.available();
  if (webClient) {
    String request = "";
    boolean currentLineIsBlank = true; 

    while (webClient.connected()) {
      if (webClient.available()) {
        char c = webClient.read();
        request += c;
        
        if (c == '\n' && currentLineIsBlank) {
          
          // =========================================================
          // 1. TRAITEMENT DES ORDRES UTILISATEUR
          // =========================================================
          
          if (request.indexOf("GET /?scan=") >= 0) {
            int id = request.substring(request.indexOf("scan=") + 5, request.indexOf("scan=") + 7).toInt();
            if(id < nbAutoclaves) pingAndSync(id);
          }
          else if (request.indexOf("GET /?start=") >= 0) {
            int id = request.substring(request.indexOf("start=") + 6, request.indexOf("start=") + 8).toInt();
            if(id < nbAutoclaves) {
              etatMachine[id] = 1;
              pingAndSync(id);
            }
          }
          else if (request.indexOf("GET /?stop=") >= 0) {
            int id = request.substring(request.indexOf("stop=") + 5, request.indexOf("stop=") + 7).toInt();
            if(id < nbAutoclaves) {
              etatMachine[id] = 0;
              pingAndSync(id);
            }
          }
          else if (request.indexOf("GET /?consigne=") >= 0) {
            int posDebut = request.indexOf("consigne=") + 9;
            int posFin = request.indexOf(" HTTP", posDebut); 
            consigneGlobale = request.substring(posDebut, posFin).toFloat();
            for(int i = 0; i < nbAutoclaves; i++) {
              if(enLigne[i]) pingAndSync(i);
            }
          }
          else if (request.indexOf("GET /?add_ip=") >= 0) {
            int posDebut = request.indexOf("add_ip=") + 7;
            int posFin = request.indexOf(" HTTP", posDebut);
            String newIpStr = request.substring(posDebut, posFin);
            
            if (nbAutoclaves < MAX_AUTOCLAVES) {
              IPAddress newIP;
              if (newIP.fromString(newIpStr)) { 
                ipAutoclaves[nbAutoclaves] = newIP;
                etatMachine[nbAutoclaves] = 0;
                pingAndSync(nbAutoclaves); 
                nbAutoclaves++;
              }
            }
          }
          else if (request.indexOf("GET /?del=") >= 0) {
            int posDebut = request.indexOf("del=") + 4;
            int posFin = request.indexOf(" HTTP", posDebut);
            int id = request.substring(posDebut, posFin).toInt();
            
            if (id >= 0 && id < nbAutoclaves) {
              for (int i = id; i < nbAutoclaves - 1; i++) {
                ipAutoclaves[i] = ipAutoclaves[i+1];
                etatMachine[i] = etatMachine[i+1];
                tempMachine[i] = tempMachine[i+1];
                enLigne[i] = enLigne[i+1];
              }
              nbAutoclaves--;
            }
          }

          // =========================================================
          // 2. GENERATION DYNAMIQUE DE LA PAGE HTML (Charte Graphique)
          // =========================================================
          webClient.println("HTTP/1.1 200 OK");
          webClient.println("Content-Type: text/html; charset=utf-8");
          webClient.println("Connection: close");
          webClient.println();
          
          webClient.println("<!DOCTYPE html><html lang='fr'><head><meta charset='UTF-8'>");
          webClient.println("<title>Supervision Carnus x R&R</title>");
          webClient.println("<style>");
          
          // --- VARIABLES DE COULEURS OFFICIELLES ---
          webClient.println(":root { --bleu-carnus: #003366; --or-carnus: #f1c40f; --rouge-rr: #c8102e; }");
          
          webClient.println("body { background-color: #121212; color: #fff; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; padding: 20px; margin: 0; }");
          webClient.println(".container { max-width: 95%; margin: 0 auto; }");
          
          // Bandeau avec dégradé diagonal séparant les deux entités
          webClient.println(".header { background: linear-gradient(135deg, var(--bleu-carnus) 0%, var(--bleu-carnus) 48%, var(--or-carnus) 50%, var(--rouge-rr) 52%, var(--rouge-rr) 100%); padding: 20px 40px; border-radius: 12px; margin-bottom: 25px; display: flex; justify-content: space-between; align-items: center; box-shadow: 0 8px 20px rgba(0,0,0,0.5); }");
          webClient.println(".header-left { text-align: left; } .header-right { text-align: right; }");
          webClient.println(".header h1 { margin: 0; font-size: 1.8em; text-transform: uppercase; letter-spacing: 2px; text-shadow: 2px 2px 4px rgba(0,0,0,0.5); }");
          webClient.println(".header p { margin: 5px 0 0 0; color: var(--or-carnus); font-weight: bold; font-size: 1.1em; letter-spacing: 1px; text-shadow: 1px 1px 2px rgba(0,0,0,0.8); }");
          
          webClient.println(".flex-panels { display: flex; justify-content: center; gap: 20px; flex-wrap: wrap; margin-bottom: 30px; }");
          webClient.println(".panel { background: #1e1e1e; padding: 20px; border-radius: 10px; border: 1px solid #333; min-width: 320px; box-shadow: 0 4px 10px rgba(0,0,0,0.3); border-top: 4px solid var(--or-carnus); }");
          
          // Conteneur forçant la ligne unique pour les autoclaves
          webClient.println(".machines-wrapper { width: 100%; overflow-x: auto; padding-bottom: 20px; }");
          webClient.println(".machines-grid { display: flex; flex-wrap: nowrap; gap: 20px; min-width: min-content; }");
          
          // Design des cartes avec bordures aux couleurs des deux établissements
          webClient.println(".machine-card { flex: 0 0 auto; min-width: 230px; background: #1e1e1e; border-radius: 10px; padding: 20px; box-shadow: 0 6px 12px rgba(0,0,0,0.4); text-align: center; border-left: 5px solid var(--bleu-carnus); border-right: 5px solid var(--rouge-rr); transition: transform 0.2s; }");
          webClient.println(".machine-card:hover { transform: translateY(-5px); }");
          
          webClient.println(".temp-val { font-size: 2.8em; font-weight: bold; margin: 15px 0; color: #e74c3c; text-shadow: 0 0 10px rgba(231,76,60,0.2); }");
          webClient.println(".btn { padding: 12px 15px; text-decoration: none; color: white; border-radius: 6px; display: inline-block; font-weight: bold; width: 100%; box-sizing: border-box; margin-bottom: 10px; text-transform: uppercase; font-size: 0.9em; transition: 0.2s; }");
          webClient.println(".btn-start { background: #27ae60; } .btn-start:hover { background: #2ecc71; }");
          webClient.println(".btn-stop { background: var(--rouge-rr); } .btn-stop:hover { background: #e74c3c; }");
          webClient.println(".btn-ping { background: var(--bleu-carnus); } .btn-ping:hover { background: #004b99; }");
          webClient.println(".btn-add { background: var(--rouge-rr); border: none; cursor: pointer; color: white; } .btn-add:hover{ background: #e01435; }");
          webClient.println(".btn-del { background: #444; padding: 8px; margin-top: 15px; font-size: 0.85em; } .btn-del:hover { background: #e74c3c; }");
          webClient.println("input[type=number], input[type=text] { padding: 12px; font-size: 1.1em; text-align: center; border-radius: 5px; border: 1px solid #444; background: #2d2d2d; color: white; margin-bottom: 15px; width: 85%; }");
          webClient.println("button { padding: 12px 15px; background: var(--or-carnus); color: #000; border: none; font-weight: bold; cursor: pointer; border-radius: 5px; width: 85%; transition: 0.2s; text-transform: uppercase; }");
          webClient.println("button:hover { background: #f39c12; color: white; }");
          webClient.println("</style></head><body><div class='container'>");
          
          // --- En-tête ---
          webClient.println("<div class='header'>");
          webClient.println("<div class='header-left'><h1>LYCÉE CHARLES CARNUS</h1><p>BTS CIEL - Épreuve E6</p></div>");
          webClient.println("<div class='header-right'><h1>RAYNAL & ROQUELAURE</h1><p>Supervision Modbus Industrielle</p></div>");
          webClient.println("</div>");
          
          // --- Panneaux de contrôle ---
          webClient.println("<div class='flex-panels'>");
          
          webClient.println("<div class='panel'><h3>Consigne Globale (°C)</h3>");
          webClient.println("<form action='/' method='GET'>");
          webClient.print("<input type='number' name='consigne' value='");
          webClient.print(consigneGlobale, 1);
          webClient.println("' step='0.1'><br><button type='submit'>APPLIQUER LA CONSIGNE</button>");
          webClient.println("</form></div>");

          webClient.println("<div class='panel'><h3>Ajouter un Autoclave</h3>");
          if (nbAutoclaves < MAX_AUTOCLAVES) {
            webClient.println("<form action='/' method='GET'>");
            webClient.println("<input type='text' name='add_ip' placeholder='ex: 192.168.50.56' required><br>");
            webClient.println("<button type='submit' class='btn-add'>INTÉGRER & PING</button>");
            webClient.println("</form>");
          } else {
            webClient.println("<p style='color: #e74c3c; font-weight: bold;'>Capacité maximale réseau atteinte (10)</p>");
          }
          webClient.println("</div></div>");

          // --- Grille alignée sur une seule ligne ---
          webClient.println("<div class='machines-wrapper'><div class='machines-grid'>");
          
          for(int i = 0; i < nbAutoclaves; i++) {
            String ipStr = String(ipAutoclaves[i][0]) + "." + String(ipAutoclaves[i][1]) + "." + String(ipAutoclaves[i][2]) + "." + String(ipAutoclaves[i][3]);
            
            webClient.println("<div class='machine-card'>");
            webClient.println("<h3 style='color: var(--or-carnus); margin-top:0;'>Autoclave " + String(i+1) + "</h3>");
            webClient.println("<p style='margin: 5px 0; color: #aaa; font-size: 0.9em;'>IP: " + ipStr + "</p>");
            
            if (enLigne[i]) {
              webClient.println("<p style='color:#2ecc71; font-weight:bold; margin: 10px 0 5px 0;'>✓ EN LIGNE</p>");
              webClient.println("<div class='temp-val'>" + String(tempMachine[i], 1) + " °C</div>");
              
              if (etatMachine[i] == 1) {
                webClient.println("<p style='color:#f39c12; font-weight:bold; margin-bottom:15px;'>EN CHAUFFE</p>");
                webClient.println("<a href='/?stop=" + String(i) + "' class='btn btn-stop'>ARRÊTER</a>");
              } else {
                webClient.println("<p style='color:#95a5a6; font-weight:bold; margin-bottom:15px;'>À L'ARRÊT</p>");
                webClient.println("<a href='/?start=" + String(i) + "' class='btn btn-start'>DÉMARRER</a>");
              }
              webClient.println("<a href='/?scan=" + String(i) + "' class='btn btn-ping'>ACTUALISER</a>");
            } else {
              webClient.println("<p style='color:#c0392b; font-weight:bold; margin: 10px 0 5px 0;'>✗ HORS LIGNE</p>");
              webClient.println("<div class='temp-val' style='color:#7f8c8d'>--.- °C</div>");
              webClient.println("<a href='/?scan=" + String(i) + "' class='btn btn-ping'>CONNEXION</a>");
            }
            
            webClient.println("<a href='/?del=" + String(i) + "' class='btn btn-del'>🗑️ RETIRER DU RÉSEAU</a>");
            webClient.println("</div>"); // Fin machine-card
          }
          
          webClient.println("</div></div>"); // Fin machines-grid & machines-wrapper
          webClient.println("</div></body></html>");
          
          delay(10); 
          break;
        }
        
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    webClient.stop(); 
  }
}