#include <iostream>
#include <cassert>
#include "shared_info.hpp"


// Returns 2 public keys, 1 of which is real, and 1 private key
// Given bob's secret bit and 3 buffers (2 public and 1 private)
void bob_ot1(bool bit, char* p1, char* p2, char* pk)
{
  // Generate some keys!
  auto keyPair = CryptoService::generateRSAKeyPair(2048);

  // Copy into our buffers
  size_t len = keyPair.publicKey.copy(p1, RSA_BUFF);
  p1[len] = 0;
  len = keyPair.publicKey.copy(p2, RSA_BUFF);
  p2[len] = 0;
  len = keyPair.privateKey.copy(pk, RSA_BUFF);
  pk[len] = 0;
  //std::cout << "2048 bit Public RSA Key:" << std::endl << keyPair.publicKey << std::endl;
  //std::cout << "2048 bit Private RSA Key:" << std::endl << keyPair.privateKey << std::endl;

  // At this point, we both p1 and p2 are valid keys. We should break one of them
  // magic number of the last byte in the exponent field
  if (bit) // break the 0 bit key
    p1[magic_byte]--;
  else    // break the 1 bit key
    p2[magic_byte]++;
  
  //std::cout << "2048 bit Public RSA Key:" << std::endl << p1 << std::endl;
  //std::cout << "2048 bit Public RSA Key:" << std::endl << p2 << std::endl;
  //std::cout << "2048 bit Private RSA Key:" << std::endl << keyPair.privateKey << std::endl;
}

// Returns one of alice's secret messages
// Given bob's secret bits, bob's private key, and n of Alice's encrypted messages
void bob_ot2(bool bit, char* pk, char msgs[2][KEY_SIZE], char out[KEY_SIZE])
{
    std::cout << msgs[0] << std::endl;
    std::cout << msgs[1] << std::endl;
    auto decryptedText = CryptoService::decryptWithRSA(msgs[(int) bit], pk);
    std::cout << decryptedText << std::endl;
}

void send_keys(char* p1, char* p2)
{
  std::ofstream fifo(from_bob_pipe_name);
  fifo.write(p1, RSA_BUFF);
  fifo.write(p2, RSA_BUFF);
}

void await_messages(char msgs[2][KEY_SIZE])
{
  std::ifstream fifo(from_alice_pipe_name);
  fifo.read(msgs[0], KEY_SIZE);
  fifo.read(msgs[1], KEY_SIZE);
}


// Handles all of Bob's actions given Bob's secret bits
void bob(int num)
{

  // Await Alice's circuit
  //await_circuit();

  // Generate the public keys and private key (encoding bob's bits)
  
  // Allocate space for the three keys
  char p1 [RSA_BUFF];
  char p2 [RSA_BUFF];
  char pk [RSA_BUFF];
  
  char msgs [2][KEY_SIZE];
  char secret[n_bits][KEY_SIZE];
  for (int i = 0; i < n_bits; i++)
  {
    bob_ot1(true, p1, p2, pk);
  
    //Send public keys to Alice
    send_keys(p1, p2);

    // Wait for Alice's encrypted messages (passwords for circuit input)
    await_messages(msgs);

    // Decrypt one of Alice's passwords
    bob_ot2(true, pk, msgs, secret[i]);
  }

  // Evaluate Alice's circuit on the password determined by bob's bits
  //evaluate();
}



int main(int argc, char** argv)
{
  // Make the fifos
  make_fifos();

  assert(argc == 2 && "Usage: ./bob <number>");
  int num = atoi(argv[1]);
  std::cout << "Bob's Input: " << num << std::endl;
  assert(num >= 0 && num < (1 << n_bits) && "Error: Invalid number");   

  // int represents the bits :)
  bob(num);
}
