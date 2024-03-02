# Cipher@Blat 128-bit

Password generator with ~128-bit strength.

You can set a unique master phrase in any language in UTF-8 encoding to generate your own password sequence. It allows to restore all passwords using only one master phrase which must be stored in a safe place. For example, you input a "word1 word2 word3 ... wordN" master phrase and generate M passwords of Q symbols each. Next you can number passwords and assign a service and login to each. This information should be written down on a piece of paper and kept separately.

The algorithm is based on LFSR hash and Random Number Generator, https://github.com/nawww83/lfsr.
