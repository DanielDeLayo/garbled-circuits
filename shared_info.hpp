#pragma once

#include <sys/types.h>
#include <sys/stat.h>

#include <cstring>
#include <fstream>

#include "libcpp-crypto.hpp"
using namespace lklibs;


constexpr int n_bits = 2;
// Storing RSA keys
constexpr size_t RSA_BUFF = 5000;
// Byte to change in public key
constexpr int magic_byte = 417;

const char* from_alice_pipe_name = "from-alice.pipe";
const char* from_bob_pipe_name = "from-bob.pipe";
const char* alice_garbled_circuit = "from-alice-garbled-circuit.pipe";


void make_fifos()
{
  mkfifo(from_alice_pipe_name, 0666);
  mkfifo(from_bob_pipe_name, 0666);
  mkfifo(alice_garbled_circuit, 0666);
}

// Encrypted message size
constexpr int MSG_SIZE = 512;
// AES message size
constexpr int PASS_SIZE = 128;


/*
 * This gate is a simple comparison gate with carry-through.
 * That is, 4 input wires and 2 output wires. 
 * 2 input wires are override wires, representing a higher comparison.
 * 1 input wire is Bob's bit, and the other is Alice's bit.
 * The 2 output wires are the override wires for the next circuit
 * 00 encodes equality, 01 encodes greater than, and 10 encodes less than.
 * 
 * This gate also, notably, already encodes Alice's informaton.
 * There are therefore 6 valid inputs and 6 possible outputs. That is, the three carrybit states x two bob states.
*/
struct gate
{
  char valid_inputs[6][SHA256_DIGEST_LENGTH]; 
  char output_passwords[6][PASS_SIZE/2];

  //TODO: encryption and hashing
  char* evaluate(char* pass1, char* pass2)
  {
    int index = (atoi(pass1) << 2) + atoi(pass2);
    std::cout << index << std::endl;
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
  
};


