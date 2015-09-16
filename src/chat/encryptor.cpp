#include "encryptor.h"

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

