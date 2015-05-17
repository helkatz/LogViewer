#include "logfilemodel.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpressionMatch>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlError>
class Parser
{
    QFile _file;
    QTextStream _stream;
    struct TPattern
    {
        QString name;
        QString pattern;

    };
    QMap<QString, TPattern> _patternMap;
    TPattern _parsePattern;
public:
    Parser(QString fileName):
        _file(fileName),
        _stream(&_file)
    {
        _file.open(QIODevice::ReadOnly);
        TPattern pattern;
        pattern.pattern =
                "^\\[(?<datetime>[^\\]]+).*?"
                "\\[(?<level>[^\\]]+).*?"
                "\\[(?<pid>[^\\]]+).*?"
                "[ ](?<message>.*)";
        pattern.name = "apache_error_log";
        _patternMap[pattern.name] = pattern;


    }

    bool findParser()
    {
        int tryLines = 20;
        while(!_stream.atEnd() && --tryLines > 0) {
            QString line = _stream.readLine();
            foreach(TPattern pattern, _patternMap) {
                QRegularExpression re(pattern.pattern);
                QRegularExpressionMatch match = re.match(line);
                //qDebug() << match.capturedLength()<<","<<match.hasMatch();
                if(match.hasMatch() == false)
                    continue;
                //if(_parsePattern.name.length() && _parsePattern.name != pattern.name)
                _parsePattern = pattern;
                return true;
            }
        }
        return false;
    }

    quint32 calcRows()
    {
        if(findParser() == false)
            return 0;
        _stream.seek(0);
        QRegularExpression re(_parsePattern.pattern);
        quint32 rows = 0;
        quint64 lastPos = 0;
        quint32 totalSize = 0;
        quint32 totalRows = 0;
        qDebug()<<"fileSize"<<_file.size();
        qsrand(_file.size());
        while(!_stream.atEnd() && totalRows < 200) {
            QString line = _stream.readLine();
            QRegularExpressionMatch match = re.match(line);
            if(match.hasMatch() == false)
                continue;
            totalRows++;
            quint32 entrySize = _stream.pos() - lastPos;
            lastPos = _stream.pos();
            totalSize += entrySize;
            if(totalRows % 4 == 0) {
                _stream.seek(_stream.pos() + _file.size() / 200);
                //_stream.seek(qrand() % _file.size());
                _stream.readLine();
                lastPos = _stream.pos();
                //qDebug() << "rand"<<qrand()<<"pos="<<lastPos;
            }
        }
        qDebug() << "average entries"<<_file.size() / (totalSize/totalRows);
        qDebug() << "totalRows checked"<<totalRows;
        return _file.size() / (totalSize/totalRows);
    }

    QSqlRecord read(quint64 index)
    {
        QSqlRecord r;
        QSqlField f1("0", QVariant::String);
        QSqlField f2("name", QVariant::String);
        r.append(f1);
        r.append(f2);
        r.setValue("0", 1);
        r.setValue("1", "field2");
        return r;
    }

};


LogFileModel::LogFileModel(QObject *parent):
    LogModel(parent)
{

}

int LogFileModel::rowCount(const QModelIndex &) const
{
    return 2;
}

int LogFileModel::columnCount(const QModelIndex &) const
{
    return 2;
}



bool LogFileModel::loadData(const QModelIndex &index) const
{
    QSqlRecord& r = _parser->read(0);
    _dataCache[r.value(0).toInt() - 1] = r;
    /*
    int fromPos = index.row() - 100 > 0 ? index.row() - 100 : 0;
    int toPos = index.row() + 100;
    QString sql = QString(_query).arg(fromPos).arg(toPos);
    qDebug()<<"loadData for index "<<index.row()<<" sql "<<sql;
    QSqlQuery& q = *_sqlQuery;
    q.exec(sql);
    if(q.first()) {
        QSqlRecord& r = q.record();
        int fromId = r.value(0).toInt();
        int currentPos = fromPos;
        QString s = QString(",%1").arg(fromId);
        int toId = fromId;
        do
        {
            QSqlRecord& r = q.record();
            _dataCache[r.value(0).toInt() - 1] = q.record();
            //_dataCache[currentPos++] = q.record();
            toId = r.value(0).toInt();
            s += QString(",%1").arg(toId);

        } while(q.next());
        //qDebug()<<"ids"<<s;
        //qDebug()<<"fromId="<<fromId<<"toId"<<toId;
    }
    q.finish();*/
    return true;
}


bool LogFileModel::query(const Conditions &qqueryConditions)
{
    _parser = QSharedPointer<Parser>(new Parser(qc().fileName()));
    //QFile file("/xampp/apache/logs/error.log");
    _file = QSharedPointer<QFile>(new QFile(qc().fileName()));
    return true;
    if(!_file->open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", _file->errorString());
    }

    QTextStream in(&*_file);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    _sqlQuery = new QSqlQuery("", db);
    db.setDatabaseName("db.sqlite");
    if(db.open() == false)
        QMessageBox::information(0, "error", "error");

    QString pattern =
        "^\\[(?<datetime>[^\\]]+).*?"
        "\\[(?<level>[^\\]]+).*?"
        "\\[(?<pid>[^\\]]+).*?"
        "[ ](?<message>.*)";
    QString tableFields = "id integer primary key autoincrement, datetime datetime, level varchar(50), pid varchar(20), message mediumtext";
    //QString valuesPattern = "(str_to_date('\\1', '%a %b %d %H:%i:%s.%f %Y'), '\\2', '\\3', '\\4')";
    QString valuesPattern = "(null, strftime('%a %b %d %H:%i:%s.%f %Y', '\\1'), '\\2', '\\3', '\\4')";
    QRegularExpression re(pattern);
    QRegularExpression reFields("\\?<(.*?)>");

    db.exec("drop table if exists t1");
    db.exec("create table t1 (" + tableFields + ")");
    if(db.lastError().isValid())
        QMessageBox::information(0, "error", db.lastError().text());
    /*foreach(QString field, reFields.match(pattern)) {
        tableCreate += "";
    }*/

   //QRegularExpressionMatch match = re.match(line);
    QString values;
    int insertCount = 0;
    while(!in.atEnd()) {
        QString line = in.readLine();
        insertCount++;

        line = line.replace("\"","\\\""); /* 228 ``*/
        line = line.replace("'","''");
        //qDebug() << line;
        QRegularExpressionMatch match = re.match(line);
        //qDebug() << match.capturedLength()<<","<<match.hasMatch();
        if(match.hasMatch() == false)
            continue;
        continue;
        values += "," + line.replace(re, valuesPattern);
        if(++insertCount > 100) {
            values = values.mid(1);
            db.exec("insert into t1 values" + values);
            if(db.lastError().isValid()) {
                qDebug() << values + db.lastError().text();
                break;
            }
            insertCount = 0;
            values = "";
        }
        //db2.exec("insert into t1 values" + line);
    }
    qDebug() << insertCount;
    values = values.mid(1);
    db.exec("insert into t1 values" + values);
    if(db.lastError().isValid()) {
        qDebug() << values + db.lastError().text();
    }
//db2.close();
    _file->close();
    QSqlQuery q3 = db.exec("select count(*) as c from t1");
    if(q3.first()) {
        qDebug() << q3.value("c");
    }
    return true;
}

void LogFileModel::writeSettings(const QString &basePath)
{
    qc().writeSettings(basePath);
}

void LogFileModel::readSettings(const QString &basePath)
{
    qc().readSettings(basePath);
}

QModelIndex LogFileModel::find(const QModelIndex &fromIndex, QString where, bool down) const
{
    return QModelIndex();
}
