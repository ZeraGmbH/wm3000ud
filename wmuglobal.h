// globale definition

// version V1.02 ab d. 07.12.2010
// das calculate modell wurde grundlegend verändert. 
// alle korrekturwerte haben jetzt stützstellen (vielleicht auch nur eine)
// korrekturwerte werden immer aus den koeffizienten über den erforderlichen parameter berechnet
// korrekturkoeffizienten werden im server aus den abgelegten stützstellen ermittelt.
// vorteile: clients (auch justage clients) brauchen nichts zu berechnen
//                 es lassen sich so allgmein gültige justage algorithmen besser imlementieren
// version V1.03 ab d. 11.04.2011
// die evt bereiche für kanal N wurden herausgenommen, weil sie in zukunft
// hardwaremäßig nicht mehr existieren, in dem zuge fiel auf dass noch ein
// fehler beim lesen der justierwerte auftrat, wenn mehr daten, als bereiche 
// vorhanden, gelesen werden mussten.
// version V1.04 ab d. 01.05.2011 fehler bei justagedaten lesen wenn flash
// jungfräulich "FFFF" länge = -1 beseitigt
// version V1.05 ab d. 07.07.2011 fehler beim schreiben der seriennummer beseitigt (länge war 0)
// version V1.06 ab d. 30.01.2012 wie bei pcb-server wm3000id wird auch hier ein kommando zum
// schutz der sensorik eingeführt. dazu wird ein kommando an den atmel controler gesendet, welcher
// den eingang von der widerstandsteilerkette abkoppelt
// es gibt jetzt detailiertere fehlermeldungen wenn im xml justage import file fehler auftreten auf der schnittstelle
// beim lesen der justagedaten aus dem flash werden auf der hinteren stelle der versionsnummern für lca und ctrl änderungen zugelassen ohne dass es zu einer meldung nicht justiert oder so führt.
// version V1.07 ab dem 24.04.2012 doctype beim schreiben xml datei korrigiert

// version vorn erhöht zur unterscheidung alte/neue cpu
// version v2.08 einbau einer synchronisation auf atmel toggle bit. hierzu wird das zFPGA1reg device verwendet. auf register mit
// der adresse 0xfff wird in bit 0 die auswertung des atmel toggle bit gesetzt. es wird beim start des pcb servers 10sek.
// gewartet ob der atmel ins leben kommt. danach wird durchgestartet.

// V2.09 es wird beim start nachgeschaut ob die datei atmelFlashfilePath existiert,  wenn ja muss der
// atmel neu programmiert werden. hierzu wird zunächst ein reset durchgeführt, der bootloader innerhalb
// 100ms angesprochen, dadurch startet dieser nicht in die applikation durch und anschliessend das
// intel hexfile gelesen und in den atmel geschrieben. danach wir die apllikation gestartet und gewartet
// dass der atmel läuft. wenn ja, dann wird das file gelöscht.

// V2.10 es wurde die berechnung der phasenkorrektur notwendig durch die sample frequenz änderung
// des sigma delta wandlers angepasst
// V2.11 02.03.2017 kommando zum setzen des sense modus lässt jetzt 4 (dc offset justage) zu
// V2.12 11.07.2018 die amplituden korrektur des AD-Wandlers erfolgt jetzt frequenzabhängig und die änderung soll abwärtskompatibel sein
// V2.13 01.04.2019
// 1) das initialisieren der justagedaten findet nur noch 1x beim start statt. danach wird versucht die daten zu lesen.
// alle justagedaten werden übernommen, wenn seriennummer und versionsnummern stimmen oder der justagestecker steckt.
// fälschlicher weise wurde beim schreiben der daten auch immer initialisiert...
// damit werden korrekturen aber rückgängig gemacht...fehler sollte jetzt behoben sein.
// 2) nach dem schreiben von justagedaten werden die justagedaten nochmals gelesen. wenn beim lesen ein fehler auftritt wird dieser als
// antwort auf das schreibkommando gesendet.
// 3) beim initialisieren der justagewerte werden die korrekturwerte für adw's gesetzt und berechnet.
// Und es werden die status dafür eingetragen.
// V2.14 01.10.2020
// bugfix beim initialisieren der justagedaten phasenkorrektur ist default 0.0 und nicht 1.0 !!!
// V2.15 16.10.2020 wir stellen wieder um auf justage der phasenwinkel für die ad-wandler
// V2.16 07.07.2021 es kommen neue sampleraten hinzu. es müssen in folge korrekturkurven für gain hinzugefügt werden.

