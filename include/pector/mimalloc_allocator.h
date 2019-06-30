// This file is part of pector (https://github.com/aguinet/pector).
// Copyright (C) 2014-2015 Adrien Guinet <adrien@guinet.me>
// Copyright (C) 2019 degski
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef PECTOR_MIMALLOC_ALLOCATOR_H
#define PECTOR_MIMALLOC_ALLOCATOR_H

#include <cstdlib>
#include <cstddef>

#include <mimalloc.h>

#include <pector/enhanced_allocators.h>
#include <pector/mimalloc_allocator_compat.h>

namespace pt {

namespace mi_internals {
struct dummy1 { };
struct dummy2 { };
} // internals

template<class T, bool make_reallocable = true, bool make_size_aware = false>
struct __declspec(empty_bases) mimalloc_allocator : public std::conditional<make_reallocable, reallocable_allocator, mi_internals::dummy1>::type
#ifdef PT_SIZE_AWARE_COMPAT
                                                                        , public std::conditional<make_size_aware, size_aware_allocator, mi_internals::dummy2>::type
#endif
{
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef ptrdiff_t difference_type;
	typedef size_t size_type;
    template<class U> struct rebind {
        typedef mimalloc_allocator<U, make_reallocable, make_size_aware> other;
    };

public:
    mimalloc_allocator() throw() { }
    mimalloc_allocator(mimalloc_allocator const&) throw() { }
    template<class U>
	mimalloc_allocator(mimalloc_allocator<U> const&) throw() { }

    pointer address(reference x) const { return &x; }
    const_pointer address(const_reference x) const { return &x; }

    pointer allocate(size_type n, const void* /*hint*/ = 0)
	{
		pointer const ret = reinterpret_cast<pointer>(mi_malloc(n*sizeof(value_type)));
		if (ret == nullptr) {
			throw std::bad_alloc();
		}
		return ret;
    }

    void deallocate(pointer p, size_type)
	{
		mi_free(p);
    }

    size_type max_size() const throw()
	{
        size_type max = static_cast<size_type>(-1) / sizeof (value_type);
        return (max > 0 ? max : 1);
    }

    template <typename U, typename... Args>
    void construct(U *p, Args&& ... args)
	{
		::new(p) U(std::forward<Args>(args)...);
	}

    void destroy(pointer p) { p->~value_type(); }

#ifdef PT_SIZE_AWARE_COMPAT
	size_type usable_size(const_pointer p) const
	{
		return PT_MIMALLOC_USABLE_SIZE(const_cast<pointer>(p))/sizeof(value_type);
	}
#endif

	pointer realloc(pointer p, size_type const n)
	{
		pointer const ret = reinterpret_cast<pointer>(::mi_realloc(p, n*sizeof(value_type)));
		if (ret == nullptr) {
			throw std::bad_alloc();
		}
		return ret;
	}
};

template<>
class mimalloc_allocator<void>
{
public:
    typedef void* pointer;
    typedef const void* const_pointer;
    typedef void value_type;
    template<typename U> struct rebind {
        typedef mimalloc_allocator<U> other;
    };
};

} // pector

#endif // PECTOR_MIMALLOC_ALLOCATOR_H
