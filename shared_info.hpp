#pragma once

#include <sys/types.h>
#include <sys/stat.h>

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

