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

        static void AssignID( BaseSequentialIDGen<parent_t, id_t> * obj )
        {
            if(GetRecycler().empty())
            {
                obj->m_id = GetCounter();
                ++GetCounter();
            }
            else
            {
                obj->m_id = GetRecycler().front();
                GetRecycler().pop();
            }
        }

        static void ResignID( BaseSequentialIDGen<parent_t, id_t> * obj )
        {
            GetRecycler().push(obj->m_id);
        }


        //
        id_t m_id;

    public:
        BaseSequentialIDGen()
        {
            AssignID(this);
        }

        virtual ~BaseSequentialIDGen()
        {
            ResignID(this);
        }

        inline id_t getID()const {return m_id;}
    };
};

#endif // SEQUENTIALGENERATOR_HPP
