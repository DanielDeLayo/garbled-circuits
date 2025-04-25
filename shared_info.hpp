#pragma once

#include <sys/types.h>
#include <sys/stat.h>

#include <cstring>
#include <fstream>
#include <vector>

#include "libcpp-crypto.hpp"
using namespace lklibs;


constexpr int n_bits = 5;
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
// AES key half-size
constexpr int PASS_SIZE = 16;

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
  char output_passwords[6][MSG_SIZE];

  char* evaluate(const char* pass)
  {
    //std::cout << "TRYING: " << pass << std::endl;
    for (int i = 0; i < 6; i++){
      //std::cout << "ON: " << valid_inputs[i] << std::endl;
      if (strncmp(valid_inputs[i], pass, SHA256_DIGEST_LENGTH) == 0) 
        return output_passwords[i];
    }
    assert(false && "No valid input!");
    return nullptr;
  }

  gate() {}
 
  gate(bool alice, bool last, const char bob_inps[2][PASS_SIZE], const char in_pass[3][PASS_SIZE], const char out_pass[3][PASS_SIZE])
  {

    // FIXME: To properly garble the gate, we'd have to permute the gate s.t. the position isn't dependent on the input anymore
    // BUT... that's out of scope! :)  

    // We need alice's bit for the logic
    // Whether or not it's the last computation for the plaintext override
    // Bob's possible passwords
    // 3 input passwords
    // 3 output passwords
    
    // higher order bits will be the carrythrough, since 11 can never occur.
    // the lower order bit will be alice
    
    // Gross, but fine    

    std::string concats[6];
    concats[0] = std::string(in_pass[0], PASS_SIZE) + std::string(bob_inps[0], PASS_SIZE);  
    concats[1] = std::string(in_pass[0], PASS_SIZE) + std::string(bob_inps[1], PASS_SIZE);
    concats[2] = std::string(in_pass[1], PASS_SIZE) + std::string(bob_inps[0], PASS_SIZE);
    concats[3] = std::string(in_pass[1], PASS_SIZE) + std::string(bob_inps[1], PASS_SIZE);
    concats[4] = std::string(in_pass[2], PASS_SIZE) + std::string(bob_inps[0], PASS_SIZE);
    concats[5] = std::string(in_pass[2], PASS_SIZE) + std::string(bob_inps[1], PASS_SIZE);
  
    for (int  i = 0; i < 6; i++)
      strncpy(valid_inputs[i], CryptoService::hash(concats[i]).c_str(), SHA256_DIGEST_LENGTH);

    /*
    strcpy(valid_inputs[0], "000");
    strcpy(valid_inputs[1], "001");
    strcpy(valid_inputs[2], "010");
    strcpy(valid_inputs[3], "011");
    strcpy(valid_inputs[4], "100");
    strcpy(valid_inputs[5], "101");
    */

    // Helped vars for equal, greater than, less than
    std::string peq = std::string(out_pass[0], PASS_SIZE);  
    std::string pgt = std::string(out_pass[1], PASS_SIZE);  
    std::string plt = std::string(out_pass[2], PASS_SIZE);  
    // Handle final plaintext case
    if (last)
    {
      //std::cout << "LAST!" << std::endl;
      peq = std::string(eq);
      plt = std::string(lt);
      pgt = std::string(gt);
    }
    
    // Checking against alice's bit

    CryptoService::encryptWithAES(alice? pgt : peq, concats[0]).copy(output_passwords[0], MSG_SIZE);
    CryptoService::encryptWithAES(alice? peq : plt, concats[1]).copy(output_passwords[1], MSG_SIZE);
    CryptoService::encryptWithAES(pgt, concats[2]).copy(output_passwords[2], MSG_SIZE);
    CryptoService::encryptWithAES(pgt, concats[3]).copy(output_passwords[3], MSG_SIZE);
    CryptoService::encryptWithAES(plt, concats[4]).copy(output_passwords[4], MSG_SIZE);
    CryptoService::encryptWithAES(plt, concats[5]).copy(output_passwords[5], MSG_SIZE);

/*
    strncpy(output_passwords[0], alice? pgt : peq, MSG_SIZE);
    strncpy(output_passwords[1], alice? peq : plt, MSG_SIZE);
    // carry
    strncpy(output_passwords[2], pgt, MSG_SIZE);
    strncpy(output_passwords[3], pgt, MSG_SIZE);
    strncpy(output_passwords[4], plt, MSG_SIZE);
    strncpy(output_passwords[5], plt, MSG_SIZE);
*/
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

  circuit(int alice, char msgs [n_bits][2][PASS_SIZE], char passes[n_bits+1][3][PASS_SIZE]) {
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
      // Workaround for empty left password
      char workaround[PASS_SIZE];
      memset(workaround, 0, PASS_SIZE);
      std::string str(workaround, PASS_SIZE);
      str += std::string(input[0], PASS_SIZE);
      //std::cout << "INPUT: " << str << std::endl;
      char* unlocked_password = gates[0].evaluate(CryptoService::hash(str).c_str());
      auto decrypted_password = CryptoService::decryptWithAES(unlocked_password, str);
      //std::cout << "LEN : " << str.length() << std::endl;
      //std::cout << "UNLOCKED: " << unlocked_password << std::endl;
  
      // Evaluate with previous
      for (int i = 1; i < n_bits; i++)
      {
          str = decrypted_password + std::string(input[i], PASS_SIZE);
          //std::cout << "LEN : " << str.length() << std::endl;
          unlocked_password = gates[i].evaluate(CryptoService::hash(str).c_str());
          decrypted_password = CryptoService::decryptWithAES(unlocked_password, str);
          //std::cout << "UNLOCKED: " << unlocked_password << "!" <<  std::endl;
      }
      // Evaluate output
      //std::cout << "ANS: " << decrypted_password << std::endl;
      return strcmp(decrypted_password.c_str(), lt) != 0;
  }

  void send(std::ofstream& fifo)
  {
    for (int i = 0; i < n_bits; i++)
    {
      for (int j = 0; j < 6; j++)
      {
        fifo.write(gates[i].valid_inputs[j], SHA256_DIGEST_LENGTH);
        fifo.write(gates[i].output_passwords[j], MSG_SIZE);
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
        fifo.read(gates[i].output_passwords[j], MSG_SIZE);
      }
    }
  }
};

