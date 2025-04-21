#include "shared_info.hpp"


// Returns n public keys, 1 of which is real, and 1 private key
// Given bob's secret bits
void bob_ot1()
{

}

// Returns one of alice's secret messages
// Given bob's secret bits, bob's private key, and n of Alice's encrypted messages
void bob_ot2()
{

}

// Handles all of Bob's actions given Bob's secret bits
void bob()
{

  // Await Alice's circuit
  await_circuit();

  // Generate the public keys and private key (encoding bob's bits)
  bob_ot1();
  
  //Send public keys to Alice
  send_keys();

  // Wait for Alice's encrypted messages (passwords for circuit input)
  await_messages();

  // Decrypt one of Alice's passwords
  bob_ot2();

  // Evaluate Alice's circuit on the password determined by bob's bits
  evaluate();
}



int main()
{

  //TODO: Take bits as input


  bob();
}
