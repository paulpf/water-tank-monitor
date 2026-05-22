# Lötplan Streifenraster 75 x 100 mm (2,54 mm)

## 1) Machbarkeit

Ja, das ist **grundsätzlich möglich** auf 75 x 100 mm Hartpapier-Streifenraster (einseitig, 39 Leiterbahnen), mit den im Projekt verwendeten Modulen:

- WeMos D1 Mini
- ADS1115 Breakout
- 4-20 mA Empfänger (I+/I-/Vout/GND)
- Boost-Converter 5V -> 12V
- Schraubklemmen für Sensoranschluss

Wichtig: Bei sehr großen Boost-/Empfänger-Modulen kann es eng werden. Der hier definierte Plan geht von typischen kleinen LM2596-/Mini360-Modulen und einem üblichen 4-20 mA Receiver-Modul aus.

## 2) Raster- und Koordinatensystem

- Lochraster: 2,54 mm
- Nutzfeld: 39 Reihen (A bis AM), 30 Spalten (1 bis 30) als Planungsraster
- Leiterbahnen verlaufen **horizontal je Reihe** (typische Streifenraster-Platine)
- Koordinate: `Reihe/Spalte` (z. B. `H/12`)

## 3) Platzierung (Top-View, Bauteilseite)

- D1 Mini (Buchse nach außen erreichbar): Reihen `F..N`, Spalten `20..28`
- ADS1115: Reihen `F..K`, Spalten `13..18`
- 4-20 mA Empfänger: Reihen `P..W`, Spalten `10..19`
- Boost-Converter: Reihen `Y..AH`, Spalten `4..13`
- Sensor-Klemme 2-polig (Braun/Blau): Reihen `Y..AA`, Spalten `22..24`
- GND-/Service-Klemme 2-polig (optional): Reihen `AC..AE`, Spalten `22..24`

## 4) Track-Cuts (Leiterbahnunterbrechungen)

Die folgenden Unterbrechungen trennen Signale und verhindern ungewollte Brücken:

- `H/19`, `J/19`, `L/19` (Trennung I2C/Signalbereich vs D1-Mini-Seite)
- `R/9`, `T/9`, `V/9` (Trennung Empfänger-Eingangsseite)
- `R/20`, `T/20`, `V/20` (Trennung Empfänger-Ausgangsseite)
- `AA/14`, `AC/14`, `AE/14` (Trennung Boost-Bereich)

Hinweis: Nach jedem Cut mit Multimeter Durchgang prüfen.

## 5) Netze / Verdrahtung

### Versorgung und Masse

- `D1 Mini 5V` -> `Boost IN+`
- `D1 Mini GND` -> `Boost IN-`
- `Boost OUT+ (12V)` -> `Sensor + (Braun)`
- `Sensor - (Blau)` -> `Empfänger I+`
- `Empfänger I-` -> `GND-Schiene`
- `ADS1115 VDD` -> `D1 Mini 3V3`
- `ADS1115 GND` -> `GND-Schiene`

### Messsignal und I2C

- `Empfänger Vout` -> `ADS1115 A0`
- `ADS1115 SCL` -> `D1 Mini D1`
- `ADS1115 SDA` -> `D1 Mini D2`
- `ADS1115 ADDR` -> `GND` (Adresse 0x48)

## 6) Empfohlene Drahtfarben

- 12V: Rot
- 5V: Orange
- 3V3: Rot/Weiß markiert
- GND: Schwarz
- I2C SCL/SDA: Gelb/Grün
- 4-20 mA Loop: Blau
- 0-3,3V Analog: Violett

## 7) Löt-Reihenfolge (professionell, risikoarm)

1. Bauteile trocken platzieren und USB-/Klemmen-Zugänglichkeit prüfen.
2. Alle Track-Cuts setzen und elektrisch prüfen.
3. Niedrige Bauteile und Drahtbrücken löten.
4. Sockel/Pinleisten für D1 Mini und ADS1115 löten.
5. Boost-Converter und Empfänger mechanisch stabil fixieren (Abstandshalter/Kleber).
6. Versorgungsnetze löten (5V, 3V3, GND, 12V).
7. Signalnetze löten (I2C, Vout, Sensor-Loop).
8. Sichtprüfung unter Lupe: Lötbrücken, kalte Lötstellen, Polarität.
9. Erstinbetriebnahme mit Labornetzteil/Strombegrenzung oder USB mit Sicherung.

