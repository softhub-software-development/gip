
//
//  base_dictionary_inline.h
//
//  Created by Christian Lehner on 5/16/11.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_DICTIONARY_INLINE_H
#define BASE_DICTIONARY_INLINE_H

namespace SOFTHUB {
namespace BASE {

//
// class Dictionary<K,V,CID>
//

template <typename K, typename V, long CID>
V Dictionary<K,V,CID>::null;

template <typename K, typename V, long CID>
Dictionary<K,V,CID>::Dictionary() : elements(new Hash_map<K,V>())
{
}

template <typename K, typename V, long CID>
Dictionary<K,V,CID>::Dictionary(int size) : elements(new Hash_map<K,V>(size))
{
}

template <typename K, typename V, long CID>
Dictionary<K,V,CID>::Dictionary(const Dictionary& dict) : elements(dict.elements)
{
}

template <typename K, typename V, long CID>
Dictionary<K,V,CID>::~Dictionary()
{
    if (ref_count == 0)
        delete elements;
}

template <typename K, typename V, long CID>
Dictionary<K,V,CID>& Dictionary<K,V,CID>::operator=(const Dictionary& dict)
{
    elements = dict->elements;
    return *this;
}

template <typename K, typename V, long CID>
void Dictionary<K,V,CID>::put(const K& key, const V& value)
{
    (*elements)[key] = value;
}

template <typename K, typename V, long CID>
const V& Dictionary<K,V,CID>::get(const K& key, const V& default_val) const
{
    const_iterator it = elements->find(key);
    const_iterator tail = elements->end();
    return it != tail ? it->second : default_val;
}

template <typename K, typename V, long CID>
V& Dictionary<K,V,CID>::get(const K& key, V& default_val)
{
    iterator it = elements->find(key);
    iterator tail = elements->end();
    return it != tail ? it->second : default_val;
}

template <typename K, typename V, long CID>
bool Dictionary<K,V,CID>::contains(const K& key) const
{
    return elements->contains(key);
}

/* TODO: equals and ==
template <typename K, typename V>
bool Dictionary<K,V,CID>::equals(const Interface* obj) const
{
    const Dictionary<K,V,CID>* dict = dynamic_cast<Dictionary<K,V,CID>*>(obj);
    if (dict == 0)
        return false;
    int n = get_size();
    if (n != dict->get_size())
        return false;
    Hash_map<K,V,CID>::const_iterator it = elements.begin();
    Hash_map<K,V,CID>::const_iterator tail = elements.begin();
    while (it != tail) {
        const K& key = *it.first;
        const V& val = *it.second;
        const V& other_val = dict->get(key);
        if (val != other_val)
            return false;
    }
    return true;
}
*/

template <typename K, typename V, long CID>
void Dictionary<K,V,CID>::serialize(Serializer* serializer) const
{
    large size = get_size();
    assert(size >= 0);
    serializer->write(size);
    const_iterator it = elements->begin();
    const_iterator tail = elements->end();
    while (it != tail) {
        const std::pair<K,V>& pair = *it++;
        serializer->write(pair.first);
        serializer->write(pair.second);
    }
}

template <typename K, typename V, long CID>
void Dictionary<K,V,CID>::deserialize(Deserializer* deserializer)
{
    large size;
    deserializer->read(size);
#ifdef _DEBUG
    // tell us if we allocate huge junks of memory
    assert(0 <= size && size < (1 << 23));
#endif
    elements = new Hash_map<K,V>(size);
    for (int i = 0; i < size; i++) {
        K key;
        V val;
        deserializer->read(key);
        deserializer->read(val);
        elements->insert(key, val);
    }
}

}}

#endif
