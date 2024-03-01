#pragma once

/*
Random numbers generator with ~155 bit period.
*/

#include "lfsr_hash/lfsr.h"

#include <utility>
#include <cmath>
#include <numeric>
#include <array>

namespace lfsr_rng {

using u16 = lfsr8::u16;
using u32 = lfsr8::u32;
using u64 = lfsr8::u64;
using u128 = std::pair<lfsr8::u64, lfsr8::u64>;

static constexpr std::array<int, 4> p {19, 17, 17, 13};
static constexpr int m = 8;

using LFSR_pair_1 = lfsr8::LFSR_paired_2x4<p[0]>;
using LFSR_pair_2 = lfsr8::LFSR_paired_2x4<p[1]>;
using LFSR_pair_3 = lfsr8::LFSR_paired_2x4<p[2]>;
using LFSR_pair_4 = lfsr8::LFSR_paired_2x4<p[3]>;
using STATE = lfsr8::u16x8;

static constexpr STATE K1 = {9, 5, 2, 0, 4, 2, 2, 6};    // p=19
static constexpr STATE K2 = {3, 4, 2, 1, 6, 1, 2, 1};    // p=17
static constexpr STATE K3 = {3, 2, 3, 4, 6, 2, 0, 7};    // p=17
static constexpr STATE K4 = {2, 3, 1, 1, 2, 0, 1, 7};    // p=13
static constexpr std::array<int, 4> primes {7, 11, 11, 11}; // Sawtooth periods: such that T = p^4 - 1 is not divisible by the primes

//
static constexpr std::array<int, 8> primes_duplicates {7, 7, 11, 11, 11, 11, 11, 11};

static_assert((primes[0] == primes_duplicates[0]) && (primes[0] == primes_duplicates[1]));
static_assert((primes[1] == primes_duplicates[2]) && (primes[1] == primes_duplicates[3]));
static_assert((primes[2] == primes_duplicates[4]) && (primes[2] == primes_duplicates[5]));
static_assert((primes[3] == primes_duplicates[6]) && (primes[3] == primes_duplicates[7]));

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

template <size_t N>
inline void sawtooth(std::array<u16, N>& v, const std::array<int, N>& p) {
    size_t i = 0;
    for (auto& el : v) {
        el++;
        el %= p[i];
        i++;
    }
}

template <size_t N>
inline void increment(std::array<u32, N>& v) {
    for (auto& el : v) {
        el++;
    }
}

struct gens {
    using lfsr_ptr = lfsr8::LFSR_paired_2x4<p[0]>*;
    LFSR_pair_1 gp1;
    LFSR_pair_2 gp2;
    LFSR_pair_3 gp3;
    LFSR_pair_4 gp4;
    std::array<u32, 8> Tc{};
    std::array<u32, 8> Tref{};
    std::array<u16, 8> ii_saw{};  // Sawtooth states
    std::array<u16, 8> ii0_saw{};  // Stored initial states
    int is_finded;
public:
    constexpr gens(): gp1(K1), gp2(K2),  gp3(K3), gp4(K4),
        is_finded(0)
    {}
    bool is_succes() const {
        return (is_finded != 0);
    }
    // dirty hack
    lfsr_ptr gp(int i) {
        void* ptrs[4] = {&gp1, &gp2, &gp3, &gp4};
        return (lfsr_ptr)ptrs[i];
    }
    void seed(STATE st) {
        // distribute the state "st" to all LFSRs
        std::array<u16, 4> h {1, 2, 2, 3}; // a salt
        for (int i=0; i<4; ++i) {
            STATE tmp = st;
            for (auto& el : tmp) {
                el >>= i*4;
                el %= 16;
                h[i] ^= el;
            }
            gp(i)->set_state(tmp);
        }
        int lcm = std::lcm((int)primes[0], (int)primes[1]);
        lcm = std::lcm(lcm, (int)primes[2]);
        lcm = std::lcm(lcm, (int)primes[3]);
        for (int i=0; i<lcm; ++i) { // saturate LFSRs enough
            gp1.next(h[0]);
            gp2.next(h[1]);
            gp3.next(h[2]);
            gp4.next(h[3]);
            sawtooth<4>(h, primes);
        }
        const auto ref1 = gp1.get_state();
        const auto ref2 = gp2.get_state();
        const auto ref3 = gp3.get_state();
        const auto ref4 = gp4.get_state();
        auto test251 = [ref1, ref2, ref3, ref4, this](u16 i01, u16 i02, u16 i03, u16 i04) -> int {
            Tc = {1, 1, 1, 1, 1, 1, 1, 1};   // counters
            Tref = {0, 0, 0, 0, 0, 0, 0, 0}; // reference periods
            const std::array<u32, 4> Tmax {(u32)std::pow((long)p[0], 4) - 1,
                                          (u32)std::pow((long)p[1], 4) - 1,
                                          (u32)std::pow((long)p[2], 4) - 1,
                                              (u32)std::pow((long)p[3], 4) - 1}; // maximal possible periods
            gp1.set_state(ref1);
            gp2.set_state(ref2);
            gp3.set_state(ref3);
            gp4.set_state(ref4);
            std::array<u16, 4> i {i01, i02, i03, i04};
            while (true) {
                gp1.next(i[0]); //  Sawtooth modulation
                gp2.next(i[1]); //  to get random periods T[i] such that sum of T[i] is equal to q*T0, where the q - sawtooth period
                gp3.next(i[2]);
                gp4.next(i[3]);
                //  The remainder mod(p^4 - 1 , q) is not zero => we achieve all indexes i in [0, q) when LFSR is in the same reference state
                sawtooth(i, primes); // We will visit almost all i ecxept one => we set the restriction T < q*T0 below
                Tref[0] = (! gp1.is_state_low(ref1)) ? Tref[0] : ((Tc[0] < primes_duplicates[0]*Tmax[0]) ? Tc[0] : Tref[0]);
                Tref[1] = (! gp1.is_state_high(ref1)) ? Tref[1] : ((Tc[1] < primes_duplicates[1]*Tmax[0]) ? Tc[1] : Tref[1]);
                Tref[2] = (! gp2.is_state_low(ref2)) ? Tref[2] : ((Tc[2] < primes_duplicates[2]*Tmax[1]) ? Tc[2] : Tref[2]);
                Tref[3] = (! gp2.is_state_high(ref2)) ? Tref[3] : ((Tc[3] < primes_duplicates[3]*Tmax[1]) ? Tc[3] : Tref[3]);
                Tref[4] = (! gp3.is_state_low(ref3)) ? Tref[4] : ((Tc[4] < primes_duplicates[4]*Tmax[2]) ? Tc[4] : Tref[4]);
                Tref[5] = (! gp3.is_state_high(ref3)) ? Tref[5] : ((Tc[5] < primes_duplicates[5]*Tmax[2]) ? Tc[5] : Tref[5]);
                Tref[6] = (! gp4.is_state_low(ref4)) ? Tref[6] : ((Tc[6] < primes_duplicates[6]*Tmax[3]) ? Tc[6] : Tref[6]);
                Tref[7] = (! gp4.is_state_high(ref4)) ? Tref[7] : ((Tc[7] < primes_duplicates[7]*Tmax[3]) ? Tc[7] : Tref[7]);
                increment(Tc);
                // All counters are out of the range
                if ((Tc[0] >= primes_duplicates[0]*Tmax[0]) &&
                    (Tc[1] >= primes_duplicates[1]*Tmax[0]) &&
                    (Tc[2] >= primes_duplicates[2]*Tmax[1]) &&
                    (Tc[3] >= primes_duplicates[3]*Tmax[1]) &&
                    (Tc[4] >= primes_duplicates[4]*Tmax[2]) &&
                    (Tc[5] >= primes_duplicates[5]*Tmax[2]) &&
                    (Tc[6] >= primes_duplicates[6]*Tmax[3]) &&
                    (Tc[7] >= primes_duplicates[7]*Tmax[3])) {
                    break;
                }
            }
            auto gcd1 = std::gcd(std::gcd(Tref[0], Tref[1]), std::gcd(Tref[2], Tref[3]));
            auto gcd2 = std::gcd(std::gcd(Tref[4], Tref[5]), std::gcd(Tref[6], Tref[7]));
            auto gcd = std::gcd(gcd1, gcd2);
            return ((gcd < 2) &&
                    (Tref[0] > Tmax[0]) &&
                    (Tref[1] > Tmax[0]) &&
                    (Tref[2] > Tmax[1]) &&
                    (Tref[3] > Tmax[1]) &&
                    (Tref[4] > Tmax[2]) &&
                    (Tref[5] > Tmax[2]) &&
                    (Tref[6] > Tmax[3]) &&
                    (Tref[7] > Tmax[3])) ? 1 : 0;
        };
        is_finded = 0;
        for (u16 i1=1; i1<primes[0]; ++i1) {
            for (u16 i2=1; i2<primes[1]; ++i2) {
                for (u16 i3=1; i3<primes[2]; ++i3) {
                    for (u16 i4=1; i4<primes[3]; ++i4) {
                        is_finded = test251(i1, i2, i3, i4);
                        if (is_finded != 0) {
                            ii0_saw[0] = i1;
                            ii0_saw[1] = i1;
                            ii0_saw[2] = i2;
                            ii0_saw[3] = i2;
                            ii0_saw[4] = i3;
                            ii0_saw[5] = i3;
                            ii0_saw[6] = i4;
                            ii0_saw[7] = i4;
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
        ii_saw = ii0_saw; // initialize sawtooth
        Tc = {1, 1, 1, 1, 1, 1, 1, 1}; // reset counters
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
            gp1.next(ii_saw[0], ii_saw[1]); // must: the same operator as in the seed()
            gp2.next(ii_saw[2], ii_saw[3]);
            gp3.next(ii_saw[4], ii_saw[5]);
            gp4.next(ii_saw[6], ii_saw[7]);
            sawtooth<8>(ii_saw, primes_duplicates);
            // reset if the period was achieved
            for (int j=0; j<8; ++j) {
                ii_saw[j] = (Tc[j] != Tref[j]) ? ii_saw[j] : ii0_saw[j];
                Tc[j] = (Tc[j] != Tref[j]) ? Tc[j] : 0;
            }
            increment(Tc);
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
