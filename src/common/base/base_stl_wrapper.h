
//
//  base_stl_wrapper.h
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_STL_WRAPPER_H
#define BASE_STL_WRAPPER_H

#include "base_reference.h"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#ifdef _MSC_VER

namespace stdext {

template<typename T> struct hash {
    size_t operator()(T val) const
    {
        return std::hash<T>()(val.c_str());
    }
};

template<> struct hash<std::string> {
    size_t operator()(const std::string& val) const
    {
        return std::hash<const char*>()(val.c_str());
    }
};

template<> struct hash<long long> {
    size_t operator()(long long val) const
    {
        return std::hash<int>()(int(val));
    }
};

}

#endif

namespace std {

template <typename T>
struct hash<SOFTHUB::BASE::Reference<T> > {
    size_t operator()(const SOFTHUB::BASE::Reference<T> val) const
    {
        return val->hash();
    }
};

template <typename T>
struct hash<const SOFTHUB::BASE::Reference<T> > {
    size_t operator()(const SOFTHUB::BASE::Reference<T> val) const
    {
        return val->hash();
    }
};

template<>
struct hash<const SOFTHUB::BASE::Serializable*> {
    size_t operator()(const SOFTHUB::BASE::Serializable* obj) const
    {
        return reinterpret_cast<size_t>(obj);
    }
};

template<>
struct hash<SOFTHUB::BASE::Object_id>
{
    size_t operator()(const SOFTHUB::BASE::Object_id& value) const { return value.hash(); }
};

}

typedef std::unique_ptr<byte> unique_byte_ptr;

#include <list>

namespace SOFTHUB {
namespace BASE {

template <typename K, typename V> class Hash_map;
template <typename V> class Hash_set;

//
// class List
//

template <typename T>
class List : public std::list<T> {

    static bool cmp_lt(const T& lhs, const T& rhs);
    static bool cmp_gt(const T& lhs, const T& rhs);

public:
    typedef typename std::list<T>::iterator iterator;
    typedef typename std::list<T>::const_iterator const_iterator;

    List() {}

    void append(const T& element);
    void append(const List& list);
    void prepend(const T& element);
    void remove(const T& element);
    bool contains(const T& element) const;
    void sort(bool descending = false);
};

//
// class Vector
//

template <typename T>
class Vector : public std::vector<T> {

    static bool cmp_lt(const T& lhs, const T& rhs);
    static bool cmp_gt(const T& lhs, const T& rhs);

public:
    Vector() {}
    Vector(int len) : std::vector<T>(len) {}

    void append(const T& element);
    void append_all(const Hash_set<T>& set);
    void insert(size_t pos, const T& element);
    bool remove(const T& element);
    void remove(size_t pos);
    bool contains(const T& element) const;
    void sort(bool descending = false);
};

//
// class Stack
//

template <typename T>
class Stack : public std::vector<T> {

public:
    Stack() {}

    void push(const T& element);
    T pop();
    const T& top() const;
    const T& index(size_t offset) const;
    T& index(size_t offset);
    void dup();
    void exch();
    void erase(size_t count);
    bool contains(const T& element) const;
};

//
// class Hash_map
//

template <typename K, typename V>
class Hash_map : public std::unordered_map<K,V> {

    static V null;

public:
    typedef typename std::unordered_map<K,V>::iterator iterator;
    typedef typename std::unordered_map<K,V>::const_iterator const_iterator;

    Hash_map();
    Hash_map(size_t initial_capacity);

    void rehash(size_t n);
    bool insert(const K& key, const V& val);
    void insert(const Hash_map<K,V>& map);
    bool remove(const K& key);
    bool remove(const Hash_map<K,V>& map);
    bool contains(const K& key) const;
    const V& get(const K& key, const V& default_value = null) const;
    V& get(const K& key, V& default_value = null);

    using std::unordered_map<K,V>::insert;
};

//
// class Hash_set
//

template <typename V>
class Hash_set : public std::unordered_set<V> {

    static V null;

public:
    typedef typename std::unordered_set<V>::iterator iterator;
    typedef typename std::unordered_set<V>::const_iterator const_iterator;

    Hash_set();

    bool insert(const V& val);
    void insert(const Hash_set<V>& set);
    void insert(const Vector<V>& v);
    bool remove(const V& val);
    bool remove(const Hash_set<V>& set);
    bool contains(const V& val) const;
    const V& get(const V& val) const;

    using std::unordered_set<V>::insert;
};

//
// class Hash_array
//

template <typename K, typename V>
class Hash_array : public Vector<K> {

    static V null;

    Hash_map<K,V> map;

public:
    typedef typename Vector<K>::iterator iterator;
    typedef typename Vector<K>::const_iterator const_iterator;

    Hash_array();
    Hash_array(size_t initial_capacity);

    bool insert(const K& key, const V& val);
    void insert(const Hash_array<K,V>& map);
    bool rename(size_t key_index, const K& new_key);
    bool remove(const K& key);
    bool remove(const Hash_array<K,V>& map);
    void clear();
    bool contains(const K& key) const;
    const V& get(const K& key, const V& default_value = null) const;
    V& get(const K& key, V& default_value = null);
    const K& key_at(size_t index) const;
    K& key_at(size_t index);
    const V& value_at(size_t index) const;
    V& value_at(size_t index);
};

//
// class Cache
//

template <typename K, typename V, typename I = IAny>
class Cache : public I {

public:
    typedef BASE::List<K> Keys;
    typedef BASE::Hash_map<K,V> Map;

protected:
    size_t capacity;
    Keys list;
    Map container;

    static V null;

public:
    typedef typename Map::iterator iterator;
    typedef typename Map::const_iterator const_iterator;
    typedef typename Map::size_type size_type;
    typedef typename Map::value_type value_type;

    Cache(int capacity) : capacity(capacity) {}

    size_t get_size() const { return list.size(); }
    void set_capacity(size_t capacity) { this->capacity = capacity; }
    size_t get_capacity() const { return capacity; }
    const Keys& get_keys() const { return list; }
    bool is_empty() const { return list.size() == 0; }
    bool is_full() const { return list.size() >= capacity; }
    void push_back(const value_type& val) { store(val.first, val.second); }
    bool store(const K& key, const V& val);
    bool remove(const K& key);
    void clear();
    bool contains(const K& key) const { return container.contains(key); }
    const V& find(const K& key) const;
    V& find(const K& key);
    const V& find_lru() const;
    const V& find_mru() const;
    void touch(const K& key);
};

//
// class Ring_buffer
//

template <typename T>
class Ring_buffer {

    T* elements;
    unsigned head;
    unsigned tail;
    unsigned size;
    unsigned count;

public:
    Ring_buffer(unsigned size = 0);
    ~Ring_buffer();

    bool is_empty() const { return count == 0; }
    bool is_full() const { return count == size; }
    void resize(unsigned size);
    void reset();
    void append(const T& element);
    T remove();
    T average() const;
    T average_eliminate_extremes() const;
};

typedef BASE::List<std::string> String_list;
typedef BASE::Vector<std::string> String_vector;
typedef BASE::Hash_set<std::string> String_set;
typedef BASE::Hash_map<std::string,std::string> String_map;

}}

#include "base_stl_wrapper_inline.h"

#endif
