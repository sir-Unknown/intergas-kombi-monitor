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

Bouw volgens [GUIDE.md](../../GUIDE.md); de HRE-stap staat in
[Block 1](../../GUIDE.md#block-1-power-boiler-side):

- C1 + **direct aan het U2 OUT-pootje**, niet aan de "boiler 3.3 V"-draad.
- **U2 OUT naar de "boiler 3.3 V"-draad via een losse draadbrug**, aan beide kanten gesoldeerd.
  Laat de meting op de ketel 5 V-signalen zien, dan verplaats je alleen het U2-eind van die
  draadbrug van OUT naar IN. C1 blijft zo op de uitgang van U2 en de print hoeft niet te worden
  omgebouwd.
- **Adapter al gebouwd** zonder draadbrug: niets aanpassen zolang ketel-TX in rust ≈ 3,3 V meet.
  Meet je ≈ 5 V, onderbreek dan de verbinding tussen U2 OUT en de "boiler 3.3 V"-lijn en verbind
  die lijn met U2 IN (+5 V). Zat C1 + op de lijn, zet hem dan direct aan U2 OUT. Volg daarna de
  5 V-regel in [Meten op de ketel](#meten-op-de-ketel-vóór-aansluiten).

Verder ongewijzigd: U1/U3 4N25, Q1/Q2 BC337, R2/R4 220 Ω, R1/R3 1 kΩ, C1 22 µF.

## Bankcontroles

Uitgevoerd volgens [GUIDE.md § Block 5](../../GUIDE.md#block-5-checks-before-power) en
[§ Block 6](../../GUIDE.md#block-6-bench-test-with-power); samengevat:

| Meting | Eis | Als het niet klopt |
|---|---|---|
| Ketel-TX-draad tegen "boiler 3.3 V" → spanning GPIO16 t.o.v. ESP-GND | **≥ 2,5 V** | R2 220 Ω → 150 Ω |
| Losse GPIO17-draad (Q2 B) tegen ESP-3V3 → ketel-RX-draad t.o.v. ketel-GND | **≥ 2,5 V** | R4 220 Ω → 150 Ω |
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
| ≈ 5 V | U2-eind van de draadbrug van OUT naar IN. Dan R1 waarschijnlijk 2,2 kΩ; **op de bank controleren** (Block 6, nu met 5 V op de lijn) dat ketel-RX hoog ≥ 4 V wordt (Schmitt-trigger-ingang van een 5 V-microcontroller vraagt 0,8 × 5 V) voordat je aansluit |
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

## Adapters in community-projecten

Gelezen uit de schema's en code van de repositories (november 2026). Geen van hen meldt het
gemeten signaalniveau van X5.

| Project | Ketel | Ketel-TX → ESP | ESP → ketel-RX | Voeding ketelzijde | ESP-voeding | Controller, pinnen |
|---|---|---|---|---|---|---|
| little-chef / deze repo ([GUIDE.md](../../GUIDE.md), zelfde onderdelen en schema) | — | BC337-emittervolger (C aan 3,3 V) → 220 Ω → 4N25; uitgang 1 kΩ naar GND | idem gespiegeld | LP2950-3.3 uit X5 +5 V | eigen USB | ESP32-WROOM-32 (NodeMCU-32S, of Joy-IT NodeMCU ESP32 USB-C), UART2 GPIO16/17 |
| Meewis (`intergas-esp8266.pdf`) | IC3 | BC337-emittervolger, **collector aan X5 V+** → 220 Ω → 4N25; uitgang 1 kΩ | BC337-emittervolger aan 3,3 V → 220 Ω → 4N25; opto-collector aan **X5 V+**, 1 kΩ naar GND | geen regelaar: X5 V+ direct | LM117 → 3V3 uit een losse "+V"-voeding op ESP-massa; niet verbonden met X5 V+ of X5 GND (voeding zelf niet gespecificeerd) | ESP-01, UART0 |
| Edwin Bontenbal (`InterGas.kicad_sch`, netlijst uitgelezen) | HRE IC2 | **Gemeenschappelijke emitter**: 68 kΩ basis, LED + 220 Ω aan 3,3 V; uitgang 470 Ω | idem gespiegeld | L78L33 uit X5 +5 V; 100 nF / 10 nF | jumpers J4/J5 verbinden GND en +5 V met X5 (README: "run paricitair on the boiler x5 PCB connector (without external power)") | Wemos D1 mini, SoftwareSerial D6 (TX) / D7 (RX) |
| keesma (handschets + Wemos-schema) | Prestige CW6 | **Opto-LED direct uit ketel-TX** via 200–500 Ω; uitgang 1 kΩ | 220–500 Ω → LED; opto-collector aan X5 V+, 1 kΩ naar GND | X5 V+ direct | HiLink 3,3 V | ESP8266, SoftwareSerial GPIO4 (RX) / GPIO5 (TX) |
| So871 | HRE 36/48 A / HReco | niet gedocumenteerd | | | | ESP32, UART2 GPIO16/17 (zelfde als little-chef/GUIDE.md) |
| Pluimvee | HRE 24/18 | geen schema in de repo | | | | Wemos D1 mini; Intergas2MQTT: RX op D7 (UART0 omgezet, alleen ontvangen), TX op D4 (UART1, alleen zenden); ESPHome-versie: GPIO1/3 met logger `baud_rate: 0` |
| rixvet | Prestige CW6 | FTDI TTL-232 5 V, zonder scheiding | | V+ niet aangesloten | USB | Raspberry Pi |

Wat hieruit volgt voor deze adapter:

- **Topologie bevestigd.** Op een HRE IC2 (Edwin) en een IC3 (Meewis) werkt een 4N25 met
  transistor ervoor en een 470 Ω–1 kΩ uitgangsweerstand. Op een CW6 (keesma) werkt de opto zonder
  transistor, rechtstreeks uit ketel-TX. Ketel-TX kan daar dus enkele mA leveren.
- **Ketel-RX hoog = X5 V+ (≈ 5 V) bij Meewis en keesma.** Hun ketels accepteren dat. Dat 3,3 V ook
  als hoog geldt, tonen Edwin (IC2, via opto) en BAS_esp (HReco, ESP direct op X5) aan; voor een
  HRE A met IC3 is het niet gemeld. Daarom blijft de draadbrug uit
  [Voorbereiding bij het solderen](#voorbereiding-bij-het-solderen) nodig.
- **Emittervolger of gemeenschappelijke emitter.** Edwin stuurt de LED met een schakelende
  transistor. Met 68 kΩ is de basisstroom ≈ (3,3 − 0,7) / 68 kΩ ≈ 38 µA. Een LED-stroom van ≈ 9 mA
  vraagt dan h<sub>FE</sub> ≥ 240: dat haalt een BC337-40 wel, maar een BC337 of BC337-16 niet
  zeker. De emittervolger in GUIDE.md hangt niet af van h<sub>FE</sub>.
- **Pinnummering verschilt per bron, de posities niet.** Meewis nummert 1 TX, 2 RX, 3 V+, 4 GND;
  Edwin 1 TX, 2 RX, 3 GND, 4 +5 V. Pluimvee hergebruikt het plaatje van rixvet (PL2303-kleuren:
  wit = "Rx", groen = "Tx", zwart = GND, V+ niet aangesloten). Alle drie passen bij de regel in
  [Pinout](#pinout).
- **Bootruis op ESP8266.** Pluimvee legt TX bewust op D4 (UART1), omdat UART0 met pinswap TX op
  D8 zet en die pin bij het opstarten laag moet blijven. Dit probleem bestaat niet bij een ESP32 op
  UART2.
