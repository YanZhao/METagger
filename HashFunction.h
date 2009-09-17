
#ifndef _HASHFUNCTION_H 
#define _HASHFUNCTION_H 1

#include <string>
#include <ext/hash_map>

using  namespace std;
using namespace __gnu_cxx;
namespace HitZy
{
	#define UPDC16(octet, crc) (crc_16_tab[((crc) ^(octet)) & 0xff] ^ ((crc) >> 8))
	
	extern unsigned short HASH_SIZE;
	unsigned long HashFunction(const std::string& key, unsigned short ulsize=HASH_SIZE);
	
	size_t NewHashFunction(const std::string& key, unsigned short ulsize=HASH_SIZE );
}
#endif 

