#ifndef MABIT_STREAM_HPP
#define MABIT_STREAM_HPP

#include <iostream>
#include <string>
#include <vector>
#include "mabit_traits.hpp"
#include "mabit.hpp"

namespace Mabit
{
  enum base_t
    {
      BIN = 2,
      OCT = 8,
      DEC = 10,
      HEX = 16
    };

  template<typename word_t>
  class	mabit_stream
  {
  public:
    typedef typename mabit_traits<word_t>::msize_t	msize_t;
    typedef mabit<word_t>				mabit_t;

    static std::string	to_string(const mabit_t& nb, const base_t base, const char sep = 0)
    {
      switch (base)
	{
	case BIN:
	  return to_bin(nb, sep);
	case OCT:
	  return to_oct(nb, sep);
	case DEC:
	  return to_dec(nb, sep);
	case HEX:
	  return to_hex(nb, sep);
	}
      return to_bin(nb, sep);
    }

  private:
    static std::string	to_bin(const mabit_t& nb, const char sep)
    {
      std::string	ret;

      if (!nb._sign)
	ret += '-';
	  
      msize_t		words = nb.used_words();
      
      ret.resize(words * nb._set.BITS_IN_WORD + 1);
      
      if (!words)
	{
	  for (msize_t i = 0; i < nb._set.BITS_IN_WORD; ++i)
	    ret += '0';
	  return ret;
	}
      
      bool		first = true;
      word_t		simulated_abs;
      
      for (; words > 0; --words)
	{
	  simulated_abs = nb.simulate_abs(words - 1);
	  
	  if (first && !simulated_abs)
	    {
	      first = false;
	      continue ;
	    }
	  
	  for (msize_t offset = 0; offset < nb._set.BITS_IN_WORD; ++offset)
	    ret += !nb.get_bit(simulated_abs, nb._set.BITS_IN_WORD - 1 - offset) ? '0' : '1';
	  
	  if (sep && words > 1)
	    ret += sep;
	  
	  first = false;
	}
      return ret;
    }
    
    static std::string	to_oct(const mabit_t& nb, const char sep)
    {
      std::string	ret;

      return ret;
    }
  
    static std::string	to_dec(const mabit_t& nb, const char sep)
    {
      std::vector<char>	tmp;
      std::string	ret;

      tmp.push_back('0');
	  
      word_t		simulated_abs;
      
      for (msize_t words = nb.used_words(); words > 0; --words)
	{
	  simulated_abs = nb.simulate_abs(words - 1);
	  
	  for (msize_t i = 0; i < nb._set.BITS_IN_WORD; ++i)
	    {
	      mult_by(tmp, 2, Mabit::DEC);
	      if (nb.get_bit(simulated_abs, nb._set.BITS_IN_WORD - 1 - i))
		add_one(tmp, 0, Mabit::DEC);
	    }
	}

      ret.resize(tmp.size() + tmp.size() / 3);

      for (size_t i = 0; i < tmp.size(); ++i)
	{
	  ret += tmp[i];
	  if (sep && (i && (i % 3) == 2 && i != tmp.size() - 1))
	    ret += sep;
	}
    
      if (!nb._sign)
	ret += '-';
      
      std::reverse(ret.begin(), ret.end());
      return ret;
    }

    static std::string	to_hex(const mabit_t& nb, const char sep)
    {
      std::string	ret;

      return ret;
    }

    static void		add_one(std::vector<char>& nb, size_t i, const base_t base)
    {
      const char	limit = (static_cast<int>(base) - 1) + '0';

      for (; i < nb.size(); ++i)
	{
	  if (nb[i] < limit)
	    {
	      nb[i] += 1;
	      return ;
	    }
	  nb[i] = '0';
	}
      nb.push_back('1');
    }

    static void		mult_by(std::vector<char>& nb, unsigned int mult_by, const base_t base)
    {
      std::vector<char>	ret;
      const size_t	size = nb.size();

      for (size_t i = 0; i < size; ++i)
	{
	  unsigned int	add_how_much = (nb[i] - '0') * mult_by;

	  if (!add_how_much && i < size - 1)
	    ret.push_back('0');

	  for (; add_how_much > 0; --add_how_much)
	    add_one(ret, i, base);
	}
      nb = std::move(ret);
    }
  };
}

template<class Ch, class Tr, class word_t>
std::basic_ostream<Ch, Tr>&	operator << (std::basic_ostream<Ch, Tr>& s, const Mabit::mabit<word_t>& obj)
{
  const char sep			= std::use_facet<std::numpunct<char> >(s.getloc()).thousands_sep();
  const std::ios_base::fmtflags	flags	= s.flags();
  Mabit::base_t	base			= Mabit::BIN;

  if (flags & std::ios_base::oct)
    base = Mabit::OCT;
  else if (flags & std::ios_base::dec)
    base = Mabit::DEC;
  else if (flags & std::ios_base::hex)
    base = Mabit::HEX;

  s << Mabit::mabit_stream<word_t>::to_string(obj, base, sep);
  return s;
}

#endif // !MABIT_STREAM_HPP
