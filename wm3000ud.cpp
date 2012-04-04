// implementation des eigenlichen servers wm3000u(d)

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <iostream>
#include <syslog.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qtextstream.h>
#include <qfile.h>                                                                                   
#include <qdom.h>
#include <qbuffer.h>
#include <unistd.h>
#include "zeraglobal.h"
#include "wmuglobal.h"
#include "wmjustdata.h"
#include "cmdinterpret.h"
#include "zhserver.h"
#include "wm3000ud.h"
#include "i2ceeprom.h"
#include "crcutils.h"
#include "i2cutils.h"
#include "intelhexfileio.h"


extern cNode* InitCmdTree();

// bereiche von kanal 0 bzw. 1

static sRange RangeCh0[ch0_n]={{"ADW80"  ,"ADW80"   ,"500000"  ,255,  Volt , rngVirt, NULL, NULL},
			       {"ADW256"  ,"ADW256"   ,"500000"  ,255,  Volt , rngVirt, NULL, NULL},
			       
                                                 {"480V"   ,"480.0"  ,"4730418"  ,0 ,  Volt, rngPhys, NULL, NULL},
 			       { "240V"   ,"240.0"  ,"4730418"  ,1 , Volt, rngPhys,NULL, NULL},
			       { "120V"   ,"120.0"  ,"4730418"  ,2 , Volt, rngPhys,NULL, NULL},
			       { "60V"     ,"60.0"   ,"4730418"  ,3 , Volt, rngPhys,NULL, NULL},
			       { "30V"     ,"30.0"   ,"47304180"  ,4 , Volt, rngPhys,NULL, NULL},
			       { "15V"     ,"15.0"   ,"4730418"  ,5 , Volt, rngPhys,NULL, NULL},
			       { "7.5V"    ,"7.5"     ,"4730418"  ,6 , Volt, rngPhys,NULL, NULL},
			       { "3.75V"  ,"3.75"   ,"4730418"  ,7 , Volt, rngPhys,NULL, NULL}};

/*
			       
			       { "E15.0V"    , "15.0"   ,"2831155" ,10, Volt, rngPhys,NULL, NULL},
			       { "E10.0V"    , "10.0"   ,"4718592" ,11,  Volt, rngPhys,NULL, NULL},
			       { "E5.0V"      ,"5.0"      ,"4718592" ,12,Volt, rngPhys,NULL, NULL},
			       { "E2.5V"   ,"2.5"      ,"4718592" ,13,Volt, rngPhys,NULL, NULL},
			       { "E1.0V"      , "1.0"     ,"4718592" ,14,Volt, rngPhys,NULL, NULL},
			       { "E500mV", "0.5"     ,"2995931" ,15,Volt, rngPhys,NULL, NULL},
			       { "E250mV", "0.25"   ,"2995931" ,16,Volt, rngPhys,NULL, NULL},
			       { "E100mV", "0.1"     ,"2995931" ,17,Volt, rngPhys,NULL, NULL},
			       { "E50mV"  , "0.05"   ,"2995931" ,18,Volt, rngPhys,NULL, NULL},
			       { "E25mV"  , "0.025" ,"2995931" ,19,Volt, rngPhys,NULL, NULL}};
*/

static sRange RangeCh1[ch1_n]={{"ADW80"  ,"ADW80"  ,"500000"  ,255,    Volt, rngVirt, NULL, NULL},
			       {"ADW256"  ,"ADW256"  ,"500000"  ,255,    Volt, rngVirt,NULL, NULL},
			       
                                                 {"480V"   ,"480.0" ,"4730418"  ,0 ,   Volt, rngPhys,NULL, NULL},
			       { "240V"   ,"240.0" ,"4730418"  ,1 ,  Volt, rngPhys,NULL, NULL},
			       { "120V"   ,"120.0" ,"4730418"  ,2 ,  Volt, rngPhys,NULL, NULL},	 			       { "60V"     ,"60.0"  ,"4730418"  ,3 ,  Volt, rngPhys,NULL, NULL},
			       { "30V"     ,"30.0"  ,"4730418"  ,4 ,  Volt, rngPhys,NULL, NULL},
			       { "15V"     ,"15.0"  ,"4730418"  ,5 ,  Volt, rngPhys,NULL, NULL},
			       { "7.5V"    ,"7.5"    ,"4730418"  ,6 ,  Volt, rngPhys,NULL, NULL},
			       { "3.75V"  ,"3.75"  ,"4730418"  ,7 ,  Volt, rngPhys,NULL, NULL},
			       
			       { "E15.0V"    , "15.0"   ,"2831155" ,10 , Volt, rngPhys,NULL, NULL},
			       { "E10.0V"    , "10.0"   ,"4718592" ,11 , Volt, rngPhys,NULL, NULL},
			       { "E5.0V"      ,"5.0"      ,"4718592" ,12,Volt, rngPhys,NULL, NULL},
			       { "E2.5V"   ,"2.5"      ,"4718592" ,13,Volt, rngPhys,NULL, NULL},
			       { "E1.0V"      , "1.0"     ,"4718592" ,14,Volt, rngPhys,NULL, NULL},
			       { "E500mV", "0.5"     ,"2995931" ,15,Volt, rngPhys,NULL, NULL},
			       { "E250mV", "0.25"   ,"2995931" ,16,Volt, rngPhys,NULL, NULL},
			       { "E100mV", "0.1"     ,"2995931" ,17,Volt, rngPhys,NULL, NULL},
			       { "E50mV"  , "0.05"   ,"2995931" ,18,Volt, rngPhys,NULL, NULL},
			       { "E25mV"  , "0.025" ,"2995931" ,19,Volt, rngPhys,NULL, NULL}};


QStringList MeasChannelList;

cWM3000uServer::cWM3000uServer() 
    :cZHServer() {
    
    cParse* parser = new(cParse); // das ist der parser
    pCmdInterpreter = new cCmdInterpreter(this,InitCmdTree(),parser); // das ist der kommando interpreter
    
    MeasChannelList << "ch0" << "ch1";
    CValueList << "CAMPLITUDE" << "CPHASE" << "COFFSET";
    
    ChannelRangeArrayMap["ch0"] = &RangeCh0[0]; // alle sRange* / kanal
    ChannelRangeArrayMap["ch1"] = &RangeCh1[0];
    sRange* sr=&RangeCh0[0];
    for (unsigned int i = 0; i<(sizeof(RangeCh0)/sizeof(sRange)); i++,sr++) {
	Ch0RangeList << sr->RName;
	sr->pJustData=new cWMJustData; // default justage werte
	sr->pOldJData=new cOldWMJustData; // dito
    }
    sr=&RangeCh1[0];
    for (unsigned int i = 0; i<(sizeof(RangeCh1)/sizeof(sRange)); i++,sr++) {
	Ch1RangeList << sr->RName;
	sr->pJustData=new cWMJustData; // default justage werte
	sr->pOldJData=new cOldWMJustData; // dito
    }
    ChannelRangeListMap["ch0"] = &Ch0RangeList;
    ChannelRangeListMap["ch1"] = &Ch1RangeList;
    ChannelCValueListMap["ch0"] = &CValueList; // die sensorik für beide kanäle ist gleich
    ChannelCValueListMap["ch1"] = &CValueList; // -> auch die gleichen korrekturwerte listen
    CCoeffientList << "GCC0" << "GCC1" << "PCC0" << "PCC1" << "PCC2" << "PCC3" << "OCC0";  // wir brauchen hier nur eine für beide kanäle  
    ChannelCCoeffientListMap["ch0"] = &CCoeffientList; // s.o. beide gleich
    ChannelCCoeffientListMap["ch1"] = &CCoeffientList;
    
    CNodeList << "GCN0" << "GCN1" << "PCN0" << "PCN1" << "PCN2" << "PCN3" << "OCN0";    
    ChannelCNodeListMap["ch0"] = &CNodeList; // s.o. beide gleich
    ChannelCNodeListMap["ch1"] = &CNodeList;
    
    ChannelSockListMap["ch0"] = &SockList0;
    ChannelSockListMap["ch1"] = &SockList1;
    
    DebugLevel = 0; // MaxDebugLevel; // default
    PSamples = 80; // default
    SMode = 0; // default
    SyncSource = 0; // default 0 = extern
    SyncPeriod = 1000; // default 1000ms
    SampleFrequency = 50.0; // default 50 hz abtastfrequenz
    sI2CDevNode = I2CDeviceNode; //default
    I2CMasterAdr = I2CMasterAdress; //default 0x20
    I2CSlaveAdr = I2CSlaveAdress; // default  0x21
    DateTime = QDateTime(QDate(8000,12,24));
    sSerialNumber = mGetSerialNumber();
    sDeviceVersion = mGetDeviceVersion();
    
    RangeTranslationMap["E15V"] = "E15.0V"; // wegen hubertus und kompatiblitätszwang 
    RangeTranslationMap["E10V"] = "E10.0V"; // übersetzungstabelle anlegen
    RangeTranslationMap["E5V"] = "E5.0V";
    RangeTranslationMap["E1V"] = "E1.0V";
     
    ReadJustData();
}


int cWM3000uServer::sDebugLevel(char* s) {
    QString p = s;
    int dl = p.toInt();
    if ( (dl>=0) && (dl<=MaxDebugLevel) ) {
	DebugLevel=dl;
	return 0;
    }
    else return 1;
}


int cWM3000uServer::SetI2CDevNode(char* s) {
    sI2CDevNode=s;
    return(0); // immer ok ?
}


int cWM3000uServer::SetI2CSlaveAdr(char* s) {
    bool ok=true;
    int r=1; // erstmal falsch
    short adr= QString(s).toShort(&ok);
    if ( (ok) && (adr > 0) && (adr < 128) && (adr != I2CMasterAdr) ) {
	I2CSlaveAdr=adr;
	r=0;
    }
    return (r);
}


int cWM3000uServer::SetI2CMasterAdr(char* s) {
    bool ok=true;
    int r=1; // erstmal falsch
    short adr= QString(s).toShort(&ok);
    if ( (ok) && (adr > 0) && (adr < 128) && (adr != I2CSlaveAdr) ) {
	I2CMasterAdr=adr;
	r=0;
    }
    return (r);
}


int cWM3000uServer::I2CBootloaderCommand(bl_cmd* blc) {
    int rlen = -1; // rückgabewert länge ; < 0 bedeutet fehler
    char inpBuf[5]; // die antwort auf command ist immer 5 bytes lang
  
    GenBootloaderCommand(blc);
    struct i2c_msg Msgs[2] = { {addr :I2CSlaveAdr, flags: 0,len: blc->cmdlen, buf: blc->cmddata}, // 2 messages (tagged format )
			  {addr :I2CSlaveAdr, flags: (I2C_M_RD+I2C_M_NOSTART), len: 5, buf: inpBuf} };   
    
    struct i2c_rdwr_ioctl_data comData = {  msgs: Msgs, nmsgs: 2 };
 
     if DEBUG2 syslog(LOG_INFO,"i2c write bootloader command %d bytes to i2cslave at 0x%x",blc->cmdlen,I2CSlaveAdr);
     if (! I2CTransfer(sI2CDevNode,I2CSlaveAdr,DebugLevel,&comData)) { // wenn kein fehler
	 if (inpBuf[4] == CalcBlockCRC(inpBuf, 4) ) { 
	     rlen = (inpBuf[2] << 8) + inpBuf[3];
	     blc->RM = (inpBuf[0] << 8) + inpBuf[1];
	     if (blc->RM)
		 if DEBUG1 syslog(LOG_ERR,"i2ctransfer error 0x%x with i2cslave at 0x%x failed",blc->RM,I2CSlaveAdr);
	 }
     }
     else
     {
	 if DEBUG1 syslog(LOG_ERR,"i2ctransfer to i2cslave at 0x%x failed",I2CSlaveAdr);
     }
     delete blc->cmddata;
     return rlen; // -1 wenn fehler ; sonst länge des erzeugten output
 }


