#ifndef SEQUENTIALGENERATOR_HPP
#define SEQUENTIALGENERATOR_HPP
#include <queue>

namespace utils
{

    /*
     *
    */
    template<class T>
        struct CounterWrap
    {
        T val = 0;
        T & get() {return val;}
        const T & get()const {return val;}
    };

    /*
    */
    template<class _PARENTTY, class _IDTY=unsigned long long>
        class BaseSequentialIDGen
    {
    protected:
        typedef _PARENTTY parent_t;
        typedef _IDTY     id_t;

        static id_t & GetCounter()
        {
            static CounterWrap<id_t> s_IDCounter;
            return s_IDCounter.get();
        }

        static std::queue<id_t> & GetRecycler()
        {
            static std::queue<id_t> s_IDRecycler;
            return s_IDRecycler;
        }

        static id_t AssignID()
        {
            id_t outid = 0;
            if(GetRecycler().empty())
            {
                outid= GetCounter();
                ++GetCounter();
            }
            else
            {
                outid = GetRecycler().front();
                GetRecycler().pop();
            }
            return outid;
        }

        static void ResignID(id_t id)
        {
            GetRecycler().push(id);
        }


        //
        id_t m_id;

    public:
        BaseSequentialIDGen()
        {
            m_id = AssignID();
        }

        virtual ~BaseSequentialIDGen()
        {
            ResignID(m_id);
        }

        inline id_t getID()const {return m_id;}
    };
};

#endif // SEQUENTIALGENERATOR_HPP
