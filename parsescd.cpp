#include "parsescd.h"

ScdToData::ScdToData()
{
}

ScdToData::~ScdToData()
{
}

QMap<QString, stIedData> ScdToData::GetIedData()
{
    return mapIedData;
}

QMap<QString, QList<stPointData> > ScdToData::GetPointData()
{
    return mapPointData;
}

int ScdToData::ConvertScd2Data(QString csScdFile, QString csInitFile, QList<QString> &lstErrors)
{
    if(!InitCfgFile(csInitFile,lstErrors))
    {
        return -1;
    }

    QFile *file = new QFile(csScdFile);
    if(!file->open(QFile::ReadOnly | QFile::Text))
    {
        lstErrors.push_back(csScdFile + " open fail");
        return -1;
    }
    QXmlStreamReader xmlReader(file);
    while(!xmlReader.atEnd() && !xmlReader.hasError())
    {
        QXmlStreamReader::TokenType token = xmlReader.readNext();
        if(token == QXmlStreamReader::StartDocument)
        {
            continue;
        }
        if(token == QXmlStreamReader::StartElement)
        {
            if(xmlReader.name() == QStringLiteral("Communication"))
            {
                parseCommunication(xmlReader);
            }
            if(xmlReader.name() == QStringLiteral("IED"))
            {
                parseIED(xmlReader);
            }
            if(xmlReader.name() == QStringLiteral("DataTypeTemplates"))
            {
                parseDataTypeTemplates(xmlReader);
            }
        }
    }
    formPointTable(lstErrors);                 //把读取到的数据组成点表
    writePointDataToFile(lstErrors);        //点表数据组成所需结构
    xmlReader.clear();
    file->close();
    qDebug()<<"succed";
    return 0;
}




int ScdToData::InitCfgFile(QString csInitFile,QList<QString> &lstErrors)
{
    QFile file(csInitFile);
    if(file.open(QIODevice::ReadOnly))
    {
        QSettings *settings = new QSettings(csInitFile,QSettings::IniFormat,NULL);
        if (settings != NULL)
        {
            settings->beginGroup("FC");
            QStringList FCkeys = settings->allKeys();
            for (int i = 0 ; i < FCkeys.size(); ++i)
            {
                if (settings->value(FCkeys.at(i)).toString() == "1")
                {
                    mapFCFilterType[FCkeys.at(i)] = settings->value(FCkeys.at(i)).toString();
                }
            }
            settings->endGroup();

            settings->beginGroup("IEDTYPE");
            QStringList IEDTypekeys = settings->allKeys();
            for(int i = 0; i < IEDTypekeys.size(); ++i)
            {
                mapIEDFilterType[IEDTypekeys.at(i)] = settings->value(IEDTypekeys.at(i)).toString();
            }
            settings->endGroup();
        }
        file.close();
        return 1;
    }
   lstErrors.push_back("not find inifile");
    return -1;
}

void ScdToData::parseCommunication(QXmlStreamReader &xmlReader)
{
    while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "Communication"))
    {
        if(xmlReader.tokenType() == QXmlStreamReader::StartElement && xmlReader.name() == "SubNetwork")
        {
            parseSubNetwork(xmlReader);
        }
        xmlReader.readNext();
    }
}

void ScdToData::parseSubNetwork(QXmlStreamReader &xmlReader)
{
    while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "SubNetwork"))
    {
        if(xmlReader.tokenType() == QXmlStreamReader::StartElement && xmlReader.name() == "ConnectedAP")
        {
            parseConnectedAP(xmlReader);
        }
        xmlReader.readNext();
    }
}

void ScdToData::parseConnectedAP(QXmlStreamReader &xmlReader)
{
    Cur_Parse_IED_Name_ = xmlReader.attributes().value("iedName").toString();
    while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "ConnectedAP"))
    {
        if(xmlReader.tokenType() == QXmlStreamReader::StartElement && xmlReader.name() == "Address")
        {
            parseAddress(xmlReader);
        }
        xmlReader.readNext();
    }
}

void ScdToData::parseAddress(QXmlStreamReader &xmlReader)
{
    while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "Address"))
    {
        if(xmlReader.tokenType() == QXmlStreamReader::StartElement && xmlReader.name() == "P")
        {
            if(xmlReader.attributes().value("type").toString() == QStringLiteral("IP"))
            {
                xmlReader.readNext();
                if(mapAddress.count(Cur_Parse_IED_Name_) == 0)
                {
                    mapAddress[Cur_Parse_IED_Name_].IpA_ = xmlReader.text().toString();
                }
                else
                {
                    mapAddress[Cur_Parse_IED_Name_].IpB_ = xmlReader.text().toString();
                }
            }
        }
        xmlReader.readNext();
    }
}