int cWM3000uServer::I2CWriteCommand(hw_cmd* hc) {
    int rlen = -1; // rückgabewert länge ; < 0 bedeutet fehler
    char inpBuf[5]; // die antwort auf command ist immer 5 bytes lang
  
    GenCommand(hc);
    struct i2c_msg Msgs[2] = { {addr :I2CSlaveAdr, flags: 0,len: hc->cmdlen, buf: hc->cmddata}, // 2 messages (tagged format )
			  {addr :I2CSlaveAdr, flags: (I2C_M_RD+I2C_M_NOSTART), len: 5, buf: inpBuf} };   
    
    struct i2c_rdwr_ioctl_data comData = {  msgs: Msgs, nmsgs: 2 };
 
     if DEBUG2 syslog(LOG_INFO,"i2c writecommand %d bytes to i2cslave at 0x%x",hc->cmdlen,I2CSlaveAdr);
     if (! I2CTransfer(sI2CDevNode,I2CSlaveAdr,DebugLevel,&comData)) { // wenn kein fehler
	 if (inpBuf[4] == CalcBlockCRC(inpBuf, 4) ) { 
	     rlen = (inpBuf[2] << 8) + inpBuf[3];
	     hc->RM = (inpBuf[0] << 8) + inpBuf[1];
	     if (hc->RM)
		 if DEBUG1 syslog(LOG_ERR,"i2ctransfer error 0x%x with i2cslave at 0x%x failed",hc->RM,I2CSlaveAdr);
	 }
     }
     else
     {
	 if DEBUG1 syslog(LOG_ERR,"i2ctransfer to i2cslave at 0x%x failed",I2CSlaveAdr);
     }
     delete hc->cmddata;
     return rlen; // -1 wenn fehler ; sonst länge des erzeugten output
 }


int cWM3000uServer::I2CReadOutput(char* data, int dlen) { 
    int rlen = -1; // rückgabewert länge ; < 0 bedeutet fehler
    
    struct i2c_msg Msgs = {addr :I2CSlaveAdr, flags: I2C_M_RD,len: dlen,buf: data}; // 1 message
    struct i2c_rdwr_ioctl_data comData = {  msgs: &Msgs, nmsgs: 1 };
 
     if DEBUG2 syslog(LOG_INFO,"i2c readoutput %d bytes from i2cslave at 0x%x",dlen+1,I2CSlaveAdr);
     if (! I2CTransfer(sI2CDevNode,I2CSlaveAdress,DebugLevel,&comData)) { // wenn kein fehler
	 if (data[dlen-1] == CalcBlockCRC(data, dlen-1) )
	     rlen = dlen;
     }
     else
     {
	 if DEBUG1 syslog(LOG_ERR,"i2ctransfer to i2cslave at 0x%x failed",I2CSlaveAdr);
     }
     return rlen; // -1 wenn fehler ; sonst die gelesenen zeichen
 }


void  cWM3000uServer::GenCommand(hw_cmd* hc) {
    short len = 6 + hc->plen;
    char* p = new char[len];
    hc->cmddata = p;
    
    *p++ = len >> 8;
    *p++ = len & 0xFF;
    *p++ = ((hc->cmdcode) >> 8) & 0xFF;
    *p++ = (hc->cmdcode) & 0xFF;
    *p++ = hc->device;
    
    const char* ppar = hc->par;
    for (int i = 0; i < hc->plen;i++)
	*p++ = *ppar++;
    
    *p = CalcBlockCRC(hc->cmddata,len-1);
    hc->cmdlen = len;
}


void  cWM3000uServer::GenBootloaderCommand(bl_cmd* blc) {
    short len = 4 + blc->plen;
    char* p = new char[len];
    blc->cmddata = p;
    
    *p++ = blc->cmdcode;
    *p++ = blc->plen >> 8;
    *p++ = blc->plen & 0xFF;
    
    char* ppar = blc->par;
    for (int i = 0; i < blc->plen; i++)
	*p++ = *ppar++;
    
    *p = CalcBlockCRC(blc->cmddata,len-1);
    blc->cmdlen = len;
}


char* cWM3000uServer::GenAdressPointerParameter(uchar adresspointerSize, ulong adr)
{
    char* par = new char(adresspointerSize);
    char* pptr = par;
    for (int  i = 0; i < adresspointerSize; i++)
	*pptr++ = (char) ((adr >> (8* ( (adresspointerSize-1) - i)) ) & 0xff);
    return par;
}


bool cWM3000uServer::ReadJustData() { 
    m_nJDataStat = 0; // status erst mal reset
    QByteArray ba(6); // byte array zur aufnahme länge und checksumme
    c24LC256* Flash=new c24LC256(sI2CDevNode,DebugLevel,I2CEEPromAdress);
    if ( (6 - Flash->ReadData(ba.data(),6,0)) >0 ) {
	if DEBUG1 syslog(LOG_ERR,"error reading flashmemory\n");
	delete Flash;
	return(false); // lesefehler
    }
    QDataStream bastream( ba, IO_ReadOnly );
    uint count;
    Q_UINT16 chksumCMP = 0;
    bastream >> count >> m_nChksumFlash; // länge der flashdaten u. checksumme 
    // syslog(LOG_ERR,"flash length is %d, chksum is %d\n",count,m_nChksumFlash);	
    if ( count > (uint)Flash->size() ) {
	if DEBUG1 syslog(LOG_ERR,"error reading flashmemory, count > flash\n");
	delete Flash;
	return(false); // lesefehler
    }
    QByteArray ba2(count); // byte array zur aufnahme der gesamten daten
    if ( (count - Flash->ReadData(ba2.data(),count,0)) >0 ) {
	if DEBUG1 syslog(LOG_ERR,"error reading flashmemory\n");
	delete Flash;
	return(false); // lesefehler
    }
    
    QBuffer mem;
    mem.setBuffer(ba2);
    mem.open(IO_ReadWrite);
    mem.at(0);
    
    QByteArray ca(6); // qbyte array mit 6 bytes
    QDataStream castream( ca, IO_WriteOnly );
    castream << count << chksumCMP;
    
    mem.writeBlock(ca); // 0 setzen der checksumme
        
    chksumCMP = qChecksum(ba2.data(),ba2.size());
    // syslog(LOG_ERR,"computed flash chksum is %d\n",chksumCMP);	
    if (chksumCMP != m_nChksumFlash) {
	if DEBUG1 syslog(LOG_ERR,"invalid checksum encountered reading flashmemory\n");
	return(false); // daten fehlerhaft
    }
    
     // jetzt die daten noch einsortieren
    QString SVersion;
    QDataStream ba2stream( ba2, IO_ReadOnly );
    char flashdata[100];
    char* s = flashdata;
    ba2stream >> count >> chksumCMP;
    
    ba2stream >> s;
    
    // ab version v1.02 version testen
    if (QString(s) == "ServerVersion") {
	ba2stream >> s;
	SVersion = QString(s);
	ba2stream >> s; // jetzt steht in s auch für die neue version der leiterplatten name
    }
    else
	SVersion == "V0.00"; // dann ist es eine alte version
	
    
    if (QString(s) != LeiterkartenName) {
	if DEBUG1 syslog(LOG_ERR,"flashmemory read, contains wrong pcb name\n");
	return false; // leiterkarten name falsch
    }
    
    ba2stream >> s;
    
    QString qs = QString(s); 
    if (qs != sDeviceVersion) {
	// test ob sich nur die hinteren nummern der lca bzw. ctrl version geändert haben
	// indem die hinteren stellen der nummern aus sDeviceVersion nach s übertragen werden
	// und anschliessend nochmal verglichen wird
	
	QString ss, sd, ss2, sd2;
	ss = qs.section(';',2,2); // LCA: x.xx
	ss2 = '.' +ss.section('.',1,1); // .xx
	sd = sDeviceVersion.section(';',2,2); // LCA: x.yy
	sd2 = '.' +sd.section('.',1,1); // .yy
	ss.replace(ss2,sd2); // tausch .xx durch .yy
	qs.replace(qs.section(';',2,2), ss); // LCA: x.yy -> s
	
	ss = qs.section(';',3,3); // CTRL: x.xx
	ss2 = '.' +ss.section('.',1,1); // .xx
	sd = sDeviceVersion.section(';',3,3); // CTRL: x.yy
	sd2 = '.' +sd.section('.',1,1); // .yy
	ss.replace(ss2,sd2); // tausch .xx durch .yy
	qs.replace(qs.section(';',3,3), ss); // CTRL: x.yy -> s
	
	if (qs != sDeviceVersion) {
	    if DEBUG1 syslog(LOG_ERR,"flashmemory read, contains wrong versionnumber\n");
	    m_nJDataStat += wrongVERS;
	    if (!EEPromAccessEnable()) return false; // versionsnummer falsch
	}
    }
    
    ba2stream >> s;
    if (QString(s) != sSerialNumber) {
	if DEBUG1 syslog(LOG_ERR,"flashmemory read, contains wrong serialnumber\n");
	m_nJDataStat += wrongSNR;
	if (!EEPromAccessEnable()) return false; // seriennummer falsch
    }
    
    ba2stream >> s;
    DateTime.fromString(QString(s),Qt::TextDate); // datum und uhrzeit übernehmen
    
    if (SVersion != "V0.00") { // es ist nicht eine alte server version bei der die serverversion noch nicht eingetragen war 
	while (!ba2stream.atEnd()) {
	    ba2stream >> s;
	    QString chname=s; // für welchen kanal
	    
	    ba2stream >> count;
	    
	    sRange dummy;
	    dummy.pJustData=new cWMJustData; // default justage werte
	    for (uint i=0;i<count;i++) {
		ba2stream >> s;
		QString rngname=s; // für welchen bereich
		sRange* rng = SearchRange(chname,rngname);
		if (!rng) { // wenn wir den bereich nicht gefunden haben
		    rngname = RangeTranslationMap[rngname]; // prüfen wir ob der bereichname
		    rng = SearchRange(chname,rngname); // übersetzt werden muss
		} // und ob wir dann den bereich finden
		
		if (!rng) // wenn wir den bereich nicht gefunden haben, "saugen" wir die daten
		    rng = &dummy; // nach dummy
		rng->pJustData->Deserialize(ba2stream); // justagedaten  (koeffizienten, stützstellen und status
	    }
	    delete dummy.pJustData;
	}
    }
    else
    {
	while (!ba2stream.atEnd()) {
	    ba2stream >> s;
	    QString chname=s; // für welchen kanal
	    
	    ba2stream >> count;
	    sRange dummy;
	    dummy.pOldJData=new cOldWMJustData;
	    for (uint i=0;i<count;i++) {
		ba2stream >> s;
		QString rngname=s; // für welchen bereich
		sRange* rng = SearchRange(chname,rngname);
		sRange dummy;
		if (!rng) { // wenn wir den bereich nicht gefunden haben
		    rngname = RangeTranslationMap[rngname]; // prüfen wir ob der bereichname
		    rng = SearchRange(chname,rngname); // übersetzt werden muss
		} // und ob wir dann den bereich finden
		if (!rng)
		    rng = &dummy;
		rng->pOldJData->m_JData.Deserialize(ba2stream); // justagedaten 
		rng->pOldJData->m_NInfo.Deserialize(ba2stream); // stützstellen
	    }
	    delete dummy.pOldJData;  
	}
	
	// wir haben die alten justage daten jetzt müssen wir sie noch umwandeln
	for (QStringList::iterator it=MeasChannelList.begin(); it !=MeasChannelList.end(); it++) {		    QStringList* sl=ChannelRangeListMap.find(*it).data(); 
	    QStringList::Iterator it2;
	    for ( it2 = sl->begin(); it2 != sl->end(); ++it2 ) {
		sRange* rng = SearchRange(*it,*it2);
		rng->pJustData->setStatus( rng->pOldJData->m_JData.m_Status); // status übernommen
		rng->pJustData->m_pOffsetCorrection->setCoefficient(0, rng->pOldJData->m_JData.m_OffsetCorrection);
		rng->pJustData->m_pOffsetCorrection->setNode(0, cJustNode(rng->pOldJData->m_JData.m_OffsetCorrection, 0.0)); // offset korrektur übernommen
		
		rng->pJustData->m_pGainCorrection->setCoefficient(0, rng->pOldJData->m_JData.m_GainCorrection);
		rng->pJustData->m_pGainCorrection->setNode(0, cJustNode(rng->pOldJData->m_JData.m_GainCorrection, 0.0)); // gain korrektur übernommen
		
		for (int i = 0; i < 4; i++)
		{
		    rng->pJustData->m_pPhaseCorrection->setCoefficient(i, rng->pOldJData->m_JData.m_PhaseCoefficient[3-i]); // koeffizienten stehen im neuen format anders sortiert
		    rng->pJustData->m_pPhaseCorrection->setNode(i, cJustNode(rng->pOldJData->m_NInfo.m_NInformation[i*2+1],rng->pOldJData->m_NInfo.m_NInformation[i*2])); // phase korrektur übernommen
		}
	    }
	}
    }
    return(true);
}

