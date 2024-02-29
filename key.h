#ifndef KEY_H
#define KEY_H

#include <QString>
#include <QVector>

namespace key {

class Key
{
    const int _N = 8;
public:
    Key() {
        keys_str.resize(_N);
    }
    void set_key(int key, int idx) {
        keys_str[idx] = QString("%1").arg(QString::number(key, 16), 4, QChar('0'));
        update_key();
    }
    auto get_str_key() const {
        return key;
    }
    int get_key(int idx) const {
        bool ok;
        return keys_str[idx].toInt(&ok, 16);
    }
    int N() const {
        return _N;
    }
private:
    QString key {};
    QVector<QString> keys_str{};
    void update_key() {
        key.clear();
        for (auto it=keys_str.begin(); it != keys_str.end(); it++) {
            key.push_back( *it );
            key.push_back(" ");
        }
    }
};

}

#endif // KEY_H
