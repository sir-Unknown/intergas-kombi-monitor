# Intergas Kombi Kompakt HRE — analyse IDS-ondersteuning

Deze map bevat de analyse of, en hoe, deze repository (geschreven voor de Intergas **Xtreme**)
geschikt gemaakt kan worden voor de Intergas **Kombi Kompakt HRE** (24/18 A, 28/24 A, 36/30 A,
36/48 A). Er is in deze analyse **geen firmwarecode gewijzigd**; alleen documentatie en een
read-only testconfiguratie zijn toegevoegd.

Analysedatum: november 2026.

## Conclusie in het kort

| Onderwerp | Conclusie | Zekerheid |
|---|---|---|
| Huidige firmware op een HRE | **Niet gebruiken.** Ander protocol (checksum), andere commando's, andere byte-indeling, andere parameteradressen; schrijfacties komen op verkeerde parameters terecht. | Hoog (IDS-broncode vs. repo-code) |
| Adapter (GUIDE.md) | Schakeling is bruikbaar, mits de signaalspanning op X5 vooraf gemeten wordt. | Middel; meting nodig |
| Aansluiting | **X5** ("Computer interface") i.p.v. X14; zelfde Molex Mini-Fit Jr 2×2. | Hoog |
| Protocol | IDS-protocol **zonder checksum**, 9600 8N1, 32-byte antwoorden. Volledig vastgesteld uit de officiële Intergas Diagnostic Software (IDS) v1.83. | Hoog |
| Parameters | Alle installateursparameters uit de handleiding staan in `V?` (0–30) en `V1` (32–63), met namen uit IDS. | Hoog |
| Schrijven | `P` + positie + waarde (3 bytes, geen antwoord). | Hoog (IDS) / middel (praktijk: één gebruiker) |
| Werk | Gedeelde transportlaag + HRE-decoders; Xtreme-decoders voor `S2` en (IC3) `HN` zijn herbruikbaar. | — |

## Documenten

| Bestand | Inhoud |
|---|---|
| [protocol.md](./protocol.md) | Volledige protocolbeschrijving: commando's, byte-indeling, parameters, bedrijfscodes, gevaarlijke commando's |
| [hardware.md](./hardware.md) | X5-aansluiting, pinout, signaalniveaus, metingen vóór aansluiten, adapterberekening |
| [verschillen-xtreme.md](./verschillen-xtreme.md) | Concrete afwijkingen tussen de huidige code en de HRE, met regelverwijzingen |
| [onzekerheden.md](./onzekerheden.md) | Alles wat niet (volledig) vaststaat, met hoe het te verifiëren is |
| [implementatieplan.md](./implementatieplan.md) | Werkpakketten met acceptatiecriteria en verificatieprocedure op de ketel |
| [bronnen.md](./bronnen.md) | Alle gebruikte bronnen, wat eruit gehaald is, betrouwbaarheid, checksums en reproductie |
| [intergas-hre-probe.yaml](./intergas-hre-probe.yaml) | Read-only ESPHome-testconfiguratie voor de eerste verbinding |

## Volgorde voor de gebruiker

1. Adapter solderen volgens [GUIDE.md](../../GUIDE.md), met de voorbereiding uit
   [hardware.md § Voorbereiding bij het solderen](./hardware.md#voorbereiding-bij-het-solderen).
2. Bankcontroles uit [hardware.md](./hardware.md#bankcontroles).
3. X5 doormeten op de ketel ([hardware.md § Meten op de ketel](./hardware.md#meten-op-de-ketel-vóór-aansluiten)).
4. [intergas-hre-probe.yaml](./intergas-hre-probe.yaml) flashen en één volledige ronde loggen.
5. Log vergelijken met het keteldisplay volgens
   [implementatieplan.md § Verificatie op de ketel](./implementatieplan.md#verificatie-op-de-ketel).

## Wat niet in deze map staat

De gedecompileerde IDS-code zelf wordt **niet** in de repository opgenomen (auteursrecht Intergas).
[bronnen.md](./bronnen.md#reproductie) beschrijft hoe de analyse te reproduceren is vanaf de
officiële download.
