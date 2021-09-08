#ifndef JUSTDATAV215_H
#define JUSTDATAV215_H

#include <qdatastream.h>

#include "justdatabase.h"

class cJustDataV215:public cJustDataBase { // klasse für justage koeffizienten und stützstellen altes format
public:
    cJustDataV215(int order,double init);
    virtual ~cJustDataV215();
    virtual void Serialize(QDataStream&); // zum schreiben der justagedaten in flashspeicher
    virtual void Deserialize(QDataStream&); // reicht eine routine für koeffizienten und nodes
    virtual void setStatus(int);
    virtual int getStatus();

    virtual QString SerializeStatus();
    virtual void DeserializeStatus(QString);


private:
    int m_nStatus; // der status wird nur bei den neueren justage werten ab V2.15 verwendet
};

#endif // JUSTDATAV215_H
