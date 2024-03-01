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
