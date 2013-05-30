#ifndef MABITSET_CTRL_HPP
#define MABITSET_CTRL_HPP

#include <algorithm>
#include "mabitset.hpp"

namespace Mabit
{
  template<typename word_t>
  class mabitset_ctrl : public mabitset<word_t>
  {
  public:
    typedef typename mabitset_ctrl<word_t>::msize_t	msize_t;
    typedef typename mabitset_ctrl<word_t>::word_r	word_r;
    typedef typename mabitset_ctrl<word_t>::word_cr	word_cr;

    typedef mabitset<word_t>				mabitset_t;
    typedef mabitset_ctrl<word_t>			mabitset_ctrl_t;

    static const msize_t				BITS_IN_WORD = sizeof(word_t) * 8;

    mabitset_ctrl() : mabitset_t()
    {
    }

    mabitset_ctrl(const mabitset_ctrl_t& other) : mabitset_t(other)
    {
    }

    mabitset_ctrl(mabitset_ctrl_t&& other) : mabitset_t(other)
    {
    }

    mabitset_ctrl_t&	operator = (const mabitset_ctrl_t& other)
    {
      if (this != &other)
	mabitset_t::operator = (other);
      return *this;
    }

    mabitset_ctrl_t&	operator = (mabitset_ctrl_t&& other)
    {
      if (this != &other)
	mabitset_t::operator = (other);
      return *this;
    }

    mabitset_ctrl_t&	operator &= (const mabitset_ctrl_t& other)
    {
      apply(*this, other, [] (word_r a, word_cr b) { a &= b; });
      sanitize(other._size);
      return *this;
    }

    mabitset_ctrl_t&	operator |= (const mabitset_ctrl_t& other)
    {
      apply(*this, other, [] (word_r a, word_cr b) { a |= b; });
      sanitize(other._size);
      return *this;
    }

    mabitset_ctrl_t&	operator ^= (const mabitset_ctrl_t& other)
    {
      apply(*this, other, [] (word_r a, word_cr b) { a ^= b; });
      sanitize(other._size);
      return *this;
    }

    mabitset_ctrl_t&	operator <<= (const msize_t shift)
    {
      if (!shift)
	return *this;
      if (shift >= this->_size * BITS_IN_WORD)
	{
	  this->fill(0);
	  return *this;
	}

      const msize_t	block_shift = shift / BITS_IN_WORD;
      const msize_t	offset = shift % BITS_IN_WORD;

      if (!offset)
	for (msize_t i = this->_size - 1; i >= block_shift; --i)
	  this->_set[i] = this->_set[i - block_shift];
      else
	{
	  const msize_t	sub_offset = BITS_IN_WORD - offset;

	  for (msize_t i = this->_size - 1; i > block_shift; --i)
	    this->_set[i] = (this->_set[i - block_shift] << offset) | (this->_set[i - block_shift - 1] >> sub_offset);
	  this->_set[block_shift] = this->_set[0] << offset;
	}
      std::fill(this->_set, this->_set + block_shift, 0);
      return *this;
    }

    mabitset_ctrl_t&	operator >>= (const msize_t shift)
    {
      if (!shift)
	return *this;
      if (shift >= this->_size * BITS_IN_WORD)
	{
	  this->fill(0);
	  return *this;
	}

      const msize_t	block_shift = shift / BITS_IN_WORD;
      const msize_t	offset = shift % BITS_IN_WORD;
      msize_t		limit = this->_size - block_shift;

      if (limit)
	--limit;

      if (!offset)
	for (msize_t i = 0; i <= limit; ++i)
	  this->_set[i] = this->_set[i + block_shift];
      else
	{
	  const msize_t	sub_offset = BITS_IN_WORD - offset;

	  for (msize_t i = 0; i < limit; ++i)
	    this->_set[i] = (this->_set[i + block_shift] >> offset) | (this->_set[i + block_shift + 1] << sub_offset);
	  this->_set[limit] = this->_set[this->_size - 1] >> offset;
	}
      std::fill(this->_set + limit + 1, this->_set + this->_size, 0);
      return *this;
    }

    void		flip()
    {
      for (auto& w : *this)
	w = ~w;
    }

  private:
    template<typename Container, typename Function>
    void		apply(Container& a, const Container& b, Function f)
    {
      auto		aItr = a.begin();
      auto		bItr = b.begin();

      for (; aItr != a.end() && bItr != b.end(); ++aItr, ++bItr)
	f(*aItr, *bItr);
    }

    void		sanitize(const msize_t from)
    {
      if (from < this->_size)
	std::for_each(this->begin() + from, this->end(), [] (word_r w) { w = 0; });
    }
  };
}

#endif // !MABITSET_CTRL_HPP