sRange* cWM3000uServer::SearchRange(QString& ch,QString& rng) { // holt einen zeiger auf sRange abhängig v. kanal,range 
    tChannelListMap::Iterator it=ChannelRangeListMap.find(ch);
    QStringList* sl=it.data();
    int i=sl->findIndex(rng);
    if ( i < 0 ) return(NULL); // den bereich gibt es nicht
    tChannelRangeArrayMap::Iterator it2=ChannelRangeArrayMap.find(ch);
    sRange* lr=it2.data();
    lr+=i;    
    return(lr);
}

bool cWM3000uServer::GetAdjInfo(QDomNode n) { // n steht auf einem element dessen tagname channel ist
    typedef QMap<QString,cWMJustData*> tRangeJustMap; // zur temporären speicherung von justagedaten
    tRangeJustMap RangeJustMap;
    tChannelListMap::Iterator it=NULL;
    QStringList* sl = NULL;
    QStringList::iterator it2;
    QString ch;
    
    QDomNodeList nl=n.childNodes();
    tRangeJustMap::iterator it4;
    
    for (uint i=0; i<nl.length(); i++) {
	QDomNode n=nl.item(i);
	QDomElement e=n.toElement();
	QString tName=e.tagName();
	if (tName=="Name") {
	    if (it!=NULL) return(false); // kanal name mehr als 1x ist ein fehler
	    ch=e.text();
	    tChannelListMap::Iterator it=ChannelRangeListMap.find(ch);
	    if (it==ChannelRangeListMap.end()) return(false); // den kanal gibt es nicht, auch ein fehler
	    sl=it.data();
	    for (it2 = sl->begin(); it2 != sl->end() ; it2++) RangeJustMap[*it2]=new cWMJustData(); // default daten bei import
	} else
	if (tName=="Range") {
	    QDomNodeList nl2=n.childNodes(); // nl2 ist eine qdomnode liste mit allen nodes unterhalb "Range"  
	    QString rname;
	    for (uint j=0; j<nl2.length(); j++) {
		n=nl2.item(j);
		QDomElement e2=n.toElement();
		QString tName2=e2.tagName();
		if (tName2=="Name") { 
		    rname=e2.text();		    
		} else
		    
	              if (tName2=="Status") {    
		    it4=RangeJustMap.find(rname);
		    if (it4 != RangeJustMap.end() ) { // den bereich gibts
			cWMJustData* jwmd=it4.data();
			jwmd->DeserializeStatus(e2.text());
		    }
		}
		    
		if (tName2 == "Gain" || tName2 == "Phase" || tName2 == "Offset") {    
		    it4=RangeJustMap.find(rname);
		    if (it4 != RangeJustMap.end() ) { // den bereich gibts
			cWMJustData* jwmd=it4.data();
			cJustData* jd=NULL;
			if (tName2 == "Gain")
			    jd = jwmd->m_pGainCorrection;
			if (tName2 == "Phase")
			    jd = jwmd->m_pPhaseCorrection;
			if (tName2 == "Offset")
			    jd = jwmd->m_pOffsetCorrection;
			QDomNodeList nl3=n.childNodes();
			QDomNode n2;
			QDomElement e3;
			QString tName3;
			for (uint k=0;k<nl3.length(); k++) {
			    n2 = nl3.item(k);
			    e3=n2.toElement();
			    tName3=e3.tagName();
			    if (tName3 == "Coefficients") {
				if (jd)
					jd->DeserializeCoefficients(e3.text());
			    }
			    if (tName3 == "Nodes") {
				if (jd)
					jd->DeserializeNodes(e3.text());
			    }
			}
		    }
		}

	    }
	}
    }

	
    if (sl) 	
    {
    	tChannelRangeArrayMap::Iterator it3=ChannelRangeArrayMap.find(ch);
    	sRange* srdest=it3.data();
    	for (it2 = sl->begin(); it2 != sl->end() ; it2++,srdest++) {
		it4=RangeJustMap.find(*it2);
		*(srdest->pJustData) = *(it4.data());
    	}
    }
    return (true);
}


const char* cWM3000uServer::mSetPPSSync() {
    ulong PPSPar; 
    PPSPar = (ulong) (SyncSource << 31) | (ulong) (SyncPeriod * 1e5); 
 
    char FPAR[4];
    for (int i = 0; i < 4; i++)
	FPAR[i] = (PPSPar >> ((3-i)*8)) & 0xff;
    hw_cmd CMD1 = { cmdcode: hwSetPPSSync, device: 0, par: FPAR, plen: 4,cmdlen: 0,cmddata: 0, RM:0 };
    if ( (I2CWriteCommand(&CMD1) == 0) &&  (CMD1.RM == 0) ) 
	return ACKString; // acknowledge
    else
	return ERREXECString;    
}


const char* cWM3000uServer::mSetSyncPeriod(char* s) {
    bool ok;
    QString par = pCmdInterpreter->m_pParser->GetKeyword(&s); // holt den parameter aus dem kommando
    int t = par.toInt(&ok);
    
    if ( (ok) && (t>=100) && (t<=10000) ) {
	SyncPeriod = t;
	Answer = mSetPPSSync(); 
    }
    else Answer = ERRVALString;
    return Answer.latin1();        
}
 

const char* cWM3000uServer::mSetSyncSource(char* s) {
    bool ok;
    QString par = pCmdInterpreter->m_pParser->GetKeyword(&s); // holt den parameter aus dem kommando
    int so = par.toInt(&ok);
    
    if ( (ok) && ((so == 0) || (so ==1)) ) {
	SyncSource = so;
	Answer = mSetPPSSync(); 
    }
    else Answer = ERRVALString;
    return Answer.latin1();            
}


const char* cWM3000uServer::mSetPSamples(char* s) {
    bool ok;
    QString par = pCmdInterpreter->m_pParser->GetKeyword(&s); // holt den parameter aus dem kommando
    int ps = par.toInt(&ok);
    
    if ( (ok) && ((ps == 80) || (ps == 256)) ) {
	char PSPAR[2];
	PSPAR[0] = (ps >> 8) & 0xff;
	PSPAR[1] = ps & 0xff;
	hw_cmd CMD = { cmdcode: hwSetSRate, device: 0, par: PSPAR, plen: 2,cmdlen: 0,cmddata: 0, RM:0 };
	if ( (I2CWriteCommand(&CMD) == 0) &&  (CMD.RM == 0) ) 
	    Answer = ACKString; // acknowledge
	else
	    Answer = ERREXECString; 
    }
    else Answer = ERRVALString;
    return Answer.latin1();        
}	
 

const char* cWM3000uServer::mSetSampleMode(char* s) {
    bool ok;
    QString par = pCmdInterpreter->m_pParser->GetKeyword(&s); // holt den parameter aus dem kommando
    int sm = par.toInt(&ok);
    
    if ( (ok) && (sm > -1) && (sm< 4) ) {
	char PAR[1];
	PAR[0] = sm;
	hw_cmd CMD = { cmdcode: hwSetMode, device: 0, par: PAR, plen: 1,cmdlen: 0,cmddata: 0, RM:0 };
	if ((I2CWriteCommand(&CMD) == 0) &&  (CMD.RM == 0)) 
	    Answer = ACKString; // acknowledge
	else
	    Answer = ERREXECString;
    }
    else Answer = ERRVALString;
    return Answer.latin1();    
}


const char* cWM3000uServer::mSetSampleFrequency(char* s) {
    bool ok;
    QString par = pCmdInterpreter->m_pParser->GetKeyword(&s); // holt den parameter aus dem kommando
    double fr = par.toDouble(&ok);
    SampleFrequency = fr; // wir merken uns die sampling frequenz
    if ( (ok) && (fr >= 10.0) && (fr <= 70.0)  ) { 
	ulong fpar = (ulong) (fr * 1e6); 						   
	char FPAR[4];
	for (int i = 0; i < 4; i++)
	    FPAR[i] = (fpar >> ((3-i)*8)) & 0xff;

	hw_cmd CMD = { cmdcode: hwSetFrequency, device: 0, par: FPAR, plen: 4,cmdlen: 0,cmddata: 0, RM:0 };
	if ( (I2CWriteCommand(&CMD) == 0) &&  (CMD.RM == 0) ) 
	    return Answer = ACKString; // acknowledge
	else						   
	    return Answer = ERREXECString;
    }
    else Answer = ERRVALString;
    return Answer.latin1();            
}	


const char* cWM3000uServer::mFile2Justdata(char* s) {
    if ( !EEPromAccessEnable() ){
	Answer = ERRAUTString; // nicht erlaubt
	return Answer.latin1();
    }
    QString filename = pCmdInterpreter->m_pParser->GetKeyword(&s); // holt den parameter aus dem kommando
    QFile file( filename+".xml" );
    if ( !file.open( IO_ReadOnly ) ) {
	if DEBUG1 syslog(LOG_ERR,"justdata import,xml file does not exist\n");
	Answer = ERRPATHString; // falscher filename
	return Answer.latin1();
    }
    
    QDomDocument justdata( "TheDocument" );
    if ( !justdata.setContent( &file ) ) {
	file.close();
	if DEBUG1 syslog(LOG_ERR,"justdata import, format error in xml file\n");
	Answer = ERRXMLFORMATString;; // fehler im xml file
	return Answer.latin1();
    }
    file.close();

   QDomDocumentType TheDocType=justdata.doctype ();
   if  (TheDocType.name() != "WM3000UAdjustmentData") {
              if DEBUG1 syslog(LOG_ERR,"justdata import, wrong xml documentype\n");
     	Answer = ERRXMLDOCTYPEString; // document type inkorrekt
	return Answer.latin1();
    }
   
   QDomElement rootElem = justdata.documentElement(); 
   QDomNodeList nl=rootElem.childNodes();
   
   bool VersionNrOK=false;
   bool SerialNrOK=false;
   bool DateOK=false;
   bool TimeOK=false;
   
   for (uint i=0; i<nl.length() ; i++) {
       QDomNode n = nl.item(i);
       QDomElement e=n.toElement();
       if ( e.isNull() ) { 
	   Answer = ERRXMLFORMATString;
	   return Answer.latin1();
       }
       QString tName=e.tagName();
       if (tName == "SerialNumber") {	
	   if (  !(SerialNrOK = (e.text() == sSerialNumber )) ) {
	       if DEBUG1 syslog(LOG_ERR,"justdata import, wrong serialnumber in xml file\n");
	       Answer = ERRXMLSERIALString;
	       return Answer.latin1();
	   }
       } else
       if (tName == "VersionNumber") {
	   if ( ! ( VersionNrOK= (e.text() == sDeviceVersion) ) ) {
	       if DEBUG1 syslog(LOG_ERR,"justdata import, wrong versionnumber in xml file\n");
	       Answer = ERRXMLVERSIONString;
	       return Answer.latin1();
	   }
       } else
       if (tName=="Date") {
	    QDate d=QDate::fromString(e.text(),Qt::TextDate);
	    DateTime.setDate(d);
	    DateOK=true;
       } else
       if (tName=="Time") {
	    QTime t=QTime::fromString(e.text(),Qt::TextDate);
	    DateTime.setTime(t);			
	    TimeOK=true;
       } else	   
       if (tName == "Adjustment") {
	   if ( VersionNrOK && SerialNrOK && DateOK && TimeOK) {
	       QDomNodeList nl2=e.elementsByTagName ("Channel") ;
	       for (uint j=0;j<nl2.length();j++) {
		   n=nl2.item(j);
		   if ( !GetAdjInfo(n) ) {
		       if DEBUG1 syslog(LOG_ERR,"justdata import, wrong channel in xml file\n");
		       Answer = ERRXMLNODEString + n.nodeName();
		       return Answer.latin1();
		   }
	       }
	       Answer = ACKString;
	       return Answer.latin1();
	   }
       }
   }
   if DEBUG1 syslog(LOG_ERR,"justdata import, strange xml file\n");
   Answer = ERRXMLFORMATString;
   return Answer.latin1();
}


