# garbled-circuits
Implementation based on https://hackmd.io/@aardvark/BJYYcR1N1g

Can be compiled with make

Usage: ./bob <x> or ./alice <x>, where x is a positive number less than 10 bits
(10 can be changed in shared_info.hpp)

Note: There's a small change of public key corruption. In that case, simply ctrl+c bob's terminal and re-run.
Communication is over FIFOs. 


