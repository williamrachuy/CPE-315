#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "CPU.h"
#include "ProgramMem.h"

IF_ID_basket F_D;
ID_IE_basket D_E;
IE_MEM_basket E_M;
MEM_WB_basket M_W;

void if(){
   
   if(pc < mem_ptr){
      F_D.dInstr = makeInstruction(mem[pc]);
   
      F_D.PC_curr = pc;
      
      pc += 4;
      
   }
}

void id(){
   
   if(F_D.active){
      getType(unsigned op, unsigned funct, char *type, char *functStr)
   }
   
   
}


void ex(){
   
   
}

void mem(){
   
   
}

void wb(){
   
   
   
}


// Creates the instruction parameters, as used by different instruction types
Instruction makeInstruction(unsigned instr) {
   Instruction iStruct;

   iStruct.op = (instr >> 26) & 0x3F;
   iStruct.rs = (instr >> 21) & 0x1F;
   iStruct.rt = (instr >> 16) & 0x1F;
   iStruct.rd = (instr >> 11) & 0x1F;
   iStruct.shamt = (instr >> 6) & 0x1F;
   iStruct.funct = instr & 0x3F;
   iStruct.imm = instr & 0xFFFF;
   iStruct.addr = instr & 0x03FFFFFF;

   return iStruct;
}


// Returns sign extended value of input
unsigned signExtendHalfWord(unsigned orig){
   unsigned extended = orig & 0x0000FFFF;
   
   if (orig & 0x8000){                 // If MSB set, extend bits
      extended |= 0xFFFF0000;          // Sign extend
   }
   
   return extended;                    // Return sign extended value
}

// Returns sign extended value of input
unsigned signExtendByte(unsigned orig){
   unsigned extended = orig & 0x000000FF;
   
   if (orig & 0x80){                 // If MSB set, extend bits
      extended |= 0xFFFFFF00;          // Sign extend
   }
   
   return extended;                    // Return sign extended value
}

// Executes R type instructions. Each instruction decoded should be self explanatory
void execTypeR(Instruction iStruct, char *functStr) {
   unsigned rs = iStruct.rs,
            rt = iStruct.rt,
            rd = iStruct.rd,
            shamt = iStruct.shamt;

   if      (strcmp(functStr, "sll")    == SAME) reg[rd] =  (unsigned)reg[rt] << shamt;
   else if (strcmp(functStr, "srl")    == SAME) reg[rd] =  (unsigned)reg[rt] >> shamt;
   else if (strcmp(functStr, "sra")    == SAME) reg[rd] =    (signed)reg[rt] >> shamt;
   else if (strcmp(functStr, "sllv")   == SAME) reg[rd] =  (unsigned)reg[rt] << reg[rs];
   else if (strcmp(functStr, "srlv")   == SAME) reg[rd] =  (unsigned)reg[rt] >> reg[rs];
   else if (strcmp(functStr, "srav")   == SAME) reg[rd] =    (signed)reg[rt] >> reg[rs];
   else if (strcmp(functStr, "jr")     == SAME)      pc =  reg[rs] - WORD_SIZE;
   else if (strcmp(functStr, "jalr")   == SAME){reg[ra] =  pc + WORD_SIZE; pc = reg[rs] - WORD_SIZE;}
   else if (strcmp(functStr, "add")    == SAME) reg[rd] =  (signed)  reg[rt] +   (signed)reg[rs];
   else if (strcmp(functStr, "addu")   == SAME) reg[rd] =  (unsigned)reg[rt] + (unsigned)reg[rs];
   else if (strcmp(functStr, "sub")    == SAME) reg[rd] =    (signed)reg[rs] -   (signed)reg[rt];
   else if (strcmp(functStr, "subu")   == SAME) reg[rd] =  (unsigned)reg[rs] - (unsigned)reg[rt];
   else if (strcmp(functStr, "and")    == SAME) reg[rd] =            reg[rt] & reg[rs];
   else if (strcmp(functStr, "or")     == SAME) reg[rd] =            reg[rt] | reg[rs];
   else if (strcmp(functStr, "xor")    == SAME) reg[rd] =            reg[rt] ^ reg[rs];
   else if (strcmp(functStr, "nor")    == SAME) reg[rd] =          ~(reg[rt] | reg[rs]);
   else if (strcmp(functStr, "slt")    == SAME) reg[rd] =   ((signed)reg[rs] <   (signed)reg[rt]) ? TRUE : FALSE;
   else if (strcmp(functStr, "sltu")   == SAME) reg[rd] = ((unsigned)reg[rs] < (unsigned)reg[rt]) ? TRUE : FALSE;
}






