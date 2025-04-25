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
  char valid_inputs[6][SHA256_DIGEST_LENGTH+1]; 
  char output_passwords[6][PASS_SIZE/2];

  char* evaluate(const char* pass)
  {
    for (int i = 0; i < 6; i++)
      if (strcmp(valid_inputs[i], pass) == 0) 
        return output_passwords[i];
    assert(false && "No valid input!");
    return nullptr;
  }
 
  gate() {}
 
  gate(bool alice, bool last, char bob_inps[2][PASS_SIZE/2], char in_pass[3][PASS_SIZE/2], char out_pass[3][PASS_SIZE/2])
  {
    // We need alice's bit for the logic
    // Whether or not it's the last computation for the plaintext override
    // Bob's possible passwords
    // 3 input passwords
    // 3 output passwords
    
    // higher order bits will be the carrythrough, since 11 can never occur.
    // the lower order bit will be alice
    
    // Gross, but fine    
    strcpy(valid_inputs[0], CryptoService::hash(std::string(in_pass[0]) + std::string(bob_inps[0])).c_str());
    strcpy(valid_inputs[1], CryptoService::hash(std::string(in_pass[0]) + std::string(bob_inps[1])).c_str());
    strcpy(valid_inputs[2], CryptoService::hash(std::string(in_pass[1]) + std::string(bob_inps[0])).c_str());
    strcpy(valid_inputs[3], CryptoService::hash(std::string(in_pass[1]) + std::string(bob_inps[1])).c_str());
    strcpy(valid_inputs[4], CryptoService::hash(std::string(in_pass[2]) + std::string(bob_inps[0])).c_str());
    strcpy(valid_inputs[5], CryptoService::hash(std::string(in_pass[2]) + std::string(bob_inps[1])).c_str());

    /*
    strcpy(valid_inputs[0], "000");
    strcpy(valid_inputs[1], "001");
    strcpy(valid_inputs[2], "010");
    strcpy(valid_inputs[3], "011");
    strcpy(valid_inputs[4], "100");
    strcpy(valid_inputs[5], "101");
    */

    // Helped vars for equal, greater than, less than
    char* peq = out_pass[0];  
    char* pgt = out_pass[1];
    char* plt = out_pass[2];
    
    // Checking against alice's bit
    strcpy(output_passwords[0], alice? pgt : peq);
    strcpy(output_passwords[1], alice? peq : plt);
    // carry
    strcpy(output_passwords[2], pgt);
    strcpy(output_passwords[3], pgt);
    strcpy(output_passwords[4], plt);
    strcpy(output_passwords[5], plt);

    /*
    // Checking against alice's bit
    strcpy(output_passwords[0], alice? gt : eq);
    strcpy(output_passwords[1], alice? eq : lt);
    // carry
    strcpy(output_passwords[2], gt);
    strcpy(output_passwords[3], gt);
    strcpy(output_passwords[4], lt);
    strcpy(output_passwords[5], lt);
    */
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

  circuit(int alice, char msgs [n_bits][2][PASS_SIZE/2], char passes[n_bits][3][PASS_SIZE/2]) {
    for (int i = 0; i < n_bits; i++)
    {
      // gate 0 becomes highest order bit
      bool alice_bit = alice & (1 << (n_bits-1 -i));
      gates[i] = gate(alice_bit, i == n_bits-1, msgs[(n_bits-1 -i)], passes[i], passes[i+1]);
    }
  }

  // highest order bit is index 0
  bool evaluate(const char input[n_bits][PASS_SIZE]) {
      // Evaluate with just the password
      std::string str(input[0]);
      char* unlocked_password = gates[0].evaluate(CryptoService::hash(str).c_str());
      //std::cout << "UNLOCKED: " << unlocked_password << std::endl;
  
      // Evaluate with previous
      for (int i = 1; i < n_bits; i++)
      {
          str = std::string(unlocked_password) + std::string(input[i]);
          unlocked_password = gates[i].evaluate(CryptoService::hash(str).c_str());
          std::cout << "UNLOCKED: " << unlocked_password << "!" <<  std::endl;
      }
      // Evaluate output
      return strcmp(unlocked_password, lt) != 0;
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

