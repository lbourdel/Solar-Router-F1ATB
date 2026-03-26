# LBR
Use Arduino IDE
Tools menu:
Board ESP32S3 Dev Module
Flash size 16MB
Partition scheme Custom

Flag CODE_UNUSED_LBR removes all unused source code
Flag DEBUG_LBR activate serial print for servo algorithm
In Arduino IDE:
- set in File/Preferences/Sketchbook location : dir where are .INO source code
- !!! source folder must be named as main filename i.e. .../Solar_Router/Solar_Router.ino !!!
- set in File/Preferences/Additional boards manager !!! take latest version from github !!! :
https://espressif.github.io/arduino-esp32/package_esp32_index.json
https://github.com/espressif/arduino-esp32/releases/download/3.3.1/package_esp32_dev_index.json
or
https://espressif.github.io/arduino-esp32/package_esp32_index.json

- set in Tools/Board manager to "esp32 by Espressif"
V3.3.7

- set in Tools/Board to "ESP32S3 dev module" or "ESP32-S3-WROOM-1":
!!! don't take ESP32-dev but ESP32S3 Dev Module !!!
Model used for ECS:
    ESP32-S3-WROOM-1-N16R8
    Flash 16MB
    PSRAM 8MB
    https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf
- install in Tools/Manage libraries
    * DallasTemperature by Miles Burton
    * EthernetESP32 by Juraj Andrassy
    * LovyanGFX by lovyan03
    * OneWire by Jim Studt, ..
    * PubSubClient by Nick O'Leary
    * UrlEncode by MasayukiSugahara
- if needed, patch LovyanGFX lib as described: https://github.com/lovyan03/LovyanGFX/commit/ac9198d

API WEB:
http://192.168.1.125/ActionsUpdateAction?NumAction=1&periode=1&value=600
NumAction : 1 à X (0 reserved for Triac)
periode : 0 à Y
value : 0 à 2400 :  end time of selected period (0275=2h45)
                    = start time of next period

IDE Options:
compile_commands.json:
   "-DARDUINO_FQBN=\"esp32:esp32:esp32s3:UploadSpeed=921600,USBMode=hwcdc,CDCOnBoot=default,MSCOnBoot=default,DFUOnBoot=default,UploadMode=default,CPUFreq=240,FlashMode=qio,FlashSize=16M,PartitionScheme=custom,DebugLevel=none,PSRAM=disabled,LoopCore=1,EventsCore=1,EraseFlash=all,JTAGAdapter=default,ZigbeeMode=default\"",
   "-DESP32=ESP32"

# ⚡ Routeur Photovoltaïque F1ATB

Routeur photovoltaïque basé sur **ESP32**, permettant d’optimiser l’autoconsommation d’une installation solaire en redirigeant les surplus de production vers des charges (chauffe-eau, chauffage, pompe, etc.) au lieu de les injecter sur le réseau.

[**Détails Routeur F1ATB**](https://f1atb.fr/fr)

---

## 🚀 Fonctionnalités principales

- 🔋 **Optimisation automatique de l’autoconsommation**
  - Mesure en temps réel de la puissance active importée/exportée.
  - Répartition intelligente des surplus photovoltaïques vers une ou plusieurs charges.
  - Réduction de l’énergie injectée sur le réseau à 0 W.

- ⚙️ **Mesures de puissance multi-sources**
  - Lecture directe du **compteur Linky** via **prise TIC**.
  - Mesure par méthode **UxI**, **UxIx2**, ou **UxIx3** à l’aide de sondes de courant.
  - Support des capteurs externes via **MQTT**, **Shelly EM**, etc.

- 🔌 **Pilotage des charges**
  - Sorties pour  **relais statiques (SSR)**.
  - Pilotage par **Triac** avec découpe sinusoïdale (suivi de phase secteur).
  - Modes d’actionnement configurables : tout ou rien, proportionnel, sinusoïdal, multi-sinus.

- 🌐 **Interface Web intégrée**
  - Visualisation en temps réel de la puissance, de la tension, du courant, de l’état des sorties.
  - Configuration des seuils, horaires, priorités, et paramètres réseau.
  - Historique et graphiques accessibles via navigateur.

- 🖥️ **Affichage local (selon version)**
  - Écran couleur 320×240  en option affichant les mesures instantanées et l’état des charges.
  - Navigation par boutons tactiles.

- 💬 **Communication et intégration domotique**
  - Serveur Web HTTP intégré.
  - Serveur Telnet (port 23 ) intégré.
  - **MQTT** pour l’intégration dans Home Assistant, Domoticz, Jeedom, etc.
  - **OTA (Over-The-Air)** : mise à jour logicielle à distance via WiFi ou Ethernet.
  - Configuration via  page Web embarquée.

- 🔒 **Sécurité et fiabilité**
  - **Watchdog** matériel/logiciel pour redémarrage automatique en cas de blocage.


- 🧩 **Compatibilité matérielle**
  - ESP32 Wroom,  WT32-ETH01, ESP32-ETH01, etc.
  - Connexion **WiFi** ou **Ethernet (PHY_LAN8720)**.
  - Alimentation possible via PoE (selon module).

---

## 📊 Exemple d’interface Web

![Interface Web](https://f1atb.fr/wp-content/uploads/2024/06/Capture-decran-2024-06-01-081356.png)

---

## ⚙️ Configuration typique

| Élément | Fonction | Interface |
|----------|-----------|------------|
| ESP32 | Microcontrôleur principal | WiFi / Ethernet |
| Linky TIC | Mesure puissance réseau | UART |
| Capteur UxI | Mesure courant + tension | ADC |
| Chauffe-eau | Charge 1 | Triac 1 |
| Chauffage | Charge 2 | Relais 2 |
| Capteur DS18B20 | Température | 1-Wire |
| Serveur MQTT | Communication domotique | TCP/IP |

---

## 🔧 Installation

1. **Cloner le dépôt**
   ```bash
   git clone https://github.com/F1ATB/Solar-Router-F1ATB.git
   cd Routeur-Photovoltaique-F1ATB
