#include "justdatav215.h"
#include "wmjustdatav215.h"


cWMJustDataV215::cWMJustDataV215()
{
    m_pGainCorrection = new cJustDataV215(3, 1.0); // 3.ordnung
    m_pPhaseCorrection = new cJustDataV215(3, 0.0); // 3.ordnung
    m_pOffsetCorrection =  new cJustDataV215(0, 0.0); // 0.te ordnung
}


cWMJustDataV215::~cWMJustDataV215()
{
    delete m_pGainCorrection;
    delete m_pPhaseCorrection;
    delete m_pOffsetCorrection;
}


void cWMJustDataV215::Serialize(QDataStream& qds)  // zum schreiben aller justagedaten in flashspeicher
{
    m_pGainCorrection->Serialize(qds);
    m_pPhaseCorrection->Serialize(qds);
    m_pOffsetCorrection->Serialize(qds);
}


void cWMJustDataV215::Deserialize(QDataStream& qds) // zum lesen aller justagedaten aus flashspeicher
{
    m_pGainCorrection->Deserialize(qds);
    m_pPhaseCorrection->Deserialize(qds);
    m_pOffsetCorrection->Deserialize(qds);
}

QString cWMJustDataV215::SerializeStatus()
{
    return QString(""); // wir haben hier keinen status
}


void cWMJustDataV215::DeserializeStatus(QString)
{
}


void cWMJustDataV215::setStatus(int)
{
}


int cWMJustDataV215::getStatus()
{
    return (m_pGainCorrection->getStatus() & m_pPhaseCorrection->getStatus() & m_pOffsetCorrection->getStatus());
}

