/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef NEXUS_KEY_H
#define NEXUS_KEY_H

#include <stdexcept>
#include <vector>

#include "../LLU/allocators.h"
#include "../LLH/uint1024.h"

#include <openssl/ec.h> // for EC_KEY definition

namespace Wallet
{
	class key_error : public std::runtime_error
	{
	public:
		explicit key_error(const std::string& str) : std::runtime_error(str) {}
	};


	// secure_allocator is defined in serialize.h
	// CPrivKey is a serialized private key, with all parameters included (279 bytes)
	typedef std::vector<unsigned char, secure_allocator<unsigned char> > CPrivKey;
	// CSecret is a serialization of just the secret parameter (32 bytes)
	typedef std::vector<unsigned char, secure_allocator<unsigned char> > CSecret;

	/** An encapsulated OpenSSL Elliptic Curve key (public and/or private) */
	class CKey
	{
	protected:
		EC_KEY* pkey;
		bool fSet;
		bool fCompressedPubKey;

		void SetCompressedPubKey();

	public:

		void Reset();

		CKey();
		CKey(const CKey& b);

		CKey& operator=(const CKey& b);

		~CKey();

		bool IsNull() const;
		bool IsCompressed() const;

		void MakeNewKey(bool fCompressed);
		bool SetPrivKey(const CPrivKey& vchPrivKey);
		bool SetSecret(const CSecret& vchSecret, bool fCompressed = false);
		CSecret GetSecret(bool &fCompressed) const;
		CPrivKey GetPrivKey() const;
		bool SetPubKey(const std::vector<unsigned char>& vchPubKey);
		std::vector<unsigned char> GetPubKey() const;

		bool SignCompact(uint256 hash, std::vector<unsigned char>& vchSig);
		bool SetCompactSignature(uint256 hash, const std::vector<unsigned char>& vchSig);
		
		bool Sign(uint1024 hash, std::vector<unsigned char>& vchSig, int nBits);
		bool Verify(uint1024 hash, const std::vector<unsigned char>& vchSig, int nBits);
		bool IsValid();
	};
}
#endif
