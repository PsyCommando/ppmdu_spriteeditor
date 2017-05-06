#ifndef ALGO_HPP
#define ALGO_HPP
/*
algo.hpp
2017/05/05
psycommando@gmail.com
Description: Misc algorithms.
*/

namespace utils
{

    /************************************************************************************
        advAsMuchAsPossible
            Advance an iterator until either the given number of increments are made,
            or the end is reached!
    ************************************************************************************/
    template<class _init>
        inline _init advAsMuchAsPossible( _init iter, _init itend, unsigned int displacement )
    {
        for( unsigned int i = 0; i < displacement && iter != itend; ++i, ++iter );
        return iter;
    }


    /************************************************************************************
        CountEqualConsecutiveElem
            Count the ammount of similar consecutive values between two sequences.
            It stops counting once it stumbles on a differing value.
    ************************************************************************************/
    template<class _init>
        inline unsigned int CountEqualConsecutiveElem( _init first_1,
                                                       _init last_1,
                                                       _init first_2,
                                                       _init last_2 )
    {
        unsigned int cpt = 0;
        for(; first_1 != last_1 &&
              first_2 != last_2 &&
              *first_1 == *first_2;
              ++first_1, ++first_2, ++cpt );
        return cpt;
    }

};

#endif // ALGO_HPP
