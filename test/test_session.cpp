#include <gtest/gtest.h>
#include <session/AESEncryption.h>


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