void ScdToData::parseIED(QXmlStreamReader &xmlReader)
{
    Cur_Parse_IED_Name_ = xmlReader.attributes().value("name").toString();  //保存当前解析IED的name
    mapIedData[Cur_Parse_IED_Name_].IpA_ = mapAddress[Cur_Parse_IED_Name_].IpA_;
     mapIedData[Cur_Parse_IED_Name_].IpB_ = mapAddress[Cur_Parse_IED_Name_].IpB_;

    if(mapIedData[Cur_Parse_IED_Name_].IpB_.isEmpty())         //如果B网不存在，把A网的第二个字段加1
    {
        QStringList tmpList = mapIedData[Cur_Parse_IED_Name_].IpA_.split(".");
        tmpList[1]  = QString::number(tmpList.at(1).toInt() + 1);
        mapIedData[Cur_Parse_IED_Name_].IpB_ = tmpList.join(".");
    }
    if(mapIEDFilterType.count(Cur_Parse_IED_Name_.at(0)) == 1)
    {
        mapIedData[Cur_Parse_IED_Name_].Type_ = mapIEDFilterType[Cur_Parse_IED_Name_.at(0)];
    }
    mapIedData[Cur_Parse_IED_Name_].Manu_ = xmlReader.attributes().value("manufacturer").toString();
    mapIedData[Cur_Parse_IED_Name_].Model_ = xmlReader.attributes().value("type").toString();
    mapIedData[Cur_Parse_IED_Name_].Config_ = xmlReader.attributes().value("configVersion").toString();
    mapIedData[Cur_Parse_IED_Name_].Desc_ = xmlReader.attributes().value("desc").toString();

    while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "IED"))
    {
        if(xmlReader.tokenType() == QXmlStreamReader::StartElement && xmlReader.name() == "AccessPoint")
        {
            parseAccessPoint(xmlReader);
        }
        xmlReader.readNext();
    }
}

void ScdToData::parseAccessPoint(QXmlStreamReader &xmlReader)
{
    while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "AccessPoint"))
    {
        if(xmlReader.tokenType() == QXmlStreamReader::StartElement && xmlReader.name() == "Server")
        {
            parseServer(xmlReader);
        }
        xmlReader.readNext();
    }
}

void ScdToData::parseServer(QXmlStreamReader &xmlReader)
{
    while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "Server"))
    {
        if(xmlReader.tokenType() == QXmlStreamReader::StartElement && xmlReader.name() == "LDevice")
        {
            parseLDevice(xmlReader);
        }
        xmlReader.readNext();
    }
}

void ScdToData::parseLDevice(QXmlStreamReader &xmlReader)
{
    Cur_Parse_LDevice_Inst_ = xmlReader.attributes().value("inst").toString();       //保存当前解析LDevice的inst
    while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "LDevice"))
    {
        if(xmlReader.tokenType() == QXmlStreamReader::StartElement && (xmlReader.name() == "LN" || xmlReader.name() == "LN0"))
        {
            parseLN(xmlReader);
        }
        xmlReader.readNext();
    }
}

void ScdToData::parseLN(QXmlStreamReader &xmlReader)
{
    QXmlStreamAttributes attributes = xmlReader.attributes();
    QString lnClass = attributes.value("lnClass").toString();
    QString lnType = attributes.value("lnType").toString();
    QString inst = attributes.value("inst").toString();
    QString desc = attributes.value("desc").toString();
    QString prefix =  attributes.value("prefix").toString();
    stLN *pLN = new stLN();

    while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && (xmlReader.name() == "LN" || xmlReader.name() == "LN0")))
    {
        if(xmlReader.tokenType() == QXmlStreamReader::StartElement && xmlReader.name() == "DOI")
        {
            pLN->mapDOI.insert(xmlReader.attributes().value("name").toString(),xmlReader.attributes().value("desc").toString());
        }
        xmlReader.readNext();
    }
     //保存所有逻辑节点
     pLN->desc_ = desc;
     pLN->ied_ = Cur_Parse_IED_Name_;
     pLN->lnType_ = lnType;

     mapAllLNode.insert(Cur_Parse_IED_Name_ + Cur_Parse_LDevice_Inst_ + "/" + prefix + lnClass + inst,pLN);
}

