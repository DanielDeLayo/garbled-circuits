#include <iostream>
#include <cassert>
#include <string>
#include "shared_info.hpp"


// Returns 2 encrypted messages, 1 of which is garbage
// Given 2 public keys, 1 of which 1 garbage, and Alice's secret bit
void alice_ot1(char msgs[2][PASS_SIZE/2], char keys[2][RSA_BUFF], char enc_msgs[2][MSG_SIZE])
{
  assert(keys[0][magic_byte]+1 == keys[1][magic_byte] && "Alice received bad keys!");

  auto encryptedText = CryptoService::encryptWithRSA(msgs[0], keys[0]);
  auto encryptedText2 = CryptoService::encryptWithRSA(msgs[1], keys[1]);

  size_t len = encryptedText.copy(enc_msgs[0], MSG_SIZE);
  enc_msgs[0][len] = 0;
  len = encryptedText2.copy(enc_msgs[1], MSG_SIZE);
  enc_msgs[1][len] = 0;
}

void await_keys(char* p1, char* p2)
{
  std::ifstream fifo(from_bob_pipe_name);
  fifo.read(p1, RSA_BUFF);
  fifo.read(p2, RSA_BUFF);
  //std::cout << p1 << p2 << std::endl;
}

void send_messages(char msgs[2][MSG_SIZE])
{
  std::ofstream fifo(from_alice_pipe_name);
  fifo.write(msgs[0], MSG_SIZE);
  fifo.write(msgs[1], MSG_SIZE);
  //std::cout << p1 << p2 << std::endl;
}

template <unsigned int n_bits>
circuit<n_bits> garble(int num, char msgs [n_bits][2][PASS_SIZE/2])
{
  char passes[n_bits][3][PASS_SIZE/2];
  // Start at 1: There is no carry password there
  for (int i = 1; i < n_bits; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      // seems good enough as a password
      auto pass = CryptoService::hash(std::to_string(i) + std::string("werhgiusedrgsgrsdrgeriugh") + std::to_string(j));
      pass.copy(passes[i][j], PASS_SIZE/2);
      passes[i][j][PASS_SIZE/2-1] = 0;
    }
  } 
  
  return circuit<n_bits>(num, msgs, passes);
}

template <unsigned int n_bits>
void send_circuit(circuit<n_bits> msg)
{
  std::ofstream fifo(alice_garbled_circuit);
  msg.send(fifo);
  //std::cout << p1 << p2 << std::endl;
}


// Handles all of Alice's actions given Alice's secret bits
void alice(int num)
{
  // plaintext secrets
  char msgs [n_bits][2][PASS_SIZE/2];
  for (int i = 0; i < n_bits; i++)
  {
    //TODO: passwordify
    std::string m0 = "0";
    std::string m1 = "1";
    m0.copy(msgs[i][0], PASS_SIZE/2);
    m1.copy(msgs[i][1], PASS_SIZE/2);
  }
  
  // Alice garbles a circuit 
  // It contains her secret information
  auto circuit = garble<n_bits>(num, msgs);

  // Write down the garbled circuit for Bob
  send_circuit<n_bits>(circuit);

  // Allocate space for the public keys
  char keys [2][RSA_BUFF];
  // encrypted secrets
  char enc_msgs [2][MSG_SIZE];


  for (int i = n_bits-1; i >=0; i--)
  {
    // Wait for Bob to send n public keys
    await_keys(keys[0], keys[1]);

    // Generate encrypted messages
    alice_ot1(msgs[i], keys, enc_msgs);

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
