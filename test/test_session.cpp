#include <gtest/gtest.h>
#include <session/AESEncryption.h>
#include <session/DHKeyExchange.h>
#include <session/SHA256Algorithm.h>
#include <session/RSAKeyPair.h>


TEST(TestAES, TestEncryptionClassCorrect) {
    AESEncryption::KeyType key{};
    key.fill(1);
    AESEncryption encryption{};
    encryption.SetPrivateKey(key);

    std::vector<unsigned char> data {1, 2, 3, 4, 5, 6};
    auto encrypted = encryption.Encrypt(data);
    EXPECT_NE(encrypted, data);
    auto decrypted = encryption.Decrypt(encrypted);
    ASSERT_EQ(decrypted, data);
}

TEST(TestAES, TestNonTrustworthyEncryption) {
    AESEncryption::KeyType key{};
    key.fill(1);
    AESEncryption encryption{};
    encryption.SetPrivateKey(key);

    std::vector<unsigned char> data {1, 2, 3, 4, 5, 6};
    auto encrypted = encryption.Encrypt(data);

    encrypted[encrypted.size() / 2] ^= 136;

    ASSERT_ANY_THROW(encryption.Decrypt(encrypted));
}

TEST(TestKeyExchange, TestExchangeCorrect){
    DHKeyExchange key_1(AESEncryption::KEY_SIZE);
    DHKeyExchange key_2(AESEncryption::KEY_SIZE);

    key_1.InitializeParameters();
    key_2.InitializeParameters();
    key_1.GeneratePublicKey();
    key_2.GeneratePublicKey();

    key_2.SetPeerPublicKey(key_1.GetPublicKey());
    key_1.SetPeerPublicKey(key_2.GetPublicKey());
    key_1.DeriveSharedSecret();
    key_2.DeriveSharedSecret();

    ASSERT_EQ(key_1.GetSharedSecret(), key_2.GetSharedSecret());
}

TEST(TestHash, TestHashInPartsCorrect) {
    SHA256Algorithm hash;
    auto full_hash = hash.HashBytes({1, 2, 3, 4});

    hash.Update({1, 2});
    hash.Update({3, 4});
    auto hash_from_parts = hash.GetFinalHash();

    ASSERT_EQ(full_hash, hash_from_parts);
}


TEST(TestRSA, TestRSAEncryption) {
    RSAKeyPair encryption;
    encryption.GenerateKeys();

    std::vector<unsigned char> data {1, 2, 3, 4, 5};
    auto encrypted = encryption.Encrypt(data);
    EXPECT_NE(encrypted, data);
    auto decrypted = encryption.Decrypt(encrypted);
    ASSERT_EQ(decrypted, data);
}

TEST(TestRSA, TestSignature) {
    RSAKeyPair encryption;
    encryption.GenerateKeys();

    std::vector<unsigned char> data_to_sign {1, 2, 3, 4, 5};
    auto signature = encryption.Sign(data_to_sign);
    ASSERT_TRUE(encryption.Verify(data_to_sign, signature));
}

TEST(TestRSA, TestSignatureFails) {
    RSAKeyPair encryption;
    encryption.GenerateKeys();

    std::vector<unsigned char> data_to_sign {1, 2, 3, 4, 5};
    auto signature = encryption.Sign(data_to_sign);
    data_to_sign[2] = 100;
    ASSERT_FALSE(encryption.Verify(data_to_sign, signature));
}