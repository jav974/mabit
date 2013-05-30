#ifndef MABIT_TRAITS_HPP
#define MABIT_TRAITS_HPP

#include <iterator>

using std::reverse_iterator;

namespace Mabit
{
  template<typename word_t>
  struct mabit_traits
  {
    typedef const word_t		word_c;
    typedef word_t*			word_p;
    typedef const word_t*		word_cp;
    typedef word_t&			word_r;
    typedef const word_t&		word_cr;
    typedef reverse_iterator<word_p>	r_word_p;
    typedef reverse_iterator<word_cp>	r_word_cp;
    typedef size_t			msize_t;
  };
}

#endif // !MABIT_TRAITS_HPP