## 8) Inbetriebnahme-Checkliste

- Boost vor Anschluss auf exakt 12,0 V einstellen.
- Zwischen 12V und GND auf Kurzschluss prüfen.
- D1 Mini alleine starten (ohne Sensor), dann ADS1115 erkennen.
- Empfänger ZERO/SPAN gemäß README kalibrieren:
  - 4 mA -> 0,0 V
  - 20 mA -> 3,3 V

## 9) Hinweise zur Sicherheit/EMV

- Sensorleitung (4-20 mA) möglichst verdrillt führen.
- Masse sternförmig in einen zentralen GND-Knoten führen.
- 100 nF + 10 uF nahe ADS1115 VDD/GND ergänzen.
- Bei langen Sensorleitungen optional TVS-Diode am Eingang des Empfängers.

## 10) Fotobasierte Modulzuordnung (validiert)

Die Fotos unter `docs/hardware` passen zu folgenden Modulen und Pinbezeichnungen:

- `20260521_123843.jpg`: D1-Mini-kompatibles ESP8266-Board (A-Delivery, ESP8266MOD-12-F)
- `20260521_123857.jpg`: ADS1115-Breakout, I2C-Header links (`VDD`, `GND`, `SCL`, `SDA`, `ADDR`, `ALRT`) und Analogpins `A0..A3`
- `20260521_123901.jpg`: 4-20-mA-Empfänger mit Trimmern (`ZERO`, `SPAN`) und Klemmen für `VCC`, `VOUT`, `GND` sowie `I+`, `I-`
- `20260521_123853.jpg`: XL6009-Boost-Converter mit `IN+`, `IN-`, `OUT+`, `OUT-`

Hinweis: Damit ist der hier dokumentierte Aufbau konsistent zu deinem realen Modul-Set.

## 11) Importierbare Dateien und Nutzung

Bereits vorhanden und direkt nutzbar:

- `docs/veroboard_75x100_layout.svg`: visuelles Top-View-Layout für den Aufbau
- `docs/veroboard_75x100_nets.csv`: Netzliste für Dokumentation oder Import in eigene Tools
- `docs/wiring.drawio`: Verdrahtungsdiagramm in draw.io

Empfohlener Workflow:

1. `wiring.drawio` für die elektrische Kontrolle öffnen.
2. `veroboard_75x100_layout.svg` als mechanische Aufbauvorlage nutzen.
3. `veroboard_75x100_nets.csv` als Checkliste beim Verdrahten abarbeiten.

## 12) Loch-für-Loch-Dateien (neu)

Für den praktischen Aufbau sind zusätzlich zwei Listen enthalten:

- `docs/veroboard_75x100_wiring_steps.csv`: Schrittfolge mit Start-/Zielkoordinate je Draht
- `docs/veroboard_75x100_trackcuts.csv`: separate Track-Cut-Prüfliste zum Abhaken

Koordinaten-Präfixe in der Verdrahtungsliste:

- `D1`: D1-Mini-Bereich
- `ADS`: ADS1115-Bereich
- `RCV`: 4-20-mA-Empfängerbereich
- `BOOST`: XL6009-Bereich
- `SENS`: Sensor-Klemmenbereich
- `SVK`: optionale Service-Klemme
- `GND_BUS`: gemeinsame Masse-Sammelschiene

Empfohlene Ausführung:

1. Track-Cuts vollständig setzen und mit Multimeter prüfen.
2. GND-Bus aufbauen.
3. Versorgung (5V/3V3/12V) verdrahten.
4. Signalpfade (I2C, A0, 4-20 mA Loop) verdrahten.
5. Schrittliste gegenmessen und jede Position abhaken.
