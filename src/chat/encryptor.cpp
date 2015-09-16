#include "encryptor.h"

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
    const int DATA_SIZE = data.size();
    Botan::byte msgtoencrypt[DATA_SIZE];

    for (unsigned int i = 0; i < DATA_SIZE; i++)
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

