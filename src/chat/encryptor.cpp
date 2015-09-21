#include "encryptor.h"

#include <botan/cryptobox.h>

const size_t SERPENT_KEY_SIZE = 128;

Encryptor::Encryptor(QObject *parent) : QObject(parent)
{
}

KeyPair Encryptor::createAsymmetricKeys()
{
    // Private key generation
    Botan::AutoSeeded_RNG rng;
    Botan::RSA_PrivateKey key(rng, 4096);

    // Generate public key
    std::string pub = Botan::X509::PEM_encode(key);
    std::string priv = Botan::PKCS8::PEM_encode(key);

    Botan::DataSource_Memory key_pub(pub);
    Botan::DataSource_Memory key_priv(priv);

    // Load keys
    auto publicRsaKey = Botan::X509::load_key(key_pub);
    auto privateRsaKey = Botan::PKCS8::load_key(key_priv, rng);

    KeyPair pair;
    pair.publicKey = publicRsaKey;
    pair.privateKey = privateRsaKey;

    return pair;
}

Botan::SymmetricKey Encryptor::createSymmetricKey()
{
    Botan::AutoSeeded_RNG rng;
    Botan::SymmetricKey serpentKey(rng, SERPENT_KEY_SIZE);

    return serpentKey;
}

QByteArray Encryptor::encryptAsymmetricly(ConnectedUser *user, std::string &data)
{
    const uint DATA_SIZE = data.size();
    Botan::byte msgtoencrypt[DATA_SIZE];

    for (uint i = 0; i < DATA_SIZE; i++)
    {
        msgtoencrypt[i] = data[i];
    }

    Botan::PK_Encryptor_EME encryptor(user->publicKey(), "EME1(SHA-256)");
    Botan::AutoSeeded_RNG rng;
    std::vector<Botan::byte> ciphertext = encryptor.encrypt(msgtoencrypt, DATA_SIZE, rng);

    QByteArray keyCipherData;
    keyCipherData.resize(ciphertext.size());

    for ( uint i = 0; i < ciphertext.size(); i++ ) {
        keyCipherData[i] = ciphertext.at(i);
    }

    return keyCipherData;
}

QByteArray Encryptor::decryptAsymmetricly(ConnectedUser *user, QByteArray &data)
{
    const int DATA_SIZE = data.size();
    Botan::PK_Decryptor_EME decryptor(user->privateKey(), "EME1(SHA-256)");

    Botan::byte msgToDecrypt[DATA_SIZE];

    for (int i = 0; i < DATA_SIZE; i++)
    {
        Botan::byte character = (Botan::byte) data.at(i);
        msgToDecrypt[i] = character;
    }

    // Decrypt key with private key
    Botan::secure_vector<Botan::byte> decryptedSerpentKey = decryptor.decrypt(msgToDecrypt, DATA_SIZE);

    // The key is transfered in hex
    QByteArray decryptedData;

    for (uint i = 0; i < decryptedSerpentKey.size(); ++i) {
        Botan::byte dataByte = decryptedSerpentKey[i];
        decryptedData.append((char) dataByte);
    }

    return decryptedData;
}

QByteArray Encryptor::encryptSymmetricly(ConnectedUser *user, QByteArray data)
{
    return encryptSymmetricly(user->symmetricKey().as_string(), data);
}

QByteArray Encryptor::encryptSymmetricly(std::string password, QByteArray data)
{
    const int inputSize = data.length();
    std::vector<Botan::byte> inputData;
    //inputData.reserve(inputSize);

    for (int i = 0; i < inputSize; ++i) {
        inputData.push_back(data.at(i));
    }

    return encryptSymmetricly(password, inputData);
}

QByteArray Encryptor::encryptSymmetricly(std::string password, std::vector<Botan::byte> &data)
{
    Botan::AutoSeeded_RNG rng;
    std::string encryptedString = Botan::CryptoBox::encrypt(data.data(), data.size(), password, rng);

    return QByteArray::fromStdString(encryptedString);
}

QByteArray Encryptor::decryptSymmetricly(ConnectedUser *user, QByteArray data)
{
    return decryptSymmetricly(user->symmetricKey().as_string(), data);
}

QByteArray Encryptor::decryptSymmetricly(std::string password, QByteArray data)
{
    std::string decryptedString = Botan::CryptoBox::decrypt(data.toStdString(), password);
    return QByteArray::fromStdString(decryptedString);
}