const char* cWM3000uServer::mJustData2File(char* s) {
    QDomDocument justdata( "WM3000uAdjustmentData" );
    
    QDomElement pcbtag = justdata.createElement( "PCB" );
    justdata.appendChild( pcbtag );
    
    QDomElement tag = justdata.createElement( "Type" );
    pcbtag.appendChild( tag );
    QDomText t = justdata.createTextNode( LeiterkartenName );
    tag.appendChild( t );
        
    tag = justdata.createElement( "VersionNumber" );
    pcbtag.appendChild( tag );
    t = justdata.createTextNode( sDeviceVersion );
    tag.appendChild( t );
   
    tag = justdata.createElement( "SerialNumber" );
    pcbtag.appendChild( tag );
    t = justdata.createTextNode( sSerialNumber );
    tag.appendChild( t );
    
    tag = justdata.createElement( "Date" );
    pcbtag.appendChild( tag );
    QDate d=DateTime.date();
    t = justdata.createTextNode(d.toString(Qt::TextDate));
    tag.appendChild( t );
		
    tag = justdata.createElement( "Time" );
    pcbtag.appendChild( tag );
    QTime ti=DateTime.time();
    t = justdata.createTextNode(ti.toString(Qt::TextDate));
    tag.appendChild( t );
    
    QDomElement adjtag = justdata.createElement( "Adjustment" );
    pcbtag.appendChild( adjtag );
    
    QDomElement chksumtag = justdata.createElement("Chksum");
    adjtag.appendChild(chksumtag);
    t = justdata.createTextNode(QString("0x%1").arg(m_nChksumFlash,0,16));
    chksumtag.appendChild(t);
    
    for (QStringList::iterator it=MeasChannelList.begin(); it !=MeasChannelList.end(); it++) {
	QDomElement chtag = justdata.createElement( "Channel" );
	adjtag.appendChild( chtag );
	
	tag = justdata.createElement( "Name" );
	chtag.appendChild( tag );
	t = justdata.createTextNode(*it);
	tag.appendChild( t );
	
	QStringList* sl=ChannelRangeListMap.find(*it).data(); 
	QStringList::Iterator it3;
	for ( it3 = sl->begin(); it3 != sl->end(); ++it3 ) {
	    sRange* rng = SearchRange(*it,*it3);
	    QDomElement rtag = justdata.createElement( "Range" );
	    chtag.appendChild( rtag );
	    tag = justdata.createElement( "Name" );
	    rtag.appendChild( tag );
	    t = justdata.createTextNode(rng->RName);
	    tag.appendChild( t );
	    
	    
	    tag = justdata.createElement( "Status" );
	    rtag.appendChild( tag );
	    QString jdata = rng->pJustData->SerializeStatus();
	    t = justdata.createTextNode(jdata);
	    tag.appendChild(t);
		    
	    QDomElement gpotag = justdata.createElement( "Gain" );
	    rtag.appendChild(gpotag);    
	    tag = justdata.createElement( "Coefficients" );
	    gpotag.appendChild(tag);
	    jdata = rng->pJustData->m_pGainCorrection->SerializeCoefficients();
	    t = justdata.createTextNode(jdata);
	    tag.appendChild(t);
	    tag = justdata.createElement( "Nodes" );
	    gpotag.appendChild(tag);
	    jdata = rng->pJustData->m_pGainCorrection->SerializeNodes();
	    t = justdata.createTextNode(jdata);
	    tag.appendChild(t);
	    
	    gpotag = justdata.createElement( "Phase" );
	    rtag.appendChild(gpotag);    
	    tag = justdata.createElement( "Coefficients" );
	    gpotag.appendChild(tag);
	    jdata = rng->pJustData->m_pPhaseCorrection->SerializeCoefficients();
	    t = justdata.createTextNode(jdata);
	    tag.appendChild(t);
	    tag = justdata.createElement( "Nodes" );
	    gpotag.appendChild(tag);
	    jdata = rng->pJustData->m_pPhaseCorrection->SerializeNodes();
	    t = justdata.createTextNode(jdata);
	    tag.appendChild(t);
	    
	    gpotag = justdata.createElement( "Offset" );
	    rtag.appendChild(gpotag);    
	    tag = justdata.createElement( "Coefficients" );
	    gpotag.appendChild(tag);
	    jdata = rng->pJustData->m_pOffsetCorrection->SerializeCoefficients();
	    t = justdata.createTextNode(jdata);
	    tag.appendChild(t);
	    tag = justdata.createElement( "Nodes" );
	    gpotag.appendChild(tag);
	    jdata = rng->pJustData->m_pOffsetCorrection->SerializeNodes();
	    t = justdata.createTextNode(jdata);
	    tag.appendChild(t);
	}
    }
    
    QString xml = justdata.toString();
    QString filename = pCmdInterpreter->m_pParser->GetKeyword(&s); // holt den parameter aus dem kommando
    filename+=".xml";
    
    QFile file(filename);
    if ( file.open( IO_WriteOnly ) ) {
	QTextStream stream( &file );
	stream << xml;
	file.close();
	Answer = ACKString;
	return Answer.latin1();
    }
    if DEBUG1 syslog(LOG_ERR,"justdata export, could not open xml file\n");
    Answer = ERRPATHString;
    return Answer.latin1();
}
 

const char* cWM3000uServer::mEEProm2JustData(char* s) {
     if (pCmdInterpreter->m_pParser->GetChar(&s) ) { // schau mal nach ob noch was kommt
	Answer = NACKString; // not acknowledge 	    
	return Answer.latin1();
    }
    if (ReadJustData()) Answer = ACKString;
    else Answer = ERRMMEMString;
    return Answer.latin1();
}
 

const char* cWM3000uServer::mJustData2EEProm(char* s) {
     if (pCmdInterpreter->m_pParser->GetChar(&s) ) { // schau mal nach ob noch was kommt
	Answer = NACKString; // not acknowledge 	    
	return Answer.latin1();
    }
     if ( !EEPromAccessEnable() ){
	 Answer = ERRAUTString; // nicht erlaubt
	 return Answer.latin1();
    }
     
    uint count=0;
    Q_UINT16 chksum=0;
    QByteArray ba;

    QDataStream stream(ba,IO_ReadWrite);
    
    stream << count;
    stream << chksum; // checksumme
    
    // ab version v1.02
    stream << "ServerVersion";
    stream << ServerVersion;
    
    stream << LeiterkartenName; // leiterkarten name
    stream << sDeviceVersion.latin1(); // versionsnummer
    stream << sSerialNumber.latin1(); // seriennummer
    stream << DateTime.toString(Qt::TextDate).latin1(); // datum,uhrzeit
    for (QStringList::iterator it=MeasChannelList.begin(); it !=MeasChannelList.end(); it++) {
	stream << (*it).latin1(); // kanalname
	QStringList* sl=ChannelRangeListMap.find(*it).data(); 
	count = sl->count();
	stream << count;
	QStringList::Iterator it3;
	for ( it3 = sl->begin(); it3 != sl->end(); ++it3 ) { // für alle bereiche 
	    sRange* rng = SearchRange(*it,*it3);
	    stream << rng->RName; // bereich
	    rng->pJustData->Serialize(stream); // justagedaten (koeffizienten, nodes und status)
	}
    }	
    
    count = ba.count(); // um die länge zu bestimmen
    QByteArray ca(6); // qbyte array mit 6 bytes
    
    QDataStream castream( ca, IO_WriteOnly );
    castream << count << chksum;
	
    QBuffer mem(ba);
    mem.open(IO_ReadWrite);
    mem.at(0); // qbuffer auf den anfang positionieren
    mem.writeBlock(ca); // überschreibt die länge + checksumme (noch 0)
	
    chksum = qChecksum(ba.data(),ba.size()); // +crc-16
    QDataStream castream2( ca, IO_WriteOnly );
    castream2 << count << chksum;
	
    mem.at(0); 
    mem.writeBlock(ca); // überschreibt die länge und jetzt die richtige checksumme
	
    // ein datenblock mit justagedaten für 1 kanal liegt in qbuffer mem 
	
    mem.close(); // wird nicht mehr benötigt
   
    c24LC256* Flash=new c24LC256(sI2CDevNode,DebugLevel,I2CEEPromAdress);
    int written = Flash->WriteData(ba.data(),ba.size(),0);
    if ( (count - written) > 0) {
	if DEBUG1 syslog(LOG_ERR,"error writing flashmemory\n");
	Answer = ERRMMEMString; // fehler beim flash schreiben
    }
    
    else
    {
	ReadJustData();
	Answer = ACKString;
    }
    
    return Answer.latin1();
}


const char* cWM3000uServer::mGetEEpromEnable() {
    if ( EEPromAccessEnable() )
	Answer="1";
    else
	Answer="0";
    return Answer.latin1();
}


const char* cWM3000uServer::mGetEEPrChksum() {
    Answer = QString("0x%1").arg(m_nChksumFlash,0,16);
    return Answer.latin1();
}


const char* cWM3000uServer::mGetPSamples() {
    char PAR[1];
    struct hw_cmd CMD = { cmdcode: hwGetSRate, device: 0, par: PAR, plen: 0,cmdlen: 0,cmddata: 0, RM:0 };
    Answer = ERREXECString;
    if ( ( I2CWriteCommand(&CMD) == 3) && (CMD.RM == 0)) {
	char answ[3];
	if (I2CReadOutput(answ,3) == 3) {
	    short ps =0;
	    for (int i = 0; i < 2; i++)
		ps = (ps << 8) + answ[i];
	    Answer = QString::number(ps);
	    return Answer.latin1();
	}
    }
    return Answer.latin1();
}	


const char* cWM3000uServer::mGetSyncPeriod() {
    char PAR[1];
    struct hw_cmd CMD = { cmdcode: hwGetPPSSync, device: 0, par: PAR, plen: 0,cmdlen: 0,cmddata: 0, RM:0 };
    Answer = ERREXECString;
    if ( ( I2CWriteCommand(&CMD) == 5) && (CMD.RM == 0)) {
	char answ[5];
	if (I2CReadOutput(answ,5) == 5) {
	    ulong PPSPar = 0;
	    for (int i = 0; i < 4; i++)
		PPSPar = (PPSPar << 8) + answ[i];
	    float p = (PPSPar & 0xFFFFFFF) / 1.0e5;  // 28 bit 10ns -> 1ms
	    PPSPar = (ulong) p; 
	    Answer = QString::number(PPSPar);
	    return Answer.latin1();
	}
    }
    return Answer.latin1();    
}


