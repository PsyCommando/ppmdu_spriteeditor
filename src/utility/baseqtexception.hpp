#ifndef BASEQTEXCEPTION_HPP
#define BASEQTEXCEPTION_HPP
#include <QException>
#include <QString>

/*
 * Sub-type of QException with a message string added.
*/
class BaseException : public QException
{
public:
    BaseException(const QString & msg)
        : _msg(msg)
    {
    }

    BaseException(const BaseException & cp)
    {
        _msg = cp._msg;
    }

    BaseException(BaseException && mv)
    {
        _msg = qMove(mv._msg);
    }

    BaseException & operator=( const BaseException & cp)
    {
        _msg = cp._msg;
        return *this;
    }

    BaseException & operator=( BaseException && mv)
    {
        _msg = qMove(mv._msg);
        return *this;
    }

    virtual ~BaseException()
    {
    }

    inline const QString & getMsg()const
    {
        return _msg;
    }

private:
    QString _msg;

    // exception interface
public:
    const char *what() const noexcept override
    {
        return qPrintable(_msg);
    }

    // QException interface
public:
    void raise() const override
    {
        throw *this;
    }

    QException *clone() const override
    {
        return new BaseException(*this);
    }
};

#endif // BASEQTEXCEPTION_HPP
