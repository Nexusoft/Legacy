/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++

[Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#include <map>

#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>

#include "key.h"
#include "../util/util.h"

namespace Wallet
{

    // Generate a private key from just the secret parameter
    int EC_KEY_regenerate_key(EC_KEY *eckey, BIGNUM *priv_key)
    {
        int ok = 0;
        BN_CTX *ctx = NULL;
        EC_POINT *pub_key = NULL;

        if (!eckey) return 0;

        const EC_GROUP *group = EC_KEY_get0_group(eckey);

        if ((ctx = BN_CTX_new()) == NULL)
            goto err;

        pub_key = EC_POINT_new(group);

        if (pub_key == NULL)
            goto err;

        if (!EC_POINT_mul(group, pub_key, priv_key, NULL, NULL, ctx))
            goto err;

        EC_KEY_set_private_key(eckey,priv_key);
        EC_KEY_set_public_key(eckey,pub_key);

        ok = 1;

    err:

        if (pub_key)
            EC_POINT_free(pub_key);
        if (ctx != NULL)
            BN_CTX_free(ctx);

        return(ok);
    }

    // Perform ECDSA key recovery (see SEC1 4.1.6) for curves over (mod p)-fields
    // recid selects which key is recovered
    // if check is nonzero, additional checks are performed
    int ECDSA_SIG_recover_key_GFp(EC_KEY *eckey, BIGNUM* sig_r, BIGNUM* sig_s, const unsigned char *msg, int msglen, int recid, int check)
    {
        if (!eckey) return 0;

        int ret = 0;
        BN_CTX *ctx = NULL;

        BIGNUM *x = NULL;
        BIGNUM *e = NULL;
        BIGNUM *order = NULL;
        BIGNUM *sor = NULL;
        BIGNUM *eor = NULL;
        BIGNUM *field = NULL;
        EC_POINT *R = NULL;
        EC_POINT *O = NULL;
        EC_POINT *Q = NULL;
        BIGNUM *rr = NULL;
        BIGNUM *zero = NULL;
        int n = 0;
        int i = recid / 2;

        const EC_GROUP *group = EC_KEY_get0_group(eckey);
        if ((ctx = BN_CTX_new()) == NULL) { ret = -1; goto err; }
        BN_CTX_start(ctx);
        order = BN_CTX_get(ctx);
        if (!EC_GROUP_get_order(group, order, ctx)) { ret = -2; goto err; }
        x = BN_CTX_get(ctx);
        if (!BN_copy(x, order)) { ret=-1; goto err; }
        if (!BN_mul_word(x, i)) { ret=-1; goto err; }
        if (!BN_add(x, x, sig_r)) { ret=-1; goto err; }
        field = BN_CTX_get(ctx);
        if (!EC_GROUP_get_curve_GFp(group, field, NULL, NULL, ctx)) { ret=-2; goto err; }
        if (BN_cmp(x, field) >= 0) { ret=0; goto err; }
        if ((R = EC_POINT_new(group)) == NULL) { ret = -2; goto err; }
        if (!EC_POINT_set_compressed_coordinates_GFp(group, R, x, recid % 2, ctx)) { ret=0; goto err; }
        if (check)
        {
            if ((O = EC_POINT_new(group)) == NULL) { ret = -2; goto err; }
            if (!EC_POINT_mul(group, O, NULL, R, order, ctx)) { ret=-2; goto err; }
            if (!EC_POINT_is_at_infinity(group, O)) { ret = 0; goto err; }
        }
        if ((Q = EC_POINT_new(group)) == NULL) { ret = -2; goto err; }
        n = EC_GROUP_get_degree(group);
        e = BN_CTX_get(ctx);
        if (!BN_bin2bn(msg, msglen, e)) { ret=-1; goto err; }
        if (8*msglen > n) BN_rshift(e, e, 8-(n & 7));
        zero = BN_CTX_get(ctx);
        if (!BN_zero(zero)) { ret=-1; goto err; }
        if (!BN_mod_sub(e, zero, e, order, ctx)) { ret=-1; goto err; }
        rr = BN_CTX_get(ctx);
        if (!BN_mod_inverse(rr, sig_r, order, ctx)) { ret=-1; goto err; }
        sor = BN_CTX_get(ctx);
        if (!BN_mod_mul(sor, sig_s, rr, order, ctx)) { ret=-1; goto err; }
        eor = BN_CTX_get(ctx);
        if (!BN_mod_mul(eor, e, rr, order, ctx)) { ret=-1; goto err; }
        if (!EC_POINT_mul(group, Q, eor, R, sor, ctx)) { ret=-2; goto err; }
        if (!EC_KEY_set_public_key(eckey, Q)) { ret=-2; goto err; }

        ret = 1;

    err:
        if (ctx) {
            BN_CTX_end(ctx);
            BN_CTX_free(ctx);
        }
        if (R != NULL) EC_POINT_free(R);
        if (O != NULL) EC_POINT_free(O);
        if (Q != NULL) EC_POINT_free(Q);
        return ret;
    }

    void CKey::SetCompressedPubKey()
    {
        EC_KEY_set_conv_form(pkey, POINT_CONVERSION_COMPRESSED);
        fCompressedPubKey = true;
    }

    void CKey::Reset()
    {
        fCompressedPubKey = false;
        pkey = EC_KEY_new_by_curve_name(NID_sect571r1);
        if (pkey == NULL)
            throw key_error("CKey::CKey() : EC_KEY_new_by_curve_name failed");
        fSet = false;
    }

    CKey::CKey()
    {
        Reset();
    }

    CKey::CKey(const CKey& b)
    {
        pkey = EC_KEY_dup(b.pkey);
        if (pkey == NULL)
            throw key_error("CKey::CKey(const CKey&) : EC_KEY_dup failed");
        fSet = b.fSet;
    }

    CKey& CKey::operator=(const CKey& b)
    {
        if (!EC_KEY_copy(pkey, b.pkey))
            throw key_error("CKey::operator=(const CKey&) : EC_KEY_copy failed");
        fSet = b.fSet;
        return (*this);
    }

    CKey::~CKey()
    {
        EC_KEY_free(pkey);
    }

    bool CKey::IsNull() const
    {
        return !fSet;
    }

    bool CKey::IsCompressed() const
    {
        return fCompressedPubKey;
    }

    void CKey::MakeNewKey(bool fCompressed)
    {
        if (!EC_KEY_generate_key(pkey))
            throw key_error("CKey::MakeNewKey() : EC_KEY_generate_key failed");
        if (fCompressed)
            SetCompressedPubKey();
        fSet = true;
    }

    bool CKey::SetPrivKey(const CPrivKey& vchPrivKey)
    {
        const unsigned char* pbegin = &vchPrivKey[0];
        if (!d2i_ECPrivateKey(&pkey, &pbegin, vchPrivKey.size()))
            return false;
        fSet = true;
        return true;
    }

    bool CKey::SetSecret(const CSecret& vchSecret, bool fCompressed)
    {
        EC_KEY_free(pkey);
        pkey = EC_KEY_new_by_curve_name(NID_sect571r1);
        if (pkey == NULL)
            throw key_error("CKey::SetSecret() : EC_KEY_new_by_curve_name failed");
        if (vchSecret.size() != 72)
            throw key_error("CKey::SetSecret() : secret must be 32 bytes");
        BIGNUM *bn = BN_bin2bn(&vchSecret[0],72,BN_new());
        if (bn == NULL)
            throw key_error("CKey::SetSecret() : BN_bin2bn failed");
        if (!EC_KEY_regenerate_key(pkey,bn))
        {
            BN_clear_free(bn);
            throw key_error("CKey::SetSecret() : EC_KEY_regenerate_key failed");
        }
        BN_clear_free(bn);
        fSet = true;
        if (fCompressed || fCompressedPubKey)
            SetCompressedPubKey();
        return true;
    }

    CSecret CKey::GetSecret(bool &fCompressed) const
    {
        CSecret vchRet;
        vchRet.resize(72);
        const BIGNUM *bn = EC_KEY_get0_private_key(pkey);
        int nBytes = BN_num_bytes(bn);
        if (bn == NULL)
            throw key_error("CKey::GetSecret() : EC_KEY_get0_private_key failed");
        int n=BN_bn2bin(bn,&vchRet[72 - nBytes]);
        if (n != nBytes)
            throw key_error("CKey::GetSecret(): BN_bn2bin failed");
        fCompressed = fCompressedPubKey;
        return vchRet;
    }

    CPrivKey CKey::GetPrivKey() const
    {
        int nSize = i2d_ECPrivateKey(pkey, NULL);
        if (!nSize)
            throw key_error("CKey::GetPrivKey() : i2d_ECPrivateKey failed");
        CPrivKey vchPrivKey(nSize, 0);
        unsigned char* pbegin = &vchPrivKey[0];
        if (i2d_ECPrivateKey(pkey, &pbegin) != nSize)
            throw key_error("CKey::GetPrivKey() : i2d_ECPrivateKey returned unexpected size");
        return vchPrivKey;
    }

    bool CKey::SetPubKey(const std::vector<unsigned char>& vchPubKey)
    {
        const unsigned char* pbegin = &vchPubKey[0];
        if (!o2i_ECPublicKey(&pkey, &pbegin, vchPubKey.size()))
            return false;
        fSet = true;
        if (vchPubKey.size() >= 33)
            SetCompressedPubKey();
        return true;
    }

    std::vector<unsigned char> CKey::GetPubKey() const
    {
        int nSize = i2o_ECPublicKey(pkey, NULL);
        if (!nSize)
            throw key_error("CKey::GetPubKey() : i2o_ECPublicKey failed");
        std::vector<unsigned char> vchPubKey(nSize, 0);
        unsigned char* pbegin = &vchPubKey[0];
        if (i2o_ECPublicKey(pkey, &pbegin) != nSize)
            throw key_error("CKey::GetPubKey() : i2o_ECPublicKey returned unexpected size");
        return vchPubKey;
    }

    bool CKey::Sign(uint1024 hash, std::vector<unsigned char>& vchSig, int nBits)
    {
        unsigned int nSize = ECDSA_size(pkey);
        vchSig.resize(nSize); // Make sure it is big enough

        bool fSuccess = false;
        if(nBits == 256)
        {
            uint256 hash256 = hash.getuint256();
            fSuccess = (ECDSA_sign(0, (unsigned char*)&hash256, sizeof(hash256), &vchSig[0], &nSize, pkey) == 1);
        }
        else if(nBits == 512)
        {
            uint512 hash512 = hash.getuint512();
            fSuccess = (ECDSA_sign(0, (unsigned char*)&hash512, sizeof(hash512), &vchSig[0], &nSize, pkey) == 1);
        }
        else
            fSuccess = (ECDSA_sign(0, (unsigned char*)&hash, sizeof(hash), &vchSig[0], &nSize, pkey) == 1);

        if(!fSuccess)
        {
            vchSig.clear();
            return false;
        }

        vchSig.resize(nSize); // Shrink to fit actual size
        return true;
    }

    // create a compact signature (65 bytes), which allows reconstructing the used public key
    // The format is one header byte, followed by two times 32 bytes for the serialized r and s values.
    // The header byte: 0x1B = first key with even y, 0x1C = first key with odd y,
    //                  0x1D = second key with even y, 0x1E = second key with odd y
    bool CKey::SignCompact(uint256 hash, std::vector<unsigned char>& vchSig)
    {
        bool fOk = false;
        ECDSA_SIG *sig = ECDSA_do_sign((unsigned char*)&hash, sizeof(hash), pkey);
        if (sig == nullptr)
            throw key_error("CKey::SignCompact() : Failed to make signature");

        vchSig.clear();
        vchSig.resize(145,0);

        const BIGNUM* sig_r = nullptr;
        const BIGNUM* sig_s = nullptr;;
        #if OPENSSL_VERSION_NUMBER >= 0x10100000L
            ECDSA_SIG_get0(sig, &sig_r, &sig_s);
        #else
            sig_r = sig->r;
            sig_s = sig->s;
        #endif

        int nBitsR, nBitsS;
        #if OPENSSL_VERSION_NUMBER >= 0x10100000L
            nBitsR = BN_num_bits(sig_r);
            nBitsS = BN_num_bits(sig_s);
        #else
            nBitsR = BN_num_bits(sig->r);
            nBitsS = BN_num_bits(sig->s);
        #endif


        if (nBitsR <= 571 && nBitsS <= 571)
        {
            int nRecId = -1;
            for (int i=0; i < 9; i++)
            {
                CKey keyRec;
                keyRec.fSet = true;
                if (fCompressedPubKey)
                    keyRec.SetCompressedPubKey();

                #if OPENSSL_VERSION_NUMBER >= 0x10100000L
                if (ECDSA_SIG_recover_key_GFp(keyRec.pkey, sig_r, sig_s, (unsigned char*)&hash, sizeof(hash), i, 1) == 1)
                {
                    if (keyRec.GetPubKey() == this->GetPubKey())
                    {
                        nRecId = i;
                        break;
                    }
                }
                #else
                if (ECDSA_SIG_recover_key_GFp(keyRec.pkey, sig->r, sig->s, (unsigned char*)&hash, sizeof(hash), i, 1) == 1)
                {
                    if (keyRec.GetPubKey() == this->GetPubKey())
                    {
                        nRecId = i;
                        break;
                    }
                }
                #endif


            }

            if (nRecId == -1)
            {
                ECDSA_SIG_free(sig);
                throw key_error("CKey::SignCompact() : unable to construct recoverable key");
            }

            vchSig[0] = nRecId+27+(fCompressedPubKey ? 4 : 0);

            #if OPENSSL_VERSION_NUMBER >= 0x10100000L
                BN_bn2bin(sig_r, &vchSig[73-(nBitsR+7)/8]);
                BN_bn2bin(sig_s, &vchSig[145-(nBitsS+7)/8]);
            #else
                BN_bn2bin(sig->r, &vchSig[73-(nBitsR+7)/8]);
                BN_bn2bin(sig->s, &vchSig[145-(nBitsS+7)/8]);
            #endif

            fOk = true;
        }
        ECDSA_SIG_free(sig);

        return fOk;
    }

    // reconstruct public key from a compact signature
    // This is only slightly more CPU intensive than just verifying it.
    // If this function succeeds, the recovered public key is guaranteed to be valid
    // (the signature is a valid signature of the given data for that key)
    bool CKey::SetCompactSignature(uint256 hash, const std::vector<unsigned char>& vchSig)
    {
        if (vchSig.size() != 145)
            return false;
        int nV = vchSig[0];
        if (nV<27 || nV>=35)
            return false;
        ECDSA_SIG* sig = ECDSA_SIG_new();
        if (nullptr == sig)
            return false;

        #if OPENSSL_VERSION_NUMBER > 0x10100000L
            // sig_r and sig_s will be deallocated by ECDSA_SIG_free(sig)
            BIGNUM* sig_r = BN_bin2bn(&vchSig[1], 72, BN_new());
            BIGNUM* sig_s = BN_bin2bn(&vchSig[73], 72, BN_new());

            if ((sig_r == nullptr) || (sig_s == nullptr))
            {
                ECDSA_SIG_free(sig);
                throw key_error("CKey::VerifyCompact() : r or s can't be null");
            }

            // set r and s values, this transfers ownership to the ECDSA_SIG object
            ECDSA_SIG_set0(sig, sig_r, sig_s);
        #else
            BN_bin2bn(&vchSig[1], 72, sig->r);
            BN_bin2bn(&vchSig[73], 72, sig->s);
        #endif

        EC_KEY_free(pkey);
        pkey = EC_KEY_new_by_curve_name(NID_sect571r1);
        if (nV >= 31)
        {
            SetCompressedPubKey();
            nV -= 4;
        }

        #if OPENSSL_VERSION_NUMBER > 0x10100000L
        if (ECDSA_SIG_recover_key_GFp(pkey, sig_r, sig_s, (unsigned char*)&hash, sizeof(hash), nV - 27, 0) == 1)
        {
            fSet = true;
            ECDSA_SIG_free(sig);
            return true;
        }
        #else
        if (ECDSA_SIG_recover_key_GFp(pkey, sig->r, sig->s, (unsigned char*)&hash, sizeof(hash), nV - 27, 0) == 1)
        {
            fSet = true;
            ECDSA_SIG_free(sig);
            return true;
        }
        #endif

        ECDSA_SIG_free(sig);
        return false;
    }

    bool CKey::Verify(uint1024 hash, const std::vector<unsigned char>& vchSig, int nBits)
    {
        bool fSuccess = false;
        if(nBits == 256)
        {
            uint256 hash256 = hash.getuint256();
            fSuccess = (ECDSA_verify(0, (unsigned char*)&hash256, sizeof(hash256), &vchSig[0], vchSig.size(), pkey) == 1);
        }
        else if(nBits == 512)
        {
            uint512 hash512 = hash.getuint512();
            fSuccess = (ECDSA_verify(0, (unsigned char*)&hash512, sizeof(hash512), &vchSig[0], vchSig.size(), pkey) == 1);
        }
        else
            fSuccess = (ECDSA_verify(0, (unsigned char*)&hash, sizeof(hash), &vchSig[0], vchSig.size(), pkey) == 1);

        return fSuccess;
    }

    bool CKey::IsValid()
    {
        if (!fSet)
            return false;

        bool fCompr;
        CSecret secret = GetSecret(fCompr);
        CKey key2;
        key2.SetSecret(secret, fCompr);
        return GetPubKey() == key2.GetPubKey();
    }
}
