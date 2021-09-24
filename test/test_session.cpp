#include <gtest/gtest.h>
#include <session/AESEncryption.h>
#include <session/DHKeyExchange.h>


TEST(TestSession, TestEncryptionClassCorrect) {
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

TEST(TestSession, TestNonTrustworthyEncryption) {
    AESEncryption::KeyType key{};
    key.fill(1);
    AESEncryption encryption{};
    encryption.SetPrivateKey(key);

    std::vector<unsigned char> data {1, 2, 3, 4, 5, 6};
    auto encrypted = encryption.Encrypt(data);

    encrypted[encrypted.size() / 2] ^= 136;

    ASSERT_ANY_THROW(encryption.Decrypt(encrypted));
}

TEST(TestSession, TestKeyExchange){
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