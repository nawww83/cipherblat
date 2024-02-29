#include "worker.h"
#include <QtConcurrent/QtConcurrent>
#include <QPair>


QFuture<lfsr_rng::gens> worker::seed(lfsr_rng::STATE st) {
    auto f = [](lfsr_rng::STATE st) {
        lfsr_rng::gens g;
        g.seed(st);
        return g;
    };
    return QtConcurrent::run(f, st);
}

QFuture<QVector<lfsr8::u64> > worker::gen_n(lfsr_rng::gens& g, int n)
{
    auto f = [&g](int n) {
        QVector<lfsr8::u64> v{};
        v.reserve(n);
        for (int i=0; i<n; ++i) {
            v.push_back( g.next_u64() );
        }
        QThread::msleep(20); // do not rush! :)
        return v;
    };
    return QtConcurrent::run(f, n);
}

QFuture<lfsr8::u64 > worker::check_period(lfsr_rng::gens& g, lfsr8::u64 &ref)
{
    auto f = [&g, &ref, this]() {
        mStopPeriodChecking.storeRelaxed(0);
        QPair<lfsr8::u64, lfsr8::u64> p;
        lfsr8::u64 T = 1;
        lfsr8::u64 ref0 = ref;
        const lfsr8::u64 mask = (1ull << 32) - 1;
        int c = 0;
        while (true) {
            c++;
            ref = g.next_u64();
            if ((ref0 & mask) != (ref & mask)) {
                T++;
                if ((c & 65535) != 0) {
                    continue;
                }
                if (mStopPeriodChecking.loadRelaxed()) {
                    break;
                }
                continue;
            }
            break;
        }
        return T;
    };
    return QtConcurrent::run(f);
}