void ScdToData::parseReportControl(QXmlStreamReader &xmlReader)
{
    while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "LN0"))
    {
        if(xmlReader.tokenType() == QXmlStreamReader::StartElement && xmlReader.name() == "ReportControl")
        {
            parseRptEnabled(xmlReader);
        }
        xmlReader.readNext();
    }
}

void ScdToData::parseRptEnabled(QXmlStreamReader &xmlReader)
{
    QXmlStreamAttributes attributes = xmlReader.attributes();               //保存报告控制块的属性
    QString RptCntl;
    RptCntl = Cur_Parse_IED_Name_ + Cur_Parse_LDevice_Inst_ + "/" + Cur_Parse_LN_inClass_;
    if(attributes.value("buffered") == "true")
    {
        RptCntl += "$BR$";
    }
    else
    {
        RptCntl += "$RP$";
    }
    RptCntl += attributes.value("name").toString();

    while(!(xmlReader.tokenType()== QXmlStreamReader::EndElement && xmlReader.name() == "ReportControl"))
    {
        if(xmlReader.tokenType() == QXmlStreamReader::StartElement && xmlReader.name() == "RptEnabled")
        {
            int Max = xmlReader.attributes().value("max").toInt();       //报告控制块的个数
            for(int i = 1; i <= Max; ++i)
            {
                listAllRptCntl<<RptCntl + QString("%1").arg(i,QString::number(Max).size(),10,QChar('0'));
            }
        }
        xmlReader.readNext();
    }
}

void ScdToData::parseDataTypeTemplates(QXmlStreamReader &xmlReader)
{
    while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "DataTypeTemplates"))
    {
        if(xmlReader.tokenType() == QXmlStreamReader::StartElement)
        {
            if(xmlReader.name() == "LNodeType")
            {
                parseLNodeType(xmlReader);
            }
            if(xmlReader.name() == "DOType")
            {
                parseDOType(xmlReader);
            }
            if(xmlReader.name() == "DAType")
            {
                parseDAType(xmlReader);
            }
        }
        xmlReader.readNext();
    }
}

void ScdToData::parseLNodeType(QXmlStreamReader &xmlReader)
{
    Cur_Parse_LNodeType_id_ = xmlReader.attributes().value("id").toString();   //保存当前解析LNodeType的id

    QList<stDO*> listDO;
    while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "LNodeType"))
    {
        if(xmlReader.tokenType() == QXmlStreamReader::StartElement && xmlReader.name() == "DO")
        {
            parseDO(xmlReader,listDO);
        }
        xmlReader.readNext();
    }
     mapLNodeType_DO.insert(Cur_Parse_LNodeType_id_,listDO);
}

void ScdToData::parseDO(QXmlStreamReader &xmlReader,QList<stDO*> &listDO)
{
    QXmlStreamAttributes attributes = xmlReader.attributes();

    stDO* Do = new stDO();
    Do->name_ = attributes.value("name").toString();
    Do->type_ = attributes.value("type").toString();
    Do->desc_ = attributes.value("desc").toString();
    listDO.append(Do);                                      //保存每个逻辑节点下的数据对象
}

void ScdToData::parseDOType(QXmlStreamReader &xmlReader)
{
    Cur_Parse_DOType_id_ = xmlReader.attributes().value("id").toString();      //保存当前解析DOType的id
    xmlReader.attributes().value("cdc").toString();

    QList<stDA*> listDA;
    while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "DOType"))
    {
        if(xmlReader.tokenType() == QXmlStreamReader::StartElement)
        {
            if(xmlReader.name() == "DA")
            {
                parseDA(xmlReader, listDA);
            }
            else if(xmlReader.name() == "SDO")
            {
                parseSDO(xmlReader, listDA);
            }
        }
        xmlReader.readNext();
    }
    mapDOType_DA.insert(Cur_Parse_DOType_id_, listDA);
}

void ScdToData::parseDA(QXmlStreamReader &xmlReader,QList<stDA*>& listDA)
{
    QXmlStreamAttributes attributes = xmlReader.attributes();

    stDA* DA = new stDA();
    DA->name_ = attributes.value("name").toString();
    DA->bType_ = attributes.value("bType").toString();
    DA->fc_ = attributes.value("fc").toString();
    DA->type_ = attributes.value("type").toString();
    listDA.append(DA);                                             //保存每个数据对象下的所有数据属性
}

