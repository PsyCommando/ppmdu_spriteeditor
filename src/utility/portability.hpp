#ifndef PORTABILITY_HPP
#define PORTABILITY_HPP
#include <QtGlobal>
#include <QString>
#include <QHash>
#include <unordered_map>
#include <typeinfo>

//////////////////////////////////////////////////////////////////////////////
//Helpers for Qt versions under 5.15
//////////////////////////////////////////////////////////////////////////////
    #if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))

        //hash generator for standard containers with QStrings
        namespace std
        {
            template<> struct hash<QString>
            {
                std::size_t operator()(QString const& s) const noexcept
                {
                    return qHash(s);
                }
            };
        };

        //Inserting a map into another
        #define QMAP_INSERT_QMAP(SRC, DEST)\
                for(auto it = SRC.begin(); it != SRC.end(); ++it)\
                {\
                    DEST.insert(it.key(), it.value());\
                }
    #else
        #define QMAP_INSERT_QMAP(SRC, DEST)\
                DEST.insert(SRC);
    #endif

//////////////////////////////////////////////////////////////////////////////
//Helpers for Qt versions under 5.14
//////////////////////////////////////////////////////////////////////////////

    //Missing QVector range constructor
    #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
        #define QVECTOR_RANGE_CTOR(VEC_T, BEG, END)\
            [](auto beg, auto end){\
                    VEC_T nv(std::distance(beg, end));\
                    std::copy(beg, end, nv.begin());\
                    return nv;\
            }(BEG, END)
    #else
        #define QVECTOR_RANGE_CTOR(VEC_T, BEG, END)\
            VEC_T(BEG, END)
    #endif

//////////////////////////////////////////////////////////////////////////////
//Helpers for Qt versions under 5.13
//////////////////////////////////////////////////////////////////////////////

    //Missing disable copy and move constructor macro
    #if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
        #define Q_DISABLE_COPY_MOVE(X) \
            X(const X &) = delete;\
            X(X &&) = delete;
    #endif

#endif // PORTABILITY_HPP
