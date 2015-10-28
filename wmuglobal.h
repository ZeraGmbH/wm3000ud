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

#ifndef WMGOBAL_H
#define WMGLOBAL_H

#define FPGADeviceNode "/dev/zFPGA1reg"
#define CheckSumOffset 56
#define LeiterkartenName "wm3000u"
#define ServerBasisName "wm3000ud"
#define ServerVersion "V2.10"
#define InpBufSize 4096
#define atmelFlashfilePath "/opt/zera/bin/atmel.hex"
#define atmelResetBit 16

// wenn WMDEBUG -> kein fork() 
//#define WMDEBUG 1

#endif
