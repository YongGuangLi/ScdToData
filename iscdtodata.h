#ifndef ICIDTODATAH_H
#define ICIDTODATAH_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QList>

typedef struct IedData stIedData;
typedef struct PointData stPointData;
struct IedData
{
    IedData():PortA_(102),PortB_(102),Type_("6")
    {}
    int PortA_;
    int PortB_;
    QString IpA_;
    QString IpB_;
    QString Type_;     //IED类型
    QString Desc_;     //IED描述
    QString Config_;   //版本
    QString Manu_;     //制造商
    QString Model_;    //型号
};


struct PointData
{
    PointData()
    {}

    QString RedisAddr_;
    QString Name_;
    QString Desc_;               //逻辑节点描述/数据对象描述
    QString Type_;
    QString DoName_;
    QString iedName_;
    QString DataType_;      // YC or YX
};

class IScdToData : public QObject
{
    Q_OBJECT
public:
    virtual int ConvertScd2Data(QString csScdFile,QString csInitFile,QList<QString> &lstErrors) = 0;
    virtual QMap<QString,stIedData> GetIedData() = 0;
    virtual QMap<QString,QList<stPointData>> GetPointData() = 0;
};

#ifdef SCDTODATA_EXPORTS
#	define SCDTODATA_API __declspec(dllexport)
#else
#	define SCDTODATA_API __declspec(dllimport)
#endif

extern "C"
{
    SCDTODATA_API IScdToData* CreateModule(void* pIService);
    SCDTODATA_API void DeleteModule(IScdToData* pModule);
}


#endif // ICIDTODATAH_H
