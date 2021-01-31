#include "ui_helpers.hpp"
#include <QFontMetrics>
#include <QFont>

const QFont DefaultModelViewFont {"Sergoe UI", 9};

QSize CalculateTextSizeForView(const QString &text, const QFont * font)
{
    const QFontMetrics fm ( (!font)? DefaultModelViewFont : *font );
    return QSize(fm.horizontalAdvance(text) + 4, fm.height() + 4);
}

QString GetNestedExceptionsText(const std::exception &e)
{
    try
    {
        std::rethrow_if_nested(e);
        return QString::fromLatin1(e.what());
    }
    catch (const std::exception & ex)
    {
        return QString("%1, %2").arg(QString::fromLatin1(ex.what()), GetNestedExceptionsText(ex));
    }

}
