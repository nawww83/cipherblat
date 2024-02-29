#pragma once

/*
Random numbers generator with ~155 bit period.
*/

#include "../lfsr_hash/lfsr.h"

#include <utility>
#include <cmath>
#include <numeric>
#include <array>


namespace lfsr_rng {

using u16 = lfsr8::u16;
using u32 = lfsr8::u32;
using u64 = lfsr8::u64;
using u128 = std::pair<lfsr8::u64, lfsr8::u64>;

static constexpr u16 p1 = 19;
static constexpr u16 p2 = 17;
static constexpr u16 p3 = 17;
static constexpr u16 p4 = 13;
static constexpr int m = 8;

using LFSR_pair_1 = lfsr8::LFSR_paired_2x4<p1>;
using LFSR_pair_2 = lfsr8::LFSR_paired_2x4<p2>;
using LFSR_pair_3 = lfsr8::LFSR_paired_2x4<p3>;
using LFSR_pair_4 = lfsr8::LFSR_paired_2x4<p4>;
using STATE = lfsr8::u16x8;

static constexpr STATE K1 = {9, 5, 2, 0, 4, 2, 2, 6};    // p=19
static constexpr STATE K2 = {3, 4, 2, 1, 6, 1, 2, 1};    // p=17
static constexpr STATE K3 = {3, 2, 3, 4, 6, 2, 0, 7};    // p=17
static constexpr STATE K4 = {2, 3, 1, 1, 2, 0, 1, 7};    // p=13
static constexpr std::array<u16, 4> primes {7, 11, 11, 11}; // Sawtooth periods: such that T = p^4 - 1 is not divisible by the primes

inline STATE operator^(const STATE& x, const STATE& y) {
    STATE st;
    for (int i=0; i<m; ++i) {
        st[i] = x[i] ^ y[i];
    }
    return st;
}

inline STATE operator%(const STATE& x, u32 p) {
    STATE st;
    for (int i=0; i<m; ++i) {
        st[i] = x[i] % p;
    }
    return st;
}

inline void operator%=(STATE& x, u32 p) {
    for (int i=0; i<m; ++i) {
        x[i] %= p;
    }
}


struct gens {
    LFSR_pair_1 gp1;
    LFSR_pair_2 gp2;
    LFSR_pair_3 gp3;
    LFSR_pair_4 gp4;
    std::array<u32, 16> T;
    u16 ii11;  // Sawtooth initial states
    u16 ii12;  //
    u16 ii21;  //
    u16 ii22;  //
    u16 ii31;  //
    u16 ii32;  //
    u16 ii41;  //
    u16 ii42;  //
    u16 ii01;  // initial states
    u16 ii02;  //
    u16 ii03;  //
    u16 ii04;  //
    int is_finded;
public:
    constexpr gens(): gp1(K1), gp2(K2),  gp3(K3), gp4(K4), T({}),
        ii11(0), ii12(0), ii21(0), ii22(0), ii31(0), ii32(0), ii41(0), ii42(0),
        ii01(0), ii02(0), ii03(0), ii04(0),
        is_finded(0)
    {}
    bool is_succes() const {
        return (is_finded != 0);
    }
    void seed(STATE st) {
        STATE tmp1 = st;
        STATE tmp2 = st;
        STATE tmp3 = st;
        STATE tmp4 = st;
        for (auto& el : tmp1) {
            el %= 16;
        }
        for (auto& el : tmp2) {
            el >>= 4;
            el %= 16;
        }
        for (auto& el : tmp3) {
            el >>= 8;
            el %= 16;
        }
        for (auto& el : tmp4) {
            el >>= 12;
            el %= 16;
        }
        gp1.set_state(tmp1);
        gp2.set_state(tmp2);
        gp3.set_state(tmp3);
        gp4.set_state(tmp4);
        u16 h1 = 1;
        for (const auto& el : tmp1) {
            h1 ^= el;
        }
        u16 h2 = 2;
        for (const auto& el : tmp2) {
            h2 ^= el;
        }
        u16 h3 = 2;
        for (const auto& el : tmp3) {
            h3 ^= el;
        }
        u16 h4 = 3;
        for (const auto& el : tmp4) {
            h4 ^= el;
        }
        u16 i1 = h1;
        u16 i2 = h2;
        u16 i3 = h3;
        u16 i4 = h4;
        int lcm = std::lcm((int)primes[0], (int)primes[1]);
        lcm = std::lcm(lcm, (int)primes[2]);
        lcm = std::lcm(lcm, (int)primes[3]);
        for (int i=0; i<lcm; ++i) { // saturate LFSRs
            gp1.next(i1);
            gp2.next(i2);
            gp3.next(i3);
            gp4.next(i4);
            i1++; i2++; i3++; i4++;
            i1 %= primes[0];
            i2 %= primes[1];
            i3 %= primes[2];
            i4 %= primes[3];
        }
        const auto ref1 = gp1.get_state();
        const auto ref2 = gp2.get_state();
        const auto ref3 = gp3.get_state();
        const auto ref4 = gp4.get_state();
        auto test251 = [ref1, ref2, ref3, ref4, this](u16 i01, u16 i02, u16 i03, u16 i04) -> int {
            T = {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
            const u32 T01 = std::pow((long)p1, 4) - 1;
            const u32 T02 = std::pow((long)p2, 4) - 1;
            const u32 T03 = std::pow((long)p3, 4) - 1;
            const u32 T04 = std::pow((long)p4, 4) - 1;
            gp1.set_state(ref1);
            gp2.set_state(ref2);
            gp3.set_state(ref3);
            gp4.set_state(ref4);
            u16 i1 = i01;
            u16 i2 = i02;
            u16 i3 = i03;
            u16 i4 = i04;
            while (true) {
                gp1.next(i1); //  Sawtooth modulation
                gp2.next(i2); //  to get random periods T[i] such that sum of T[i] is equal to q*T0, where the q - sawtooth period
                gp3.next(i3);
                gp4.next(i4);
                i1++; i2++;   //  The remainder mod(p^4 - 1 , q) is not zero => we achieve all indexes i in [0, q) when LFSR is in the same reference state
                i3++; i4++;
                i1 %= primes[0];
                i2 %= primes[1]; // We visit almost all i ecxept one => we set the restriction T < q*T0 below
                i3 %= primes[2];
                i4 %= primes[3];
                T[8] = (! gp1.is_state_low(ref1)) ? T[8] : ((T[0] < primes[0]*T01) ? T[0] : T[8]);
                T[9] = (! gp1.is_state_high(ref1)) ? T[9] : ((T[1] < primes[0]*T01) ? T[1] : T[9]);
                T[10] = (! gp2.is_state_low(ref2)) ? T[10] : ((T[2] < primes[1]*T02) ? T[2] : T[10]);
                T[11] = (! gp2.is_state_high(ref2)) ? T[11] : ((T[3] < primes[1]*T02) ? T[3] : T[11]);
                T[12] = (! gp3.is_state_low(ref3)) ? T[12] : ((T[4] < primes[2]*T03) ? T[4] : T[12]);
                T[13] = (! gp3.is_state_high(ref3)) ? T[13] : ((T[5] < primes[2]*T03) ? T[5] : T[13]);
                T[14] = (! gp4.is_state_low(ref4)) ? T[14] : ((T[6] < primes[3]*T04) ? T[6] : T[14]);
                T[15] = (! gp4.is_state_high(ref4)) ? T[15] : ((T[7] < primes[3]*T04) ? T[7] : T[15]);
                T[0]++; T[1]++;
                T[2]++; T[3]++;
                T[4]++; T[5]++;
                T[6]++; T[7]++;
                // All counters are out of the range
                if ((T[0] >= primes[0]*T01) && (T[1] >= primes[0]*T01) && (T[2] >= primes[1]*T02) && (T[3] >= primes[1]*T02) &&
                    (T[4] >= primes[2]*T03) && (T[5] >= primes[2]*T03) && (T[6] >= primes[3]*T04) && (T[7] >= primes[3]*T04)) {
                    break;
                }
            }
            auto gcd1 = std::gcd(std::gcd(T[8], T[9]), std::gcd(T[10], T[11]));
            auto gcd2 = std::gcd(std::gcd(T[12], T[13]), std::gcd(T[14], T[15]));
            auto gcd = std::gcd(gcd1, gcd2);
            return ((gcd < 2) && (T[8] > T01) && (T[9] > T01) && (T[10] > T02) && (T[11] > T02) &&
                    (T[12] > T03) && (T[13] > T03) && (T[14] > T04) && (T[15] > T04)) ? 1 : 0;
        };
        is_finded = 0;
        ii01 = 0;
        ii02 = 0;
        ii03 = 0;
        ii04 = 0;
        for (u16 i1=1; i1<primes[0]; ++i1) {
            for (u16 i2=1; i2<primes[1]; ++i2) {
                for (u16 i3=1; i3<primes[2]; ++i3) {
                    for (u16 i4=1; i4<primes[3]; ++i4) {
                        is_finded = test251(i1, i2, i3, i4);
                        if (is_finded != 0) {
                            ii01 = i1;
                            ii02 = i2;
                            ii03 = i3;
                            ii04 = i4;
                            break;
                        }
                    } // i4
                    if (is_finded != 0) {
                        break;
                    }
                } // i3
                if (is_finded != 0) {
                    break;
                }
            } // i2
            if (is_finded != 0) {
                break;
            }
        } // i1
        ii11 = ii01;
        ii12 = ii01;
        ii21 = ii02;
        ii22 = ii02;
        ii31 = ii03;
        ii32 = ii03;
        ii41 = ii04;
        ii42 = ii04;
        T[0] = 1; // reset counters
        T[1] = 1;
        T[2] = 1;
        T[3] = 1;
        T[4] = 1;
        T[5] = 1;
        T[6] = 1;
        T[7] = 1;
        gp1.set_state(ref1); // must: restore initial states
        gp2.set_state(ref2);
        gp3.set_state(ref3);
        gp4.set_state(ref4);
    }
    u64 next_u64() {
        u64 x = 0;
        STATE mSt;
        //
        for (int i=0; i<4; ++i) {
            gp1.next(ii11, ii12); // must: the same operator as in the seed()
            gp2.next(ii21, ii22);
            gp3.next(ii31, ii32);
            gp4.next(ii41, ii42);
            ii11++; ii12++; ii21++; ii22++;
            ii31++; ii32++; ii41++; ii42++;
            ii11 %= primes[0];  // Sawtooth
            ii12 %= primes[0];
            ii21 %= primes[1];
            ii22 %= primes[1];
            ii31 %= primes[2];
            ii32 %= primes[2];
            ii41 %= primes[3];
            ii42 %= primes[3];
            ii11 = (T[0] != T[8]) ? ii11 : ii01;
            ii12 = (T[1] != T[9]) ? ii12 : ii01;
            ii21 = (T[2] != T[10]) ? ii21 : ii02;
            ii22 = (T[3] != T[11]) ? ii22 : ii02;
            ii31 = (T[4] != T[12]) ? ii31 : ii03;
            ii32 = (T[5] != T[13]) ? ii32 : ii03;
            ii41 = (T[6] != T[14]) ? ii41 : ii04;
            ii42 = (T[7] != T[15]) ? ii42 : ii04;
            T[0] = (T[0] != T[8]) ? T[0] : 0; // reset counters
            T[1] = (T[1] != T[9]) ? T[1] : 0;
            T[2] = (T[2] != T[10]) ? T[2] : 0;
            T[3] = (T[3] != T[11]) ? T[3] : 0;
            T[4] = (T[4] != T[12]) ? T[4] : 0;
            T[5] = (T[5] != T[13]) ? T[5] : 0;
            T[6] = (T[6] != T[14]) ? T[6] : 0;
            T[7] = (T[7] != T[15]) ? T[7] : 0;
            T[0]++; T[1]++; T[2]++; T[3]++;
            T[4]++; T[5]++; T[6]++; T[7]++;
            //
            mSt = gp1.get_state() ^ gp2.get_state() ^ gp3.get_state() ^ gp4.get_state();
            mSt %= 16;
            x <<= 4;
            x |= mSt[0] ^ mSt[4];
            x <<= 4;
            x |= mSt[1] ^ mSt[5];
            x <<= 4;
            x |= mSt[2] ^ mSt[6];
            x <<= 4;
            x |= mSt[3] ^ mSt[7];
        }
        //
        return x;
    }
};

}
