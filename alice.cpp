#include <iostream>
#include <cassert>
#include "shared_info.hpp"


// Returns 2 encrypted messages, 1 of which is garbage
// Given 2 public keys, 1 of which 1 garbage, and Alice's secret bit
void alice_ot1()
{

}

void await_keys(char* p1, char* p2, char* pk)
{
  std::ifstream fifo(from_bob_pipe_name);
  fifo >> p1 >> p2 >> pk;
}

// Handles all of Alice's actions given Alice's secret bits
void alice(int num)
{
  // Alice garbles a circuit 
  // It contains her secret information
  //auto circuit = garble();

  // Write down the garbled circuit for Bob
  //send_circuit();

  // Allocate space for the three keys
  char p1 [RSA_BUFF];
  char p2 [RSA_BUFF];
  char pk [RSA_BUFF];

  for (int i = 0; i < n_bits; i++)
  {

    // Wait for Bob to send n public keys
    await_keys(p1, p2, pk);

      // Generate encrypted messages
      //alice_ot1();

      // Send them to bob
  //send_messages();
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
