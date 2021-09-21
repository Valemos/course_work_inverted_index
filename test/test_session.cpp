#include <gtest/gtest.h>
#include <session/AESEncryption.h>


TEST(TestSession, TestEncryptionClass) {
    AESEncryption encryption;

    std::vector<char> data {1, 2, 3, 4, 5, 6};
    auto encrypted = encryption.Encrypt(data);
    EXPECT_NE(encrypted, data);
    auto decrypted = encryption.Decrypt(encrypted);
    ASSERT_EQ(decrypted, data);
}
