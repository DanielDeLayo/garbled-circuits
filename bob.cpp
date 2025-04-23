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
  keyPair.publicKey.copy(p1, RSA_BUFF, 0);
  keyPair.publicKey.copy(p2, RSA_BUFF, 0);
  keyPair.privateKey.copy(pk, RSA_BUFF, 0);
  //std::cout << "2048 bit Public RSA Key:" << std::endl << keyPair.publicKey << std::endl;
  //std::cout << "2048 bit Private RSA Key:" << std::endl << keyPair.privateKey << std::endl;

  // At this point, we both p1 and p2 are valid keys. We should break one of them
  // magic number of the last byte in the exponent field
  if (bit) // break the 0 bit key
    p1[417]--;
  else    // break the 1 bit key
    p2[417]++;
  
  //std::cout << "2048 bit Public RSA Key:" << std::endl << p1 << std::endl;
  //std::cout << "2048 bit Public RSA Key:" << std::endl << p2 << std::endl;
  //std::cout << "2048 bit Private RSA Key:" << std::endl << keyPair.privateKey << std::endl;
}

// Returns one of alice's secret messages
// Given bob's secret bits, bob's private key, and n of Alice's encrypted messages
void bob_ot2()
{

}

void send_keys(char* p1, char* p2, char* pk)
{
  std::ofstream fifo(from_bob_pipe_name);
  fifo << p1 << p2 << pk;
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
  for (int i = 0; i < n_bits; i++)
  {
    bob_ot1(true, p1, p2, pk);
  
    //Send public keys to Alice
    send_keys(p1, p2, pk);

    // Wait for Alice's encrypted messages (passwords for circuit input)
    //await_messages();

    // Decrypt one of Alice's passwords
    //bob_ot2();
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
