#ifndef MABITSET_HPP
#define MABTISET_HPP

#include <algorithm>
#include <cstddef>
#include "mabit_traits.hpp"

namespace Mabit
{
  template<typename word_t>
  class mabitset
  {
  public:
    typedef typename mabit_traits<word_t>::word_c	word_c;
    typedef typename mabit_traits<word_t>::word_p	word_p;
    typedef typename mabit_traits<word_t>::word_cp	word_cp;
    typedef typename mabit_traits<word_t>::word_r	word_r;
    typedef typename mabit_traits<word_t>::word_cr	word_cr;
    typedef typename mabit_traits<word_t>::r_word_p	r_word_p;
    typedef typename mabit_traits<word_t>::r_word_cp	r_word_cp;
    typedef typename mabit_traits<word_t>::msize_t	msize_t;

    typedef mabitset<word_t>				mabitset_t;

    mabitset() : _real_size(0), _size(0), _set(nullptr)
    {
    }

    mabitset(const mabitset_t& other) : _real_size(0), _size(0), _set(nullptr)
    {
      resize(other._size);

      for (msize_t i = 0; i < other._size; ++i)
	_set[i] = other._set[i];
    }

    mabitset(mabitset_t&& other) : _real_size(other._real_size), _size(other._size), _set(std::move(other._set))
    {
      other._set = nullptr;
    }

    virtual ~mabitset()
    {
      if (_set != nullptr)
	{
	  delete[] _set;
	  _set = nullptr;
	}
    }

    mabitset_t&	operator = (const mabitset_t& other)
    {
      if (this != &other)
	{
	  resize(other._size);

	  for (msize_t i = 0; i < other._size; ++i)
	    _set[i] = other._set[i];
	}
      return *this;
    }

    mabitset_t&	operator = (mabitset_t&& other)
    {
      if (this != &other)
	{
	  if (_set != nullptr)
	    delete[] _set;

	  _real_size = other._real_size;
	  _size = other._size;
	  _set = std::move(other._set);
	  other._set = nullptr;
	}
      return *this;
    }

    word_p	begin()
    {
      return _set;
    }

    word_p	end()
    {
      return _set + _size;
    }

    word_cp	begin() const
    {
      return _set;
    }

    word_cp	end() const
    {
      return _set + _size;
    }

    r_word_p	rbegin()
    {
      return r_word_p(end());
    }

    r_word_p	rend()
    {
      return r_word_p(begin());
    }

    r_word_cp	rbegin() const
    {
      return r_word_cp(end());
    }

    r_word_cp	rend() const
    {
      return r_word_cp(begin());
    }

    word_r	operator [] (const msize_t at)
    {
      return _set[at];
    }

    word_cr	operator [] (const msize_t at) const
    {
      return _set[at];
    }

    msize_t	size() const
    {
      return _size;
    }

    void	resize(const msize_t size, word_c init_val = 0)
    {
      if (_real_size >= size)
	{
	  if (_size < size)
	    std::fill(_set + _size, _set + size, init_val);
	  _size = size;
	  return ;
	}

      msize_t	i	= 0;
      word_p	set	= new word_t[size];

      if (_set != nullptr)
	{
	  msize_t	limit = std::min(_size, size);

	  for (; i < limit; ++i)
	    set[i] = _set[i];

	  delete[] _set;
	}
      for (; i < size; ++i)
	set[i] = init_val;

      _size = size;
      _real_size = size;
      _set = set;
    }

    void	fill(word_c val)
    {
      std::fill(_set, _set + _size, val);
    }

    bool	is_empty() const
    {
      return !_size;
    }

  protected:
    msize_t	_real_size;
    msize_t	_size;
    word_p	_set;
  };
}

#endif // !MABITSET_HPP
