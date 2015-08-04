# Secret Whisperer

## Concept

1. Server started and waiting for new clients
2. Client x connects to the server via Tls 1.2 which uses perfect forward secrecy
3. Client x tells the server under which name he wants to be addressed with, this is used later for connecting to other clients
4. Client y does the same as client x did
5. Client x tells the server he wants to connect to client y
6. Via private / public key encryption client x sends client y his newly generated aes 256 key
7. Client x sagt dem Server er möchte dem Client y eine Nachricht schicken, die wiederum mit dem bereits geteilten aes Key verschlüsselt wurde
8. und von nun an machen das die beiden hin und her