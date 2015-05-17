#include "utils.h"

Utils::Utils()
{
}
QString& operator<<(QString &out, const QString& var)
{
    out += var;
    return out;
}
QString& operator<<(QString &out, const int var)
{
    out += QString("%1").arg(var);
    return out;
}
