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

#ifndef WMGOBAL_H
#define WMGLOBAL_H

#define CheckSumOffset 56
#define LeiterkartenName "wm3000u"
#define ServerBasisName "wm3000ud"
#define ServerVersion "V1.07"
#define InpBufSize 4096

// wenn WMDEBUG -> kein fork() 
//#define WMDEBUG 1

#endif