// Determines the shift amount to access bytes or half. Memsize is either 2 for halfwords, or 4 for byte memory access
unsigned memAddrAdjust(unsigned memAddress){
   unsigned shiftAmt;
   
   shiftAmt = memAddress % 4;
   memAddress -= shiftAmt;
   return shiftAmt;
}

// Gets the effective 32 bit value from memory, accounting for the 4 byte per address mem storage method.
// Each memory value is located in increments of 4, so returning a value that is not at a address multiple of 4
// requires grabbing a two portions from the adjacent 2 addresses.
// For example, grabbing word at address 0x00000005 requires grabbing upper byte of location 0x00000004, and upper 3 bytes
// from location 0x00000008, and then combining the two values to make an effective word value.
// The project was started using this memory addressing schema based on how the previous lab was setup, and it was easier
// to do this conversion then to change the entire lab to have sequential byte addresses
signed getMemoryValue(unsigned memAddress){
   unsigned lowAddressVal, highAddressVal;
   unsigned memShiftAmt = memAddrAdjust(memAddress);
   
   // If the memory address is a multiple of 4, just return the word at that location
   if(memShiftAmt == 0) return mem[memAddress];
   
   // Lower address value grabs the byte quantity needed and shifts to place into lower portion of the return word
   lowAddressVal = mem[memAddress - memShiftAmt] >> memShiftAmt * 8;

   // Higher address value grabs the byte quantity needed and shifts to place into upper portion of the return word
   highAddressVal = mem[memAddress + (4- memShiftAmt)] << ((4 - memShiftAmt) * 8);
    
   return lowAddressVal | highAddressVal;
}

// Sets memory, like the above function. Confusing for the way we stored memory
void setMemoryValue(unsigned memAddress, signed value, unsigned size){
   unsigned lowAddressVal, highAddressVal;
   unsigned memShiftAmt = memAddrAdjust(memAddress);
   
   if(memShiftAmt == 0) mem[memAddress] = (mem[memAddress]  & (0xFFFFFFFF << size * 8)) | (value & (0xFFFFFFFF >> size * 8));
   else if(size == 1) {mem[memAddress - memShiftAmt] = (mem[memAddress - memShiftAmt] & (0x0000FF00 << ((memShiftAmt - 1) * 8)) | ((value & 0x000000FF) << (memShiftAmt * 8)));}
   else if(size == 2) {
      mem[memAddress - memShiftAmt] = (mem[memAddress - memShiftAmt] & (0x0000FFFF << (memShiftAmt * 8)) | (value & 0x0000FFFF) << (memShiftAmt * 8));
      
      if(memShiftAmt == 3){
         mem[memAddress + memShiftAmt] = (mem[memAddress + memShiftAmt] & 0x000000FF) | (value >> 8 & 0x000000FF);
      }
   }
   else if(size == 4){
      mem[memAddress - memShiftAmt] = (mem[memAddress - memShiftAmt] & (0xFFFFFFFF << (memShiftAmt * 8)) | (value << (memShiftAmt * 8)));
      mem[memAddress + memShiftAmt] = (mem[memAddress + memShiftAmt] & (0xFFFFFFFF >> ((4-memShiftAmt) * 8)) | (value >> ((4-memShiftAmt) * 8)));
   }      
}

