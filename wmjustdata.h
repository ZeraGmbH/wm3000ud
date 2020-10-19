// header datei wmjustdata.h

#ifndef WMJUSTDATA_H
#define WMJUSTDATA_H

#include <qdatastream.h>
#include <q3textstream.h>
#include "justdata.h"


enum jDataStatus { wrongVERS = 2, wrongSNR = 4};

const int GainCorrOrder = 3;
const int PhaseCorrOrder  = 3;
const int OffsetCorrOrder = 0;
	

class cJData {
public:
    cJData(){SetDefault();};
    ~cJData(){};
    void Serialize(QDataStream&);
    void Deserialize(QDataStream&);
    void SetDefault();
    
    double m_GainCorrection;
    double m_OffsetCorrection;
    double m_PhaseCoefficient[4]; // phasengang ist 3. ordnung !
    int m_Status;
};

class cNInformation { // node information
public:    
    cNInformation(){SetDefault();};
    ~cNInformation(){};
    void Serialize(QDataStream&);
    void Deserialize(QDataStream&);
    void SetDefault();
    
    double m_NInformation[8]; // phasengang ist 3. ordnung !
};

class cOldWMJustData {
public: 
    cOldWMJustData(){};
    virtual ~cOldWMJustData(){};
    
    cJData m_JData;
    cNInformation m_NInfo;
};



class cWMJustData { // alle korrekturdaten für einen bereich + status 
public:
    cWMJustData();
    ~cWMJustData();
    
    cJustData* m_pGainCorrection;
    cJustData* m_pPhaseCorrection; 
    cJustData* m_pOffsetCorrection;
    
    void Serialize(QDataStream&); // zum schreiben aller justagedaten in flashspeicher
    void Deserialize(QDataStream&); // zum lesen aller justagedaten aus flashspeicher
    
    QString SerializeStatus(); // fürs xml file
    void DeserializeStatus(const QString&);
    void setStatus(int stat);
    int getStatus();
    
private:    
    int m_nStatus;
};


#endif

