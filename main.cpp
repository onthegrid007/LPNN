#include <iostream>
#include <fstream>
#include "net.hpp"
// #include <openssl/sha.h>
// #include <openssl/ripemd.h>
// #include <openssl/bn.h>
// #include <openssl/ecdsa.h>
// #include <openssl/obj_mac.h>
// #include "other/libethcrypto/secp256k1/include/secp256k1.h"

// const char* to_base58(const uint8_t* data, size_t size) {
//     static char base58chars[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
//     BN_CTX* ctx = BN_CTX_new();
//     BIGNUM *num = BN_new();
//     BIGNUM *rem = BN_new();
//     BIGNUM *B58 = BN_new();
//     BIGNUM *div = BN_new();
//     // div, , B58;
//     // BN_init(num);
//     // BN_init(&div);
//     // BN_init(&rem);
//     // BN_init(&B58);
//     BN_bin2bn(data, size, num);
//     BN_set_word(B58, 58);
//     char* result = (char*)malloc(100);
//     memset(result, 0, 100);
//     int index = 99;
//     while (BN_cmp(num, BN_value_one()) > 0) {
//         BN_div(div, rem, num, B58, ctx);
//         BN_copy(num, div);
//         uint64_t r = BN_get_word(rem);
//         result[--index] = base58chars[r];
//     }
//     result[--index] = base58chars[BN_get_word(num)];
//     BN_CTX_free(ctx);
//     return result + index;
// }

int main() {
    using namespace LPNN;
    typedef double T;
    const Net<T>::Topology top{
        {2, Layer<T>::ActivationFunction::TANH},
        {4, Layer<T>::ActivationFunction::TANH},
        {1, Layer<T>::ActivationFunction::TANH}
    };
    const Net<T>::Topology top2{
        {264, Layer<T>::ActivationFunction::TANH},
        // {256, Layer<T>::ActivationFunction::TANH},
        {1024, Layer<T>::ActivationFunction::TANH},
        {512, Layer<T>::ActivationFunction::TANH},
        // {1024, Layer<T>::ActivationFunction::TANH},
        // {1024, Layer<T>::ActivationFunction::TANH},
        // {1024, Layer<T>::ActivationFunction::TANH},
        // {512, Layer<T>::ActivationFunction::TANH},
        {256, Layer<T>::ActivationFunction::TANH}
    };
    Net<T> net{top2};
    std::ifstream acc("btc.txt");
    std::string line;
    std::vector<T> in, tar;
    for(int i{0}; i < 100000; i++) {
        acc >> line;
        in.clear();
        for(const char& c : line)
            in.emplace_back((c == '0') ? 0 : 1);
        
        acc >> line;
        tar.clear();
        for(const char& c : line)
            tar.emplace_back((c == '0') ? 0 : 1);
    //     const int i1{(T(rand()) / RAND_MAX < 0.5) ? 0 : 1};
    //     const int i2{(T(rand()) / RAND_MAX < 0.5) ? 0 : 1};
    //     const int t{i1 ^ i2};
    //     const std::vector<T> in{T(i1), T(i2)};
    //     const std::vector<T> tar{T(t)};
    //     std::vector<T> out;
        // std::cout << "i1: " << in.size() << std::endl;
    //     std::cout << "i2: " << i2 << std::endl;
    //     std::cout << "t: " << t << std::endl;
        
    //     uint8_t pubKey[65];
    // size_t pubKeyLen = 65;
    // EC_KEY* key = EC_KEY_new_by_curve_name(NID_secp256k1);
    // BIGNUM* privKey = BN_new();
    // BN_hex2bn(&privKey, "30caae2fcb7c34ecadfddc45e0a27e9103bd7cfc87730d7818cc096b1266a683");
    // EC_KEY_set_private_key(key, privKey);
    // EC_KEY_generate_key(key);
    // EC_KEY_check_key(key);
    // EC_POINT_point2oct(EC_KEY_get0_group(key), EC_KEY_get0_public_key(key), POINT_CONVERSION_UNCOMPRESSED, pubKey, pubKeyLen, NULL);
    // uint8_t hash[SHA256_DIGEST_LENGTH];
    // SHA256(pubKey, sizeof(pubKey), hash);
    // uint8_t ripeHash[RIPEMD160_DIGEST_LENGTH];
    // RIPEMD160(hash, sizeof(hash), ripeHash);
    // uint8_t extendedRipe[1 + RIPEMD160_DIGEST_LENGTH + 4];
    // extendedRipe[0] = 0x00;
    // memcpy(extendedRipe + 1, ripeHash, sizeof(ripeHash));
    // SHA256(extendedRipe, sizeof(ripeHash) + 1, hash);
    // SHA256(hash, sizeof(hash), hash);
    // memcpy(extendedRipe + 21, hash, 4);
    // printf("Bitcoin address: %s\n", to_base58(extendedRipe, sizeof(extendedRipe)));
        
        // BADLOGV(i1);
        // BADLOGV(i1);
        // BADLOG("t: " << t << std::endl);
        for(int o{0}; o < 100; o+=1)
            std::cout << "m_averageError: " << [](){  return ""; }() << net.train(in, tar).m_averageError << std::endl;
        // net.out(out);
        // std::cout << "out: " << out[0] << std::endl << std::endl;
        //  std::cout << ((fabs(t - out[0]) < 0.1) ? "PASS\n\n" : "FAIL\n\n");
    }
}