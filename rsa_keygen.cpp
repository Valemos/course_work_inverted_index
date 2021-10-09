
#include "src/session/RSAKeyPair.h"


int main() {
    RSAKeyPair rsa;
    rsa.GenerateKeys();
    rsa.SaveToDirectory("./keys_server");
    rsa.GenerateKeys();
    rsa.SaveToDirectory("./keys_client");
    return 0;
}
