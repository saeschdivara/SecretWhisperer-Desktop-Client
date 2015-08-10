#include "connecteduser.h"

ConnectedUser::ConnectedUser(Botan::Public_Key *publicKey, Botan::Private_Key *privateRsaKey, bool hasUserInitieted) :
    QObject(0),
    publicRsaKey(publicKey),
    privateRsaKey(privateRsaKey),
    hasUserInitieted(hasUserInitieted)
{

}