void ScdToData::parseSDO(QXmlStreamReader &xmlReader,QList<stDA*>& listDA)
{
    QXmlStreamAttributes attributes = xmlReader.attributes();

    stDA* DA = new stDA();                                       //用DA的结构保存SDO的数据
    DA->name_ = attributes.value("name").toString();
    DA->type_ = attributes.value("type").toString();

    listDA.append(DA);                                           //保存每个数据对象下的所有数据属性
}

void ScdToData::parseDAType(QXmlStreamReader &xmlReader)
{
    Cur_Parse_DAType_id_ = xmlReader.attributes().value("id").toString();
    QList<stBDA*> listBDA;
    while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "DAType"))
    {
        if(xmlReader.tokenType() == QXmlStreamReader::StartElement && xmlReader.name() == "BDA")
        {
            parseBDA(xmlReader, listBDA);
        }
        xmlReader.readNext();
    }
    mapDAType_DBA.insert(Cur_Parse_DAType_id_, listBDA);
}

void ScdToData::parseBDA(QXmlStreamReader &xmlReader,QList<stBDA*>& listBDA)
{
    QXmlStreamAttributes attributes = xmlReader.attributes();

    stBDA* BDA = new stBDA();
    BDA->name_ = attributes.value("name").toString();
    BDA->bType_ = attributes.value("bType").toString();

    if(attributes.hasAttribute("type"))
    {
        BDA->type_ = attributes.value("type").toString();
    }
    listBDA.append(BDA);
}


void ScdToData::formPointTable(QList<QString> &lstErrors)
{
    for(QMap<QString,stLN*>::iterator it = mapAllLNode.begin(); it != mapAllLNode.end(); ++it)
    {
        stLN* pLN =  it.value();
        QList<stDO*> listDO = mapLNodeType_DO[pLN->lnType_];          //找到逻辑节点下的所有数据对象
        Cur_Parse_IED_Name_ = pLN ->ied_;             //当前逻辑节点所属IED名字
        Cur_Parse_LN_Desc_ = pLN->desc_;              //当前逻辑节点描述
        for(int i = 0; i < listDO.size(); ++i)
        {
            stDO *DO = listDO.at(i);
            Cur_Parse_DO_Name_ = DO->name_;
            if(pLN->mapDOI.count(DO->name_) == 1)
            {
                  Cur_Parse_DO_Desc_ = pLN->mapDOI[DO->name_];             //当前DO的desc;
            }
            else
            {
                  Cur_Parse_DO_Desc_ = DO->desc_;                                       //当前DO的desc;
            }
            initDOType(DO->type_,it.key() + "$myFC$" + DO->name_);
        }
    }

    //清除之前保存的数据，清空内存
    for(QMap<QString,QList<stDO*>>::iterator it = mapLNodeType_DO.begin(); it != mapLNodeType_DO.end(); ++it)
    {
        QList<stDO*> listDO = it.value();
        qDeleteAll(listDO);
        listDO.clear();
    }
    mapLNodeType_DO.clear();

    for(QMap<QString,QList<stDA*>>::iterator it = mapDOType_DA.begin(); it != mapDOType_DA.end(); ++it)
    {
        QList<stDA*> listDA = it.value();
        qDeleteAll(listDA);
        listDA.clear();
    }
    mapDOType_DA.clear();

    for(QMap<QString,QList<stBDA*>>::iterator it = mapDAType_DBA.begin(); it != mapDAType_DBA.end(); ++it)
    {
        QList<stBDA*> listBDA = it.value();
        qDeleteAll(listBDA);
        listBDA.clear();
    }
    mapDAType_DBA.clear();

}

