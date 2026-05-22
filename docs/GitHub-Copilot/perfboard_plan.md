# Streifenraster-Lötplan, vorläufig

## Einschätzung

Ja, das Projekt passt grundsätzlich auf eine Streifenrasterplatine mit 75 x 100 mm und 39 Leiterbahnen, wenn du die Standard-Module verwendest und den Aufbau kompakt entlang der langen Seite anordnest.

Die kritischen Punkte sind nicht die Elektronik selbst, sondern die mechanischen Details:

- Größe der konkreten Module von Boost-Converter, 4-20-mA-Wandler und ADS1115-Breakout
- Höhe der aufgelöteten Stiftleisten
- Platz für Schraubklemmen oder Kabelführungen
- USB-Stecker des D1 Mini am Platinrand

Wenn deine Module den üblichen Mini-Formfaktor haben, ist das realistisch. Mit großen Klemmen oder sehr breiten Breakout-Boards wird es eng.

## Zielbild

- D1 Mini an einer Schmalseite, damit der Micro-USB-Port frei bleibt
- ADS1115 direkt neben dem D1 Mini, damit I2C kurz bleibt
- 4-20-mA-Wandler in der Mitte, nahe am Sensor-Eingang
- Boost-Converter an der Versorgungseingangsseite
- Eine gemeinsame GND-Schiene über die gesamte Platine

## Empfohlene Anordnung

Board-Ausrichtung:

- 100 mm in Längsrichtung
- 75 mm in Querrichtung
- Leiterbahnen quer zur Längsrichtung nutzen, damit du Versorgungs- und Signalleitungen sauber trennen kannst

Von links nach rechts:

1. USB / 5-V-Einspeisung
2. Boost-Converter 5 V -> 12 V
3. 4-20-mA-Wandler
4. ADS1115
5. D1 Mini

## Verdrahtungsnetz

| Netz | Verbindung |
|---|---|
| 5V | D1 Mini 5V -> Boost IN+ |
| GND | D1 Mini GND, Boost IN-, Boost OUT-, 4-20-mA-Wandler GND, ADS1115 GND, ADDR nach GND |
| 12V | Boost OUT+ -> TL-136 braun (+) |
| Loop | TL-136 blau (-) -> I+ des 4-20-mA-Wandlers |
| Analog | Vout des Wandlers -> ADS1115 A0 |
| I2C SCL | ADS1115 SCL -> D1 Mini D1 |
| I2C SDA | ADS1115 SDA -> D1 Mini D2 |
| 3V3 | ADS1115 VDD -> D1 Mini 3V3 |

## Praktischer Lötaufbau

### 1. Leiterbahnen vorbereiten

- Unter jedem Modul die nicht benötigten Kupferstreifen trennen.
- Vor dem Einlöten die Cuts mit Durchgangsprüfer kontrollieren.
- Die 5-V- und 12-V-Bereiche räumlich trennen.

### 2. Erst die Versorgung

- USB-5V auf die Platine führen.
- Gemeinsame GND-Schiene aufbauen.
- Boost-Converter einlöten und zuerst ohne Last auf 12 V einstellen.

### 3. Dann das Messsignal

- 4-20-mA-Wandler nahe am Sensor-Eingang platzieren.
- Ausgang des Wandlers direkt an ADS1115 A0 führen.
- ADS1115 mit 3V3 versorgen, ADDR fest auf GND legen.

### 4. Zum Schluss den ESP8266

- D1 Mini an den Rand setzen.
- D1/D2 für I2C kurz und parallel führen.
- USB-Port frei halten, damit Programmierung und OTA-Notbetrieb möglich bleiben.

## Empfehlung für die Montage

- Stiftleisten vor dem endgültigen Einlöten trocken auf der Platine ausrichten.
- Erst die mechanisch größten Teile setzen, dann die kleineren Breakouts.
- Wenn möglich, die Module mit Buchsenleisten statt direkt mit Draht zu verbinden. Das erleichtert Tausch und Fehlersuche.

## Plausibilitätscheck vor dem finalen Löten

1. Liegt der D1 Mini am Rand so, dass das USB-Kabel passt?
2. Bleiben zwischen 12-V-Teil und ADS1115 ausreichend Abstände?
3. Ist die GND-Verbindung sternförmig oder zumindest niederimpedant?
4. Sind alle ADC-Eingänge garantiert nur im Bereich 0 bis 3,3 V?
5. Ist der Boost-Converter vor dem Anschluss auf 12 V eingestellt?

## Status mit den vorhandenen Modulfotos

Die Fotos in `docs/hardware` sind ausreichend, um die verwendeten Module eindeutig zuzuordnen:

- `20260521_123853.jpg`: Boost-Converter mit XL6009E1
- `20260521_123901.jpg`: 4-20-mA-Empfänger (VCC/VOUT/GND + I+/I-)
- `20260521_123857.jpg`: ADS1115-Breakout (VDD/GND/SCL/SDA/ADDR/ALRT/A0..A3)
- `20260521_123843.jpg`: D1-Mini-kompatibles ESP8266-Board (A-Delivery)

Damit ist ein belastbarer Lochraster-Aufbau möglich.

Wenn du als letzten Schritt noch ein Foto mit Lineal neben jedem Modul ergänzt, kann der Plan zusätzlich auf Millimetermaß (statt nur Rasterkoordinaten) verifiziert werden.
