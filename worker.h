#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QFuture>

#include "stream_cipher.h"

class worker : public QObject
{
public:
    QFuture<lfsr_rng::gens> seed(lfsr_rng::STATE);
    QFuture<QVector<lfsr8::u64> > gen_n(lfsr_rng::gens &g, int n);
    QFuture<lfsr8::u64 > check_period(lfsr_rng::gens &g, lfsr8::u64& ref);
private:
    ;
};



#endif // WORKER_H
