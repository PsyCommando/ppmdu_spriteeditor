#include "ui_helpers.hpp"
#include <src/utility/baseqtexception.hpp>
#include <QFontMetrics>
#include <QFont>

const QFont DefaultModelViewFont {"Sergoe UI", 9};

QSize CalculateTextSizeForView(const QString &text, const QFont * font)
{
    const QFontMetrics fm ( (!font)? DefaultModelViewFont : *font );
    return QSize(fm.horizontalAdvance(text) + 4, fm.height() + 4);
}

QString GetNestedExceptionsText(std::exception_ptr e)noexcept
{
    try
    {
        std::rethrow_if_nested(e);
        try{ std::rethrow_exception(e); }
        catch (const BaseException & except)
        {
            return except.getMsg();
        }
        catch (const std::exception & except)
        {
            return except.what();
        }
        catch(...){}
    }
    catch(const BaseException & bex)
    {
        QString extext = bex.getMsg();
        return QString("%1, %2").arg(extext, GetNestedExceptionsText(bex));
    }
    catch (const std::exception & ex)
    {
        QString extext = ex.what();
        return QString("%1, %2").arg(extext, GetNestedExceptionsText(ex));
    }
    catch(...)
    {
    }
    return QString();
}

QString GetNestedExceptionsText(const std::exception & e)noexcept
{
    return GetNestedExceptionsText(std::make_exception_ptr(e));
}
