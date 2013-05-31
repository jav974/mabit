#ifndef MABIT_HPP
#define MABIT_HPP

#include <iostream>
#include <string>
#include <type_traits>
#include <algorithm>
#include "mabitset_ctrl.hpp"

namespace Mabit
{
  enum base_t
    {
      BIN,
      DEC,
      OCT,
      HEX
    };

  template<typename word_t>
  class mabit
  {
  public:
    static_assert(std::is_unsigned<word_t>::value, "Mabit: template parameter `word_t` should be unsigned.");

    typedef typename mabit_traits<word_t>::msize_t	msize_t;
    typedef typename mabit_traits<word_t>::word_r	word_r;
    typedef typename mabit_traits<word_t>::word_cr	word_cr;

    typedef mabit<word_t>				mabit_t;
    typedef mabitset_ctrl<word_t>			set_t;

    static const msize_t				MIN_SIZE = sizeof(unsigned long long) / sizeof(word_t);
    static const word_t					WORD_MAX = ~static_cast<word_t>(0);

    mabit() : _sign(true)
    {
      resize(MIN_SIZE);
    }

    mabit(const mabit_t& other) : _set(other._set), _sign(other._sign)
    {
    }

    mabit(mabit_t&& other) : _set(std::move(other._set)), _sign(other._sign)
    {
    }

    template<typename word_type>
    mabit(const word_type val) : _sign(true)
    {
      resize(MIN_SIZE);

      if (std::is_signed<word_type>::value && val < 0)
	{
	  add(0, -val, true);
	  negate();
	}
      else if (val > 0)
	{
	  add(0, val, true);
	}
    }

    ~mabit()
    {
    }

    /************************************************************************/
    /* ASSIGNMENT OPERATORS  =                                              */
    /************************************************************************/
    mabit_t&			operator = (const mabit_t& other)
    {
      if (this != &other)
	{
	  _sign = other._sign;
	  _set = other._set;
	}
      return *this;
    }

    mabit_t&			operator = (mabit_t&& other)
    {
      if (this != &other)
	{
	  _sign = other._sign;
	  _set = std::move(other._set);
	}
      return *this;
    }

    /************************************************************************/
    /* COMPARISON OPERATORS  ==  <=  >=  <  >  !=                           */
    /************************************************************************/
    bool			operator == (const mabit_t& other) const
    {
      if (this == &other)
	return true;

      if (_sign != other._sign)
	return false;
      
      if (used_words() != other.used_words())
	return false;

      for (auto i = _set.begin(), j = other._set.begin(); i != _set.end() && j != other._set.end(); ++i, ++j)
	if (*i != *j)
	  return false;

      return true;
    }

    bool			operator != (const mabit_t& other) const
    {
      return !(*this == other);
    }

    bool			operator <= (const mabit_t& other) const
    {
      if (this == &other)
	return true;

      if (_sign && !other._sign)
	return false;

      if (!_sign && other._sign)
	return true;

      msize_t			words = used_words();
      const msize_t		o_words = other.used_words();

      if (!words && !o_words)
	return true;

      // If we are both negative and i have more bits than the other, then i'm smaller than him
      // Or if we are both positive and i have less bits than the other, then i'm smaller than him
      if ((!_sign && words > o_words) || (_sign && words < o_words))
	return true;
      
      // If we are both negative and i have less bits than the other, then i'm bigger than him
      // Or if we are both positive and i have more bits than the other, then i'm bigger than him
      if ((!_sign && words < o_words) || (_sign && words > o_words))
	return false;
      
      for (--words; words > 0; --words)
	{
	  if (_set[words] == other._set[words])
	    continue;
	  return _set[words] < other._set[words];
	}
      return _set[0] <= other._set[0];
    }

    bool			operator < (const mabit_t& other) const
    {
      if (*this == other)
	return false;
      return *this <= other;
    }

    bool			operator >= (const mabit_t& other) const
    {
      return !(*this < other);
    }

    bool			operator > (const mabit_t& other) const
    {
      return !(*this <= other);
    }

    /************************************************************************/
    /* ARITHMETIC OPERATORS  +=  -=  *=  /=  %=  +  -  *  /  %              */
    /************************************************************************/
    mabit_t&			operator += (const mabit_t& other)
    {
      addition(*this, other, true);
      return *this;
    }

