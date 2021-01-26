#ifndef UI_HELPERS_HPP
#define UI_HELPERS_HPP
/*
 * Helpers for common operations done with UI, and view model stuff
*/
#include <QString>
#include <QSize>
#include <QFont>

//The default font used in the view controls
extern const QFont DefaultModelViewFont;

//Helper for SizeHint in the gui's view
QSize CalculateTextSizeForView(const QString &text, const QFont * font = nullptr);

#endif // UI_HELPERS_HPP