// allgemein zu justagedaten .......
// im allerersten ansatz wurden für die wm3000 ein korrekturwert für gain und offset sowie ein polygon
// 3. ordnung für die phasenkorrektur verwendet. allgemein kann man sagen, bei gain und offset handelt es
// sich um ein polygon 0. ordnung. für polygone aber braucht es koeffizienten, die wiederum aus stützstellen
// berechnet werden müssen. im ersten ansatz wurde das aber leider noch nicht so allgemeingültig implementiert.
// weil wie bereits gesagt für gain und offset nur ein korrektur wert gespeichert wurde.
// im zuge der laufenden erfahrungen mit der wm3000 hat sich gezeigt dass es für die gain korrekturen ein
// polygon 1. ordnung braucht. das war der zeitpunkt zum umstieg auf diesen eben genannten allgemeingültigeren
// ansatz. um kompatibilität mit bereits ausgelieferten geräten zu gewährleisten wurden oder werden die
// "alten" daten in "neue" daten konvertiert.
// nochmal
// 1. ansatz(ohne versionsnr im flash) gainorder=0 (aber nur korrekturwert), offsetorder=0 (dito), phaseorder=3 (incl. stützstellen)

// 2. ansatz(ab versionsnr v1.02) gainorder=1 (incl. stützstellen), offsetorder=0 (incl....) phaseorder=3 (incl....)
// jahre später zeigte sich dass in abhängigkeit vom verhältnis der samplerate der ad wandler zu der signalfrequenz
// amplitudenfehler hinzukamen. dies resultiert aus der 3. filterstufe der sigmadelta adcs.

// 3. ansatz(ab versionnr v2.12) es musste daher eine gain korrektur 3. ordnung für die adc's eingeführt werden. Es wurden deshalb
// ADW "Bereiche" eingeführt, z.b. ADW80.16 od. ADW256.50. die samples pro periode und die signalfrequenz
// bestimmen hierbei das zu verwendende polygon. die polygone werden als default werte abgelegt.
// da die adc's bis dahin keine phasenkorrekturpolygone brauchten, weil der phasenkorrekturwert über
// eine von analog devices angegebene formel berechnet wurde, wurden für die adc's die stützstellen und
// koeffizienten der phase für diese gainkorrektur missbraucht. das ersparte jede menge aufwand, reduzierte
// die fehlerträchtigkeit, ist aber nicht sehr schön und zumindest für aussenstehende schlecht nachvollziehbar.
// und wie sich gleich zeigen wird wurde der aufwand damit nur in die zukunft verschoben.

// düstere wolken ..... es zeichnet sich immer mehr ab dass der absolute phasenwinkel unserer wm3000 nicht
// stimmt. die genauigkeit ist relevant bei der prüfung der nicht konventionellen wandler. die ptb bescheinigt
// uns einen phasenwinkelfehler von 0.06°..... viel zu viel....die korrektur aus der berechnung nach der
// von analog devices angegeben formel scheint falsch zu sein. eine implementierte phasenjustage für die
// ad wandler bringt genauigkeiten um 0.001° :-) .

// 4. ansatz gainorder=3, offsetorder=0, phaseorder=3
// wie oben gesagt..... jetzt ist der aufwand da und wir müssen die justagedaten unterscheiden, wenn
// wir mit updates kompatibel bleiben wollen. selbstverständlich können geräte mit der neuen software allein
// nicht die genauigkeit erzielen. dazu muss dann erst ein abgleich erfolgt sein. aber wir können mit
// updates fehler fixen ohne dass die geräte danach nicht justiert sind.

#ifndef WMGOBAL_H
#define WMGLOBAL_H

#define FPGADeviceNode "/dev/zFPGA1reg"
#define CheckSumOffset 56
#define LeiterkartenName "wm3000u"
#define ServerBasisName "wm3000ud"
#define ServerVersion "V2.16"
#define InpBufSize 4096
#define atmelFlashfilePath "/opt/zera/bin/atmel.hex"
#define atmelResetBit 16

// wenn WMDEBUG -> kein fork() 
//#define WMDEBUG 1

#endif