    mabit_t			operator + (const mabit_t& other) const
    {
      return mabit_t(*this) += other;
    }

    mabit_t&			operator ++ ()
    {
      add(0, 1, _sign);

      if (!_sign && !any())
	_sign = true;

      return *this;
    }

    mabit_t&			operator -= (const mabit_t& other)
    {
      addition(*this, other, false);
      return *this;
    }

    mabit_t			operator - (const mabit_t& other) const
    {
      return mabit_t(*this) -= other;
    }

    mabit_t&			operator -- ()
    {
      *this -= 1;
      return *this;
    }

    mabit_t&			operator *= (const mabit_t& other)
    {
      const bool		final_sign = _sign == other._sign;
			
      if (!_sign)
	negate();
			
      multiplication(*this, other);

      if (!final_sign)
	negate();

      return *this;
    }

    mabit_t			operator * (const mabit_t& other) const
    {
      return mabit_t(*this) *= other;
    }

    mabit_t&			operator /= (const mabit_t& other)
    {
      const bool		final_sign = _sign == other._sign;

      if (!_sign)
	negate();

      if (!other._sign)
	division(*this, other.abs(), true);
      else
	// Avoids an useless copy of other if already positive
	division(*this, other, true);

      if (!final_sign)
	negate();

      return *this;
    }

    mabit_t			operator / (const mabit_t& other) const
    {
      return mabit_t(*this) /= other;
    }

    mabit_t&			operator %= (const mabit_t& other)
    {
      const bool		final_sign = _sign == other._sign ? _sign : (!_sign ? false : true);

      if (!_sign)
	negate();

      if (!other._sign)
	division(*this, other.abs(), false);
      else
	// Avoids an useless copy of other if already positive
	division(*this, other, false);

      if (!final_sign)
	negate();

      return *this;
    }

    mabit_t			operator % (const mabit_t& other) const
    {
      return mabit_t(*this) %= other;
    }

    /************************************************************************/
    /* WORD ACCESS OPERATOR  []                                             */
    /************************************************************************/
    word_r			operator [] (const msize_t at)
    {
      return _set[at];
    }

    word_cr			operator [] (const msize_t at) const
    {
      return _set[at];
    }

    /************************************************************************/
    /* BINARY OPERATORS  &=  &  |=  |  ^=  ^  <<=  <<  >>=  >>  ~           */
    /************************************************************************/
    mabit_t&			operator &= (const mabit_t& other)
    {
      _set &= other._set;
      return *this;
    }

    mabit_t			operator & (const mabit_t& other)
    {
      return mabit_t(*this) &= other;
    }

    mabit_t&			operator |= (const mabit_t& other)
    {
      _set |= other._set;
      return *this;
    }

    mabit_t			operator | (const mabit_t& other)
    {
      return mabit_t(*this) |= other;
    }

    mabit_t&			operator ^= (const mabit_t& other)
    {
      _set ^= other._set;
      return *this;
    }

    mabit_t			operator ^ (const mabit_t& other)
    {
      return mabit_t(*this) ^= other;
    }

    mabit_t&			operator <<= (const msize_t shift)
    {
      _set <<= shift;
      return *this;
    }

    mabit_t			operator << (const msize_t shift)
    {
      return mabit_t(*this) <<= shift;
    }

    mabit_t&			operator >>= (const msize_t shift)
    {
      _set >>= shift;
      return *this;
    }

    mabit_t			operator >> (const msize_t shift)
    {
      return mabit_t(*this) >>= shift;
    }

    mabit_t			operator ~ () const
    {
      mabit_t			ret(*this);

      ret.flip();
      return ret;
    }

    /************************************************************************/
    /* UNARY MINUS OPERATOR  -                                              */
    /************************************************************************/
    mabit_t			operator - () const
    {
      mabit_t			ret(*this);

      ret.negate();
      return ret;
    }

