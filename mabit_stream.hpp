#ifndef MABIT_STREAM_HPP
#define MABIT_STREAM_HPP

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
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
	  return to_oct(nb, '.');
	case DEC:
	  return to_dec(nb, sep);
	case HEX:
	  return to_hex(nb, '.');
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
      std::vector<char>	tmp;
      std::string	ret;

      bin_to_base(nb, OCT, tmp);
      build_str(ret, tmp, sep, nb._sign, 2, OCT);

      return ret;
    }

    static std::string	to_dec(const mabit_t& nb, const char sep)
    {
      std::vector<char>	tmp;
      std::string	ret;

      bin_to_base(nb, DEC, tmp);
      build_str(ret, tmp, sep, nb._sign, 3, DEC);

      return ret;
    }

    static std::string	to_hex(const mabit_t& nb, const char sep)
    {
      std::vector<char>	tmp;
      std::string	ret;

      bin_to_base(nb, HEX, tmp);
      build_str(ret, tmp, sep, nb._sign, 2, HEX);

      return ret;
    }

    static void		bin_to_base(const mabit_t& data, const base_t base, std::vector<char>& ret)
    {
      word_t		abs;

      ret.push_back(0);

      for (msize_t i = data.used_words(); i > 0; --i)
	{
	  abs = data.simulate_abs(i - 1);

	  for (msize_t j = 0; j < data._set.BITS_IN_WORD; ++j)
	    {
	      mult_by(ret, 2, base);
	      if (data.get_bit(abs, data._set.BITS_IN_WORD - 1 - j))
		add_one(ret, 0, base, 1);
	    }
	}
    }

    static void		build_str
    (
     std::string& ret,
     const std::vector<char>& nb,
     const char sep,
     const bool sign,
     const size_t delimit_pos,
     const base_t base
     )
    {
      for (size_t i = 0; i < nb.size(); ++i)
	{
	  char a = nb[i];

	  ret += (base != HEX ? a + '0' : (a >= 10 ? (a - 10) + 'A' : a + '0'));

	  if (sep && (i && (i % delimit_pos) == (delimit_pos - 1) && i != nb.size() - 1))
	    ret += sep;
	}
      if (!sign)
	ret += '-';

      std::reverse(ret.begin(), ret.end());
    }

    static void		add_one(std::vector<char>& nb, size_t i, const base_t base, int to_add = 1)
    {
      const char	limit = (static_cast<int>(base) - 1);

      for (; i < nb.size(); ++i)
	{
	  if (nb[i] < limit)
	    {
	      nb[i] += 1;
	      return ;
	    }
	  nb[i] = 0;
	}
      nb.push_back(1);
    }

    static void		mult_by(std::vector<char>& nb, unsigned int mult_by, const base_t base)
    {
      std::vector<char>	ret;
      const size_t	size = nb.size();

      for (size_t i = 0; i < size; ++i)
	{
	  unsigned int	add_how_much = nb[i] * mult_by;

	  if (!add_how_much && i < size - 1)
	    ret.push_back(0);

	  for (; add_how_much > 0; --add_how_much)
	    add_one(ret, i, base, add_how_much);
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