const char* cWM3000uServer::mGetSyncSource() {
    char PAR[1];
    struct hw_cmd CMD = { cmdcode: hwGetPPSSync, device: 0, par: PAR, plen: 0,cmdlen: 0,cmddata: 0, RM:0 };
    Answer = ERREXECString;
    if ( ( I2CWriteCommand(&CMD) == 5) && (CMD.RM == 0)) {
	char answ[5];
	if (I2CReadOutput(answ,5) == 5) {
	    ulong PPSPar = answ[0];
	    PPSPar = (PPSPar >> 7) & 1;
	    Answer = QString::number(PPSPar);
	    return Answer.latin1();
	}
    }
    return Answer.latin1();    
}


const char* cWM3000uServer::mGetSampleMode() {
    char PAR[1];
    struct hw_cmd CMD = { cmdcode: hwGetMode, device: 0, par: PAR, plen: 0,cmdlen: 0,cmddata: 0, RM:0 };
    Answer = ERREXECString;
    if ( ( I2CWriteCommand(&CMD) == 2) && (CMD.RM == 0)) {
	char answ[2];
	if (I2CReadOutput(answ,2) == 2) {
	    int pm = answ[0];
	    Answer = QString::number(pm);
	    return Answer.latin1();
	}
    }
    return Answer.latin1();
}
    

const char* cWM3000uServer::mGetSampleFrequency() {
    char PAR[1];
    struct hw_cmd CMD = { cmdcode: hwGetFrequency, device: 0, par: PAR, plen: 0,cmdlen: 0,cmddata: 0, RM:0 };
    if ( ( I2CWriteCommand(&CMD) == 5) && (CMD.RM == 0)) {
	char answ[5];
	if (I2CReadOutput(answ,5) == 5) {
	    ulong FPar = 0;
	    for (int i = 0; i < 4;i++) 
		FPar = (FPar << 8) + answ[i];
	    float fr = FPar / 1.0e6;
	    Answer = QString::number(fr,'f',6);
	    return Answer.latin1();
	}
    }
    return Answer.latin1();    
}


const char* cWM3000uServer::mSetSerialNumber(char* s) {
    QString spar = pCmdInterpreter->m_pParser->GetKeyword(&s); // holt den parameter aus dem kommando   
    int lplen = spar.length();
    if ( (lplen<1) || (lplen>24) ) Answer = NACKString;
    else
    {
	struct hw_cmd CMD = { cmdcode: hwSetSerialNr, device: 0, par: spar.latin1(), plen: lplen,cmdlen: 0,cmddata: 0, RM:0 };
	if ( (I2CWriteCommand(&CMD) == 0) && (CMD.RM == 0) )
	{
	    Answer = ACKString;
	    sSerialNumber = spar;
	}
	else
	    Answer = ERREXECString;
    }
    return Answer.latin1();
}


const char* cWM3000uServer::mControlerMemUpdate(bl_cmdcode blwriteCmd,char* s)
{
    bool execError  = false;
    QString filename = pCmdInterpreter->m_pParser->GetKeyword(&s); // holt den parameter aus dem kommando       
    cIntelHexFileIO IntelHexData;
    if (IntelHexData.ReadHexFile(filename))
    {
	if ( true  ) // bootloader läuft hoffentlich schon ...
	{
	    int dlen;
	    char PAR[1];
	    struct bl_cmd blInfoCMD = {cmdcode: blReadInfo, par:  PAR, plen: 0, cmdlen: 0, cmddata: 0, RM:  0};
	    dlen = I2CBootloaderCommand(&blInfoCMD);
	    if ( (dlen > 5) && (blInfoCMD.RM == 0) ) // es müssen mindestens 6 zeichen sein
	    { // 5 bytes rückmeldung und kein fehler
		dlen++; // länge bezieht sich nur auf daten, wir wollen auch das crc-byte
		char blInput[255];
		int read = I2CReadOutput(blInput, dlen);
		if ( read == dlen)
		{ // wir haben die benötigten informationen vom bootloader
		    blInfo BootloaderInfo;
		    char* dest = (char*) &BootloaderInfo;
		    int pos = strlen(blInput);
		    int i;
		    for (i = 0; i < 4; i++)
			dest[i ^ 1] = blInput[pos+1+i]; // little endian ... big endian
		    dest[i] = blInput[pos+1+i];
		    ulong MemAdress = 0;
		    ulong MemOffset;
		    QByteArray MemByteArray;
		    IntelHexData.GetMemoryBlock( BootloaderInfo.MemPageSize, MemAdress, MemByteArray, MemOffset);
		    while ( (MemByteArray.count()) && !(execError) ) // solange wir noch daten aus hexfile bekommen
		    {
			char* adrParameter;
			int adrParLen = BootloaderInfo.AdressPointerSize;
			adrParameter = GenAdressPointerParameter(adrParLen, MemAdress); 
			struct bl_cmd blAdressCMD = {cmdcode: blWriteAddressPointer, par: adrParameter, plen: adrParLen, cmdlen: 0, cmddata: 0, RM: 0};
			if ( (I2CBootloaderCommand(&blAdressCMD) == 0) && (blAdressCMD.RM == 0) )
			{ // wir sind die adresse los geworden
			    char* memdat = MemByteArray.data();
			    short memlen = MemByteArray.count();
			    struct bl_cmd blwriteMemCMD = {cmdcode: blwriteCmd, par: memdat, plen: memlen, cmdlen: 0, cmddata: 0, RM: 0};
			    if ( (I2CBootloaderCommand(&blwriteMemCMD) == 0) && (blwriteMemCMD.RM == 0) )
			    { // wir sind die daten los geworden und unterstellen dass die daten im flash sind wenn sie über die i2c schnittstelle korrekt übertragen wurden
				MemAdress += BootloaderInfo.MemPageSize;
				IntelHexData.GetMemoryBlock( BootloaderInfo.MemPageSize, MemAdress, MemByteArray, MemOffset); // versuch weitere daten aus hexfile zu lesen
			    }
			    else
			    {
				execError = true;
			    }
			}
			else
			{
			    execError = true;
			}
		    }
		}
		else
		{
		    // fehler sind wenn debug gesetzt im syslog
		    execError = true;
		}
	    }
	    else
	    {
		execError = true;
	    }
	}
	else
	{
	    // fehler sind wenn debug gesetzt im syslog
	    execError = true;
	}
    }
    else
    {
	// ??? fehlermeldungen von cIntelHexFileIO rausnudeln ???
	execError = true;
    }
    
    if (execError)
	Answer = ERREXECString;
    else
	Answer = ACKString;
    
    return Answer.latin1();
}


const char* cWM3000uServer::mControlerFlashUpdate(char* s)
{
    return mControlerMemUpdate(blWriteFlashBlock, s);
}


const char* cWM3000uServer::mControlerEEpromUpdate(char* s)
{
    return mControlerMemUpdate(blWriteEEPromBlock, s);
}


const char* cWM3000uServer::mControlerStartBootloader(char* s)
{
     if (pCmdInterpreter->m_pParser->GetChar(&s) ) { // schau mal nach ob noch was kommt
	Answer = NACKString; // not acknowledge 	    
	return Answer.latin1();
    }
     
     char PAR[1];
     struct hw_cmd CMD = { cmdcode: hwStartBootloader, device: 0, par: PAR, plen: 0,cmdlen: 0,cmddata: 0, RM:0 };
     if ( (I2CWriteCommand(&CMD) == 0) && (CMD.RM == 0) ) // bootloader gestartet ...
	 Answer = ACKString;
     else
	 Answer = ERREXECString;
    
    return Answer.latin1();
}


const char* cWM3000uServer::mControlerStartProgram(char* s)
{
    if (pCmdInterpreter->m_pParser->GetChar(&s) ) { // schau mal nach ob noch was kommt
	Answer = NACKString; // not acknowledge 	    
	return Answer.latin1();
    }
 
    char PAR[1];
    struct bl_cmd blStartProgramCMD = {cmdcode: blStartProgram, par:  PAR, plen: 0, cmdlen: 0, cmddata: 0, RM:  0};
    if ( (I2CBootloaderCommand(&blStartProgramCMD) != 0) || (blStartProgramCMD.RM) )	
    	Answer = ERREXECString;
    else
	Answer = ACKString;
    
    return Answer.latin1();
}


const char* cWM3000uServer::mGetText(hw_cmdcode hwcmd) {
    char PAR[1];
    int rlen;
    struct hw_cmd CMD = { cmdcode: hwcmd, device: 0, par: PAR, plen: 0,cmdlen: 0,cmddata: 0, RM:0 };
    if ( ( (rlen = I2CWriteCommand(&CMD)) > 0) && (CMD.RM == 0)) {
	char answ[rlen];
	if (I2CReadOutput(answ,rlen) == rlen) {
	    answ[rlen-1] = 0;
	    Answer = answ;
	    return Answer.latin1();
	}
    }
    return("Unknown");    
}


const char* cWM3000uServer::mGetSerialNumber() {
    return mGetText(hwGetSerialNr);
}


const char* cWM3000uServer::mSetPCBVersion(char* s) {
   QString spar = pCmdInterpreter->m_pParser->GetKeyword(&s); // holt den parameter aus dem kommando   
    int slen = spar.length();
    if ( (slen<1) || (slen>24) ) Answer = NACKString;
    else
    {
	struct hw_cmd CMD = { cmdcode: hwSetPCBVersion, device: 0, par: spar.latin1(), plen: slen,cmdlen: 0,cmddata: 0, RM:0 };
	if ( (I2CWriteCommand(&CMD) == 0) && (CMD.RM == 0) )
	    Answer = ACKString;
	else
	    Answer = ERREXECString;
    }
    return Answer.latin1();    
}


const char* cWM3000uServer::mGetPCBVersion() {
    return mGetText(hwGetPCBVersion);
};


const char* cWM3000uServer::mGetCTRLVersion() {
    return mGetText(hwGetCtrlVersion);
}


const char* cWM3000uServer::mGetLCAVersion() {
    return mGetText(hwGetLCAVersion);
}


const char* cWM3000uServer::mGetI2CMasterAdress() {
    Answer = QString::number(I2CMasterAdr);
    return Answer.latin1();      
}


const char* cWM3000uServer::mGetI2CSlaveAdress() {
    Answer = QString::number(I2CSlaveAdr);
    return Answer.latin1();          
}


const char* cWM3000uServer::mGetI2CDeviceNode() {
    return sI2CDevNode.latin1();      
}
 

const char* cWM3000uServer::mSetDebugLevel(char* s) {
    bool ok;
    QString par = pCmdInterpreter->m_pParser->GetKeyword(&s); // holt den parameter aus dem kommando
    int dl=par.toInt(&ok);
    
    if ( (ok) && (dl>=0) & (dl<=MaxDebugLevel) ) {
	DebugLevel=dl;
	Answer = ACKString; // acknowledge
    }
    else Answer = ERRVALString; // fehler wert
    return Answer.latin1();
}


const char* cWM3000uServer::mGetDebugLevel() {
    Answer = QString::number(DebugLevel);
    return Answer.latin1();          
}
 

const char* cWM3000uServer::mGetDeviceVersion() {
     hw_cmdcode command[4] = {hwGetDevName, hwGetPCBVersion, hwGetLCAVersion, hwGetCtrlVersion};
    QString vs[4];
    for (int i = 0; i < 4; i++) 
	vs[i] = mGetText(command[i]);
    Answer = QString ("DEVICE: %1;PCB: %2;LCA: %3;CTRL: %4").arg(vs[0]).arg(vs[1]).arg(vs[2]).arg(vs[3]);
    return Answer.latin1();
}
 