    template<typename word_out>
    word_out			to_integer() const
    {
      const msize_t		bits = used_bits();

      if ((_sign && !any()) || bits > sizeof(word_out) * 8)
	return 0;

      word_out			ret = 0;

      if ((sizeof(word_out)) == sizeof(word_t))
	ret = simulate_abs(0);
      else
	{
	  for (msize_t words = word_ceil(bits); words > 0; --words)
	    {
	      ret <<= _set.BITS_IN_WORD;
	      ret += simulate_abs(words - 1);
	    }
	}
      if (!_sign && std::is_signed<word_out>::value)
	ret = -ret;
      return ret;
    }

    long long			to_llong() const
    {
      return to_integer<long long>();
    }

    long			to_long() const
    {
      return to_integer<long>();
    }

    short			to_short() const
    {
      return to_integer<short>();
    }

    char			to_char() const
    {
      return to_integer<char>();
    }

    unsigned long long		to_ullong() const
    {
      return to_integer<unsigned long long>();
    }

    unsigned long		to_ulong() const
    {
      return to_integer<unsigned long>();
    }

    unsigned short		to_ushort() const
    {
      return to_integer<unsigned short>();
    }

    unsigned char		to_uchar() const
    {
      return to_integer<unsigned char>();
    }

    void			turn_bits(const bool val)
    {
      _set.fill(val ? WORD_MAX : 0);
    }

    bool			get_bit(const msize_t bit) const
    {
      if (bit >= _set.size() * _set.BITS_IN_WORD)
	return false;

      return (_set[bit / _set.BITS_IN_WORD] & (1UL << (bit % _set.BITS_IN_WORD))) != 0;
    }

    void			set_bit(const msize_t bit, const bool val)
    {
      if (bit >= _set.size() * _set.BITS_IN_WORD)
	return ;

      if (val)
	// Turns bit on
	_set[bit / _set.BITS_IN_WORD] |= (1UL << (bit % _set.BITS_IN_WORD));
      else
	// Turns bit off
	_set[bit / _set.BITS_IN_WORD] &= ~(1UL << (bit % _set.BITS_IN_WORD));
    }

    bool			get_bit(const word_t word, const msize_t pos) const
    {
      return (word & static_cast<word_t>(1UL << pos)) != 0;
    }

    void			flip()
    {
      _set.flip();
    }

    void			clear()
    {
      _sign = true;
      _set.fill(0);
    }

    void			resize(const msize_t size)
    {
      _set.resize(size, _sign ? 0 : WORD_MAX);
    }

    msize_t			size() const
    {
      return _set.size();
    }

    void			negate()
    {
      if (!any())
	{
	  _sign = true;
	  return ;
	}

      _sign = !_sign;
      flip();
      add(0, 1);

      if (!_sign && !get_bit(_set.size() * _set.BITS_IN_WORD - 1))
	resize(_set.size() + 1);
    }

    bool			is_power_of_2() const
    {
      msize_t			words = used_words();

      if (!words)
	return false;

      const word_t		w = _set[words - 1];

      if (w == WORD_MAX || (_sign && !(w & (w - 1)))
	  || (!_sign && !static_cast<word_t>((~w + 1) & ((~w + 1) - 1)))) // Word: WORD_MAX OR POWER_OF_TWO
	{
	  if (words == 1)
	    return w != WORD_MAX;

	  msize_t		i = words - 2;

	  for (; i > 0 && !_set[i]; --i) ;

	  if (!i && !_set[0])
	    return true;
	}
      return false;
    }

    bool			any() const
    {
      for (auto i : _set)
	if (i != 0)
	  return true;
      return false;
    }

    bool			all() const
    {
      for (auto i : _set)
	if (i != WORD_MAX)
	  return false;
      return true;
    }

    mabit_t			abs() const
    {
      return _sign ? *this : -*this;
    }

    /**
     **	\brief
     ** Returns the number of representative words
     */
    msize_t			used_words() const
    {
      msize_t			ret = _set.size();

      for (auto i = _set.rbegin(); i != _set.rend() && ret > 0; ++i, --ret)
	if ((_sign && *i != 0) || (!_sign && *i != WORD_MAX))
	  break ;
			
      if (!_sign)
	{
	  if (ret)
	    --ret;

	  if (!get_bit(_set[ret], _set.BITS_IN_WORD - 1))
	    ++ret;

	  ++ret;
	}
      return ret;
    }

