#include <iostream>
#include <cassert>
#include <string>
#include "shared_info.hpp"


// Returns 2 encrypted messages, 1 of which is garbage
// Given 2 public keys, 1 of which 1 garbage, and Alice's secret bit
void alice_ot1(char msgs[2][KEY_SIZE], char keys[2][RSA_BUFF], char enc_msgs[2][KEY_SIZE])
{
  assert(keys[0][magic_byte]+1 == keys[1][magic_byte] && "Alice received bad keys!");

  auto encryptedText = CryptoService::encryptWithRSA(msgs[0], keys[0]);
  auto encryptedText2 = CryptoService::encryptWithRSA(msgs[1], keys[1]);

  size_t len = encryptedText.copy(enc_msgs[0], KEY_SIZE);
  enc_msgs[0][len] = 0;
  len = encryptedText2.copy(enc_msgs[1], KEY_SIZE);
  enc_msgs[1][len] = 0;
  std::cout << enc_msgs[0] << std::endl;
  std::cout << enc_msgs[1] << std::endl;
}

void await_keys(char* p1, char* p2)
{
  std::ifstream fifo(from_bob_pipe_name);
  fifo.read(p1, RSA_BUFF);
  fifo.read(p2, RSA_BUFF);
  //std::cout << p1 << p2 << std::endl;
}

void send_messages(char msgs[2][KEY_SIZE])
{
  std::ofstream fifo(from_alice_pipe_name);
  fifo.write(msgs[0], KEY_SIZE);
  fifo.write(msgs[1], KEY_SIZE);
  //std::cout << p1 << p2 << std::endl;
}






// Handles all of Alice's actions given Alice's secret bits
void alice(int num)
{
  // Alice garbles a circuit 
  // It contains her secret information
  //auto circuit = garble();

  // Write down the garbled circuit for Bob
  //send_circuit();

  // Allocate space for the public keys
  char keys [2][RSA_BUFF];
  // plaintext messages
  char msgs [2][KEY_SIZE];
  // encrypted messages
  char enc_msgs [2][KEY_SIZE];

  std::string m0 = "0"; 
  std::string m1 = "1"; 

  m0.copy(msgs[0], KEY_SIZE);
  m1.copy(msgs[1], KEY_SIZE);

  for (int i = 0; i < n_bits; i++)
  {

    // Wait for Bob to send n public keys
    await_keys(keys[0], keys[1]);

    // Generate encrypted messages
    alice_ot1(msgs, keys, enc_msgs);

    // Send them to bob
    send_messages(enc_msgs);
  }
}


int main(int argc, char** argv)
{
  // Make the fifos
  make_fifos();
  
  assert(argc == 2 && "Usage: ./alice <number>");
  int num = atoi(argv[1]);
  std::cout << "Alice's Input: " << num << std::endl;
  assert(num >= 0 && num < (1 << n_bits) && "Error: Invalid number");

  // What better way to pass around bits than an integer?
  alice(num);
}
