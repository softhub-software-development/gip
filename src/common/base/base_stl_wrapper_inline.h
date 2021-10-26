
//
//  base_stl_wrapper_inline.h
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_STL_WRAPPER_INLINE_H
#define BASE_STL_WRAPPER_INLINE_H

namespace SOFTHUB {
namespace BASE {

//
// class List
//

template <typename T>
void List<T>::append(const T& element)
{
    std::list<T>::push_back(element);
}

template <typename T>
void List<T>::append(const List& elements)
{
    typename std::list<T>::const_iterator it = elements.begin();
    typename std::list<T>::const_iterator tail = elements.end();
    while (it != tail) {
        const T& element = *it++;
        add(element);
    }
}

template <typename T>
void List<T>::prepend(const T& element)
{
    std::list<T>::push_front(element);
}

template <typename T>
void List<T>::remove(const T& element)
{
    std::list<T>::remove(element);
}

template <typename T>
bool List<T>::contains(const T& element) const
{
    return find(std::list<T>::begin(), std::list<T>::end(), element) != std::list<T>::end();
}

template <typename T>
void List<T>::sort(bool descending)
{
    if (descending)
        std::list<T>::sort(cmp_gt);
    else
        std::list<T>::sort(cmp_lt);
}

template <typename T>
bool List<T>::cmp_lt(const T& lhs, const T& rhs)
{
    return lhs < rhs;
}

template <typename T>
bool List<T>::cmp_gt(const T& lhs, const T& rhs)
{
    return rhs < lhs;
}

//
// class Vector
//

template <typename T>
void Vector<T>::append(const T& element)
{
    std::vector<T>::push_back(element);
}

template <typename T>
void Vector<T>::append_all(const Hash_set<T>& set)
{
    std::vector<T>::insert(this->end(), set.begin(), set.end());
}

template <typename T>
void Vector<T>::insert(size_t pos, const T& element)
{
    typename Vector<T>::const_iterator it = this->begin() + pos;
    std::vector<T>::insert(it, element);
}

template <typename T>
bool Vector<T>::remove(const T& element)
{
    typename Vector<T>::iterator head = this->begin();
    typename Vector<T>::iterator tail = this->end();
    typename Vector<T>::iterator it = std::remove(head, tail, element);
    bool found = it != tail;
    std::vector<T>::erase(it, tail);
    return found;
}

template <typename T>
void Vector<T>::remove(size_t pos)
{
    typename Vector<T>::const_iterator it = this->begin() + pos;
    std::vector<T>::erase(it);
}

template <typename T>
bool Vector<T>::contains(const T& element) const
{
    return find(std::vector<T>::begin(), std::vector<T>::end(), element) != std::vector<T>::end();
}

template <typename T>
void Vector<T>::sort(bool descending)
{
    if (descending)
        std::sort(std::vector<T>::begin(), std::vector<T>::end(), cmp_gt);
    else
        std::sort(std::vector<T>::begin(), std::vector<T>::end(), cmp_lt);
}

template <typename T>
bool Vector<T>::cmp_lt(const T& lhs, const T& rhs)
{
    return lhs < rhs;
}

template <typename T>
bool Vector<T>::cmp_gt(const T& lhs, const T& rhs)
{
    return rhs < lhs;
}

//
// class Stack
//

template <typename T>
void Stack<T>::push(const T& element)
{
    std::vector<T>::push_back(element);
}

template <typename T>
T Stack<T>::pop()
{
    T val = std::vector<T>::back();
    std::vector<T>::pop_back();
    return val;
}

template <typename T>
const T& Stack<T>::top() const
{
    return std::vector<T>::back();
}

template <typename T>
const T& Stack<T>::index(size_t offset) const
{
    return std::vector<T>::at(std::vector<T>::size() - offset - 1);
}

template <typename T>
T& Stack<T>::index(size_t offset)
{
    return std::vector<T>::at(std::vector<T>::size() - offset - 1);
}

template <typename T>
void Stack<T>::dup()
{
    push(top());
}

template <typename T>
void Stack<T>::exch()
{
    T b = pop();
    T a = pop();
    push(b);
    push(a);
}

template <typename T>
void Stack<T>::erase(size_t count)
{
    std::vector<T>::erase(std::vector<T>::end() - count, std::vector<T>::end());
}

template <typename T>
bool Stack<T>::contains(const T& element) const
{
    return std::find(this->rbegin(), this->rend(), element) != this->rend();
}

//
// class Hash_map
//

template <typename K, typename V>
Hash_map<K,V>::Hash_map()
{
}

template <typename K, typename V>
Hash_map<K,V>::Hash_map(size_t initial_capacity) :
    std::unordered_map<K,V>(initial_capacity)
{
}

template <typename K, typename V>
void Hash_map<K,V>::rehash(size_t n)
{
    std::unordered_map<K,V>::resize(n);
}

template <typename K, typename V>
bool Hash_map<K,V>::insert(const K& key, const V& val)
{
    return std::unordered_map<K,V>::insert(std::pair<K,V>(key, val)).second;
}

template <typename K, typename V>
void Hash_map<K,V>::insert(const Hash_map<K,V>& map)
{
    std::unordered_map<K,V>::insert(map.begin(), map.end());
}

template <typename K, typename V>
bool Hash_map<K,V>::remove(const K& key)
{
    return std::unordered_map<K,V>::erase(key) > 0;
}

template <typename K, typename V>
bool Hash_map<K,V>::remove(const Hash_map<K,V>& map)
{
    return std::unordered_map<K,V>::erase(map.begin(), map.end());
}

template <typename K, typename V>
const V& Hash_map<K,V>::get(const K& key, const V& default_value) const
{
    const_iterator it = std::unordered_map<K,V>::find(key);
    return it != std::unordered_map<K,V>::end() ? it->second : default_value;
}

template <typename K, typename V>
V& Hash_map<K,V>::get(const K& key, V& default_value)
{
    iterator it = std::unordered_map<K,V>::find(key);
    return it != std::unordered_map<K,V>::end() ? it->second : default_value;
}

template <typename K, typename V>
bool Hash_map<K,V>::contains(const K& key) const
{
    return std::unordered_map<K,V>::find(key) != std::unordered_map<K,V>::end();
}

template <typename K, typename V> V Hash_map<K,V>::null;

//
// class Hash_set
//

template <typename V>
Hash_set<V>::Hash_set()
{
}

template <typename V>
bool Hash_set<V>::insert(const V& val)
{
    return std::unordered_set<V>::insert(val).second;
}

template <typename V>
void Hash_set<V>::insert(const Hash_set<V>& set)
{
    std::unordered_set<V>::insert(set.begin(), set.end());
}

template <typename V>
void Hash_set<V>::insert(const Vector<V>& v)
{
    std::unordered_set<V>::insert(v.begin(), v.end());
}

template <typename V>
bool Hash_set<V>::remove(const V& val)
{
    return std::unordered_set<V>::erase(val) > 0;
}

template <typename V>
bool Hash_set<V>::remove(const Hash_set<V>& set)
{
    return std::unordered_set<V>::erase(set.begin(), set.end()) > 0;
}

template <typename V>
bool Hash_set<V>::contains(const V& val) const
{
    return std::unordered_set<V>::find(val) != std::unordered_set<V>::end();
}

template <typename V>
const V& Hash_set<V>::get(const V& val) const
{
    const_iterator it = std::unordered_set<V>::find(val);
    return it != std::unordered_set<V>::end() ? *it : Hash_set<V>::null;
}

template <typename V> V Hash_set<V>::null;

//
// class Hash_array
//

template <typename K, typename V>
Hash_array<K,V>::Hash_array()
{
}

template <typename K, typename V>
Hash_array<K,V>::Hash_array(size_t initial_capacity) :
    Vector<K>(initial_capacity),
    map(initial_capacity)
{
}

template <typename K, typename V>
bool Hash_array<K,V>::insert(const K& key, const V& val)
{
    bool result = map.insert(key, val);
    if (result)
        this->append(key);
    return result;
}

template <typename K, typename V>
void Hash_array<K,V>::insert(const Hash_array<K,V>& map)
{
    map.insert(map.begin(), map.end());
}

template <typename K, typename V>
bool Hash_array<K,V>::rename(size_t key_index, const K& new_key)
{
    const K& old_key = (*this)[key_index];
    if (old_key == new_key)
        return false;
    if (!contains(old_key))
        return false;
    if (contains(new_key))
        return false;
    V val = map.get(old_key);
    if (!map.erase(old_key))
        return false;
    (*this)[key_index] = new_key;
    map.insert(new_key, val);
    return true;
}

template <typename K, typename V>
bool Hash_array<K,V>::remove(const K& key)
{
    bool result = map.erase(key) > 0;
    if (result)
        Vector<K>::remove(key);
    return result;
}

template <typename K, typename V>
void Hash_array<K,V>::clear()
{
    Vector<K>::clear();
    map.clear();
}

template <typename K, typename V>
bool Hash_array<K,V>::contains(const K& key) const
{
    return map.find(key) != map.end();
}

template <typename K, typename V>
const V& Hash_array<K,V>::get(const K& key, const V& default_value) const
{
    typename Hash_map<K,V>::const_iterator it = map.find(key);
    return it != map.end() ? it->second : default_value;
}

template <typename K, typename V>
V& Hash_array<K,V>::get(const K& key, V& default_value)
{
    typename Hash_map<K,V>::iterator it = map.find(key);
    return it != map.end() ? it->second : default_value;
}

template <typename K, typename V>
const K& Hash_array<K,V>::key_at(size_t index) const
{
    assert(0 <= index && index < this->size());
    return (*this)[index];
}

template <typename K, typename V>
K& Hash_array<K,V>::key_at(size_t index)
{
    assert(0 <= index && index < this->size());
    return (*this)[index];
}

template <typename K, typename V>
const V& Hash_array<K,V>::value_at(size_t index) const
{
    assert(0 <= index && index < this->size());
    const K& key = key_at(index);
    return get(key);
}

template <typename K, typename V>
V& Hash_array<K,V>::value_at(size_t index)
{
    assert(0 <= index && index < this->size());
    K& key = key_at(index);
    return get(key);
}

template <typename K, typename V> V Hash_array<K,V>::null;

//
// class Cache
//

template <typename K, typename V, typename I>
bool Cache<K,V,I>::store(const K& key, const V& val)
{
    bool result = false;
    const_iterator it = container.find(key);
    const_iterator tail = container.end();
    if (it == tail) {
        size_t n = this->list.size();
        if (n > 0 && n >= this->capacity) {
            const K& obj = this->list.front();
#ifdef _DEBUG
            bool found =
#endif
            container.remove(obj);
#ifdef _DEBUG
            if (!found)
                log_message(ERR, "obj not found in cache");
#endif
            this->list.pop_front();
        }
        this->list.append(key);
        container.insert(key, val);
    } else {
        result = container.insert(key, val);
        this->list.remove(key);
        this->list.append(key);
    }
#if UTIL_DEBUG
    size_t m = this->list.size();
    size_t n = container.size();
    assert(m <= this->capacity && n <= this->capacity);
    assert(m == n);
#endif
    return result;
}

template <typename K, typename V, typename I>
bool Cache<K,V,I>::remove(const K& key)
{
    size_t n = this->list.size();
    this->list.remove(key);
    container.remove(key);
#if UTIL_DEBUG
    assert(this->list.size() == container.size());
#endif
    return this->list.size() == n - 1;
}

template <typename K, typename V, typename I>
void Cache<K,V,I>::clear()
{
    container.clear();
    this->list.clear();
#if UTIL_DEBUG
    assert(this->list.size() == 0 && this->size() == 0);
#endif
}

template <typename K, typename V, typename I>
const V& Cache<K,V,I>::find(const K& key) const
{
    typename Map::const_iterator it = container.find(key);
    typename Map::const_iterator tail = container.end();
    return it == tail ? null : it->second;
}

template <typename K, typename V, typename I>
V& Cache<K,V,I>::find(const K& key)
{
    typename Map::iterator it = container.find(key);
    typename Map::iterator tail = container.end();
    return it == tail ? null : it->second;
}

template <typename K, typename V, typename I>
const V& Cache<K,V,I>::find_lru() const
{
    if (this->list.empty())
        return null;
    const K& key = this->list.front();
    return container.get(key);
}

template <typename K, typename V, typename I>
const V& Cache<K,V,I>::find_mru() const
{
    if (this->list.empty())
        return null;
    const K& key = this->list.back();
    return container.get(key);
}

template <typename K, typename V, typename I>
void Cache<K,V,I>::touch(const K& key)
{
    this->list.remove(key);
    this->list.append(key);
}

template <typename K, typename V, typename I> V Cache<K,V,I>::null;

}}

#endif
