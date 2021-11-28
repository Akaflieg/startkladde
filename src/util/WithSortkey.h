#ifndef WITHSORTKEY_H
#define WITHSORTKEY_H

template <class T, class K>
class WithSortkey {
private:
    T thing;
    K key;
public:
    WithSortkey(T thing, K key) : thing(thing), key(key) { }
    WithSortkey(WithSortkey const& other) : thing(other.thing), key(other.key) { }

    T const& getThing() const { return thing; }
    const K getKey() const { return key; }

    bool operator< (WithSortkey const& other) {
        return this->key > other.key;
    }

    bool operator== (WithSortkey const& other) {
        return this->thing == other.thing;
    }

    WithSortkey& operator= (WithSortkey const& other) {
        this->thing = other.thing;
        this->key = other.key;
        return *this;
    }
};

#endif // WITHSORTKEY_H
