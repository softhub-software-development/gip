
//
//  util_crypt.h
//
//  Created by Christian Lehner on 11/07/18.
//  Copyright © 2019 softhub. All rights reserved.
//
// https://cboard.cprogramming.com/c-programming/172534-rsa-key-generate-then-encrypt-decrypt-error.html
//

#ifndef LIB_UTIL_CRYPT_H
#define LIB_UTIL_CRYPT_H

#include <base/base.h>
#if FEATURE_NET_SSL
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#endif

namespace SOFTHUB {
namespace UTIL {

#if FEATURE_NET_SSL

class Crypt {

    RSA* rsa;

    void reset();

    static void report_error(const std::string& msg);

public:
    Crypt();
    ~Crypt();

    void init_private(const byte* key);
    void init_public(const byte* key);
    bool validate_key_pair(const byte* pri_key, const byte* pub_key);
    int encrypt(const byte* text, int len, byte** cipher) const;
    int decrypt(const byte* cipher, int len, byte** text) const;

    static bool generate_key_pair(int key_size, byte** pri_key, byte** pub_key);
    static void sha256(const std::string& str, std::string& digest);
    static void sha256(const byte* msg, size_t msg_len, unique_byte_ptr& digest, size_t& digest_len);
};

//
// class Crypt_validation_io
//

template <typename T>
class Crypt_validation_io : public BASE::Stream_io<T> {

protected:
    byte hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    bool finalized;
    int io_count;

public:
    Crypt_validation_io(T& stream) : BASE::Stream_io<T>(stream), finalized(false), io_count(0) {}
};

//
// class Crypt_validation_stream_writer
//

class Crypt_validation_stream_writer : public Crypt_validation_io<std::ostream> {

    friend class Crypt_validation_serializer;

    void finalize();

public:
    Crypt_validation_stream_writer(std::ostream& stream);

#if 0
    bool is_valid() const;
    large tell() const;
    void seek(large pos);
    bool read(char& c);
    void write(char c);
    size_t read(char* buf, size_t len);
    void write(const char* buf, size_t len);
#endif
    void write(char c);
    void write(const char* buf, size_t len);
};

//
// class Crypt_validation_stream_reader
//

class Crypt_validation_stream_reader : public Crypt_validation_io<std::istream> {

    friend class Crypt_validation_deserializer;

    void finalize();

public:
    Crypt_validation_stream_reader(std::istream& stream);

#if 0
    bool is_valid() const;
    large tell() const;
    void seek(large pos);
    bool read(char& c);
    void write(char c);
    size_t read(char* buf, size_t len);
    void write(const char* buf, size_t len);
#endif
    bool read(char& c);
    size_t read(char* buf, size_t len);
};

//
// class Crypt_validation_serializer
//

class Crypt_validation_serializer : public BASE::Stream_serializer {

protected:
    void write_size(unsigned size);

public:
    Crypt_validation_serializer(std::ostream& stream);

    void sign(const Crypt& crypt);
};

//
// class Crypt_validation_deserializer
//

class Crypt_validation_deserializer : public BASE::Stream_deserializer {

protected:
    void read_size(unsigned& size);

public:
    Crypt_validation_deserializer(std::istream& stream);

    void validate(const Crypt& crypt);
};

//
// class Crypt_exception
//

class Crypt_exception : public BASE::Serialization_exception {

public:
    Crypt_exception(const std::string& msg) : BASE::Serialization_exception(msg) {}
};

#endif

}}

#endif