const char* cWM3000uServer::mGetServerVersion() {
    return sSoftwareVersion.latin1();
}


const char* cWM3000uServer::mGetAdjustmentStatus() {
    bool adjusted = true;
    int adj;
    for (QStringList::iterator it=MeasChannelList.begin(); it !=MeasChannelList.end(); it++) {	   		QStringList* sl=ChannelRangeListMap.find(*it).data(); 
	QStringList::Iterator it3;
	for ( it3 = sl->begin(); it3 != sl->end(); ++it3 ) {
	    sRange* rng = SearchRange(*it,*it3);
	    adjusted = adjusted && ( (rng -> pJustData -> getStatus() & (RangeGainJustified + RangePhaseJustified))  == (RangeGainJustified + RangePhaseJustified) );
	}
    }
    
    if (adjusted)
	adj = 0;
    else
	adj = 1;
    
    adj += m_nJDataStat;
    
    Answer = QString("%1").arg(adj);
    
    return Answer.latin1();
}

const char* cWM3000uServer::mGetDeviceStatus() {
    if (Test4HWPresent()) Answer = "avail";
    else Answer = "not avail";
    return Answer.latin1();
}


const char* cWM3000uServer::mGetChannelStatus() {
    QString  dedicatedChannel = pCmdInterpreter->dedicatedList.first();
    char dnr = (dedicatedChannel == "ch0") ? 1 : 2;
    
    char PAR[1];
    char answ[2];
    struct hw_cmd CMD = { cmdcode: hwGetStatus, device: dnr, par: PAR, plen: 0,cmdlen: 0,cmddata: 0, RM:0 };
    if  ( !( (I2CWriteCommand(&CMD) == 2) && (CMD.RM == 0) &&  (I2CReadOutput(answ,2) == 2)) ) {
	Answer = ERREXECString;
	return Answer.latin1();
    }
    
    Answer = QString::number(answ[0]);
    return Answer.latin1();
}


const char* cWM3000uServer::mOutCValueCatalog() {
    Answer="";
    QTextOStream* os=new QTextOStream(&Answer);
    for (QStringList::iterator it = CValueList.begin(); it != CValueList.end(); ++it) *os << *it << ";";
    return Answer.latin1();
}


const char* cWM3000uServer::mGetCValueCCoefficientName() {
//    QString  dedicatedChannel = pCmdInterpreter->dedicatedList.first();
    pCmdInterpreter->dedicatedList.pop_front();
    QString dedicatedCValue = pCmdInterpreter->dedicatedList.first();
    
/*    if (dedicatedChannel == "ch0")  // ist hier für alle kanäle gleich
    {*/
 
    if (dedicatedCValue == "CAMPLITUDE")
	Answer = "GCC(1)";
    if (dedicatedCValue == "CPHASE")
	Answer = "PCC(3)";
    if (dedicatedCValue == "COFFSET")
	Answer = "OCC(0)";
    
    //    }
    
/*  
	if (dedicatedChannel == "ch1") 
    {
	switch (dedicatedCValue)
	{
	    case "CAMPLITUDE" : Answer = "GCC(1)";
		break;
	    case "CPHASE" : Answer = "PCC(3)";
		break;
	    case "OFFSET" : Answer = "OCC(0)";
		break;
	}
    }
*/
	return Answer.latin1();
}


const char* cWM3000uServer::mGetCValueCNodeName() {
//    QString  dedicatedChannel = pCmdInterpreter->dedicatedList.first();
    pCmdInterpreter->dedicatedList.pop_front();
    QString dedicatedCValue = pCmdInterpreter->dedicatedList.first();
    
/*    if (dedicatedChannel == "ch0")  // ist hier für alle kanäle gleich
    {*/
    if (dedicatedCValue == "CAMPLITUDE")
	Answer = "GCN(1)";
    if (dedicatedCValue == "CPHASE")
	Answer = "PCN(3)";
    if (dedicatedCValue == "COFFSET")
	Answer = "OCN(0)";

//    }
    
/*  
	if (dedicatedChannel == "ch1") 
    {
	switch (dedicatedCValue)
	{
	    case "CAMPLITUDE" : Answer = "GCN(1)";
		break;
	    case "CPHASE" : Answer = "PCN(3)";
		break;
	    case "OFFSET" : Answer = "OCN(0)";
		break;
	}
    }
*/    
    return Answer.latin1();
}


bool cWM3000uServer::EEPromAccessEnable() { 
    char PAR[1];
    struct hw_cmd CMD = { cmdcode: hwGetFlashWriteAccess, device: 0, par: PAR, plen: 0,cmdlen: 0,cmddata: 0, RM:0 };
    if ( (I2CWriteCommand(&CMD) == 2) && (CMD.RM == 0)) {
	char answ[2];
	return ( (I2CReadOutput(answ,2) == 2) && (answ[0]) );
    }
    return(false);
}


void cWM3000uServer::AddChannelClient(QString& s) { // fügt einen client hinzu nach  open
    tChannelSockListMap::iterator it=ChannelSockListMap.find(s);
    tSockList *sl=it.data(); //  socketliste
    tSockList::iterator is=sl->find(ActSock); // hat der client schon geöffnet
    if ( is == sl->end() ) {
	if DEBUG3 syslog(LOG_INFO,"client %d opened channel %s",ActSock,s.latin1());
	sl->append(ActSock); // nein -> tut er es jetzt
    }
    return;
}


void cWM3000uServer::DelChannelClient(QString& s) { //  entfernt einen client nach close
    tChannelSockListMap::iterator it=ChannelSockListMap.find(s);
    if DEBUG3 syslog(LOG_INFO,"client %d closed channel %s",ActSock,s.latin1());
    it.data()->remove(ActSock);
    return;
}


void cWM3000uServer::DelClient(int s) { // entfernt einen client
    for ( cZHClient* client = clientlist.first(); client; client = clientlist.next() ) {
	if ((client->sock) == s) {
	    clientlist.remove(client);
	    break;
	}
    }
    ActSock=s; // damit DelChannelClient weiß um welchen socket es geht
    for (QStringList::iterator it=MeasChannelList.begin(); it !=MeasChannelList.end(); it++) DelChannelClient(*it);
    if DEBUG3 syslog(LOG_INFO,"client %d deleted\n",s);
}


bool cWM3000uServer::Test4HWPresent() {
    return( sSerialNumber != "Unknown");
}


const char* cWM3000uServer::mSetCValueCCoefficient(char* s)
{
bool ok;
    QString par = pCmdInterpreter->m_pParser->GetKeyword(&s); // holt den parameter aus dem kommando
    double coeff = par.toDouble(&ok);
    
    if (ok) {
	if ( EEPromAccessEnable() ) {
	    QString dedicatedChannel = pCmdInterpreter->dedicatedList.first();
	    pCmdInterpreter->dedicatedList.pop_front();
	    QString dedicatedRange = pCmdInterpreter->dedicatedList.first();
	    pCmdInterpreter->dedicatedList.pop_front();
	    QString dedicatedPhaseCoefficient = pCmdInterpreter->dedicatedList.first();
	    sRange* ranges=SearchRange(dedicatedChannel,dedicatedRange);
	    char cc = dedicatedPhaseCoefficient[0].latin1();
	    int ord = (dedicatedPhaseCoefficient.right(1)).toInt();
	    switch (cc)
	    {
	    case 'G':
		ranges->pJustData->m_pGainCorrection->setCoefficient(ord, coeff);
		break;
	    case 'P':
		ranges->pJustData->m_pPhaseCorrection->setCoefficient(ord, coeff);
		break;
	    case 'O':
		ranges->pJustData->m_pOffsetCorrection->setCoefficient(ord, coeff);
		break;
	    }
	    Answer = ACKString; // acknowledge
	}
	else Answer = ERRAUTString; // nicht autorisiert
    }
    else
	Answer = ERRVALString; // error value       
    return Answer.latin1();    
}


const char* cWM3000uServer::mGetCValueCCoefficient()
{
    QString dedicatedChannel = pCmdInterpreter->dedicatedList.first();
    pCmdInterpreter->dedicatedList.pop_front();
    QString dedicatedRange = pCmdInterpreter->dedicatedList.first();
    pCmdInterpreter->dedicatedList.pop_front();
    QString dedicatedCCoefficient = pCmdInterpreter->dedicatedList.first();
    sRange* ranges=SearchRange(dedicatedChannel,dedicatedRange);
    char cc = dedicatedCCoefficient[0].latin1();
    int ord = (dedicatedCCoefficient.right(1)).toInt();
    double coeff=0.0;
    bool ok = false; 	
    switch (cc)
    {
	    case 'G':
		coeff = ranges->pJustData->m_pGainCorrection->getCoefficient(ord);
		ok = true;
		break;
	    case 'P':
		coeff = ranges->pJustData->m_pPhaseCorrection->getCoefficient(ord);
		ok = true;
		break;
	    case 'O':
		coeff = ranges->pJustData->m_pOffsetCorrection->getCoefficient(ord);
		ok = true;
		break;
    }
    if (ok)	
	Answer = QString::number(coeff);
    else
	Answer = NACKString;	

    return Answer.latin1();      
}


const char* cWM3000uServer::mSetCValueCNode(char* s)
{
bool ok1, ok2;
    QString par = pCmdInterpreter->m_pParser->GetKeyword(&s); // holt den parameter aus dem kommando
    double corr = par.toDouble(&ok1);
    par = pCmdInterpreter->m_pParser->GetKeyword(&s);
    double arg = par.toDouble(&ok2);
    
    if (ok1 && ok2) {
	if ( EEPromAccessEnable() ) {
	    QString dedicatedChannel = pCmdInterpreter->dedicatedList.first();
	    pCmdInterpreter->dedicatedList.pop_front();
	    QString dedicatedRange = pCmdInterpreter->dedicatedList.first();
	    pCmdInterpreter->dedicatedList.pop_front();
	    QString dedicatedCNode = pCmdInterpreter->dedicatedList.first();
	    sRange* ranges=SearchRange(dedicatedChannel,dedicatedRange);
	    char cc = dedicatedCNode[0].latin1();
	    int ord = (dedicatedCNode.right(1)).toInt();
	    switch (cc)
	    {
	    case 'G':
		ranges->pJustData->m_pGainCorrection->setNode(ord, cJustNode(corr, arg));
		break;
	    case 'P':
		ranges->pJustData->m_pPhaseCorrection->setNode(ord, cJustNode(corr, arg));
		break;
	    case 'O':
		ranges->pJustData->m_pOffsetCorrection->setNode(ord, cJustNode(corr, arg));
		break;
	    }
	    Answer = ACKString; // acknowledge
	}
	else Answer = ERRAUTString; // nicht autorisiert
    }
    else
	Answer = ERRVALString; // error value       
    return Answer.latin1();        
}


const char* cWM3000uServer::mGetCValueCNode()
{
    QString dedicatedChannel = pCmdInterpreter->dedicatedList.first();
    pCmdInterpreter->dedicatedList.pop_front();
    QString dedicatedRange = pCmdInterpreter->dedicatedList.first();
    pCmdInterpreter->dedicatedList.pop_front();
    QString dedicatedCCoefficient = pCmdInterpreter->dedicatedList.first();
    sRange* ranges=SearchRange(dedicatedChannel,dedicatedRange);
    char cc = dedicatedCCoefficient[0].latin1();
    int ord = (dedicatedCCoefficient.right(1)).toInt();
    cJustNode* jd=NULL;
    switch (cc)
    {
	    case 'G':
		jd = ranges->pJustData->m_pGainCorrection->getNode(ord);
		break;
	    case 'P':
		jd = ranges->pJustData->m_pPhaseCorrection->getNode(ord);
		break;
	    case 'O':
		jd = ranges->pJustData->m_pOffsetCorrection->getNode(ord);
		break;
    }
    if (jd) 
    	Answer = jd->Serialize(); 
    else
	Answer =NACKString;

    return Answer.latin1();          
}


