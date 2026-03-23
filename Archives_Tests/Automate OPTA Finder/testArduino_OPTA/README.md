# Arduino OPTA - Variables Entrées/Sorties

## Description
Ce document liste l'ensemble des variables relatives aux entrées/sorties de l'automate Arduino OPTA.

---
## Notes Importantes

⚠️ **Attention au port COM** : L'automate génère 2 ports COM lors de la connexion USB. Assurez-vous de sélectionner le bon port avant de transférer le code (généralement COM11 pour cet automate).

💡 **Séquence de test** : Le code actuel fait clignoter les LEDs D0, D1, D2, et D3 en séquence pour vérifier leur fonctionnement.

---

## Variables d'Entrées/Sorties

### LEDs Intégrées ✅ (Utilisées dans le code)

| Variable | Type | Description | Broche |
|----------|------|-------------|--------|
| `LED_BUILTIN` | OUTPUT | LED intégrée principale de la carte Arduino OPTA | Broche système |
| `LEDR` | OUTPUT | LED rouge de statut (au-dessus du bouton USER) | Logique normale* |
| `LEDG` | OUTPUT | LED verte de statut (au-dessus du bouton USER) | Logique normale* |
| `LED_D0` | OUTPUT | LED utilisateur D0 (verte) | GPIO configurable |
| `LED_D1` | OUTPUT | LED utilisateur D1 (verte) | GPIO configurable |
| `LED_D2` | OUTPUT | LED utilisateur D2 (verte) | GPIO configurable |
| `LED_D3` | OUTPUT | LED utilisateur D3 (verte) | GPIO configurable |

**\*Logique normale** : `HIGH` = allumée, `LOW` = éteinte

**Signalisation WiFi actuelle :**
- 🔴🟢 Clignotement rouge/vert pendant la connexion
- 🟢 LED verte fixe = WiFi connecté
- 🔴 LED rouge fixe = Échec de connexion

### Relais ⚡ (Non utilisés)

| Variable | Type | Description | Tension Max |
|----------|------|-------------|-------------|
| `RELAY1` | OUTPUT | Relais 1 - Sortie de puissance | 250V AC / 30V DC |
| `RELAY2` | OUTPUT | Relais 2 - Sortie de puissance | 250V AC / 30V DC |
| `RELAY3` | OUTPUT | Relais 3 - Sortie de puissance | 250V AC / 30V DC |
| `RELAY4` | OUTPUT | Relais 4 - Sortie de puissance | 250V AC / 30V DC |

**Courant maximum** : 6A par relais

### Entrées Numériques 🔌 (Non utilisées)

| Variable | Type | Description | Tension Max |
|----------|------|-------------|-------------|
| `I1` | INPUT | Entrée numérique 1 | 24V |
| `I2` | INPUT | Entrée numérique 2 | 24V |
| `I3` | INPUT | Entrée numérique 3 | 24V |
| `I4` | INPUT | Entrée numérique 4 | 24V |
| `I5` | INPUT | Entrée numérique 5 | 24V |
| `I6` | INPUT | Entrée numérique 6 | 24V |
| `I7` | INPUT | Entrée numérique 7 | 24V |
| `I8` | INPUT | Entrée numérique 8 | 24V |

**Type** : Entrées optoisolées compatibles PNP/NPN

### Entrées Analogiques 📊 (Non utilisées)

**📍 Emplacement physique** : Ces entrées analogiques correspondent aux **mêmes bornes physiques** que les entrées numériques I1-I8 (bornier supérieur à 16 positions). La configuration logicielle détermine si elles fonctionnent en mode numérique (tout ou rien) ou analogique (mesure de tension).

**Correspondance :**
- `A0` = `I1` (bornier 1-2)
- `A1` = `I2` (bornier 3-4)
- `A2` = `I3` (bornier 5-6)
- `A3` = `I4` (bornier 7-8)
- `A4` = `I5` (bornier 9-10)
- `A5` = `I6` (bornier 11-12)
- `A6` = `I7` (bornier 13-14)
- `A7` = `I8` (bornier 15-16)

