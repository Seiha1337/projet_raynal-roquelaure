# 📘 Spécifications Techniques et Adressage Modbus TCP

## 1. Architecture Réseau
Le système de l'autoclave Raynal & Roquelaure repose sur une architecture Ethernet locale (LAN) TCP/IPv4.

* **Sous-réseau industriel :** `192.168.50.0 / 24`
* **Simulateur Autoclave (ESP32 WT32-ETH01) :** `192.168.50.50`
* **Automate Maître/Esclave (Finder OPTA) :** `192.168.50.210`
* **PC de Supervision (IHM / qModMaster) :** `192.168.50.100`

## 2. Paramètres du Serveur Modbus
* **Protocole :** Modbus TCP/IP
* **Port d'écoute :** 502
* **Unit ID (Slave ID) :** 1 (Par défaut)

## 3. Table de Mapping (Holding Registers - 0x03 / 0x06)

| Adresse (Déc) | Registre (Hex) | Type d'accès | Plage de valeurs | Description détaillée |
| :---: | :---: | :---: | :--- | :--- |
| **0** | `0x0000` | Lecture (R) | 200 à 1200 | **Température de la cuve** (Valeur entière x10. Ex: `265` = 26.5 °C). |
| **1** | `0x0001` | Lecture (R) | 0 à 3 | **État du cycle** (`0`: Repos, `1`: Ajustement, `2`: Stérilisation, `3`: Refroidissement). |
| **2** | `0x0002` | Lecture / Écriture (R/W) | 200 à 1100 | **Consigne de stérilisation** (Valeur entière x10. Ex: `1100` = 110.0 °C). Bridage de sécurité à 110°C. |

---
*Document généré pour le projet de fin d'études BTS.*