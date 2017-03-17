# Nexus smoke testing info.

The scripts in this directory are designed help you smoke test nexus locally.

## Prereq: Install the CPU mining code

In order to be able to produce blocks, you need to install the CPU miner code.
This is easy if you're on ubuntu. 
Grab this script and run it

https://github.com/physicsdude/nexusscripts/blob/master/install-cpu-miner-on-ubuntu.sh

Then, when you're ready to mine blocks on your mini-chain, use the script below.

## setup-two-local-test-nodes.sh

This will set up a fresh local test net with 2 nodes that communicate with each other.
Look in ~/nexustest after you run this.

## run-start-nexus-test.sh

This script will set up a test data directory and start nexus in test modes for you.
It can also be used to set up an environment suitable for step through debugging.
Check out the source for more info.

## makefile.unix.test

If you want to step-through debug, it's useful to compile with no optimizations.
Otherwise some variables get 'optimized out'.
This file is identical to makefile.unix with the setting -O0.
It's mostly for reference and if it gets out of date just grab the standard makefile.unix
and change -O2 to -O0

## setup-for-debugger.sh

Simple script to set up a nexus environment for step through debugging.

## Notes

The testnet, regtest, and istimeseed options are all important for successfully running a local testnet.
Either use or copy the configs used in run-start-nexus-test.sh.

# THE TRUTH IS OUT THERE
