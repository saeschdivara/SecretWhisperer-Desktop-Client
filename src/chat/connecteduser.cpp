#include "connecteduser.h"

#include <botan/botan.h>
#include <botan/pubkey.h>
#include <botan/pkcs8.h>
#include <botan/pk_keys.h>
#include <botan/rsa.h>

ConnectedUser::ConnectedUser(Botan::Public_Key *publicKey, Botan::Private_Key *privateRsaKey, bool hasUserInitieted) :
    QObject(0),
    publicRsaKey(publicKey),
    privateRsaKey(privateRsaKey),
    hasUserInitieted(hasUserInitieted)
{

}

QByteArray ConnectedUser::getDataFromPublicKey()
{
    QByteArray keyData;
    std::vector<unsigned char> keyDataArray = publicRsaKey->x509_subject_public_key();

    const uint total = keyDataArray.size();
    keyData.reserve(total);

    for (uint index = 0; index < total; ++index) {
        keyData[index] = keyDataArray[index];
    }

    return keyData;
}
