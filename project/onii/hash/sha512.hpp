#ifndef ONII_HASH_SHA512_HPP
#define ONII_HASH_SHA512_HPP

/////////////////////////////////////////////////
/// @file onii/hash/sha512.hpp
/////////////////////////////////////////////////

#include "detail/big_endian/add_bytes.hpp"
#include "detail/big_endian/to_uint.hpp"
#include "detail/circular_shift_right.hpp"
#include "detail/to_string.hpp"
#include "detail/ch.hpp"
#include "detail/maj.hpp"
#include "do_hash.hpp"

/////////////////////////////////////////////////
/// @namespace onii
/////////////////////////////////////////////////
namespace onii
{
/////////////////////////////////////////////////
/// @namespace onii::hash
/////////////////////////////////////////////////
namespace hash
{
/////////////////////////////////////////////////
/// @brief Calculate the SHA-512 hash of a string
///
/// @param[in] message - string to calculate
/// @return The SHA-512 hash
/////////////////////////////////////////////////
std::string sha512(std::string const &message)
{
    /*  Thanks http://en.wikipedia.org/wiki/SHA-2
        and https://tools.ietf.org/html/rfc4634 */

    // prepare the message
    std::vector<uint8_t> digest(std::begin(message), std::end(message));
    {
        // find new size
        uint32_t new_size = message.size() + 1;
        while(new_size % (1024/8) != 896/8)
            ++new_size;
        digest.resize(new_size);
    }

    // append the '1' bit; most significant bit is "first"
    digest[message.size()] = 0x80;

    // append "0" bits
    for(uint32_t i = message.size() + 1; i < digest.size(); ++i)
        digest[i] = 0x00;

    // append the size in 128 bits at the end of the buffer
    detail::big_endian::add_bytes(static_cast<uint64_t>(0), digest);
    detail::big_endian::add_bytes(static_cast<uint64_t>(message.size() * 8), digest);

    // these vars will contain the hash
    uint64_t h0 = 0x6a09e667f3bcc908;
    uint64_t h1 = 0xbb67ae8584caa73b;
    uint64_t h2 = 0x3c6ef372fe94f82b;
    uint64_t h3 = 0xa54ff53a5f1d36f1;
    uint64_t h4 = 0x510e527fade682d1;
    uint64_t h5 = 0x9b05688c2b3e6c1f;
    uint64_t h6 = 0x1f83d9abfb41bd6b;
    uint64_t h7 = 0x5be0cd19137e2179;

    // constants defined in SHA-512
    uint64_t k[80] = {
        0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f,
        0xe9b5dba58189dbbc, 0x3956c25bf348b538, 0x59f111f1b605d019,
        0x923f82a4af194f9b, 0xab1c5ed5da6d8118, 0xd807aa98a3030242,
        0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
        0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235,
        0xc19bf174cf692694, 0xe49b69c19ef14ad2, 0xefbe4786384f25e3,
        0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65, 0x2de92c6f592b0275,
        0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
        0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f,
        0xbf597fc7beef0ee4, 0xc6e00bf33da88fc2, 0xd5a79147930aa725,
        0x06ca6351e003826f, 0x142929670a0e6e70, 0x27b70a8546d22ffc,
        0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
        0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6,
        0x92722c851482353b, 0xa2bfe8a14cf10364, 0xa81a664bbc423001,
        0xc24b8b70d0f89791, 0xc76c51a30654be30, 0xd192e819d6ef5218,
        0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
        0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99,
        0x34b0bcb5e19b48a8, 0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb,
        0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3, 0x748f82ee5defb2fc,
        0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
        0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915,
        0xc67178f2e372532b, 0xca273eceea26619c, 0xd186b8c721c0c207,
        0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178, 0x06f067aa72176fba,
        0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
        0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc,
        0x431d67c49c100d4c, 0x4cc5d4becb3e42b6, 0x597f299cfc657e2a,
        0x5fcb6fab3ad6faec, 0x6c44198c4a475817
    };

    // process the message in successive 1024-bits chunks
    for(uint32_t o = 0; o < digest.size(); o += (1024/8))
    {
        // break chunk into sixteen 64-bit words w[j]
        uint64_t w[80];
        for(uint32_t j = 0; j < 16; ++j)
            w[j] = detail::big_endian::to_uint<uint64_t>(digest, o + j*8);

        // extend the sixteen 64-bit words into eighties 64-bit words
        for(uint32_t j = 16; j < 80; ++j)
        {
            uint64_t s0 = detail::circular_shift_right(w[j-15], 1)
                        ^ detail::circular_shift_right(w[j-15], 8)
                        ^ (w[j-15] >> 7);
            uint64_t s1 = detail::circular_shift_right(w[j-2], 19)
                        ^ detail::circular_shift_right(w[j-2], 61)
                        ^ (w[j-2] >> 6);
            w[j] = w[j-16] + s0 + w[j-7] + s1;
        }

        // initialize hash value for this chunk
        uint64_t a = h0;
        uint64_t b = h1;
        uint64_t c = h2;
        uint64_t d = h3;
        uint64_t e = h4;
        uint64_t f = h5;
        uint64_t g = h6;
        uint64_t h = h7;

        // main loop
        for(uint32_t i = 0; i < 80; ++i)
        {
            uint64_t S0 = detail::circular_shift_right(a, 28)
                        ^ detail::circular_shift_right(a, 34)
                        ^ detail::circular_shift_right(a, 39);
            uint64_t S1 = detail::circular_shift_right(e, 14)
                        ^ detail::circular_shift_right(e, 18)
                        ^ detail::circular_shift_right(e, 41);
            uint64_t T1 = h + S1 + detail::ch(e, f, g) + k[i] + w[i];
            uint64_t T2 = S0 + detail::maj(a, b, c);

            h = g;
            g = f;
            f = e;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;
        }

        // add this chunk's hash to result
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
        h5 += f;
        h6 += g;
        h7 += h;
    }

    // cleanup
    digest.clear();

    // get the hash
    detail::big_endian::add_bytes(h0, digest);
    detail::big_endian::add_bytes(h1, digest);
    detail::big_endian::add_bytes(h2, digest);
    detail::big_endian::add_bytes(h3, digest);
    detail::big_endian::add_bytes(h4, digest);
    detail::big_endian::add_bytes(h5, digest);
    detail::big_endian::add_bytes(h6, digest);
    detail::big_endian::add_bytes(h7, digest);

    // prepare result
    return detail::to_string(digest);
}
}
}

#endif // ONII_HASH_SHA512_HPP
