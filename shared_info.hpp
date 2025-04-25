#pragma once

#include <sys/types.h>
#include <sys/stat.h>

#include <cstring>
#include <fstream>
#include <vector>

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

auto gt = "01";
auto lt = "10";
auto eq = "00";
//00 encodes equality, 01 encodes alice greater than, and 10 encodes alice less than.

/*
 * This gate is a simple comparison gate with carry-through.
 * That is, 4 input wires and 2 output wires. 
 * 2 input wires are override wires, representing a higher comparison.
 * 1 input wire is Bob's bit, and the other is Alice's bit.
 * The 2 output wires are the override wires for the next circuit
 * 
 * This gate also, notably, already encodes Alice's informaton.
 * There are therefore 6 valid inputs and 6 possible outputs. That is, the three carrybit states x two bob states.
*/
struct gate
{
  char valid_inputs[6][SHA256_DIGEST_LENGTH]; 
  char output_passwords[6][PASS_SIZE/2];

  //TODO: encryption and hashing
  char* evaluate(char* pass)
  {
    int index = atoi(pass);
    return output_passwords[index];
  }
 
  gate() {}
 
  gate(bool alice, bool last)
  {
    // TODO: higher order bits will be the carrythrough, since 11 can never occur.
    // the lower order bit will be alice
    strcpy(valid_inputs[0], "000");
    strcpy(valid_inputs[1], "001");
    strcpy(valid_inputs[2], "010");
    strcpy(valid_inputs[3], "011");
    strcpy(valid_inputs[4], "100");
    strcpy(valid_inputs[5], "101");

    // Checking against alice's bit
    strcpy(output_passwords[0], alice? gt : eq);
    strcpy(output_passwords[1], alice? eq : lt);
    // carry
    strcpy(output_passwords[2], gt);
    strcpy(output_passwords[3], gt);
    strcpy(output_passwords[4], lt);
    strcpy(output_passwords[5], lt);
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
  gate gates[n_bits];

public:
  circuit() {}

  circuit(int alice) {
    for (int i = 0; i < n_bits; i++)
    {
      // gate 0 becomes highest order bit
      bool alice_bit = alice & (1 << (n_bits-1 -i));
      gates[i] = gate(alice_bit, i == n_bits-1);
    }
  }

  // highest order bit is index 0
  bool evaluate(char input[n_bits][PASS_SIZE]) {
      char* unlocked_password = gates[0].evaluate(input[0]);
      for (int i = 1; i < n_bits; i++)
      {
          unlocked_password = gates[i].evaluate(unlocked_password);
      }
      return unlocked_password == eq || unlocked_password == gt;
  }

  void send(std::ofstream& fifo)
  {
    for (int i = 0; i < n_bits; i++)
    {
      for (int j = 0; j < 6; j++)
      {
        fifo.write(gates[i].valid_inputs[j], SHA256_DIGEST_LENGTH);
        fifo.write(gates[i].output_passwords[j], PASS_SIZE/2);
      }
    }
  }

  void recv(std::ifstream& fifo)
  {
    for (int i = 0; i < n_bits; i++)
    {
      for (int j = 0; j < 6; j++)
      {
        fifo.read(gates[i].valid_inputs[j], SHA256_DIGEST_LENGTH);
        fifo.read(gates[i].output_passwords[j], PASS_SIZE/2);
      }
    }
  }
};

