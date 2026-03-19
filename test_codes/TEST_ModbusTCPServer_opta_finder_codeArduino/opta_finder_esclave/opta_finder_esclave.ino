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

// --- Paramètres de Scalabilité ---
#define MAX_AUTOCLAVES 10
int nbAutoclaves = 5;

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

// --- Fonction "Ping" et Synchronisation ---
void pingAndSync(int id) {
  modbusTCPClient.setTimeout(250); 
  
  if (modbusTCPClient.begin(ipAutoclaves[id], 502)) {
    enLigne[id] = true;
    modbusTCPClient.holdingRegisterWrite(0x02, (uint16_t)(consigneGlobale * 10));
    modbusTCPClient.holdingRegisterWrite(0x01, etatMachine[id]);
    long tempBrute = modbusTCPClient.holdingRegisterRead(0x00);
    if (tempBrute != -1) tempMachine[id] = tempBrute / 10.0;
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
  Serial.println("\n=== OPTA : SUPERVISION SCADA MODERNE ===");
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
          // 1. TRAITEMENT DES REQUÊTES AJAX (EN ARRIÈRE-PLAN)
          // =========================================================
          if (request.indexOf("GET /?scan=") >= 0) {
            int id = request.substring(request.indexOf("scan=") + 5, request.indexOf("scan=") + 7).toInt();
            if(id < nbAutoclaves) pingAndSync(id);
          }
          else if (request.indexOf("GET /?start=") >= 0) {
            int id = request.substring(request.indexOf("start=") + 6, request.indexOf("start=") + 8).toInt();
            if(id < nbAutoclaves) { etatMachine[id] = 1; pingAndSync(id); }
          }
          else if (request.indexOf("GET /?stop=") >= 0) {
            int id = request.substring(request.indexOf("stop=") + 5, request.indexOf("stop=") + 7).toInt();
            if(id < nbAutoclaves) { etatMachine[id] = 0; pingAndSync(id); }
          }
          else if (request.indexOf("GET /?consigne=") >= 0) {
            int posDebut = request.indexOf("consigne=") + 9;
            int posFin = request.indexOf(" HTTP", posDebut); 
            consigneGlobale = request.substring(posDebut, posFin).toFloat();
            for(int i = 0; i < nbAutoclaves; i++) { if(enLigne[i]) pingAndSync(i); }
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
          // 2. GÉNÉRATION DU HTML / CSS / JS MODERNE
          // =========================================================
          webClient.println("HTTP/1.1 200 OK");
          webClient.println("Content-Type: text/html; charset=utf-8");
          webClient.println("Connection: close");
          webClient.println();
          
          webClient.println("<!DOCTYPE html><html lang='fr'><head><meta charset='UTF-8'>");
          webClient.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
          webClient.println("<title>Raynal & Roquelaure - SCADA</title>");
          webClient.println("<style>");
          
          // --- CSS 2024 UI/UX ---
          webClient.println(":root { --bg: #0f1015; --card: #1c1d22; --accent: #c8102e; --gold: #f1c40f; --text: #e0e0e0; --success: #2ecc71; --danger: #e74c3c; --info: #3498db; }");
          webClient.println("body { background: var(--bg); color: var(--text); font-family: system-ui, -apple-system, sans-serif; margin: 0; padding: 20px; }");
          webClient.println(".container { max-width: 1400px; margin: 0 auto; }");
          
          // Header
          webClient.println(".header { background: linear-gradient(135deg, #15161a, #2a1114); padding: 25px 40px; border-radius: 16px; border-bottom: 4px solid var(--gold); display: flex; justify-content: space-between; align-items: center; box-shadow: 0 10px 30px rgba(200,16,46,0.15); margin-bottom: 30px; }");
          webClient.println(".header h1 { margin: 0; font-size: 2em; letter-spacing: 2px; color: #fff; text-transform: uppercase; }");
          webClient.println(".header p { margin: 5px 0 0 0; color: var(--gold); font-weight: 600; letter-spacing: 1px; text-transform: uppercase; }");
          
          // Panels
          webClient.println(".panels { display: flex; gap: 20px; justify-content: center; flex-wrap: wrap; margin-bottom: 30px; }");
          webClient.println(".panel { background: var(--card); padding: 20px 25px; border-radius: 16px; border: 1px solid #2a2b30; box-shadow: 0 8px 24px rgba(0,0,0,0.2); display: flex; align-items: center; gap: 15px; }");
          webClient.println(".panel h3 { margin: 0; color: #fff; font-weight: 500; font-size: 1.1em; }");
          webClient.println("input { background: #121215; border: 1px solid #333; color: #fff; padding: 10px 15px; border-radius: 8px; outline: none; font-size: 1em; width: 130px; text-align: center; transition: 0.3s; }");
          webClient.println("input:focus { border-color: var(--gold); box-shadow: 0 0 10px rgba(241,196,15,0.2); }");
          
          // Grid & Cards
          webClient.println(".grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(240px, 1fr)); gap: 20px; }");
          webClient.println(".card { background: var(--card); padding: 25px 20px; border-radius: 16px; border: 1px solid #2a2b30; box-shadow: 0 8px 24px rgba(0,0,0,0.2); text-align: center; position: relative; overflow: hidden; transition: 0.3s; }");
          webClient.println(".card:hover { transform: translateY(-5px); box-shadow: 0 12px 32px rgba(0,0,0,0.4); }");
          webClient.println(".card::before { content: ''; position: absolute; top: 0; left: 0; width: 100%; height: 4px; transition: 0.3s; }");
          webClient.println(".card.online::before { background: var(--success); box-shadow: 0 0 15px var(--success); }");
          webClient.println(".card.offline::before { background: var(--danger); box-shadow: 0 0 15px var(--danger); }");
          
          webClient.println(".card h4 { margin: 0; color: var(--gold); font-size: 1.3em; font-weight: 600; text-transform: uppercase; }");
          webClient.println(".ip { color: #888; font-size: 0.9em; margin: 5px 0 15px 0; font-family: monospace; }");
          
          // Température avec effet Dégradé
          webClient.println(".temp { font-size: 3.5em; font-weight: 800; margin: 15px 0; }");
          webClient.println(".temp.online { background: linear-gradient(45deg, #ff6b6b, #c0392b); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }");
          webClient.println(".temp.offline { color: #444; }");
          
          // Badges Statut
          webClient.println(".status { font-size: 0.85em; font-weight: 700; padding: 6px 12px; border-radius: 20px; display: inline-block; margin-bottom: 15px; letter-spacing: 1px; }");
          webClient.println(".st-on { background: rgba(46,204,113,0.1); color: var(--success); border: 1px solid rgba(46,204,113,0.2); }");
          webClient.println(".st-off { background: rgba(243,156,18,0.1); color: var(--warning); border: 1px solid rgba(243,156,18,0.2); }");
          webClient.println(".st-err { background: rgba(231,76,60,0.1); color: var(--danger); border: 1px solid rgba(231,76,60,0.2); }");
          
          // Boutons
          webClient.println(".btn { width: 100%; padding: 12px; border: none; border-radius: 8px; font-weight: 700; font-size: 0.9em; cursor: pointer; transition: 0.3s; margin-bottom: 10px; color: #fff; text-transform: uppercase; display: flex; justify-content: center; align-items: center; gap: 8px; }");
          webClient.println(".btn-primary { background: var(--accent); } .btn-primary:hover { background: #e01435; box-shadow: 0 0 15px rgba(200,16,46,0.4); }");
          webClient.println(".btn-success { background: var(--success); } .btn-success:hover { background: #27ae60; box-shadow: 0 0 15px rgba(46,204,113,0.4); }");
          webClient.println(".btn-info { background: var(--info); } .btn-info:hover { background: #2980b9; box-shadow: 0 0 15px rgba(52,152,219,0.4); }");
          webClient.println(".btn-outline { background: transparent; border: 1px solid #444; color: #888; } .btn-outline:hover { background: rgba(231,76,60,0.1); border-color: var(--danger); color: var(--danger); }");
          
          // Animation Loader Spinner CSS (C'est ça la magie JS !)
          webClient.println(".loader { width: 18px; height: 18px; border: 3px solid rgba(255,255,255,0.3); border-top-color: #fff; border-radius: 50%; animation: spin 1s linear infinite; display: none; }");
          webClient.println("@keyframes spin { 100% { transform: rotate(360deg); } }");
          webClient.println(".loading .loader { display: block; } .loading .txt { display: none; }");
          webClient.println("</style>");
          
          // --- SCRIPT JAVASCRIPT AJAX (Actualisation en Arrière-Plan) ---
          webClient.println("<script>");
          webClient.println("function req(btn, url) {");
          webClient.println("  btn.classList.add('loading');");
          webClient.println("  fetch(url).then(r=>r.text()).then(html=>{ document.open(); document.write(html); document.close(); }).catch(e=>alert('Erreur réseau avec l\\'OPTA'));");
          webClient.println("}");
          webClient.println("function frm(e, form, prefix) {");
          webClient.println("  e.preventDefault();");
          webClient.println("  form.querySelector('.btn').classList.add('loading');");
          webClient.println("  let val = form.querySelector('input').value;");
          webClient.println("  fetch(prefix + val).then(r=>r.text()).then(html=>{ document.open(); document.write(html); document.close(); });");
          webClient.println("}");
          webClient.println("</script>");
          webClient.println("</head><body><div class='container'>");
          
          // --- HTML BODY ---
          webClient.println("<div class='header'><div><h1>RAYNAL & ROQUELAURE</h1><p>Supervision SCADA Modbus TCP</p></div></div>");
          
          webClient.println("<div class='panels'>");
          webClient.println("<div class='panel'><h3>Consigne globale (°C)</h3>");
          webClient.println("<form onsubmit='frm(event, this, \"/?consigne=\")' style='display:flex; gap:10px;'>");
          webClient.print("<input type='number' step='0.1' value='");
          webClient.print(consigneGlobale, 1);
          webClient.println("'>");
          webClient.println("<button type='submit' class='btn btn-primary' style='margin:0; width:auto;'><span class='txt'>APPLIQUER</span><div class='loader'></div></button></form></div>");
          
          webClient.println("<div class='panel'><h3>Ajouter un équipement</h3>");
          if (nbAutoclaves < MAX_AUTOCLAVES) {
            webClient.println("<form onsubmit='frm(event, this, \"/?add_ip=\")' style='display:flex; gap:10px;'>");
            webClient.println("<input type='text' placeholder='IP (ex: .56)' required>");
            webClient.println("<button type='submit' class='btn btn-info' style='margin:0; width:auto;'><span class='txt'>INTÉGRER</span><div class='loader'></div></button></form>");
          } else {
            webClient.println("<span style='color:var(--danger); font-weight:bold;'>Réseau Saturé (10 max)</span>");
          }
          webClient.println("</div></div>");

          webClient.println("<div class='grid'>");
          for(int i = 0; i < nbAutoclaves; i++) {
            String ipStr = String(ipAutoclaves[i][0]) + "." + String(ipAutoclaves[i][1]) + "." + String(ipAutoclaves[i][2]) + "." + String(ipAutoclaves[i][3]);
            String cClass = enLigne[i] ? "card online" : "card offline";
            
            webClient.println("<div class='" + cClass + "'>");
            webClient.println("<h4>Autoclave " + String(i+1) + "</h4>");
            webClient.println("<p class='ip'>" + ipStr + "</p>");
            
            if (enLigne[i]) {
              if (etatMachine[i] == 1) {
                webClient.println("<div class='status st-on'>● EN CHAUFFE</div>");
                webClient.println("<div class='temp online'>" + String(tempMachine[i], 1) + "</div>");
                webClient.println("<button onclick='req(this, \"/?stop=" + String(i) + "\")' class='btn btn-primary'><span class='txt'>ARRÊTER LE CYCLE</span><div class='loader'></div></button>");
              } else {
                webClient.println("<div class='status st-off'>● EN ATTENTE</div>");
                webClient.println("<div class='temp online'>" + String(tempMachine[i], 1) + "</div>");
                webClient.println("<button onclick='req(this, \"/?start=" + String(i) + "\")' class='btn btn-success'><span class='txt'>DÉMARRER</span><div class='loader'></div></button>");
              }
              webClient.println("<button onclick='req(this, \"/?scan=" + String(i) + "\")' class='btn btn-info'><span class='txt'>ACTUALISER (PING)</span><div class='loader'></div></button>");
            } else {
              webClient.println("<div class='status st-err'>● HORS LIGNE</div>");
              webClient.println("<div class='temp offline'>--.-</div>");
              webClient.println("<button onclick='req(this, \"/?scan=" + String(i) + "\")' class='btn btn-primary'><span class='txt'>CONNECTER</span><div class='loader'></div></button>");
            }
            webClient.println("<button onclick='req(this, \"/?del=" + String(i) + "\")' class='btn btn-outline'><span class='txt'>SUPPRIMER</span><div class='loader'></div></button>");
            webClient.println("</div>");
          }
          webClient.println("</div></div></body></html>");
          
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