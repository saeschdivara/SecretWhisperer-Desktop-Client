# Secret Whisperer

## Concept

1. Server started and waiting for new clients
2. Client x connects to the server via Tls 1.2 which uses perfect forward secrecy
3. Client x tells the server under which name he wants to be addressed with, this is used later for connecting to other clients
4. Client y does the same as client x did
5. Client x tells the server he wants to connect to client y
6. Via private / public key encryption (rsa with 4096 key) client x sends client y his newly generated serpent 256 key
7. Client x tells the server it wants to send a message to client y and sends an encrypted message using the cryptobox format