| Variable | Type | Description | Plage |
|----------|------|-------------|-------|
| `A0` | INPUT | Entrée analogique 0 (= I1 en mode analogique) | 0-10V / 0-25V |
| `A1` | INPUT | Entrée analogique 1 (= I2 en mode analogique) | 0-10V / 0-25V |
| `A2` | INPUT | Entrée analogique 2 (= I3 en mode analogique) | 0-10V / 0-25V |
| `A3` | INPUT | Entrée analogique 3 (= I4 en mode analogique) | 0-10V / 0-25V |
| `A4` | INPUT | Entrée analogique 4 (= I5 en mode analogique) | 0-10V / 0-25V |
| `A5` | INPUT | Entrée analogique 5 (= I6 en mode analogique) | 0-10V / 0-25V |
| `A6` | INPUT | Entrée analogique 6 (= I7 en mode analogique) | 0-10V / 0-25V |
| `A7` | INPUT | Entrée analogique 7 (= I8 en mode analogique) | 0-10V / 0-25V |

**Résolution** : 16 bits (0-65535)

### Boutons 🔘 (Non utilisés)

| Variable | Type | Description |
|----------|------|-------------|
| `BTN_USER` | INPUT | Bouton utilisateur principal |
| `USER_BTN` | INPUT | Alias pour BTN_USER |

### Communication RS485 📡 (Non utilisée)

| Variable | Description |
|----------|-------------|
| `RS485` | Interface série RS485 half-duplex/full-duplex |

**Vitesse max** : 12 Mbps

---

## Utilisation dans le Code

### Configuration des LEDs (dans setup())
```cpp
pinMode(LED_BUILTIN, OUTPUT);
pinMode(LED_D0, OUTPUT);
pinMode(LED_D1, OUTPUT);
pinMode(LED_D2, OUTPUT);
pinMode(LED_D3, OUTPUT);
```

### Contrôle des LEDs
```cpp
// Allumer une LED
digitalWrite(LED_D0, HIGH);

// Éteindre une LED
digitalWrite(LED_D0, LOW);
```

### Configuration et Contrôle des Relais
```cpp
// Configuration (dans setup())
pinMode(RELAY1, OUTPUT);
pinMode(RELAY2, OUTPUT);
pinMode(RELAY3, OUTPUT);
pinMode(RELAY4, OUTPUT);

// Activation d'un relais
digitalWrite(RELAY1, HIGH);

// Désactivation d'un relais
digitalWrite(RELAY1, LOW);
```

### Lecture des Entrées Numériques
```cpp
// Configuration (dans setup())
pinMode(I1, INPUT);
pinMode(I2, INPUT);
// ... jusqu'à I8

// Lecture d'une entrée
int etatI1 = digitalRead(I1);
if (etatI1 == HIGH) {
  // Entrée activée
}
```

### Lecture des Entrées Analogiques
```cpp
// Configuration (dans setup())
pinMode(A0, INPUT);
pinMode(A1, INPUT);
// ... jusqu'à A7

// Lecture d'une valeur analogique (0-65535)
int valeurA0 = analogRead(A0);

// Conversion en tension (0-10V)
float tension = (valeurA0 / 65535.0) * 10.0;
```

### Lecture du Bouton Utilisateur
```cpp
// Configuration (dans setup())
pinMode(BTN_USER, INPUT);

// Lecture de l'état du bouton
if (digitalRead(BTN_USER) == LOW) {
  // Bouton pressé
}
```

### Communication RS485
```cpp
#include <RS485.h>

// Dans setup()
RS485.begin(9600); // Initialisation à 9600 bauds
RS485.beginTransmission();
RS485.write("Hello");
RS485.endTransmission();

// Lecture
if (RS485.available()) {
  char c = RS485.read();
}
```

---

## Informations Techniques

### Plateforme
- **Modèle** : Arduino OPTA WiFi RS485
- **Microcontrôleur** : STM32H747XI Dual Core
- **Architecture** : ARM Cortex-M7 + Cortex-M4

### Fonctionnalités
- WiFi intégré (WiFiNINA)
- Communication série RS485 (half-duplex/full-duplex, jusqu'à 12 Mbps)
- 4 LEDs utilisateur programmables (D0-D3)
- 1 LED système (LED_BUILTIN)
- 4 Relais de puissance (6A, 250V AC / 30V DC)
- 8 Entrées numériques optoisolées (24V, PNP/NPN)
- 8 Entrées analogiques (0-10V/0-25V, 16 bits)
- 1 Bouton utilisateur programmable
- Ethernet (selon le modèle)
- Boîtier rail DIN pour installation industrielle

---



*Dernière mise à jour : Février 2026*
