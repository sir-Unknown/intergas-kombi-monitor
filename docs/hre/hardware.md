# Hardware: aansluiten op de HRE (X5)

De adapter uit [GUIDE.md](../../GUIDE.md) (2× 4N25, 2× BC337, LP2950-3.3, ketelzijde gevoed uit de
ketel, ESP gevoed via eigen USB-lader) is elektrisch geschikt voor de HRE, op voorwaarde dat de
signaalspanning van X5 vooraf gemeten wordt. Dit document beschrijft wat afwijkt van de
Xtreme-situatie en wat vóór het aansluiten gecontroleerd moet worden.

## Aansluiting X5

| Eigenschap | Waarde | Bron |
|---|---|---|
| Naam | X5, "Computer interface" | HRE-installatievoorschrift §10.2 (p. 78) |
| Connector | Molex Mini-Fit Jr 2×2 (zelfde als ATX P4, zelfde als X14 op de Xtreme) | rixvet README; Edwin Bontenbal (kabelschema "Molex 5557/5559"); Circuitsonline (NikNeem 2010) |
| Bereikbaarheid | Branderautomaat naar voren trekken; deze kantelt naar beneden | Installatievoorschrift §5.3 stap 1–5 |
| Gevaar | Op de branderautomaat zit **230 V** (connector X2) | Installatievoorschrift §10.2 |

De bij de Xtreme gekochte Molex-behuizing (39-01-2040) en contacten passen dus ook.

## Pinout

Drie onafhankelijke bronnen geven dezelfde indeling:

