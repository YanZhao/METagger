

#ifndef StrHashTrait_h 
#define StrHashTrait_h 1

#include <functional>

#include "HashFunction.h"

using namespace std;
using namespace HitZy;
namespace HitZy{
template<class _Kty,
	class _Pr = less<_Kty> >
	class StrHashTrait
	{	// traits class for hash containers
public:
	enum
		{	// parameters for hash table
		bucket_size = 16,	// 0 < bucket_size
		min_buckets = 16384 };	// min_buckets = 2 ^^ N, 0 < N

	StrHashTrait()
		: comp()
		{	// construct with default comparator
		}

	StrHashTrait(_Pr _Pred)
		: comp(_Pred)
		{	// construct with _Pred comparator
		}

	size_t operator()(const _Kty& _Keyval) const
		{	// hash _Keyval to size_t value
		//return ((size_t)_Keyval);
			size_t result;
			result = NewHashFunction(_Keyval);
			return result;
		}

//	size_t operator()(const _Kty& _Keyval) const
//		{	// hash _Keyval to size_t value by pseudorandomizing transform
//		ldiv_t _Qrem = ldiv((size_t)_Keyval, 127773);
//		_Qrem.rem = 16807 * _Qrem.rem - 2836 * _Qrem.quot;
//		if (_Qrem.rem < 0)
//			_Qrem.rem += 2147483647;
//		return ((size_t)_Qrem.rem); }

	bool operator()(const _Kty& _Keyval1, const _Kty& _Keyval2) const
		{	// test if _Keyval1 ordered before _Keyval2
		return (comp(_Keyval1, _Keyval2));
		}

private:
	_Pr comp;	// the comparator object
	};
}
#endif
