# Test Modbus RTU RS485 - WT32-ETH01

## Description
Ce projet permet de tester la communication Modbus RTU entre deux cartes WT32-ETH01 via une interface RS485.

## Matériel nécessaire
- 2 cartes WT32-ETH01
- 2 modules convertisseurs RS485 (ex: MAX485, MAX3485)
- Câbles de connexion
- Résistances de terminaison 120Ω (optionnel mais recommandé)

## Câblage RS485

### Connexion WT32-ETH01 vers module RS485

| WT32-ETH01 | Module RS485 | Description |
|------------|--------------|-------------|
| GPIO17 (TX2) | DI (Data In) | Transmission de données |
| GPIO5 (RX2) | RO (Receiver Out) | Réception de données |
| GPIO4 | DE + RE | Contrôle direction (émission/réception) |
| GND | GND | Masse commune |
| 3.3V | VCC | Alimentation (si module 3.3V) |

### Connexion entre les deux modules RS485

| Module 1 | Module 2 |
|----------|----------|
| A | A |
| B | B |
| GND | GND |

⚠️ **Important**: 
- Les lignes A et B doivent être connectées en parallèle entre les deux modules
- Utilisez un câble torsadé (twisted pair) pour A et B si possible
- Ajoutez une résistance de terminaison de 120Ω entre A et B aux deux extrémités du bus

## Configuration du projet

### Bibliothèques utilisées
- `modbus-esp8266` v4.1.0 : Librairie Modbus RTU pour ESP32

### Structure des fichiers
```
src/
├── main.cpp          → Code Esclave (Slave) par défaut
├── main_slave.cpp    → Code Esclave (Slave)
└── main_master.cpp   → Code Maître (Master)
```

## Utilisation

### 1. Programmer la première carte en mode ESCLAVE (Slave)

Le fichier `main.cpp` contient par défaut le code esclave. Compilez et téléversez directement :

```bash
pio run -t upload
```

**Ou** copiez le contenu de `main_slave.cpp` dans `main.cpp`.

### 2. Programmer la seconde carte en mode MAÎTRE (Master)

Remplacez le contenu de `main.cpp` par celui de `main_master.cpp`, puis :

```bash
pio run -t upload
```

### 3. Tester la communication

1. Connectez les deux modules RS485 ensemble (A ↔ A, B ↔ B, GND ↔ GND)
2. Alimentez les deux cartes WT32-ETH01
3. Ouvrez le moniteur série sur les deux cartes (115200 bauds)

**Sur l'Esclave**, vous verrez :
```
=== Modbus RTU Slave ===
Carte: WT32-ETH01
Mode: ESCLAVE (Slave)
Esclave Modbus démarré
Adresse esclave: 1
Vitesse: 9600 bauds, Format: 8N1

En attente de requêtes du maître...

--- État des registres ---
Reg[0] = 0
Reg[1] = 100
Reg[2] = 200
...
```

**Sur le Maître**, vous verrez :
```
=== Modbus RTU Master ===
Carte: WT32-ETH01
Mode: MAÎTRE (Master)
Maître Modbus démarré
Esclave cible: 1

>>> Envoi requête de LECTURE <<<
Lecture de 10 registres à partir de l'adresse 0

=== Lecture réussie ===
Reg[0] = 0
Reg[1] = 100
Reg[2] = 200
...
```

## Fonctionnalités implémentées

### Mode Esclave (Slave)
- **Adresse Modbus** : 1
- **Registres disponibles** : 10 registres Holding (adresses 0 à 9)
- **Valeurs initiales** : Reg[n] = n × 100
- **Mise à jour automatique** : Le registre 0 s'incrémente toutes les 2 secondes
- **Affichage** : État des registres toutes les 2 secondes

### Mode Maître (Master)
- **Lecture** : Lit les 10 registres toutes les 3 secondes
- **Écriture** : Écrit une valeur dans le registre 5 toutes les 5 secondes
- **Valeur d'écriture** : Commence à 1000 et s'incrémente de 10 à chaque écriture
- **Callback** : Affichage des résultats de lecture/écriture

## Configuration Modbus

| Paramètre | Valeur |
|-----------|--------|
| Vitesse (Baud rate) | 9600 |
| Bits de données | 8 |
| Parité | None (N) |
| Bits d'arrêt | 1 |
| Adresse esclave | 1 |

## Personnalisation

### Changer l'adresse de l'esclave
Dans le code Slave :
```cpp
#define SLAVE_ID 2  // Nouvelle adresse
```

Dans le code Master :
```cpp
#define SLAVE_ID 2  // Adresse de l'esclave à interroger
```

### Changer le nombre de registres
```cpp
#define REGN 20  // Passer de 10 à 20 registres
```

### Changer les broches GPIO
```cpp
#define RXD2 5          // Nouvelle broche RX
#define TXD2 17         // Nouvelle broche TX
#define DE_RE_PIN 4     // Nouvelle broche DE/RE
```

### Changer la vitesse de communication
```cpp
Serial2.begin(19200, SERIAL_8N1, RXD2, TXD2);  // 19200 bauds au lieu de 9600
```

## Dépannage

### Problème : Pas de communication
- Vérifier le câblage A, B et GND
- Vérifier que les deux cartes utilisent la même vitesse (9600 bauds)
- Vérifier que l'adresse de l'esclave correspond dans les deux codes
- Vérifier l'alimentation des modules RS485

### Problème : Erreurs de lecture/écriture
- Ajouter des résistances de terminaison 120Ω
- Réduire la vitesse de communication (essayer 4800 bauds)
- Réduire la distance entre les cartes
- Vérifier l'inversion des lignes A et B

### Problème : Communication intermittente
- Vérifier la qualité des connexions
- Utiliser un câble torsadé pour A et B
- Ajouter un petit délai dans le code : `delay(10);`

## Moniteur série

Pour observer les échanges, ouvrez deux moniteurs série :

**Terminal 1 (Esclave)** :
```bash
pio device monitor -b 115200 -p COM3
```

**Terminal 2 (Maître)** :
```bash
pio device monitor -b 115200 -p COM4
```

## Références
- [Protocole Modbus RTU](https://modbus.org/)
- [Bibliothèque modbus-esp8266](https://github.com/emelianov/modbus-esp8266)
- [WT32-ETH01 Pinout](https://github.com/ldijkman/WT32-ETH01-LAN-8720-RJ45-)

## Licence
Projet pédagogique - BTS CIEL 2025-2026
Armand Payen
