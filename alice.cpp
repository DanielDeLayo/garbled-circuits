#include "shared_info.hpp"


// Returns n encrypted messages, n-1 of which are garbage
// Given n public keys, n-1 of which are garbage, and Alice's n secret messages
void alice_ot1()
{

}

// Handles all of Alice's actions given Alice's secret bits
void alice()
{
  // Alice garbles a circuit 
  // It contains her secret information
  auto circuit = garble();

  // Write down the garbled circuit for Bob
  send_circuit();

  // Wait for Bob to send n public keys
  await_keys()
  
  // Generate encrypted messages
  alice_ot1();

  // Send them to bob
  send_messages();

  // That's it!
}


int main()
{

  //TODO: Take bits as input

  alice();
}
