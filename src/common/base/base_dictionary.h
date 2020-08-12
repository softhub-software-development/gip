
//
//  base_dictionary.h
//
//  Created by Softhub.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef BASE_DICTIONARY_H
#define BASE_DICTIONARY_H

#include "base_reference.h"
#include "base_container.h"
#include "base_serialization.h"

namespace SOFTHUB {
namespace BASE {

#define FORWARD_DICT(key_class, value_class, clazz) \
    class clazz; \
    typedef SOFTHUB::BASE::Reference<SOFTHUB::BASE::Dictionary<key_class,value_class> > clazz##_ref;

#define DECLARE_DICT(key_class, value_class, clazz, cid) \
    typedef SOFTHUB::BASE::Dictionary<key_class,value_class,cid> clazz; \
    typedef SOFTHUB::BASE::Reference<clazz> clazz##_ref;

//
// IDictionary Interface
//

class IDictionary : public IContainer {

public:
    virtual ~IDictionary() {}

    virtual bool is_empty() const = 0;
    virtual size_t get_size() const = 0;
    virtual void remove_all() = 0;
//  virtual class_id_type get_element_class_id() const = 0;
//  virtual void resize(int len) = 0;
};

//
// Dictionary template class
//

template <typename K, typename V, long CID = 'shdi'>
class Dictionary : public Object<IDictionary> {

    friend class Deserializer;

    Hash_map<K,V>* elements;

    Dictionary(const Dictionary& dict);

public:
    Dictionary();
    Dictionary(int size);
    ~Dictionary();

    bool is_empty() const { return elements->empty(); }
    void put(const K& key, const V& value);
    const V& get(const K& key, const V& default_val = null) const;
    V& get(const K& key, V& default_val = null);
    bool contains(const K& key) const;
    size_t get_size() const { return elements->size(); }
    bool remove(const K& key) { return elements->remove(key); }
    void remove_all() { elements->clear(); }
//  bool equals(const Interface* obj) const;

    Dictionary<K,V,CID>& operator=(const Dictionary& dict);

    // iterator support
    typedef typename Hash_map<K,V>::const_iterator const_iterator;
    typedef typename Hash_map<K,V>::iterator iterator;
    typedef typename Hash_map<K,V>::value_type value_type;
    const_iterator begin() const { return elements->begin(); }
    const_iterator end() const { return elements->end(); }
    iterator begin() { return elements->begin(); }
    iterator end() { return elements->end(); }

    // Inherited from Serializable
    class_id_type get_class_id() const { return CID; }
    void serialize(Serializer* serializer) const;
    void deserialize(Deserializer* deserializer);

    static const class_id_type class_id = CID;
    static V null;
};

}}

#include "base_dictionary_inline.h"

#endif
