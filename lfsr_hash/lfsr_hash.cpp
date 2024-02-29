#include "lfsr_hash.h"
#include "io_utils.h"

namespace {
io_u::io_utils io;
lfsr_hash::gens g;
}

void lfsr_hash::gens::process_input(const uint8_t* input, size_t n) {
    assert(n > 0);
    if (n > 1) {
        for (size_t i=0; i<n/2; ++i) {
            u16 tmp1;
            u16 tmp2;
            io.read_mem_16(tmp1, input + 2*i, sizeof(u16));
            io.read_mem_16(tmp2, input + n - 2 - 2*i, sizeof(u16));
            // g_251x4.next(*(u16*)(input + 2*i));
            // g_241x4.next(*(u16*)(input + n - 2 - 2*i));
            g_251x4.next(tmp1);
            g_241x4.next(tmp2);
        }
    }
    if (n > 2) { // to pass 1.1 and 1.2 tests, see main.cpp
        {
            u16 tmp1;
            u16 tmp2;
            io.read_mem_16(tmp1, input + 1, sizeof(u16));
            io.read_mem_16(tmp2, input + n - 3, sizeof(u16));
            // g_251x4.next(*(u16*)(input + 1));
            // g_241x4.next(*(u16*)(input + n - 3));
            // g_251x4.next(*(u16*)(input + 1));
            // g_241x4.next(*(u16*)(input + n - 3));
            // g_251x4.next(*(u16*)(input + 1));
            // g_241x4.next(*(u16*)(input + n - 3));
            g_251x4.next(tmp1);
            g_241x4.next(tmp2);
            g_251x4.next(tmp1);
            g_241x4.next(tmp2);
            g_251x4.next(tmp1);
            g_241x4.next(tmp2);
        }
    }
    u16 x = (u16)input[0] | ((u16)(input[0]) << 8);
    g_251x4.next(x);
    g_241x4.next(x);
}


lfsr_hash::u32 lfsr_hash::hash32(const uint8_t* input, size_t n) {
    g.reset();
    g.add_salt( ((n % 2) == 0) ? S1 : S0 );
    g.process_input(input, n);
    g.add_salt( ((n % 2) == 0) ? S2 : S1 );
    u32 h = g.form_hash32();
    return h;
}

lfsr_hash::u64 lfsr_hash::hash64(const uint8_t* input, size_t n) {
    g.reset();
    g.add_salt( ((n % 2) == 0) ? S1 : S2 );
    g.process_input(input, n);
    g.add_salt( ((n % 2) == 0) ? S3 : S1 );
    u64 h1 = g.form_hash32();
    g.add_salt( ((n % 2) == 0) ? S1 : S2 );
    u64 h2 = g.form_hash32();
    return (h1 << 32) | h2;
}

lfsr_hash::u128 lfsr_hash::hash128(const uint8_t* input, size_t n) {
    g.reset();
    g.add_salt(S1);
    g.add_salt(S0);
    g.add_salt( ((n % 2) == 0) ? S0 : S4 );
    g.process_input(input, n);
    g.add_salt(S0);
    g.add_salt(S1);
    g.add_salt( ((n % 2) == 0) ? S1 : S0 );

    u64 h1 = g.form_hash32();
    g.add_salt( ((n % 2) == 0) ? S3 : S2 );
    g.add_salt( ((n % 2) == 0) ? S4 : S2 );
    u64 h2 = g.form_hash32();
    g.add_salt( ((n % 2) == 0) ? S2 : S3 );
    g.add_salt( ((n % 2) == 0) ? S3 : S1 );
    u64 h3 = g.form_hash32();
    g.add_salt( ((n % 2) == 0) ? S4 : S2 );
    g.add_salt( ((n % 2) == 0) ? S2 : S0 );
    u64 h4 = g.form_hash32();

    return {
        h1 | (h2 << 32),
        h3 | (h4 << 32)
    };
}