const char* cWM3000uServer::mCmpCCoefficient(char*)
{
   QString dedicatedChannel = pCmdInterpreter->dedicatedList.first();
   
   tChannelListMap::Iterator it=ChannelRangeListMap.find(dedicatedChannel);
   QStringList* rl=it.data();
   int nRng=rl->count(); // wir haben die anzahl der bereiche zu diesem kanal
    
    tChannelRangeArrayMap::Iterator it2=ChannelRangeArrayMap.find(dedicatedChannel);
    sRange* ranges=it2.data();
 
    pCmdInterpreter->dedicatedList.pop_front();
    QString dedicatedCValue = pCmdInterpreter->dedicatedList.first();
    
    for (int i = 0;  i < nRng; i++, ranges++)
    {
	if (dedicatedCValue == "CAMPLITUDE")
	    ranges->pJustData->m_pGainCorrection->cmpCoefficients();
	if (dedicatedCValue == "CPHASE")
	    ranges->pJustData->m_pPhaseCorrection->cmpCoefficients();
	if (dedicatedCValue == "COFFSET")
	    ranges->pJustData->m_pOffsetCorrection->cmpCoefficients();
    }
    Answer = ACKString; // acknowledge
    return Answer.latin1();
}


const char* cWM3000uServer::mSetStatus(char* s) {
    bool ok;
    QString par = pCmdInterpreter->m_pParser->GetKeyword(&s); // holt den parameter aus dem kommando
    int st=par.toInt(&ok);
    if (ok) {
	if ( EEPromAccessEnable() ) {
	    QString dedicatedChannel = pCmdInterpreter->dedicatedList.first();
	    pCmdInterpreter->dedicatedList.pop_front();
	    QString dedicatedRange = pCmdInterpreter->dedicatedList.first();
	    sRange* ranges=SearchRange(dedicatedChannel,dedicatedRange);
	    ranges->pJustData->setStatus(st);
	    Answer = ACKString; // acknowledge
	}
	else Answer = ERRAUTString; // nicht autorisiert
    }
    else Answer = ERRVALString; // error value    
    return Answer.latin1();
}
 

const char* cWM3000uServer::mGetStatus() {
    QString dedicatedChannel = pCmdInterpreter->dedicatedList.first();
    pCmdInterpreter->dedicatedList.pop_front();
    QString dedicatedRange = pCmdInterpreter->dedicatedList.first();
    sRange* ranges=SearchRange(dedicatedChannel,dedicatedRange);
    int st=ranges->pJustData->getStatus();
    Answer = QString::number(st);
    return Answer.latin1();      
}


const char* cWM3000uServer::mGetCValue(char* s) { // abfrage des korrekturwertes (ev. mit parameter)
    bool ok;
    QString par = pCmdInterpreter->m_pParser->GetKeyword(&s); // holt den parameter aus dem kommando
    QString dedicatedChannel = pCmdInterpreter->dedicatedList.first();
    pCmdInterpreter->dedicatedList.pop_front();
    QString dedicatedRange = pCmdInterpreter->dedicatedList.first();
    sRange* rangeSense=SearchRange(dedicatedChannel,dedicatedRange);
    pCmdInterpreter->dedicatedList.pop_front();
    QString dedicatedsCValue = pCmdInterpreter->dedicatedList.first();
    if (dedicatedsCValue == "CAMPLITUDE")
    {
	int samples = QString(mGetPSamples()).toInt();
	QString s;
	s = (samples == 80) ? "ADW80" : "ADW256";
	sRange* rangeADW=SearchRange(dedicatedChannel,s);
	double ampl = par.toDouble(&ok);
	if (ok) {
	    Answer = QString::number(rangeADW->pJustData->m_pGainCorrection->getCorrection(ampl) * rangeSense->pJustData->m_pGainCorrection->getCorrection(ampl)); // acknowledge
	}
	else 
	    Answer = ERRVALString; // error value
    }
    else
    if (dedicatedsCValue == "CPHASE")
    {
	// phi = delay * signalfreq * 360°
	// delay = 12.288Mhz * 2 * 283,2uS / mclk
	// mclk = samplefreq * psamples (80 od. 256) * (2 od. 8) * 256
	// -> phi = 12.288 * 2 * 283.2 * 360.0 * signalfreq/ (256 * (512 od. 640) * samplefreq)
	double f = par.toDouble(&ok);
	if (ok) {
	    int samples = QString(mGetPSamples()).toInt();
	    double pkADW = (samples == 80) ? 1/640.0 : 1/512.0;
	    // pkADW = -(2.25 + (pkADW * 12.288 * 566.4 * 360.0 * f / (256.0 * SampleFrequency)));
	    // die 2.25° sind auch frequenzabhängig -> deshalb 
	    // pkADW = -( (2.25*256.0 + pkADW * 12.288 * 566.4 * 360.0) * f / (256.0 * SampleFrequency));	
	    // es sind 360.0 / (80 * 2)  bzw. 360 / ( 256 * 2) -> 360/ 2.0*samples 
	    pkADW = -( ( (360.0*256.0 / (2.0*samples)) + pkADW * 12.288 * 566.4 * 360.0) * f / (256.0 * SampleFrequency));
	    double pkSense = rangeSense->pJustData->m_pPhaseCorrection->getCorrection(f);
	    Answer = QString::number(pkSense+pkADW);
	}
	else 
	    Answer = ERRVALString; // error value
    }
    else
    if (dedicatedsCValue == "COFFSET")
    {
	Answer = QString::number(rangeSense->pJustData->m_pOffsetCorrection->getCorrection(1.0)); // parameter dummy
    }
    
    return Answer.latin1();
}


const char* cWM3000uServer::mGetRejection() {
    QString dedicatedChannel = pCmdInterpreter->dedicatedList.first();
    pCmdInterpreter->dedicatedList.pop_front();
    QString dedicatedRange = pCmdInterpreter->dedicatedList.first();
    sRange* ranges=SearchRange(dedicatedChannel,dedicatedRange);
    Answer = ranges->RRejection;
    return Answer.latin1();
}


const char* cWM3000uServer::mGetRValue() {
    QString dedicatedChannel = pCmdInterpreter->dedicatedList.first();
    pCmdInterpreter->dedicatedList.pop_front();
    QString dedicatedRange = pCmdInterpreter->dedicatedList.first();
    sRange* ranges=SearchRange(dedicatedChannel,dedicatedRange);
    Answer = ranges->RValue;
    return Answer.latin1();
}


const char* cWM3000uServer::mOutRangeCatalog() {
    QString dedicatedChannel = pCmdInterpreter->dedicatedList.first();
    Answer="";
    QTextOStream* os=new QTextOStream(&Answer);
    tChannelListMap::Iterator it=ChannelRangeListMap.find(dedicatedChannel);
    QStringList* sl=it.data();
    for (QStringList::Iterator it = sl->begin(); it != sl->end(); ++it) *os << *it << ";" ;
    return Answer.latin1(); 
}


const char* cWM3000uServer::mGetRange() {
    QString dedicatedChannel = pCmdInterpreter->dedicatedList.first();
    char dnr = (dedicatedChannel == "ch0") ? 1 : 2;
    char PAR[1];
    int rlen;
    struct hw_cmd CMD = { cmdcode: hwGetRange, device: dnr, par: PAR, plen: 0,cmdlen: 0,cmddata: 0, RM:0 };
    char answ[2];
    
    if ( !( ( (rlen = I2CWriteCommand(&CMD)) == 2) && (CMD.RM == 0) && (I2CReadOutput(answ,rlen) == rlen)) ) {
	Answer = ERREXECString; // error execution
	return Answer.latin1();
    }
    
    int range = answ[0]; // bereich 
    
    tChannelListMap::Iterator it=ChannelRangeListMap.find(dedicatedChannel);
    QStringList* rl=it.data();
    int nRng=rl->count();
    
    tChannelRangeArrayMap::Iterator it2=ChannelRangeArrayMap.find(dedicatedChannel);
    sRange* ranges=it2.data();
    for (int i = 0;  i < nRng; i++, ranges++) {
	if ( ranges->RSelCode == range ) {
	    Answer = ranges->RName;
	    break;
	}
    }
    return Answer.latin1();
}


const char* cWM3000uServer::mSetRange(char* s) {
    char dnr;
    
    QString dedicatedChannel = pCmdInterpreter->dedicatedList.first();
    tChannelSockListMap::iterator it=ChannelSockListMap.find(dedicatedChannel);
    tSockList *sl=it.data(); //  socketliste
    int so=sl->first();
    if ( so != ActSock ) {
	Answer = ERRAUTString;
	return Answer.latin1();;
    }
    QString rng = pCmdInterpreter->m_pParser->GetKeyword(&s); // holt den parameter aus dem kommando
    
    tChannelListMap::Iterator it2=ChannelRangeListMap.find(dedicatedChannel);
    QStringList* rl=it2.data();
    int nRng=rl->count();
    
    tChannelRangeArrayMap::Iterator it3=ChannelRangeArrayMap.find(dedicatedChannel);
    sRange* ranges=it3.data();
    sRange* sr = NULL;
    for (int i = 0;  i < nRng; i++, ranges++) {
	if ( rng == ranges->RName ) {
	    sr = ranges;
	    break;
	}
    }
    if ( !sr ||  !((sr->RSpec==rngPhys) || (sr->RSpec==rngLog))  ) { // war nicht erlaubt
	Answer = NACKString; // bereich gibt es nicht, bzw. ist kein phys. bzw. log. bereich
	return Answer.latin1();
    }
    dnr = (dedicatedChannel == "ch0") ? 1 : 2;
    
    char PAR[1];
    PAR[0] = sr->RSelCode;
    struct hw_cmd CMD = { cmdcode: hwSetRange, device: dnr, par: PAR, plen: 1,cmdlen: 0,cmddata: 0, RM:0 };
    if ( !( (I2CWriteCommand(&CMD) == 0) && (CMD.RM == 0)) ) 
	Answer = ERREXECString; // error execution
    else
	Answer = ACKString;
  
    return Answer.latin1();
}    


const char* cWM3000uServer::mGetProtection() {
    char PAR[1];
    int rlen;
    struct hw_cmd CMD = { cmdcode: hwGetSenseProt, device: 0, par: PAR, plen: 0,cmdlen: 0,cmddata: 0, RM:0 };
    char answ[2];
 
    if ( !( ( (rlen = I2CWriteCommand(&CMD)) == 2) && (CMD.RM == 0) && (I2CReadOutput(answ,rlen) == rlen)) ) {
	Answer = ERREXECString; // error execution
	return Answer.latin1();
    }   
    else
    {	
	uchar prot;
	prot = answ[0];
	Answer = QString("%1").arg(prot & 1);
    }
    return Answer.latin1();
}


const char* cWM3000uServer::mSetProtection(char* s) {
    bool ok;
    QString par = pCmdInterpreter->m_pParser->GetKeyword(&s); // holt den parameter aus dem kommando
    int prot = par.toInt(&ok);
    
    if ( (ok) && (prot > -1) && (prot < 2) ) {
	char PAR[1];
	PAR[0] = prot;
	hw_cmd CMD = { cmdcode: hwSetSenseProt, device: 0, par: PAR, plen: 1,cmdlen: 0,cmddata: 0, RM:0 };
	if ((I2CWriteCommand(&CMD) == 0) &&  (CMD.RM == 0)) 
	    Answer = ACKString; // acknowledge
	else
	    Answer = ERREXECString;
    }
    else Answer = ERRVALString;
    return Answer.latin1();        
}