    /**
     **	\brief
     ** Returns the number of representative bits
     */
    msize_t			used_bits() const
    {
      msize_t			bit = used_words() * _set.BITS_IN_WORD;

      if (!bit)
	return 0;

      --bit;

      for (; bit > 0; --bit)
	if ((_sign && get_bit(bit))
	    || (!_sign && !get_bit(bit)))
	  break;

      if (!_sign && is_power_of_2())
	++bit;

      return ++bit;
    }

    std::string			to_string(base_t format, const char sep = 0) const
    {
      std::string		ret;

      if (format == BIN)
	{
	  if (!_sign)
	    ret += '-';

	  msize_t		words = used_words();
			
	  if (!words)
	    {
	      for (msize_t i = 0; i < _set.BITS_IN_WORD; ++i)
		ret += '0';
	      return ret;
	    }

	  bool			first = true;
	  word_t		simulated_abs;

	  for (; words > 0; --words)
	    {
	      simulated_abs = simulate_abs(words - 1);

	      if (first && !simulated_abs)
		{
		  first = false;
		  continue ;
		}

	      for (msize_t offset = 0; offset < _set.BITS_IN_WORD; ++offset)
		ret += !get_bit(simulated_abs, _set.BITS_IN_WORD - 1 - offset) ? '0' : '1';

	      if (sep && words > 1)
		ret += sep;

	      first = false;
	    }
	}
      else if (format == DEC)
	{
	  ret = '0';

	  word_t		simulated_abs;

	  for (msize_t words = used_words(); words > 0; --words)
	    {
	      simulated_abs = simulate_abs(words - 1);

	      for (msize_t i = 0; i < _set.BITS_IN_WORD; ++i)
		{
		  multiply_string_by(ret, 2);
		  if (get_bit(simulated_abs, _set.BITS_IN_WORD - 1 - i))
		    add_one_to_string(ret, 0);
		}
	    }
	  if (sep)
	    {
	      std::string	ret2 = "";

	      for (size_t i = 0; i < ret.length(); ++i)
		{
		  ret2 += ret[i];
		  if (i && (i % 3) == 2 && i != ret.length() - 1)
		    ret2 += sep;
		}
	      ret = ret2;
	    }
	  if (!_sign)
	    ret += '-';

	  std::reverse(ret.begin(), ret.end());
	}
      return ret;
    }

    /**
     **	\brief
     ** Returns the absolute value at position `at`
     ** If mabit is positive, it just returns the value stored at `at`
     ** If negative, it calculates the value as if mabit were positive
     */
    word_t			simulate_abs(const msize_t at) const
    {
      if (_sign)
	return _set[at];
      return simulate_opposite(at);
    }

    /**
     **	\brief
     ** Returns the value at position `at` of the bitset if mabit were the opposite of itself
     */
    word_t			simulate_opposite(const msize_t at) const
    {
      unsigned long long	tmp = 0;
      word_t			rest = 1;

      for (msize_t i = 0; i < at; ++i)
	{
	  tmp = static_cast<word_t>(~_set[i]) + rest;

	  if (tmp <= WORD_MAX)
	    {
	      if (i == at)
		return static_cast<word_t>(tmp);
	      return ~_set[at];
	    }
	  rest = static_cast<word_t>(tmp - WORD_MAX);
	}
      return ~_set[at] + rest;
    }

  private:
    set_t			_set;
    bool			_sign;

    /**
     **	\brief
     ** Returns the number of words needed to store a specific number of bits
     */
    inline msize_t		word_ceil(const msize_t bits) const
    {
      return bits / _set.BITS_IN_WORD + (!(bits % _set.BITS_IN_WORD) ? 0 : 1);
    }

