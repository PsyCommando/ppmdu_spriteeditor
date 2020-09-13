#include "randomgenhelper.hpp"

QRandomGenerator & GetRandomGenerator()
{
    static QRandomGenerator generator;
    return generator;
}