const char* cWM3000uServer::mChannelClose(char* s) {
    if (pCmdInterpreter->m_pParser->GetChar(&s) ) { // schau mal nach ob noch was kommt
	Answer = NACKString; // not acknowledge 	    
    }
    else
    {
	QString dedicatedChannel = pCmdInterpreter->dedicatedList.first();
	DelChannelClient(dedicatedChannel);
	Answer = ACKString; // acknowledge 	    
    }
    return Answer.latin1();
}


const char* cWM3000uServer::mChannelOpen(char* s) {
    if (pCmdInterpreter->m_pParser->GetChar(&s) ) { // schau mal nach ob noch was kommt
	Answer = NACKString; // not acknowledge 	    
    }
    else
    {
	QString dedicatedChannel = pCmdInterpreter->dedicatedList.first();
	AddChannelClient(dedicatedChannel);
	Answer = ACKString; // acknowledge 	    
    }
    return Answer.latin1();
}


extern cNodeZHServer* CalculateCNameRName;
extern cNodeZHServer* CalculateCNameCValue2;
extern cNodeZHServer* CalculateCNameRNameCValue;
extern cNodeZHServer* CalculateCNameRNameCCoefficient;
extern cNodeZHServer* CalculateCNameRNameCNode;
extern cNodeZHServer* CalculateComputeCValue;

const char* cWM3000uServer::mSetCalculateModel() {
    QString dedicatedChannel = pCmdInterpreter->dedicatedList.first();
    
    tChannelListMap::Iterator it = ChannelRangeListMap.find(dedicatedChannel);
    QStringList* sl = it.data();
    CalculateCNameRName->SetNodeNameList(sl);
    
    it = ChannelCValueListMap.find(dedicatedChannel);
    sl = it.data();
    CalculateCNameCValue2->SetNodeNameList(sl);
    
    CalculateCNameRNameCValue->SetNodeNameList(sl);
    CalculateComputeCValue->SetNodeNameList(sl);
    
    it = ChannelCCoeffientListMap.find(dedicatedChannel);
    sl = it.data();
    CalculateCNameRNameCCoefficient->SetNodeNameList(sl);
        
    it = ChannelCNodeListMap.find(dedicatedChannel);
    sl = it.data();
    CalculateCNameRNameCNode->SetNodeNameList(sl);
    
    Answer = "Dummy";
    return Answer.latin1();
}


const char* cWM3000uServer::mOutChannelCatalog() {
    Answer="";
    QTextOStream* os=new QTextOStream(&Answer);
    for (QStringList::iterator it = MeasChannelList.begin(); it != MeasChannelList.end(); ++it) *os << *it << ";";
    return Answer.latin1();
}

int cWM3000uServer::Execute() { // server ausführen
    int sock;
    if ( (sock = socket( PF_INET, SOCK_STREAM, 0)) == -1) { //   socket holen
	if DEBUG1 syslog(LOG_ERR,"socket() failed\n"); 
	return(1);
    }
    struct servent* se;
    if ( (se=getservbyname( sServerName.latin1(),"tcp")) == NULL ) {  // holt port nr aus /etc/services
	if DEBUG1 syslog(LOG_ERR,"internet network services not found\n");
	return(1);
    }
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = INADDR_ANY; // alle adressen des host
    addr.sin_port = se->s_port; // ! s_port ist network byte order !
    addr.sin_family=AF_INET;
    if ( bind( sock, (struct sockaddr*) &addr, sizeof(addr)) == -1) { // ip-adresse und port an socket binden
	if DEBUG1 syslog(LOG_ERR,"bind() failed\n");
	return(1);
    }
    if ( listen(sock,3) == -1) { // einrichten einer warteschlange für einlaufende verbindungsaufbauwünsche
	if DEBUG1 syslog(LOG_ERR,"listen() faild\n");
	return(1);
    }
    char InputBuffer[InpBufSize];
    int nBytes;
    fd_set rfds,wfds; // menge von deskriptoren für read bzw. write
    int fd,fdmax,s;
    for (;;) {
	FD_ZERO (&rfds);  // deskriptor menge löschen
	FD_ZERO (&wfds);  
	
	fdmax=sock; // start socket
	FD_SET(sock,&rfds); 
	if ( ! clientlist.isEmpty()) for ( cZHClient* client=clientlist.first(); client; client=clientlist.next() ) {
	    fd=client->sock;
	    FD_SET(fd,&rfds); 
	    if ( client->OutpAvail() ) 
		FD_SET(fd,&wfds);  
	    if (fd>fdmax) fdmax=fd;
	}
	select(fdmax+1,&rfds,&wfds,NULL,NULL); // warte auf daten, oder das daten gesendet werden können
	
	if ( FD_ISSET(sock,&rfds) ) { // hier ggf. client hinzunehmen
	    int addrlen=sizeof(addr);
	    if ( (s=accept(sock,(struct sockaddr*) &addr, (socklen_t*) &addrlen) ) == -1 ) {
		if DEBUG1 syslog(LOG_ERR,"accept() failed\n");
	    }
	    else {
		AddClient(s,(struct sockaddr_in*) &addr);
	    }
	}
		
	if ( ! clientlist.isEmpty()) for ( cZHClient* client=clientlist.first(); client; client=clientlist.next() ) {
	    fd=client->sock;
	    if (FD_ISSET(fd,&rfds) ) { // sind daten für den client da, oder hat er sich abgemeldet ?
		if ( (nBytes=recv(fd,InputBuffer,InpBufSize,0)) > 0  ) { // daten sind da
		    bool InpRdy=false;
		    switch (InputBuffer[nBytes-1]) { // letztes zeichen
			    case 0x0d: // cr
				InputBuffer[--nBytes]=0; // c string ende daraus machen 
				InpRdy=true;
				break;
			    case 0x0a: // linefeed
				InputBuffer[--nBytes]=0; 
				if (nBytes) 
				    if (InputBuffer[nBytes-1] == 0x0d) InputBuffer[--nBytes]=0;
				InpRdy=true;
				break;
		                  case 0x04: // eof
				InputBuffer[nBytes-1]=0; // c string ende daraus machen 
				InpRdy=true;
				break;
			    case 0:
				InpRdy=true; // daten komplett und 0 terminiert
				break;
			    default:
				InputBuffer[nBytes]=0; // teil string komplettieren
		    }
		    
		    client->AddInput(&InputBuffer[0]);
		     if (InpRdy) {
			ActSock=fd; 
		              client->SetOutput(pCmdInterpreter->CmdExecute(client->GetInput())); // führt kommando aus und setzt
			client->ClearInput();
		     }
		}
		else 
		{
		    DelClient(fd); // client hat sich abgemeldet ( hab den iterator zwar, aber DelClient ist virtuell !!! ) 
		    close(fd);
		}
	    }
	}
	
	if ( ! clientlist.isEmpty()) for ( cZHClient* client=clientlist.first(); client; client=clientlist.next() ) {
	    fd=client->sock;
	    if (FD_ISSET(fd,&wfds) ) { // soll und kann was an den client gesendet werden ?
		QString out = client->GetOutput();
		out+="\n";
		// char* out=client->GetOutput();
		send(fd,out.latin1(),out.length(),0);
		client->SetOutput(""); // kein output mehr da .
	    } 
	}
    }
    close(sock);
}

void cWM3000uServer::AddClient(int s, sockaddr_in* addr) { // fügt einen client hinzu
    clientlist.append(new cZHClient(s,addr));
    if DEBUG3 syslog(LOG_INFO,"client %d added\n",s);
}


const char* cWM3000uServer::SCPICmd( SCPICmdType cmd, char* s) {
    switch ((int)cmd)	{
	
    case   SetPSamples:		return mSetPSamples(s);	
    case 	SetSampleMode:		return mSetSampleMode(s);
    case   SetSampleFrequency:	return mSetSampleFrequency(s);
    case 	SetSerialNumber: 		return mSetSerialNumber(s);
    case	ControlerStartProgram:	return mControlerStartProgram(s);
    case 	ControlerStartBootloader:	return mControlerStartBootloader(s);
    case	ControlerFlashUpdate:	return mControlerFlashUpdate(s);
    case 	ControlerEEpromUpdate:	return mControlerEEpromUpdate(s);
    case  	File2Justdata:		return mFile2Justdata(s);	     	     
    case   JustData2File:		return mJustData2File(s);	     	     
    case 	EEProm2JustData:	return mEEProm2JustData(s);	     	     
    case 	JustData2EEProm:	return mJustData2EEProm(s);	     
    case 	SetDebugLevel:		return mSetDebugLevel(s);
    case 	SetCValueCCoefficient:	return mSetCValueCCoefficient(s);
    case 	SetCValueCNode:		return mSetCValueCNode(s);
    case	CmpCCoefficient:		return mCmpCCoefficient(s);
    case   SetStatus:		return mSetStatus(s);	
    case   SetRange:		return mSetRange(s);
    case	SetProtection:		return mSetProtection(s);
    case   ChannelClose:		return mChannelClose(s);
    case 	ChannelOpen:		return mChannelOpen(s);
    case   SetSyncSource:		return mSetSyncSource(s);
    case	SetSyncPeriod:		return mSetSyncPeriod(s);	
    case   SetPCBVersion:		return mSetPCBVersion(s);
    }
    Answer = "ProgrammierFehler"; // hier sollten wir nie hinkommen
    return Answer.latin1();
}
 

const char* cWM3000uServer::SCPIQuery( SCPICmdType cmd, char* s) {
    switch ((int)cmd)	{
	
    case		GetPSamples:		return mGetPSamples();	
    case 		GetSampleMode:		return mGetSampleMode();
    case 		GetSampleFrequency:	return mGetSampleFrequency();
    case 	 	GetSerialNumber: 		return mGetSerialNumber();
    case 		GetI2CMasterAdress:	return mGetI2CMasterAdress();
    case 		GetI2CSlaveAdress:	return mGetI2CSlaveAdress();		 
    case 		GetI2CDeviceNode:	return mGetI2CDeviceNode();
    case 		GetDebugLevel:		return mGetDebugLevel();
    case 		GetDeviceVersion:		return mGetDeviceVersion();
    case 		GetServerVersion:		return mGetServerVersion();  
    case 		GetAdjustmentStatus:	return mGetAdjustmentStatus();
    case 		GetDeviceStatus:		return mGetDeviceStatus();	     
    case 		GetChannelStatus:		return mGetChannelStatus();
    case 		OutCValueCatalog:		return mOutCValueCatalog();
    case 		SetCalculateModel:	return mSetCalculateModel();
    case 		GetCValueCCoefficient:	return mGetCValueCCoefficient();
    case 		GetCValueCCoefficientName: return mGetCValueCCoefficientName();
    case 		GetCValueCNode:		return mGetCValueCNode();	
    case 		GetCValueCNodeName:	return mGetCValueCNodeName();	
    case 		GetStatus:		return mGetStatus();
    case 		GetCValue:		return mGetCValue(s);
    case 		GetRejection:		return mGetRejection();
    case 		GetRValue:		return mGetRValue();
    case 		OutRangeCatalog:		return mOutRangeCatalog();
    case 		GetRange:		return mGetRange();
    case 		GetProtection:		return mGetProtection();
    case 		OutChannelCatalog:	return mOutChannelCatalog();
    case 		GetSyncSource:		return mGetSyncSource();
    case		GetSyncPeriod:		return mGetSyncPeriod();	
    case 		GetPCBVersion:		return mGetPCBVersion();
    case 		GetCTRLVersion:		return mGetCTRLVersion();
    case 		GetLCAVersion:		return mGetLCAVersion();	
    case 		GetEEPromEnable:	return mGetEEpromEnable();
    case 		GetEEPromChksum:	return mGetEEPrChksum();
    }
    Answer = "ProgrammierFehler"; // hier sollten wir nie hinkommen
    return Answer.latin1();
}
