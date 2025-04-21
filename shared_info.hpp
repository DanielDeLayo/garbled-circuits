#pragma once

#include <sys/types.h>
#include <sys/stat.h>

#include <openssl/aes.h>
#include <openssl/sha.h>

#include <cstring>

constexpr int n_bits = 2;

// Difference between bob's public keys
constexpr int r = 1;

std::string from_alice_pipe_name = "from-alice.pipe";
std::string from_bob_pipe_name = "from-bob.pipe";
std::string alice_garbled_circuit = "from-alice-garbled-circuit.pipe";


void make_fifos()
{
  mkfifo(from_alice_pipe_name.c_str(), 0666);
  mkfifo(from_bob_pipe_name.c_str(), 0666);
  mkfifo(alice_garbled_circuit.c_str(), 0666);
}


constexpr int KEY_SIZE = 128;

class gate
{
  char valid_inputs[4][SHA256_DIGEST_LENGTH]; 
  char output_passwords[4][KEY_SIZE/2];

  //TODO: encryption and hashing
  char* evaluate(char* pass1, char* pass2)
  {
    int index = (atoi(pass1) << 1) + atoi(pass2);
    return output_passwords[index];
  }


  gate(int output[4])
  {
    //TODO: This for real
    for (int i = 0; i < 4; i++)
    {
      if (output[i])
        strcpy(output_passwords[i], "1");
      else
        strcpy(output_passwords[i], "0");
    } 
  }
};

//TODO: Let's simplify the problem.
// Comparison is much easier with different gates
// New gate idea: 4 input wires, 2 output wires 
// 2 inputs for the 2 bits, 2 inputs for comparison carrythrough
// 2 outputs for the comparison
// No reason to break it into AND or OR gates
// This gives us a simple to understand linear depth circuit

template <unsigned int n_bits>
class circuit
{

}


