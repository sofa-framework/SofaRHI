#include <SofaRHI/RHIUtils.h>

#include <QFile>

namespace sofa::rhi::utils
{

QShader loadShader(const std::string& name)
{
    QFile f(QString::fromUtf8(name.c_str()));
    if (f.open(QIODevice::ReadOnly)) {
        const QByteArray contents = f.readAll();
        return QShader::fromSerialized(contents);
    }
    return QShader();
}



} // namespace sofa::utils::rhi
