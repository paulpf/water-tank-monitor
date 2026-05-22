# Water Tank Monitor (D1 Mini / ESP8266)

ESP8266-basierter Wasserstand-Monitor für den Longzhuo TL-136 Flüssigkeitsstand-Messumformer (4-20 mA, 12-32 VDC).
Liest den Tankfüllstand alle 5 Sekunden per ADC aus und gibt ihn über Serial aus.
WiFi-Konnektivität und OTA-Updates sind eingebaut.

## Inhaltsverzeichnis
- [Hardware](#hardware)
- [Verdrahtung](#verdrahtung)
- [Funktionsprinzip 4-20 mA](#funktionsprinzip-4-20-ma)
- [Projektstruktur](#projektstruktur)
- [Konfiguration](#konfiguration)
- [Build und Upload](#build-und-upload)
- [Serial-Ausgabe](#serial-ausgabe)
- [OTA-Updates](#ota-updates)
- [Security](#security)
- [Tests und CI](#tests-und-ci)

---

## Hardware

| Bauteil | Beschreibung |
|---|---|
| WeMos D1 Mini | ESP8266-Mikrocontroller, 3,3 V Logik |
| Longzhuo TL-136 | 2-Draht 4-20 mA Füllstandssensor, 12-32 VDC Loop-Speisung |
| Boost-Converter | 5 V USB → 12 V DC für den Sensor-Loop (Trimmer auf 12 V einstellen) |
| 4-20 mA Empfänger | Wandelt Stromsignal in Spannung: I+ / I– Eingang, 0–3,3 V Ausgang |
| ADS1115 | 16-Bit I²C ADC (Adafruit), I²C-Adresse 0x48 (ADDR → GND), GAIN_ONE ±4,096 V |

> **Auflösung:** ADS1115 liefert 0,125 mV/Bit (16-Bit) statt 3,2 mV/Bit des ESP8266-internen ADC — 25× besser.  
> **Kein separates Netzteil nötig:** Der Boost-Converter erzeugt die 12 V Loop-Spannung direkt aus USB 5 V.

---

## Verdrahtung

> Verdrahtungsdiagramm (draw.io): [docs/wiring.drawio](docs/wiring.drawio)
> Öffnen mit [draw.io Desktop](https://github.com/jgraph/drawio-desktop/releases) oder [app.diagrams.net](https://app.diagrams.net).

### Schaltplan (ASCII)

```
              Micro-USB
                  │
            [D1 Mini]
            │        │
           5V        GND ──────────────────────────────┐
            │                                          │
            └──── [Boost-Converter] ── 12V ── TL-136(+)│
                        │                       │      │
                       GND             [TL-136 Sensor] │
                        │                       │      │
                        │               TL-136(–)Blau  │
                        │                       │      │
                        │           [4-20mA Empfänger] │
                        │                  │    │      │
                        │            Vout(0–3,3V)      │
                        │                  │    GND ───┤
                        │            [ADS1115]          │
                        │            │    │    GND ───┤
                        └────────────┘    │           │
                          SCL/SDA     gemeinsame      │
                         [D1 Mini D1/D2]  GND-Schiene ┘
```

### Klemmbelegung Schritt für Schritt

| Schritt | Von | Nach | Beschreibung |
|---|---|---|---|
| 1 | D1 Mini **5V-Pin** | Boost-Converter IN+ | 5V vom USB-VBUS (Polyfuse 500 mA, passt) |
| 2 | Boost-Converter OUT+ (12 V) | TL-136 Braun (+) | Loop-Spannung |
| 3 | TL-136 Blau (–) | Empfänger I+ | 4-20 mA Signal |
| 4 | Empfänger I– | GND-Schiene | Loop-Rückleitung |
| 5 | Empfänger Vout | ADS1115 **A0** | Spannungssignal 0–3,3 V |
| 6 | ADS1115 **SCL** | D1 Mini **D1** | I²C Takt |
| 7 | ADS1115 **SDA** | D1 Mini **D2** | I²C Daten |
| 8 | ADS1115 **VDD** | D1 Mini **3V3** | ADS1115 Versorgung |
| 9 | ADS1115 **ADDR** | GND-Schiene | I²C-Adresse 0x48 |
| 10 | D1 Mini **GND** | GND-Schiene | Gemeinsame Masse |
| 11 | USB-Kabel | D1 Mini (Micro-USB) | Einzige externe Stromquelle |

> **GND-Schiene:** Boost-Converter-GND, Empfänger-GND, ADS1115-GND und D1-Mini-GND müssen alle verbunden sein.  
> **Strombudget:** ESP8266 ~170 mA + Boost ~57 mA + Rest ~6 mA = ~230 mA gesamt — USB 2.0 (500 mA) hat ausreichend Reserve.

### Kalibrierung des Signalempfängers

Der 4-20 mA Empfänger hat zwei Trimmer: **ZERO** (Nullpunkt) und **SPAN** (Vollausschlag).
Ziel: 4 mA → 0,00 V, 20 mA → 3,30 V am Ausgang.

#### Schritt 1 – ZERO einstellen (Nullpunkt, leerer Tank)

- Multimeter (DC-Spannung) zwischen Empfänger-Ausgang und GND klemmen
- Tank leeren **oder** 4 mA in I+/I– einspeisen (Kalibrierstromquelle / Shunt-Widerstand, s. u.)
- **ZERO-Trimmer** drehen bis Ausgangs­spannung = **0,00 V**

#### Schritt 2 – SPAN einstellen (Vollausschlag, voller Tank)

- Tank füllen **oder** 20 mA einspeisen
- **SPAN-Trimmer** drehen bis Ausgangs­spannung = **3,30 V**

#### Schritt 3 – Iterieren

ZERO und SPAN beeinflussen sich gegenseitig leicht.  
Schritte 1 und 2 zwei- bis dreimal wiederholen bis beide Punkte stabil sind.

#### 20 mA simulieren ohne vollen Tank

Einen bekannten Widerstand in den Loop schalten und die Spannung messen:

```
I = U / R    →    R = U / I

Beispiel: 12 V Boost, 560 Ω Widerstand → I = 12 V / 560 Ω ≈ 21,4 mA  (nah genug für SPAN)
```

Multimeter in Reihe (Strommessung) ist die präziseste Methode.

#### Hinweis: ZERO-Drift bei 12 V

Bei 12 V Loop-Spannung kann der ZERO-Punkt leicht über 0 V liegen.  
Liegt der Ausgang bei leerem Tank z. B. auf 0,05 V statt 0,00 V,  
den ZERO-Trimmer so weit wie möglich gegen 0 V trimmen — der Rest wird durch die **Software-Kalibrierung** (s. u.) korrigiert.

---

## Funktionsprinzip 4-20 mA

Der TL-136 ist ein **2-Draht Loop-Transmitter**: Er moduliert den Strom im
Loop proportional zum Füllstand. Der **4-20 mA Signalempfänger** wandelt
diesen Strom in eine Spannung um, die der ESP8266-ADC (A0, 10 Bit, 0–3,3 V) misst.

| Füllstand | Loop-Strom | Spannung an A0 (kalibriert) | ADC-Wert (0–1023) |
|---|---|---|---|
| 0 % | 4 mA | 0,00 V | 0 |
| 25 % | 8 mA | 0,83 V | ~256 |
| 50 % | 12 mA | 1,65 V | ~512 |
| 75 % | 16 mA | 2,48 V | ~768 |
| 100 % | 20 mA | 3,30 V | ~1023 |

**Formel:**

```
Spannung  = (ADC / 1023) × 3,3 V
Füllstand = (Spannung / 3,3 V) × 100   [%]
I_äquiv   = (Füllstand / 100) × 16 + 4  [mA]  ← nur für Logging/Validierung
```

Werte außerhalb 3,8–20,5 mA (äquivalent) werden als Sensor-/Verdrahtungsfehler geloggt.

---

## Projektstruktur

```text
water-tank-monitor/
├── src/
│   ├── app/
│   │   ├── application.cpp / .h    # Hauptschleife, Sensor-Polling
│   │   ├── bootstrap.cpp / .h      # Objektgraph / Dependency Injection
│   │   └── main.cpp
│   ├── config/
│   │   ├── config.h                # Alle Konstanten (Sensor, WiFi, OTA, Timings)
│   │   ├── global_defines.h
│   │   └── systemconfig.h
│   ├── contracts/
│   │   ├── ilevelsensor.h          # Interface für Füllstandssensor
│   │   ├── iotaloopcontrol.h
│   │   └── iwificonnectivity.h
│   ├── domain/
│   │   └── tanklevel.h             # Wertobjekt: currentMa + levelPercent
│   └── infrastructure/
│       ├── levelsensor.cpp / .h    # ADC-Auslese + 4-20 mA Konvertierung
│       ├── otamanager.cpp / .h
│       ├── trace.cpp / .h
│       └── wifimanager.cpp / .h
├── test/
├── scripts/
├── platformio.ini
└── README.md
```

Externe Secrets (nicht im Repo):
```text
../_secrets/
├── WifiSecret.h        # WIFI_SSID, WIFI_PWD
├── OtaSecret.h         # OTA_PASSWORD
└── last_ota_ip.txt     # zuletzt verwendete OTA-IP (automatisch)
```

---

## Konfiguration

Alle Konstanten liegen in [src/config/config.h](src/config/config.h).

### Sensor-Parameter

```cpp
constexpr uint8_t  SENSOR_ADS_I2C_ADDR    = 0x48;   // ADDR-Pin → GND
constexpr uint8_t  SENSOR_ADS_CHANNEL     = 0;      // ADS1115 Kanal A0
constexpr float    SENSOR_VREF            = 3.3f;   // V (= Vollausschlag Empfänger)
constexpr uint32_t SENSOR_READ_INTERVAL_MS = 5000;  // ms
```

Die Formel setzt voraus, dass der Signalempfänger auf **4 mA → 0 V** und **20 mA → 3,3 V** kalibriert ist (ZERO/SPAN-Trimmer, s. Abschnitt [Kalibrierung des Signalempfängers](#kalibrierung-des-signalempfängers)).

### Software-Kalibrierung (Feinabgleich)

Weicht der angezeigte Wert vom tatsächlichen Füllstand ab, kann eine lineare Korrektur in `config.h` eingetragen werden:

```cpp
constexpr float SENSOR_CAL_RAW    = 59.6f;  // angezeigter % beim Referenzpunkt
constexpr float SENSOR_CAL_ACTUAL = 71.1f;  // tatsächlicher % an diesem Punkt
```

**Vorgehen:**

1. Firmware flashen und Serial-Monitor öffnen
2. Tank auf einen **bekannten Füllstand** bringen (z. B. mit Maßband messen)
3. Den angezeigten %-Wert aus dem Serial-Log ablesen
4. `SENSOR_CAL_RAW` = angezeigter Wert, `SENSOR_CAL_ACTUAL` = echter Wert
5. Firmware neu bauen und flashen

Der Nullpunkt (0 % → 0 %) ist fix — es wird immer durch den Ursprung kalibriert.  
Der Korrekturfaktor ergibt sich automatisch: `Faktor = ACTUAL / RAW`.

> Beispiel: Angezeigt 59,6 %, tatsächlich 71,1 % → Faktor ≈ 1,193 → alle Werte werden hochskaliert.

### Library

```
adafruit/Adafruit ADS1X15 @ ^2.5.0
```

Wird automatisch über PlatformIO installiert.

### WiFi Credentials

Datei: `../_secrets/WifiSecret.h`
```cpp
#define WIFI_SSID "DEIN_NETZWERK"
#define WIFI_PWD  "DEIN_PASSWORT"
```

### OTA-Passwort

Datei: `../_secrets/OtaSecret.h`
```cpp
#define OTA_PASSWORD "dein_ota_passwort"
```

OTA ist **fail-closed**: ohne gesetztes Passwort und
`OTA_ALLOW_INSECURE_NO_PASSWORD false` bleibt OTA deaktiviert.

---

## Build und Upload

### USB (erstmaliger Flash)

```powershell
pio run -e d1-mini-usb
pio run -e d1-mini-usb -t upload -t monitor --upload-port COM3
```

### OTA (Folge-Updates)

Interaktives PowerShell-Skript (fragt IP ab, liest OTA-Passwort automatisch):

```powershell
.\scripts\upload_ota.ps1
```

Alternativ (Windows Batch):
```bat
scripts\upload_ota.bat
```

Die zuletzt verwendete IP wird in `../_secrets/last_ota_ip.txt` gespeichert.

---

## Serial-Ausgabe

Baud: **115200**

```
[INFO] Application setup started
[INFO] Startup is non-blocking, waiting for WiFi in main loop
[INFO] Startup: WiFi available, initializing OTA
[INFO] Tank: 47.3% (11.57 mA)
[INFO] Tank: 47.5% (11.60 mA)
[WARNING] Sensor out of range - check wiring    ← Kabel offen oder Kurzschluss
```

---

## OTA-Updates

Skript `scripts/upload_ota.ps1`:
- Liest `OTA_PASSWORD` direkt aus `../_secrets/OtaSecret.h`
- Fragt die Geräte-IP ab (letzter Wert als Vorschlag)
- Startet `pio run -e d1-mini-ota -t upload --upload-port <IP>`

---

## Security

- OTA nur in vertrauenswürdigen Netzen aktivieren
- Keine Secrets ins Repository committen
- `OTA_PASSWORD` immer setzen (`OTA_ALLOW_INSECURE_NO_PASSWORD false`)
- Bei WLAN-Ausfall: automatischer Reconnect mit Backoff + Jitter

---

## Tests und CI

```powershell
pio run -e d1-mini-usb          # Firmware-Build
pio test -e native              # Unit-Tests (benötigt gcc/g++ lokal)
```

CI-Workflow: `.github/workflows/ci.yml`

### Build-Cache leeren

```powershell
Remove-Item -Recurse -Force .pio\build
```

---

## Referenzen

- [Arduino Core für ESP8266](https://github.com/esp8266/Arduino)
- [ArduinoOTA (ESP8266)](https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html)
- [PlatformIO](https://platformio.org)
- [WeMos D1 Mini Pinout](https://www.wemos.cc/en/latest/d1/d1_mini.html)
