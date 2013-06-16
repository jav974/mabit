#ifndef MABITSET_HPP
#define MABITSET_HPP

#include <algorithm>					// std::fill, std::for_each
#include <utility>					// std::forward
#include <vector>
#include "mabit_traits.hpp"

namespace Mabit
{
  template<typename word_t>
  class mabitset : public std::vector<word_t>
  {
  public:
    typedef typename mabit_traits<word_t>::msize_t	msize_t;
    typedef typename mabit_traits<word_t>::word_r	word_r;
    typedef typename mabit_traits<word_t>::word_cr	word_cr;

    typedef std::vector<word_t>				container_t;
    typedef mabitset<word_t>				mabitset_t;

    static const msize_t				BITS_IN_WORD = sizeof(word_t) * 8;

    mabitset() : container_t()
    {
    }

    mabitset(const mabitset_t& other) : container_t(other)
    {
    }

    mabitset(mabitset_t&& other) : container_t(std::forward<container_t>(other))
    {
    }

    mabitset_t&		operator = (const mabitset_t& other)
    {
      if (this != &other)
	container_t::operator = (other);
      return *this;
    }

    mabitset_t&		operator = (mabitset_t&& other)
    {
      if (this != &other)
	container_t::operator = (std::forward<container_t>(other));
      return *this;
    }

    mabitset_t&		operator &= (const mabitset_t& other)
    {
      apply(*this, other, [] (word_r a, word_cr b) { a &= b; });
      sanitize(other.size());
      return *this;
    }

    mabitset_t&		operator |= (const mabitset_t& other)
    {
      apply(*this, other, [] (word_r a, word_cr b) { a |= b; });
      sanitize(other.size());
      return *this;
    }

    mabitset_t&		operator ^= (const mabitset_t& other)
    {
      apply(*this, other, [] (word_r a, word_cr b) { a ^= b; });
      sanitize(other.size());
      return *this;
    }

    mabitset_t&		operator <<= (const msize_t shift)
    {
      if (!shift)
	return *this;

      if (shift >= this->size() * BITS_IN_WORD)
	{
	  fill(0);
	  return *this;
	}

      const msize_t	block_shift = shift / BITS_IN_WORD;
      const msize_t	offset = shift % BITS_IN_WORD;

      if (!offset)
	for (msize_t i = this->size() - 1; i >= block_shift; --i)
	  (*this)[i] = (*this)[i - block_shift];
      else
	{
	  const msize_t	sub_offset = BITS_IN_WORD - offset;

	  for (msize_t i = this->size() - 1; i > block_shift; --i)
	    (*this)[i] = ((*this)[i - block_shift] << offset) | ((*this)[i - block_shift - 1] >> sub_offset);
	  (*this)[block_shift] = (*this)[0] << offset;
	}
      std::fill(this->begin(), this->begin() + block_shift, 0);
      return *this;
    }

    mabitset_t&		operator >>= (const msize_t shift)
    {
      if (!shift)
	return *this;

      if (shift >= this->size() * BITS_IN_WORD)
	{
	  fill(0);
	  return *this;
	}

      const msize_t	block_shift = shift / BITS_IN_WORD;
      const msize_t	offset = shift % BITS_IN_WORD;
      msize_t		limit = this->size() - block_shift;

      if (limit)
	--limit;

      if (!offset)
	for (msize_t i = 0; i <= limit; ++i)
	  (*this)[i] = (*this)[i + block_shift];
      else
	{
	  const msize_t	sub_offset = BITS_IN_WORD - offset;

	  for (msize_t i = 0; i < limit; ++i)
	    (*this)[i] = ((*this)[i + block_shift] >> offset) | ((*this)[i + block_shift + 1] << sub_offset);
	  (*this)[limit] = (*this)[this->size() - 1] >> offset;
	}
      std::fill(this->begin() + limit + 1, this->end(), 0);
      return *this;
    }

    void		flip()
    {
      for (auto& w : *this)
	w = ~w;
    }

    void		fill(word_cr val)
    {
      std::fill(this->begin(), this->end(), val);
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
      if (from < this->size())
	std::for_each(this->begin() + from, this->end(), [] (word_r w) { w = 0; });
    }
  };
}

#endif // !MABITSET_HPP
