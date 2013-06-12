#ifndef MABIT_HPP
#define MABIT_HPP

#include <iostream>
#include <type_traits>
#include <algorithm>
#include "mabitset_ctrl.hpp"

namespace Mabit
{
  template<typename word_t>
  class mabit_stream;

  template<typename word_t>
  class mabit
  {
  public:
    static_assert(std::is_unsigned<word_t>::value, "Mabit: template parameter `word_t` should be unsigned.");
    static_assert(sizeof(word_t) < sizeof(unsigned long long), "Mabit: sizeof `word_t` should be < 64 bits");

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

    mabit(const mabit_t& other) : _sign(other._sign), _set(other._set)
    {
    }

    mabit(mabit_t&& other) : _sign(other._sign), _set(std::move(other._set))
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

      // If we are both negative and i have more words than the other, then i'm smaller than him
      // Or if we are both positive and i have less words than the other, then i'm smaller than him
      if ((!_sign && words > o_words) || (_sign && words < o_words))
	return true;

      // If we are both negative and i have less words than the other, then i'm bigger than him
      // Or if we are both positive and i have more words than the other, then i'm bigger than him
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
      return *this -= 1;
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

      for (msize_t i = word_ceil(bits); i > 0; --i)
	{
	  ret <<= _set.BITS_IN_WORD;
	  ret |= simulate_abs(i - 1);
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

      // Word: WORD_MAX OR POWER_OF_TWO
      if (w == WORD_MAX || (_sign && !(w & (w - 1)))
	  || (!_sign && !static_cast<word_t>((~w + 1) & ((~w + 1) - 1))))
	{
	  if (words == 1)
	    return w != WORD_MAX;

	  words -= 2;

	  for (; words > 0 && !_set[words]; --words) ;

	  if (!words && !_set[0])
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
	  // Make ret an index for _set
	  if (ret)
	    --ret;

	  // If last bit equals 0 then add 1 to the final result
	  if (!get_bit(_set[ret], _set.BITS_IN_WORD - 1))
	    ++ret;

	  // transform index to real number
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

    /**
     ** \brief
     ** Inserts a stream inside mabit
     ** \param in : a valid std::istream
     */
    mabit_t&			operator << (std::istream& in)
    {
      static const msize_t	ratio = sizeof(word_t) / sizeof(char);
      size_t			length = 0;
      msize_t			pos = 0;

      in.seekg(0, in.end);
      length = in.tellg();

      msize_t			limit = length / ratio + (!(length % ratio) ? 0 : 1);

      resize(limit);
      clear();

      in.seekg(0, in.beg);

      while (length > 0)
	{
	  size_t	buf_size = std::min(static_cast<size_t>(1024), length);
	  char		buf[buf_size];
	  msize_t	tmp = 0;

	  in.read(buf, buf_size);

	  for (size_t i = 0; i < buf_size; ++i)
	    {
	      const word_t at = static_cast<unsigned char>(buf[i]);

	      _set[pos] |= (at << (tmp++ * sizeof(char) * 8));

	      if (i % ratio == ratio - 1)
		{
		  ++pos;
		  tmp = 0;
		}
	    }
	  length -= buf_size;
	}
      return *this;
    }

    /**
     ** \brief
     ** Extracts the content of mabit to an output stream
     ** \param out : a valid std::ostream
     */
    void			operator >> (std::ostream& out) const
    {
      static const msize_t	ratio = sizeof(word_t) / sizeof(char);
      static const msize_t	offset = sizeof(char) * 8;
      static const word_t	mask = static_cast<unsigned char>(~0);
      const msize_t		bits = used_bits();
      const msize_t		chars = bits / offset + (!(bits % offset) ? 0 : 1);
      msize_t			char_count = 0;

      for (auto i = _set.begin(); i != _set.end(); ++i)
	for (msize_t j = 0; j < ratio; ++j)
	  {
	    if (char_count < chars)
	      out << static_cast<char>((*i & (mask << (j * offset))) >> (j * offset));
	    char_count += 1;
	  }
    }

  private:
    bool			_sign;
    set_t			_set;

    friend class		mabit_stream<word_t>;

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
     ** Add `val` into the bitset (handles the carry), at the specific location `from`
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
    bool			sign_add(const mabit_t& a, const mabit_t& b, const bool add_or_sub) const
    {
      // (+ + +) OR (- + -)
      if (add_or_sub && a._sign == b._sign)
	return a._sign;
      // (+ - -)
      if (!add_or_sub && a._sign && !b._sign)
	return true;
      // (- - +)
      if (!add_or_sub && !a._sign && b._sign)
	return false;

      const msize_t		a_bits = a.used_bits();
      const msize_t		b_bits = b.used_bits();

      if (!a_bits && !b_bits)
	return true;

      if (a_bits > b_bits)
	return a._sign;

      if (a_bits < b_bits)
	return add_or_sub ? b._sign : !b._sign;

      for (msize_t i = word_ceil(a_bits); i > 0; --i)
	{
	  const word_t		a_abs = a.simulate_abs(i - 1);
	  const word_t		b_abs = b.simulate_abs(i - 1);

	  if (a_abs > b_abs)
	    return a._sign;

	  if (a_abs < b_abs)
	    return add_or_sub ? b._sign : !b._sign;
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

      result._sign = sign_add(*this, other, add_or_sub);

      for (; i < o_words; ++i)
	result.add(i, add_or_sub ? other._set[i] : other.simulate_opposite(i));

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

      const msize_t		words_needed = word_ceil(r_bits + o_bits) + 1;

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

      result.clear();
      result.resize(words_needed);
      tmp_result.resize(words_needed);

      for (msize_t i = 0; i < o_words; ++i)
	{
	  const msize_t		o_simulated_abs = other.simulate_abs(i);

	  tmp_result <<= (i * _set.BITS_IN_WORD);

	  for (msize_t j = 0; j < words_needed; ++j)
	    result.add(j, tmp_result[j] * o_simulated_abs);
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

      // If divisor == 0, then throw an exception
      if (!divisor.any())
	return ;
	//throw std::exception("Mabit: Division by zero");
			
      // For optimization purpose only
      if (div_or_mod && divisor.is_power_of_2())
	{ 
	  dividend >>= (divisor.used_bits() - 1);
	  return ;
	}

      mabit_t			remainder(dividend);
      mabit_t			tmpdivisor;
      msize_t			quotient_shift;

      // Calculates the secure size for tmpdivisor
      // so that left shifting its bits will always return the expected value
      const msize_t		secure_size = word_ceil(remainder.used_bits() + 1);

      tmpdivisor.resize(secure_size);

      // Clears dividends content, will contain either the result of the division or the modulo
      dividend.clear();

      while (true)
	{
	  // Division without rest
	  if (remainder == divisor)
	    {
	      // Sets the result to 0 if the operation is modulo, or adds 1 to the final result of division
	      div_or_mod ? dividend.add(0, 1) : dividend.clear();
	      return ;
	    }

	  // Remainder is smaller than divisor, cannot divide anymore
	  if (remainder < divisor)
	    {
	      // Stores either the result of modulo or division
	      dividend = div_or_mod ? dividend : remainder;
	      return ;
	    }

	  const msize_t		tmpdividend_bits = remainder.used_bits();

	  // Sets tmpdivisor to real divisor
	  tmpdivisor = divisor;

	  tmpdivisor.resize(word_ceil(tmpdividend_bits + 1));
				
	  // Calculates quotient_shift
	  quotient_shift = tmpdividend_bits - divisor.used_bits();

	  if (quotient_shift > 0)
	    --quotient_shift;

	  // Makes tmpdivisor the highest divisor possible for remainder
	  tmpdivisor <<= quotient_shift;

	  if (tmpdivisor > remainder)
	    tmpdivisor >>= 1;

	  // Adds quotient to final result (quotient is a multiple of 2, so a simple addition into the right sector is enough)
	  dividend.add(quotient_shift / _set.BITS_IN_WORD, 1UL << (quotient_shift % _set.BITS_IN_WORD));

	  // Subtracts the highest divisor found to the remainder
	  remainder -= tmpdivisor;
	}
    }    
  };
}

#include "mabit_stream.hpp"

#endif // !MABIT_HPP
