const ModbusRTU = require("modbus-serial");

console.log("==========================================================");
console.log(" 🖥️ SUPERVISION DES 6 AUTOCLAVES : RAYNAL & ROQUELAURE");
console.log("==========================================================");

const autoclaves = [
    { id: "Autoclave 1 (OPTA)", ip: "192.168.50.50", mac: "A8:61:0A:AE:76:05" },
    { id: "Autoclave 2 (ESP32)", ip: "192.168.50.51", mac: "XX:XX:XX:XX:XX:XX" },
    { id: "Autoclave 3 (ESP32)", ip: "192.168.50.52", mac: "XX:XX:XX:XX:XX:XX" },
    { id: "Autoclave 4 (ESP32)", ip: "192.168.50.53", mac: "XX:XX:XX:XX:XX:XX" },
    { id: "Autoclave 5 (ESP32)", ip: "192.168.50.54", mac: "XX:XX:XX:XX:XX:XX" },
    { id: "Autoclave 6 (ESP32)", ip: "192.168.50.55", mac: "XX:XX:XX:XX:XX:XX" }
];

// Petite fonction utilitaire pour créer une pause
const sleep = (ms) => new Promise(resolve => setTimeout(resolve, ms));

async function scruterAutoclave(machine) {
    const client = new ModbusRTU();
    client.setTimeout(2000); 

    try {
        await client.connectTCP(machine.ip, { port: 502 });
        client.setID(1);

        const data = await client.readHoldingRegisters(0, 3);
        
        const temperature = data.data[0] / 10.0;
        const etatMachine = data.data[1];
        const consigne = data.data[2] / 10.0;

        let etatTexte = "Arrêt";
        let chauffeActuelle = "OFF";
        
        if (etatMachine === 1) {
            etatTexte = "Montée en Température";
            chauffeActuelle = "ON 🔥";
        } else if (etatMachine === 2) {
            etatTexte = "Stérilisation";
            chauffeActuelle = "RÉGULATION ⚖️";
        } else if (etatMachine === 3) {
            etatTexte = "Refroidissement";
            chauffeActuelle = "OFF ❄️";
        }

        console.log(`[🟢 EN LIGNE] ${machine.id}`);
        console.log(`    ├─ Réseau   : IP ${machine.ip} | MAC ${machine.mac}`);
        console.log(`    ├─ Process  : Temp = ${temperature}°C | Consigne = ${consigne}°C`);
        console.log(`    └─ Machine  : Cycle = ${etatTexte} | Relais Chauffe = ${chauffeActuelle}\n`);

    } catch (e) {
        console.log(`[🔴 HORS LIGNE] ${machine.id} (IP: ${machine.ip}) - Erreur: ${e.message}\n`);
    } finally {
        client.close();
    }
}

async function bouclePrincipale() {
    while (true) {
        console.log("----------------------------------------------------------");
        console.log(`🕒 Début de la scrutation : ${new Date().toLocaleTimeString()}`);
        
        // On attend que TOUTES les machines soient scrutées l'une après l'autre
        for (const machine of autoclaves) {
            await scruterAutoclave(machine);
        }

        console.log("✅ Cycle complet terminé.");
        console.log("💤 Attente de 5 secondes avant le prochain passage...");
        console.log("----------------------------------------------------------\n");
        
        // C'est ici que l'on marque la pause de 5 secondes APRES le travail
        await sleep(5000);
    }
}

// Lancement de la boucle infinie
bouclePrincipale();