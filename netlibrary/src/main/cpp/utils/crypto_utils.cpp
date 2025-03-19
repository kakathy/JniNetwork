#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <string>
#include <cstring>
#include <cstdlib>

using namespace std;

class CryptoUtils {
public:
    static string base64Decode(const string &input) {
        BIO *bio, *b64;
        char *buffer = (char *) malloc(input.size());
        memset(buffer, 0, input.size());

        bio = BIO_new_mem_buf(input.data(), input.size());
        b64 = BIO_new(BIO_f_base64());
        bio = BIO_push(b64, bio);

        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // 忽略换行符

        int decodedLength = BIO_read(bio, buffer, input.size());
        string decodedData(buffer, decodedLength);

        BIO_free_all(bio);
        free(buffer);

        return decodedData;
    }

    static string decryptAES(const string &encryptedData, const string &key, const string &iv) {
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, (unsigned char *) key.c_str(),
                           (unsigned char *) iv.c_str());

        unsigned char decrypted[encryptedData.size() + AES_BLOCK_SIZE];
        int decryptedLength;
        EVP_DecryptUpdate(ctx, decrypted, &decryptedLength, (unsigned char *) encryptedData.c_str(),
                          encryptedData.size());

        int finalLength;
        EVP_DecryptFinal_ex(ctx, decrypted + decryptedLength, &finalLength);
        decryptedLength += finalLength;

        EVP_CIPHER_CTX_free(ctx);

        return string((char *) decrypted, decryptedLength);
    }

    // 生成哈希值
    static string calculateHash(const string &input, const string &hashType) {
        const EVP_MD *md;

        if (hashType == "MD5") {
            md = EVP_md5();
        } else if (hashType == "SHA-1") {
            md = EVP_sha1();
        } else if (hashType == "SHA-256") {
            md = EVP_sha256();
        } else if (hashType == "SHA-512") {
            md = EVP_sha512();
        } else {
            return "不支持的哈希类型";
        }

        EVP_MD_CTX *ctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(ctx, md, nullptr);

        EVP_DigestUpdate(ctx, input.c_str(), input.size());

        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hashLength;
        EVP_DigestFinal_ex(ctx, hash, &hashLength);

        EVP_MD_CTX_free(ctx);

        string hashHex;
        for (unsigned int i = 0; i < hashLength; i++) {
            char buf[3];
            sprintf(buf, "%02x", hash[i]);
            hashHex += buf;
        }
        return hashHex;
    }
};
