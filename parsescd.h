#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QDebug>
#include <QFile>
#include <QMap>
#include <QSettings>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QList>
#include <QByteArray>
#include <QStringList>
#include <QDateTime>
#include "iscdtodata.h"

typedef struct
{
   QString name_;
   QString type_;
   QString desc_;
}stDO;

typedef struct
{
    QString name_;
    QString bType_;
    QString fc_;
    QString type_;
}stDA;

typedef struct
{
    QString name_;
    QString bType_;
    QString type_;
}stBDA;

typedef struct
{
    QString IpA_;
    QString IpB_;
}stAddress;

typedef struct
{
    QString ied_;            //所属IED
    QString desc_;         //描述
    QString lnType_;
    QMap<QString,QString> mapDOI;      //key:数据对象name   value:数据对象desc
}stLN;

typedef struct
{
    QString desc_;    //逻辑节点描述/数据对象描述
    QString type_;
    QString ied_;
    QString do_;
}stFCDA;

class ScdToData : public IScdToData
{
    Q_OBJECT
public:
    explicit ScdToData();
    QMap<QString,stIedData> GetIedData();
    QMap<QString,QList<stPointData>> GetPointData();
    int ConvertScd2Data(QString csScdFile,QString csInitFile,QList<QString> &lstErrors);
    int InitCfgFile(QString csInitFile,QList<QString> &);

public:
    void parseCommunication(QXmlStreamReader&);
    void parseSubNetwork(QXmlStreamReader&);
    void parseConnectedAP(QXmlStreamReader&);
    void parseAddress(QXmlStreamReader&);

public:
    void parseIED(QXmlStreamReader&);
    void parseAccessPoint(QXmlStreamReader&);
    void parseServer(QXmlStreamReader&);
    void parseLDevice(QXmlStreamReader&);
    void parseLN(QXmlStreamReader&);
 public:
    void parseReportControl(QXmlStreamReader&);
    void parseRptEnabled(QXmlStreamReader&);

 public:
    void parseDataTypeTemplates(QXmlStreamReader&);
    void parseLNodeType(QXmlStreamReader&);
    void parseDO(QXmlStreamReader&,QList<stDO*>&);

    void parseDOType(QXmlStreamReader&);
    void parseDA(QXmlStreamReader&,QList<stDA*>&);
    void parseSDO(QXmlStreamReader &, QList<stDA*>&);

    void parseDAType(QXmlStreamReader&);
    void parseBDA(QXmlStreamReader&,QList<stBDA*>&);
public:
    void formPointTable(QList<QString> &lstErrors);
    void initDOType(QString,QString);
    void initDAType(QString,QString);
    void writePointDataToFile(QList<QString> &lstErrors);                   //所有数据点写入文件

private:
    QString Cur_Parse_IED_Name_;
    QString Cur_Parse_LDevice_Inst_;
    QString Cur_Parse_LN_inClass_;
    QString Cur_Parse_LN_Desc_;
    QString Cur_Parse_LNodeType_id_;
    QString Cur_Parse_DOType_id_;
    QString Cur_Parse_DAType_id_;
    QString Cur_Parse_DA_Type_;
    QString Cur_Parse_DO_Name_;
    QString Cur_Parse_DO_Desc_;

    QMap<QString,stLN*> mapAllLNode;
    QMap<QString,stAddress> mapAddress;
    QMap<QString,QString> mapAllLNode_IED;
    QMap<QString,QList<stDO*>> mapLNodeType_DO;
    QMap<QString,QList<stDA*>> mapDOType_DA;
    QMap<QString,QList<stBDA*>> mapDAType_DBA;

    QStringList listAllRptCntl;

    QMap<QString,stFCDA*> mapAllFCDA;

    QMap<QString,QString> mapFCFilterType;
    QMap<QString,QString> mapIEDFilterType;

    QMap<QString,stIedData> mapIedData;
    QMap<QString,QList<stPointData>> mapPointData;
};

#endif // WORKER_H
