# RISCV Vector Notes





## References
* RISCV Vector ISA Tutorial:
  * https://riscv.org/wp-content/uploads/2024/12/15.20-15.55-18.05.06.VEXT-bcn-v1.pdf
* RISCV Vector Extension:
  * http://0x80.pl/notesen/2024-11-09-riscv-vector-extension.html


## RISC Vector ISA Tutorial
* Overview
  * 32 vector registers (v0 ... v31)
    * each register can hold either a scalar, a vector, or a matrix
    * each vector register can optionally have an associated type (polymorphic encoding)
    * variable number of registers (dnyamically changeable)
  * Vector instruction semantics
    * All instructions controlled by Vector Length (VL) register
    * All instructions can be executed iunder mask
    * Intuitive memory ordering model
    * Precise exceptions supported
  * Vector instruction set
    * All instructions present in base line ISA are present in the vector ISA
    * Vector memory instructions supporting linear, strided and gather/scatter access patterns
    * Optional Fixed-Point set
    * Optional Transcendental set
* New Architectural State
  *


