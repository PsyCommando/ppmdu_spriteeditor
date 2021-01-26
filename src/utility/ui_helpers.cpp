#include "ui_helpers.hpp"
#include <QFontMetrics>
#include <QFont>

const QFont DefaultModelViewFont {"Sergoe UI", 9};

QSize CalculateTextSizeForView(const QString &text, const QFont * font)
{
    const QFontMetrics fm ( (!font)? DefaultModelViewFont : *font );
    return QSize(fm.horizontalAdvance(text) + 4, fm.height() + 4);
}
