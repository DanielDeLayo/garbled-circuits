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

  //FIXME: This can corrupt the public key without proper wrap around. 
  if (bit) // break the 0 bit key
    p1[magic_byte]--;
  else    // break the 1 bit key
    p2[magic_byte]++;
  
  //std::cout << "2048 bit Public RSA Key:" << std::endl << p1 << std::endl;
  //std::cout << "2048 bit Public RSA Key:" << std::endl << p2 << std::endl;
  //std::cout << "2048 bit Private RSA Key:" << std::endl << keyPair.privateKey << std::endl;
}

// Returns one of alice's secret messages
// Given bob's secret bits, bob's private key, and 2 of Alice's encrypted messages
void bob_ot2(bool bit, char* pk, char msgs[2][MSG_SIZE], char out[MSG_SIZE])
{
    auto decryptedText = CryptoService::decryptWithRSA(msgs[(int) bit], pk);
    //std::cout << "BOB GOT: " << decryptedText << std::endl;
    decryptedText.copy(out, MSG_SIZE);
}

void send_keys(char* p1, char* p2)
{
  std::ofstream fifo(from_bob_pipe_name);
  fifo.write(p1, RSA_BUFF);
  fifo.write(p2, RSA_BUFF);
}

void await_messages(char msgs[2][MSG_SIZE])
{
  std::ifstream fifo(from_alice_pipe_name);
  fifo.read(msgs[0], MSG_SIZE);
  fifo.read(msgs[1], MSG_SIZE);
}
template <unsigned int n_bits>
circuit<n_bits> await_circuit()
{
  circuit<n_bits> msg; 
  std::ifstream fifo(alice_garbled_circuit);
  msg.recv(fifo);
  return msg;
}


// Handles all of Bob's actions given Bob's secret bits
void bob(int num)
{

  // Await Alice's circuit
  auto garbled = await_circuit<n_bits>();

  // Generate the public keys and private key (encoding bob's bits)
  
  // Allocate space for the three keys
  char p1 [RSA_BUFF];
  char p2 [RSA_BUFF];
  char pk [RSA_BUFF];
  
  char msgs [2][MSG_SIZE];
  char secret[n_bits][PASS_SIZE];
  for (int i = 0; i < n_bits; i++)
  {
    // Extract the ith bit from bob's number (starting at MSB)
    bool bit = num & (1 << (n_bits-1 -i));

    bob_ot1(bit, p1, p2, pk);
  
    //Send public keys to Alice
    send_keys(p1, p2);

    // Wait for Alice's encrypted messages (passwords for circuit input)
    await_messages(msgs);

    // Decrypt one of Alice's passwords
    bob_ot2(bit, pk, msgs, secret[i]);
  }

  // Evaluate Alice's circuit on the password determined by bob's bits
  std::cout << "ALICE GREATER EQUAL: " << garbled.evaluate(secret) << std::endl;
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