// Executes I type instructions. Each instruction decoded should be self explanatory
void execTypeI(Instruction iStruct, char *functStr) {
   unsigned rs = iStruct.rs,
            rt = iStruct.rt,
            imm = iStruct.imm,
            eff = reg[iStruct.rs] + signExtendHalfWord(iStruct.imm),
            memShiftAmt;

   if      (strcmp(functStr, "beq")    == SAME)     {pc =  (reg[rs] == reg[rt]) ? (pc + (short int)(imm << 2)) : pc; stats.clocks--;}
   else if (strcmp(functStr, "bne")    == SAME)     {pc =  (reg[rs] != reg[rt]) ? (pc + (short int)(imm << 2)) : pc; stats.clocks--;}
   else if (strcmp(functStr, "addi")   == SAME) reg[rt] =    (signed)reg[rs] +   signExtendHalfWord(imm);
   else if (strcmp(functStr, "addiu")  == SAME) reg[rt] =  (unsigned)reg[rs] + (unsigned)imm;
   else if (strcmp(functStr, "slti")   == SAME) reg[rt] =   ((signed)reg[rs] <   signExtendHalfWord(imm)) ? TRUE : FALSE;
   else if (strcmp(functStr, "slti")   == SAME) reg[rt] = ((unsigned)reg[rs] < (unsigned)imm) ? TRUE : FALSE;
   else if (strcmp(functStr, "andi")   == SAME) reg[rt] =    (signed)reg[rs] &   imm;
   else if (strcmp(functStr, "ori")    == SAME) reg[rt] =    (signed)reg[rs] |   imm;
   else if (strcmp(functStr, "xori")   == SAME) reg[rt] =    (signed)reg[rs] ^   imm;
   else if (strcmp(functStr, "lui")    == SAME){reg[rt] = (imm << 16) & 0xFFFF0000; stats.clocks++;}
   else if (strcmp(functStr, "lb")     == SAME){reg[rt] = signExtendByte(getMemoryValue(eff) & 0x000000FF); stats.memRefs++; stats.clocks++;}
   else if (strcmp(functStr, "lh")     == SAME){reg[rt] = signExtendHalfWord(getMemoryValue(eff) & 0x0000FFFF); stats.memRefs++; stats.clocks++;}
   else if (strcmp(functStr, "lw")     == SAME){reg[rt] = getMemoryValue(eff); stats.memRefs++; stats.clocks++;}
   else if (strcmp(functStr, "lbu")    == SAME){reg[rt] = (unsigned)((mem[eff & 0xFFFFFFFC] >> (4 *  (eff % 4))) & 0x000000FF); stats.memRefs++; stats.clocks++;}
   else if (strcmp(functStr, "lhu")    == SAME){reg[rt] = (unsigned)((mem[eff & 0xFFFFFFFC + imm] >> (16 * (eff % 2))) & 0x0000FFFF); stats.memRefs++; stats.clocks++;}
   else if (strcmp(functStr, "sb")     == SAME){mem[eff] = reg[rt] & 0x000000FF + signExtendHalfWord(imm); stats.memRefs++;}
   else if (strcmp(functStr, "sh")     == SAME){mem[eff] = reg[rt] & 0x0000FFFF + signExtendHalfWord(imm); stats.memRefs++;}
   else if (strcmp(functStr, "sw")     == SAME){mem[eff] = reg[rt]; stats.memRefs++;}
}

// Executes J type instructions. Each instruction decoded should be self explanatory
void execTypeJ(Instruction iStruct, char *functStr) {
   unsigned addr = iStruct.addr;  
   
   if (strcmp(functStr, "jal")    == SAME){reg[ra] = pc + WORD_SIZE; pc = ((pc & 0xE0000000) | (addr << 2)) - WORD_SIZE;}
   else if (strcmp(functStr, "j")      == SAME)       pc = ((pc & 0xE0000000) | (addr << 2)) - WORD_SIZE;
          
}

// Checks for the type of instruction and calls the appropriate instruction type execution function
void execInstruction(unsigned instr) {
   char type, functStr[8];
   Instruction iStruct = makeInstruction(instr);

   // Get the type of the instruction first
   getType(iStruct.op, iStruct.funct, &type, functStr);
   
   if(instr == 0x00000000){
      // Check for nop
      stats.execs += 1;
      stats.clocks += 3;
   }
   else if(instr == 0x0000000C){
      // syscall. Assumes HALT, other syscalls to be implemented in future. If PC = mem_ptr, the PC is at end of execution
      if(reg[v0] == 0x000A) pc = mem_ptr;                                   
   }
   else if (type == 'F') {
      // Indicates an invalid instruction has been reached, however continues with program execution
      printf("   Invalid instruction at 0x%08X\n", pc);
   }
   else if (type == 'R') {
      // Executes R type instructions. Assumes 4 clock cycles, but adjusts per instruction as needed
      stats.execs += 1;
      stats.clocks += 4;
      execTypeR(iStruct, functStr);
   }
   else if (type == 'I') {
      // Executes I type instructions. Assumes 4 clock cycles, but adjusts per instruction as needed
      stats.execs += 1;
      stats.clocks += 4;
      execTypeI(iStruct, functStr);
   }
   else if (type == 'J') {
      // Executes J type instructions. Assumes 4 clock cycles, but adjusts per instruction as needed
      stats.execs += 1;
      stats.clocks += 3;
      execTypeJ(iStruct, functStr);
   }
   else {
      // Default, shouldn't be reached
      printf("   Invalid instruction at 0x%08X\n", pc);
   }
   
   // Always increment PC. Any instruction that assumes an add of +4 (like branches) doesn't do so, as it is done here
   pc += WORD_SIZE;
}