    /**
     **	\brief
     ** Add `val` into the bitset, at the specific location `from`
     ** Affects all the words needed above its position
     ** 
     ** \param from : word position
     ** \param val  : the value to be added
     ** \param auto_resize : indicates whether or not it should resize mabit to store everything
     */
    void			add(msize_t from, unsigned long long val, bool auto_resize = false)
    {
      if (!val)
	return ;

      word_t			tmp;

      for (auto i = _set.begin() + from; i != _set.end(); ++i)
	{
	  if (*i + val <= WORD_MAX)
	    {
	      *i += static_cast<word_t>(val);
	      val = 0;
	      break ;
	    }
	  tmp = *i;
	  *i = (tmp + val) & WORD_MAX;
	  val = (tmp + val) >> _set.BITS_IN_WORD;
	}
      if (val > 0 && auto_resize)
	{
	  from = _set.size();
	  resize(from + MIN_SIZE);
	  add(from, val, true);
	}
    }

    /**
     **	\brief
     ** Returns the sign after the addition of ourself with `other`
     */
    bool			sign_add(const mabit_t& other, const bool add_or_sub) const
    {
      // (+ + +) OR (- + -)
      if (add_or_sub && _sign == other._sign)
	return _sign;
      // (+ - -)
      if (!add_or_sub && _sign && !other._sign)
	return true;
      // (- - +)
      if (!add_or_sub && !_sign && other._sign)
	return false;

      const msize_t		bits = used_bits();
      const msize_t		other_bits = other.used_bits();

      if (!other_bits || bits > other_bits)
	return _sign;
      if (!bits || bits < other_bits)
	return add_or_sub ? other._sign : !other._sign;

      for (msize_t i = word_ceil(bits); i > 0; --i)
	{
	  const word_t		a = simulate_abs(i - 1);
	  const word_t		b = other.simulate_abs(i - 1);

	  if (a < b)
	    return add_or_sub ? other._sign : !other._sign;
	  else if (a > b)
	    return _sign;
	}
      return true;
    }

    /**
     **	\brief
     ** Performs the addition or subtraction between `result` and `other`, stores the result inside `result`
     */
    void			addition(mabit_t& result, const mabit_t& other, const bool add_or_sub) const
    {
      const msize_t		o_bits = other.used_bits();

      if (!o_bits)
	return ;

      const msize_t		r_bits = result.used_bits();
      const msize_t		o_words = word_ceil(o_bits);
      msize_t			r_words = word_ceil(r_bits);
      msize_t			i = 0;

      result.resize(word_ceil(std::max(r_bits, o_bits) + 1));

      result._sign = result.sign_add(other, add_or_sub);

      for (; i < o_words; ++i)
	add_or_sub ? result.add(i, other._set[i]) : result.add(i, other.simulate_opposite(i));

      if (((add_or_sub && !other._sign) || (!add_or_sub && other._sign)) && o_words < result._set.size())
	for (; i < result._set.size(); ++i)
	  result.add(i, WORD_MAX);

      r_words = result.used_words();

      if (!r_words)
	result._sign = true;

      result.resize(r_words);
    }

    /**
     **	\brief
     ** Performs the multiplication of `result` with `other`, stores the result inside `result`
     */
    void			multiplication(mabit_t& result, const mabit_t& other)
    {
      const msize_t		r_bits = result.used_bits();
      const msize_t		o_bits = other.used_bits();

      if (!o_bits || !r_bits)
	{
	  result.clear();
	  return ;
	}

      const msize_t		words_needed = word_ceil(r_bits + o_bits);
      
      // For optimization purpose only
      if (other.is_power_of_2())
	{
	  result.resize(words_needed);
	  result <<= (o_bits - 1);
	  return ;
	}
      // For optimization purpose only
      else if (result.is_power_of_2())
	{
	  result = other;

	  if (!result._sign)
	    result.negate();
				
	  result.resize(words_needed);
	  result <<= (r_bits - 1);
	  return ;
	}
      		
      const msize_t		o_words = word_ceil(o_bits);
      mabit_t			tmp_result(result);
      msize_t			last_offset = 0;
      msize_t			current_offset = 0;

      result.clear();
      result.resize(words_needed);
      tmp_result.resize(words_needed);

      for (msize_t i = 0; i < o_words; ++i)
	{
	  word_t		mask = 1;
	  const word_t		other_simulated_abs = other.simulate_abs(i);

	  for (msize_t j = 0; j < _set.BITS_IN_WORD; ++j)
	    {
	      if (other_simulated_abs & mask)
		{
		  tmp_result <<= (current_offset - last_offset);
		  result += tmp_result;
		  last_offset = current_offset;
		}
	      mask <<= 1;
	      ++current_offset;
	    }
	}
    }

