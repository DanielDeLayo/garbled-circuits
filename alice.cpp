#include <iostream>
#include <cassert>
#include "shared_info.hpp"


// Returns n encrypted messages, n-1 of which are garbage
// Given n public keys, n-1 of which are garbage, and Alice's n secret messages
void alice_ot1()
{

}

// Handles all of Alice's actions given Alice's secret bits
void alice(int num)
{
  // Alice garbles a circuit 
  // It contains her secret information
  //auto circuit = garble();

  // Write down the garbled circuit for Bob
  //send_circuit();

  // Wait for Bob to send n public keys
  //await_keys()
  
  // Generate encrypted messages
  //alice_ot1();

  // Send them to bob
  //send_messages();

  // That's it!
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
