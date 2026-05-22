# Veroboard-Lötplan (alle Pins verlötet) – basierend auf deinen Fotos

## Ziel

Alle Pins der vorhandenen Module werden auf der Streifenrasterplatine verlötet.  
Benutzte Signale werden direkt verdrahtet, unbenutzte Pins werden auf **Service-Pads** herausgeführt (damit nichts „in der Luft“ hängt und du später messen/erweitern kannst).

## Verwendete Module (aus Fotos)

- D1 Mini kompatibles ESP8266-Board (A-Delivery, 16 Pins)
- ADS1115 Breakout (`VDD,GND,SCL,SDA,ADDR,ALRT,A0,A1,A2,A3`)
- XL6009 Boost-Converter (`IN+`, `IN-`, `OUT+`, `OUT-`)
- 4-20 mA -> Spannung Wandler mit:
  - Versorgungs-/Ausgangsblock: `VCC`, `VOUT`, `GND`
  - Eingang: `I+`, `I-`
  - Jumperblock `J1` (4 Pins, Bereichsauswahl)

## Montageprinzip auf Streifenraster

- Leiterbahnen horizontal nutzen.
- Alle Module mit Stiftleisten einlöten.
- Zwischen gegenüberliegenden Pinreihen (z. B. D1 Mini links/rechts) **immer Trennschnitt** setzen, damit keine Kurzschlüsse über den durchgehenden Streifen entstehen.
- Verbindungen nur über Drahtbrücken/Lötdraht (wie von dir geplant).

## Pflicht-Netze (funktional)

- `5V_BUS`: D1 Mini `5V` -> Boost `IN+`
- `GND_BUS`: D1 Mini `G` + ADS `GND` + Boost `IN-` + Boost `OUT-` + Wandler `GND` + Wandler `I-`
- `12V_BUS`: Boost `OUT+` -> Sensor `Braun(+)` -> Wandler `VCC`
- `LOOP_IN`: Sensor `Blau(-)` -> Wandler `I+`
- `ANALOG_IN`: Wandler `VOUT` -> ADS `A0`
- `I2C_SCL`: D1 Mini `D1` -> ADS `SCL`
- `I2C_SDA`: D1 Mini `D2` -> ADS `SDA`
- `3V3_BUS`: D1 Mini `3V3` -> ADS `VDD`
- `ADDR_GND`: ADS `ADDR` -> `GND_BUS`

## Alle Pins verlöten – Pinliste

### D1 Mini (16 Pins)

- In Funktion: `5V`, `G`, `3V3`, `D1`, `D2`
- Als Service-Pad herausführen: `RST`, `A0`, `D0`, `D3`, `D4`, `D5`, `D6`, `D7`, `D8`, `TX`, `RX`

### ADS1115 (10 Pins)

- In Funktion: `VDD`, `GND`, `SCL`, `SDA`, `ADDR`, `A0`
- Als Service-Pad herausführen: `ALRT`, `A1`, `A2`, `A3`

### XL6009 (4 Pads)

- In Funktion: `IN+`, `IN-`, `OUT+`, `OUT-` (alle 4 aktiv genutzt)

### 4-20 mA Wandler

- In Funktion: `VCC`, `VOUT`, `GND`, `I+`, `I-`
- `J1` (4 Pins) ebenfalls verlöten; als steckbarer Jumper belassen:
  - Für 0–3.3 V Betrieb typischerweise Brückenpaar gemäß Modulaufdruck/Doku setzen.
  - Wegen Varianten bitte vor dem Einschalten per Aufdruck verifizieren.

## Leiterbahn-Trennungen (Cut-Liste, verbindlich)

1. Unter jeder D1-Mini-Pinreihe je Pin ein Trennschnitt zwischen linker und rechter Pinseite.
2. Unter ADS1115 ebenfalls Trennschnitte setzen, damit keine Pin-Paare über denselben Streifen gekoppelt sind.
3. Zwischen `12V_BUS` und allen Niederspannungs-/Signalleitungen (I2C/Analog) mindestens 1 freie Streifenreihe als Abstand lassen.
4. Im Bereich Wandler/Boost zusätzliche Cuts um `VOUT` und `I+` setzen, damit Analogpfad nicht mit Versorgung zusammenläuft.

Praxisregel: Jeder Pin, der auf ein anderes Netz geht, bekommt einen eigenen isolierten Streifenabschnitt.

## Drahtbrücken-Liste (Lötdraht)

1. `D1(ESP)` -> `SCL(ADS)`
2. `D2(ESP)` -> `SDA(ADS)`
3. `3V3(ESP)` -> `VDD(ADS)`
4. `ADDR(ADS)` -> `GND_BUS`
5. `A0(ADS)` -> `VOUT(Wandler)`
6. `5V(ESP)` -> `IN+(Boost)`
7. `G(ESP)` -> `IN-(Boost)`
8. `OUT-(Boost)` -> `GND_BUS`
9. `OUT+(Boost)` -> `12V_BUS`
10. `12V_BUS` -> `VCC(Wandler)`
11. `I-(Wandler)` -> `GND_BUS`
12. `I+(Wandler)` -> Sensor `Blau(-)`
13. Sensor `Braun(+)` -> `12V_BUS`

## Service-Pads (empfohlen, damit wirklich alle Pins nutzbar bleiben)

Lege am rechten Platinenrand eine 1x16 Serviceleiste an:

- `RST, A0, D0, D3, D4, D5, D6, D7, D8, TX, RX, ALRT, A1, A2, A3, GND`

So sind alle ungenutzten Pins sauber verlötet und später erreichbar.

## Prüfablauf vor erstem Einschalten

1. Alle Cuts mit Durchgangsprüfer prüfen (getrennte Streifen wirklich offen).
2. `12V_BUS` gegen `GND_BUS` auf Kurzschluss prüfen.
3. Boost ohne Last auf **12.0 V** einstellen.
4. ADS-Versorgung prüfen: `VDD` gegen `GND` = ca. 3.3 V.
5. Wandlerausgang bei 4 mA/20 mA prüfen und auf 0..3.3 V trimmen.

## Wichtige Anmerkung zu J1 (4-20 mA Wandler)

Dein Modultyp hat je nach Hersteller unterschiedliche J1-Tabellen.  
Bitte die Stellung direkt nach Modulaufdruck einstellen (0-3.3-V-Bereich), erst dann ESP/ADS verbinden.