    /**
     **	\brief
     ** Performs the division or modulo of `dividend` with `divisor`, stores the result inside `dividend`
     */
    void			division(mabit_t& dividend, const mabit_t& divisor, const bool div_or_mod) const
    {
      // If dividend == 0, then return
      if (!dividend.any())
	return ;

      const msize_t		divisor_bits = divisor.used_bits();

      // If divisor == 0, then throw an exception
      if (!divisor_bits)
	return ;
	//throw std::exception("Mabit: Division by zero");
			
      // For optimization purpose only
      if (div_or_mod && divisor.is_power_of_2())
	{ 
	  dividend >>= (divisor_bits - 1);
	  return ;
	}

      mabit_t			tmpdividend(dividend);
      mabit_t			tmpdivisor;
      msize_t			quotient_shift;
      msize_t			tmpdividend_bits;

      // Calculates the secure size for tmpdividend and quotient
      // so that left shifting the bits of these 2 will always return the expected value
      const msize_t		secure_size = word_ceil(tmpdividend.used_bits() + 1);

      tmpdivisor.resize(secure_size);

      // Clears dividends content, will contain either the result of the division or the modulo
      dividend.clear();

      while (true)
	{
	  // Division without rest
	  if (tmpdividend == divisor)
	    {
	      // Sets the result to 0 if the operation is modulo, or adds 1 to the final result of division
	      div_or_mod ? dividend.add(0, 1) : dividend.clear();
	      return ;
	    }

	  // Dividend is smaller than divisor, cannot divide anymore
	  if (tmpdividend < divisor)
	    {
	      // Stores either the result of modulo or division
	      dividend = div_or_mod ? dividend : tmpdividend;
	      return ;
	    }

	  tmpdividend_bits = tmpdividend.used_bits();

	  // Sets tmpdivisor to real divisor
	  tmpdivisor = divisor;

	  tmpdivisor.resize(word_ceil(tmpdividend_bits + 1));
				
	  // Calculates quotient_shift
	  quotient_shift = tmpdividend_bits - divisor_bits;

	  if (quotient_shift > 0)
	    --quotient_shift;

	  // Makes tmpdivisor the highest divisor possible for tmpdividend
	  tmpdivisor <<= quotient_shift;

	  if (tmpdivisor > tmpdividend)
	    tmpdivisor >>= 1;

	  // Adds quotient to final result (quotient is a multiple of 2, so a simple addition into the wright sector is enough)
	  dividend.add(quotient_shift / _set.BITS_IN_WORD, 1UL << (quotient_shift % _set.BITS_IN_WORD));

	  // Subtracts the highest divisor found to the current dividend
	  tmpdividend -= tmpdivisor;
	}
    }

    void			add_one_to_string(std::string& dec, size_t i) const
    {
      for (; i < dec.length(); ++i)
	{
	  if (dec[i] < '9')
	    {
	      dec[i] += 1;
	      return ;
	    }
	  dec[i] = '0';
	}
      dec += '1';
    }

    void			multiply_string_by(std::string& nb, unsigned int mult_by) const
    {
      std::string		ret("0");
      const size_t		nb_length = nb.length();

      for (size_t i = 0; i < nb_length; ++i)
	{
	  unsigned int		add_how_much = (nb[i] - '0') * mult_by;

	  if (!add_how_much && i < nb_length - 1)
	    ret += '0';

	  for (; add_how_much > 0; --add_how_much)
	    add_one_to_string(ret, i);
	}
      nb = ret;
    }
  };
}

template<class Ch, class Tr, class word_type>
std::basic_ostream<Ch, Tr>&	operator << (std::basic_ostream<Ch, Tr>& s, const Mabit::mabit<word_type>& obj)
{
  char separator = std::use_facet<std::numpunct<char> >(s.getloc()).thousands_sep();

  std::ios_base::fmtflags flags = s.flags();

  if (flags & std::ios_base::dec)
    s << obj.to_string(Mabit::DEC, separator);
  else
    s << obj.to_string(Mabit::BIN, ' ');
  return s;
}

#endif // !MABIT_HPP
