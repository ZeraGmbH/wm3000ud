// implemention cWMJustData

#include <qdatastream.h>
#include <q3textstream.h>
#include "wmjustdata.h"


void cJData::Serialize(QDataStream& qds)
{
    qds << m_GainCorrection <<  m_OffsetCorrection << m_PhaseCoefficient[0] << m_PhaseCoefficient[1] <<  m_PhaseCoefficient[2] << m_PhaseCoefficient[3] << m_Status;    
}
	
void cJData::Deserialize(QDataStream& qds)
{
       qds >> m_GainCorrection >>  m_OffsetCorrection >> m_PhaseCoefficient[0] >> m_PhaseCoefficient[1] >>  m_PhaseCoefficient[2] >> m_PhaseCoefficient[3] >> m_Status;    
}


void cJData::SetDefault()
{
    m_Status=0; // nicht justiert, nicht defekt 
    m_GainCorrection=1.0;
    m_OffsetCorrection=0.0;
    for (int i=0; i<4; i++) m_PhaseCoefficient[i]=0.0;    
}


void cNInformation::Serialize(QDataStream& qds)
{
    for (int i=0; i<8; i++)
	qds << m_NInformation[i];    
}


void cNInformation::Deserialize(QDataStream& qds)
{
    for (int i=0; i<8; i++)
       qds >> m_NInformation[i];    
}


void cNInformation::SetDefault()
{
    for (int i=0; i<4; i++) {
	m_NInformation[i*2] = 10.0 + i*20.0; // 10, 30, 50, 70Hz
	m_NInformation[i*2+1] = 0.0;
    }    
}


cWMJustData::cWMJustData()
{
    m_pPhaseCorrection = new cJustData(PhaseCorrOrder, 0.0);
    m_pGainCorrection = new cJustData(GainCorrOrder, 1.0); 
    m_pOffsetCorrection =  new cJustData(OffsetCorrOrder, 0.0);
    m_nStatus = 0; // nix justiert... nix kaputt
}


cWMJustData::~cWMJustData()
{
    delete m_pGainCorrection; 
    delete m_pPhaseCorrection;
    delete m_pOffsetCorrection;
}
	    

void cWMJustData::Serialize(QDataStream& qds)  // zum schreiben aller justagedaten in flashspeicher
{
    m_pGainCorrection->Serialize(qds); 
    m_pPhaseCorrection->Serialize(qds);
    m_pOffsetCorrection->Serialize(qds);
    qds << m_nStatus;
}
 
void cWMJustData::Deserialize(QDataStream& qds) // zum lesen aller justagedaten aus flashspeicher
{
    m_pGainCorrection->Deserialize(qds); 
    m_pPhaseCorrection->Deserialize(qds);
    m_pOffsetCorrection->Deserialize(qds);
    qds >> m_nStatus;
}


QString cWMJustData::SerializeStatus()
{
    return QString("%1;").arg(m_nStatus);
}


void cWMJustData::DeserializeStatus(const QString& s)
{
    m_nStatus = s.section(';',0,0).toInt();
}


void cWMJustData::setStatus(int stat)
{
    m_nStatus = stat;
}
 

int cWMJustData::getStatus()
{
    return m_nStatus;
}
