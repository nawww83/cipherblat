#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QFuture>

#include "stream_cipher.h"

class worker : public QObject
{
public:
    QFuture<lfsr_rng::gens> seed(lfsr_rng::STATE);
    QFuture<QVector<lfsr8::u64> > gen_n(lfsr_rng::gens &g, int n); // generator by reference
    QFuture<lfsr8::u64 > check_period(lfsr_rng::gens &g, lfsr8::u64& ref);
    void set_period_checking_flag(int x) {
        mStopPeriodChecking.storeRelaxed(x);
    }
private:
    QAtomicInt mStopPeriodChecking{0};
};



#endif // WORKER_H
