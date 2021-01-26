#ifndef WMJUSTDATAV215_H
#define WMJUSTDATAV215_H

#include <qdatastream.h>
#include <q3textstream.h>
#include "wmjustdatabase.h"


class cWMJustDataV215:public cWMJustDataBase { // alle korrekturdaten für einen bereich + status
public:
    cWMJustDataV215();
    ~cWMJustDataV215();

    void Serialize(QDataStream&); // zum schreiben aller justagedaten in flashspeicher
    void Deserialize(QDataStream&); // zum lesen aller justagedaten aus flashspeicher

    virtual QString SerializeStatus(); // für den xml export
    virtual void DeserializeStatus(QString);

    virtual void setStatus(int);
    virtual int getStatus(); // gibt den gesamt status der justage zurück 1 = justiert 0 nicht justiert

};

#endif // WMJUSTDATAV215_H
