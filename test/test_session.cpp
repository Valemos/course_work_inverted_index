#include <gtest/gtest.h>
#include <session/AESEncryption.h>
#include <session/DHKeyExchange.h>
#include <session/SHA256Algorithm.h>
#include <session/RSAKeyPair.h>


TEST(TestAES, TestEncryptionClassCorrect) {
    std::vector<unsigned char> key;
    key.resize(AESEncryption::KEY_SIZE, 1);

    AESEncryption encryption;
    encryption.SetPrivateKey(key.begin(),  key.end());

    std::vector<unsigned char> data {1, 2, 3, 4, 5, 6};
    auto encrypted = encryption.Encrypt(data.begin(),  data.end());
    EXPECT_NE(encrypted, data);
    auto decrypted = encryption.Decrypt(encrypted.begin(),  encrypted.end());
    ASSERT_EQ(decrypted, data);
}

TEST(TestAES, TestNonTrustworthyEncryption) {
    std::vector<unsigned char> key;
    key.resize(AESEncryption::KEY_SIZE, 1);

    AESEncryption encryption{};
    encryption.SetPrivateKey(key.begin(),  key.end());

    std::vector<unsigned char> data {1, 2, 3, 4, 5, 6};
    auto encrypted = encryption.Encrypt(data.begin(),  data.end());

    encrypted[encrypted.size() / 2] ^= 136;

    ASSERT_ANY_THROW(encryption.Decrypt(encrypted.begin(), encrypted.end()));
}

TEST(DISABLED_TestKeyExchange, TestExchangeCorrect){
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
    auto encrypted = encryption.Encrypt(data.begin(),  data.end());
    EXPECT_NE(encrypted, data);
    auto decrypted = encryption.Decrypt(encrypted.begin(),  encrypted.end());
    ASSERT_EQ(decrypted, data);
}

TEST(TestRSA, TestRSADecryptionWithOtherKey) {
    RSAKeyPair encryption;
    encryption.GenerateKeys();

    std::vector<unsigned char> data {1, 2, 3, 4, 5};
    auto encrypted = encryption.Encrypt(data.begin(),  data.end());
    EXPECT_NE(encrypted, data);

    RSAKeyPair other_encryption;
    other_encryption.GenerateKeys();

    ASSERT_ANY_THROW(other_encryption.Decrypt(encrypted.begin(), encrypted.end()));
}

TEST(TestRSA, TestSignature) {
    RSAKeyPair key_pair;
    key_pair.GenerateKeys();

    std::vector<unsigned char> data_to_sign {1, 2, 3, 4, 5};
    auto signature = key_pair.SignDigest(data_to_sign.begin(),  data_to_sign.end());

    RSAKeyPair other_user;
    other_user.SetPublicKey(key_pair.GetPublicKey());

    ASSERT_TRUE(other_user.VerifyDigest(data_to_sign.begin(),  data_to_sign.end(),
                                                signature.begin(), signature.end()));
}

TEST(TestRSA, TestSignatureFails) {
    RSAKeyPair encryption;
    encryption.GenerateKeys();

    std::vector<unsigned char> data_to_sign {1, 2, 3, 4, 5};
    auto signature = encryption.SignDigest(data_to_sign.begin(),  data_to_sign.end());
    data_to_sign[2] = 100;
    ASSERT_FALSE(encryption.VerifyDigest(data_to_sign.begin(),  data_to_sign.end(),
                                                signature.begin(), signature.end()));
}


TEST(TestRSA, TestSetAndGetPublicKey) {
    RSAKeyPair first, second;
    first.GenerateKeys();

    second.SetPublicKey(first.GetPublicKey());

    ASSERT_EQ(first.GetPublicKey(), second.GetPublicKey());
}

TEST(TestRSA, TestRSASerialization) {
    RSAKeyPair rsa;
    rsa.GenerateKeys();

    std::filesystem::create_directory("./temp");
    rsa.SaveToDirectory("./temp");
    auto rsa2 = RSAKeyPair::LoadFiles("./temp/id_rsa.pub", "./temp/id_rsa");

    EXPECT_EQ(rsa.GetPublicKey(), rsa2.GetPublicKey());
    EXPECT_EQ(rsa.GetPrivateKey(), rsa2.GetPrivateKey());
    std::filesystem::remove_all("./temp");
    std::filesystem::remove("./temp");
}