| Bron | Linksboven | Rechtsboven | Linksonder | Rechtsonder |
|---|---|---|---|---|
| Repo [pinout-X14.png](../../images/pinout-X14.png) (Xtreme X14, vergrendeling links, draadzijde) | +5 V | ketel-TX | GND | ketel-RX |
| [Edwin Bontenbal, IntergasCable.png](https://github.com/EdwinBontenbal/InterGasGateway/blob/main/Schematics/IntergasCable.png) (HRE X5; pin 1 TX, 2 RX, 3 GND, 4 +5V; foto met vergrendeling links) | 4 = +5 V | 1 = TX | 3 = GND | 2 = RX |
| [rixvet, connector-layout](https://github.com/rixvet/IntergasBoilerReader/blob/master/intergas-cw6-connector-layout.png) (Prestige CW6 X5; labels vanuit PC gezien) | V+ | "Rx" (= ketel-TX) | Gnd | "Tx" (= ketel-RX) |

Regel die onafhankelijk is van spiegeling of kijkrichting:

- **V+ en GND zitten in de kolom aan de vergrendelzijde**, TX en RX in de andere kolom.
- **Ketel-TX zit in dezelfde rij als V+**, ketel-RX in dezelfde rij als GND.

Waarschuwing van Meewis (Circuitsonline, 11-12-2021): "bij de manual van Intergas zijn TX en RX
omgedraaid". Labels "TX/RX" zijn dus niet eenduidig (ketel- of PC-perspectief). **Meten gaat vóór
plaatjes.**

## Signaalniveaus

| Waarneming | Bron | Wat het zegt |
|---|---|---|
| IDS rekent de referentiespanning van IC2 met ×5/1024 en van IC3 met ×3,3/1024 | IDS v1.83 `get_bc_crc` vs. `get_bc_crc_ic3` | IC2-processor 5 V, IC3-processor 3,3 V (**afgeleid**; X5 kan gebufferd zijn) |
| IC2-HRE 28/24 (`ic2kknl01`) werkt met een FTDI TTL-232R-**5V**-kabel | snotneus (CO, 7-1-2022) | IC2-signalen verdragen 5 V |
| HReco: ESP rechtstreeks op X5, ESP gevoed uit de "3,3 V van de X5-stekker" | BAS_esp (CO, juni en december 2025) | Op (vermoedelijk) IC3 zijn de signalen 3,3 V-compatibel |
| HRE-adapter met LP2950 → 3,3 V aan ketelzijde, 4N25 + BC337 | Edwin Bontenbal (IC2), acfnews (HRE) | Zelfde topologie als GUIDE.md werkt op HRE |
| "Op de onderste 2 pootjes 5 V en GND; op de andere twee ook 5 V met wat rommel" | melkman (CO, 2010, oudere HR) | Oude branderautomaten: 5 V-signalen |
| Branderautomaat bevat PIC18F4520, aanstuurbaar met MAX232 | Martin-B (CO, 2010) | 5 V-microcontroller op oude modellen |
| MAX3232-module (RS-232-niveaus) gaf geen communicatie; PL2303 TTL wel | BAS_esp (CO, juni 2025) | X5 is TTL, geen RS-232 |
| Zeer oude Kompakt HR (Furimat): 2400 baud 7E2, reageert niet op `S?` | EmielA (CO, januari 2022) | Niet relevant voor HRE A; wel een reden waarom een oude ketel niet antwoordt |

Het installatievoorschrift (versie 2025) beschrijft de HRE ...A-modellen met modulerende A-pomp.
Intergas levert IDS voor IC3 vanaf toestellen van 1 januari 2013 (Intergas IDS-productpagina).
Een HRE A heeft daarom vrijwel zeker een IC3 (**afgeleid**; te bevestigen met `REV`).

## Voorbereiding bij het solderen

Bouw volgens [GUIDE.md](../../GUIDE.md) met één toevoeging:

- Verbind **U2 OUT met de "boiler 3.3 V"-lijn via een losse draadbrug** (niet direct solderen). Als
  de meting op de ketel 5 V-signalen laat zien, kan de ketelzijde naar +5 V worden omgezet zonder
  print te slopen.

Verder ongewijzigd: U1/U3 4N25, Q1/Q2 BC337, R2/R4 220 Ω, R1/R3 1 kΩ, C1 22 µF.

## Bankcontroles

Aanvullend op [GUIDE.md § Block 5](../../GUIDE.md#block-5-checks-before-power) en
[§ Software, step 1](../../GUIDE.md#software-step-1-a-read-only-test).

| Meting | Eis | Als het niet klopt |
|---|---|---|
| Ketel-TX-draad tegen U2 OUT → spanning GPIO16 t.o.v. ESP-GND | **≥ 2,5 V** | R2 220 Ω → 150 Ω |
| Ketel-RX-draad t.o.v. ketel-GND, ESP in rust (GPIO17 hoog) | ≈ 3,3 V | Q2, R4, U3 controleren |
| Doorpiepen ketel-GND ↔ ESP-GND | geen piep | soldeerbrug zoeken |

### Waarom de eerste meting nodig is

| Grootheid | Waarde | Bron |
|---|---|---|
| ESP32 V<sub>IH</sub> | min. 0,75 × VDD = 2,475 V | Espressif ESP32-datasheet, tabel 5-3 |
| 4N25 CTR | min. 20 %, typisch 50 % (bij I<sub>F</sub> = 10 mA) | Vishay 4N25-datasheet |
| LED-stroom U1 | Q1 als emittervolger aan 3,3 V: V<sub>E</sub> ≈ 2,6 V → (2,6 − 1,2) / 220 Ω ≈ 6,4 mA | berekening |
| Uitgangsstroom U1 | 6,4 mA × 20 % ≈ 1,3 mA (minimum) tot ≈ 3,2 mA (typisch) | berekening |
| Spanning over R3 (1 kΩ) | 1,3 V (minimum CTR) tot verzadiging ≈ 3,3 V (typisch) | berekening |

Bij een exemplaar met lage CTR haalt GPIO16 de hoog-drempel dus niet. Met R2 = 150 Ω wordt de
LED-stroom ≈ 9,3 mA (ruim onder 60 mA absoluut maximum). Hetzelfde geldt in de andere richting
(U3 → ketel-RX); de bankmeting van de ketel-RX-draad dekt die.

## Meten op de ketel (vóór aansluiten)

1. Stekker van de ketel uit de wandcontactdoos; branderautomaat naar voren kantelen (§5.3).
2. Ketel weer onder spanning. **Alleen op X5 meten**; X2 voert 230 V.
3. Zoek **GND**: de pin waartegen de andere pinnen een positieve spanning hebben.
4. De andere pin in dezelfde kolom is **V+** (verwacht ≈ 5 V).
5. In de andere kolom: **TX** zit in dezelfde rij als V+, **RX** in dezelfde rij als GND.
6. Meet TX in rust (ketel zendt alleen op verzoek, dus TX is in rust hoog):

| TX in rust | Actie |
|---|---|
| ≈ 3,3 V | Adapter zoals gebouwd (draadbrug op U2 OUT) |
| ≈ 5 V | Draadbrug naar +5 V. Dan R1 waarschijnlijk 2,2 kΩ; **op de bank controleren** dat ketel-RX hoog ≥ 4 V wordt (Schmitt-trigger-ingang van een 5 V-microcontroller vraagt 0,8 × 5 V) voordat je aansluit |
| iets anders / onstabiel | Niet aansluiten; eerst uitzoeken |

7. Noteer het productiejaar van het typeplaatje (`******-jjmm`, installatievoorschrift p. 3).

## Voeding

- **ESP nooit uit X5 voeden.** acfnews (CO, 26-8-2023): ESP + opto's uit de 5 V van de HRE →
  ketel in storing 2 (= "Verwisseling S1 en S2", vermoedelijk door spanningsdip op de
  sensormeting). De ketelzijde van de adapter trekt alleen de LED-stroom van U1 (≈ 6–9 mA) plus de
  uitgangsstroom van U3 en ≈ 75 µA ruststroom van de LP2950 (TI-datasheet).
- Het toelaatbare stroombudget van X5 is nergens gedocumenteerd.

## ESP-specifieke aandachtspunten

- rbohm (CO, augustus 2022) gebruikte bij de ESP8266 een UART-pinswap zodat bootlogging nooit naar
  de ketel gaat, en zag dat een ESP8266 die rechtstreeks aan de ketel hangt bij het opstarten
  geblokkeerd wordt. Op een ESP32 loopt de bootlog via UART0 (GPIO1/3), niet via UART2
  (GPIO16/17) (**afgeleid**, niet getest).
- Meewis plaatste een BC337 als emittervolger tussen ESP8266-TX en de opto om opstartproblemen te
  voorkomen; GUIDE.md heeft die transistor al (Q2).