void ScdToData::initDOType(QString DO_id, QString FCDA)
{
    QList<stDA*>listDA = mapDOType_DA[DO_id];
    for(int i = 0; i < listDA.size(); ++i)
    {
        QString tmpFCDA = FCDA;
        stDA* DA = listDA.at(i);
        //因为DA结构保存了DA元素和SDO元素的数据，SDO没有FC，DA有FC
        if(DA->fc_.isEmpty())
        {
            initDOType(DA->type_, FCDA + "/" + DA->name_);        // 解析SDO的数据
        }
        else
        {
            Cur_Parse_DA_Type_ = DA->bType_;                            // 解析SDO的数据
            if(tmpFCDA.contains("myFC"))
            {
                tmpFCDA.replace(tmpFCDA.indexOf("myFC"),4, DA->fc_);
            }
            tmpFCDA += "$" + DA->name_;
            if(DA->bType_ == QString("Struct"))          //如果数据属性的类型是Struct,进一步解析数据属性的类型
            {
                initDAType(DA->type_,tmpFCDA);
            }
            else
            {
                stFCDA *pFCDA = new stFCDA();
                pFCDA->desc_ = Cur_Parse_LN_Desc_ + "/" + Cur_Parse_DO_Desc_;
                pFCDA->ied_ = Cur_Parse_IED_Name_;
                pFCDA->type_ = Cur_Parse_DA_Type_;
                pFCDA->do_ = Cur_Parse_DO_Name_;
                mapAllFCDA.insert(tmpFCDA,pFCDA);            //保存所有的FCDA
            }
        }
    }
}

void ScdToData::initDAType(QString DA_id, QString FCDA)
{
    QList<stBDA*> listBDA =mapDAType_DBA[DA_id];
    for(int i = 0; i < listBDA.size(); ++i)
    {
        stBDA* BDA = listBDA.at(i);
        if(BDA->bType_ == QString("Struct"))
        {
            initDAType(BDA->type_,FCDA + "$" + BDA->name_);
        }
        else
        {
            Cur_Parse_DA_Type_ = BDA->bType_;        //当前BDA的类型
            stFCDA *pFCDA = new stFCDA();
            pFCDA->desc_ =  Cur_Parse_LN_Desc_ + "/" + Cur_Parse_DO_Desc_;
            pFCDA->ied_ = Cur_Parse_IED_Name_;
            pFCDA->type_ = Cur_Parse_DA_Type_;
             pFCDA->do_ = Cur_Parse_DO_Name_;
            mapAllFCDA.insert(FCDA + "$" + BDA->name_,pFCDA);            //保存所有的FCDA
        }
    }
}

void ScdToData::writePointDataToFile(QList<QString> &lstErrors)
{
    QMap<QString,stFCDA*>::iterator itFCDA = mapAllFCDA.begin();
    int i = 1;
    for(  ; itFCDA != mapAllFCDA.end(); ++itFCDA)
    {
        stFCDA* pFCDA = itFCDA.value();
        if(mapFCFilterType.count(itFCDA.key().split("$").at(1)) == 0)
        {
            continue;
        }
        stPointData pointData;
        if(mapPointData.count(pFCDA->ied_ ) == 0)
        {
            i = 1;
        }
        pointData.RedisAddr_ = pFCDA->ied_ + "_" + QString::number(i);
        pointData.Name_ = itFCDA.key();
        pointData.Desc_ = pFCDA->desc_;
        pointData.Type_ = pFCDA->type_;
        pointData.DoName_ = pFCDA->do_;
        pointData.iedName_ = pFCDA->ied_;
        pointData.DataType_ = pFCDA->type_;

        mapPointData[pFCDA->ied_].push_back(pointData);
        ++i;
    }

    QMap<QString,QList<stPointData>>::iterator itPointData = mapPointData.begin();
    for( ; itPointData != mapPointData.end(); ++itPointData)
    {
        stPointData pointData;
        pointData.RedisAddr_ = itPointData.key() + QString("_00057");
        pointData.Desc_ =  QString::fromLocal8Bit("A网状态");
        pointData.DoName_ = "Alm4";
        pointData.Name_ = itPointData.key() + QString("LD0/GGIO0$ST$Alm4$stVal");
        pointData.Type_ = "BOOLEAN";

        mapPointData[itPointData.key()].push_back(pointData);        
        pointData.RedisAddr_ = itPointData.key() + QString("_00061");
        pointData.Desc_ =  QString::fromLocal8Bit("B网状态");
        pointData.DoName_ = "Alm5";
        pointData.Name_ = itPointData.key() + QString("LD0/GGIO0$ST$Alm5$stVal");

        mapPointData[itPointData.key()].push_back(pointData);
    }

    mapAllFCDA.clear();
    listAllRptCntl.clear();
    mapAllLNode_IED.clear();
    mapAllLNode.clear();
    lstErrors.push_back("succed");
}

extern "C" SCDTODATA_API  IScdToData* CreateModule(void* pIService)
{
    IScdToData* pModule = new ScdToData();
    return pModule;
}

extern "C" SCDTODATA_API void DeleteModule(IScdToData* pModule)
{
    if(pModule == NULL)
        return ;
    delete (ScdToData*)pModule;
}

