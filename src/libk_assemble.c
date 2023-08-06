/*
 *    libk_assemble.c    --    Source for KAPPA assembly
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 2, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    This file defines functions for assembling KAPPA source code.
 */
#include "libk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#include "libk_assemble.h"

/*
 *    Appends bytecode to the current function.
 *
 *    @param k_env_t *env    The environment to append bytecode to.
 *    @param char *bytecode  The bytecode to append.
 *    @param unsigned long   The length of the bytecode.
 */
void _k_append_bytecode(k_env_t *env, char *bytecode, unsigned long length) {
    env->runtime->mem = (char *)realloc(env->runtime->mem, env->runtime->size + length);

    memcpy(env->runtime->mem + env->runtime->size, bytecode, length);

    env->runtime->size  += length;
}

/*
 *    Inserts bytecode into the current function.
 *
 *    @param k_env_t  *env       The environment to insert bytecode into.
 *    @param char     *bytecode  The bytecode to insert.
 *    @param unsigned  long      The length of the bytecode.
 *    @param unsigned  long      The offset to insert the bytecode at.
 */
void _k_insert_bytecode(k_env_t *env, char *bytecode, unsigned long length, unsigned long offset) {
    env->runtime->mem = (char *)realloc(env->runtime->mem, env->runtime->size + length);

    memmove(env->runtime->mem + offset + length, env->runtime->mem + offset, env->runtime->size - offset);

    memcpy(env->runtime->mem + offset, bytecode, length);

    env->runtime->size += length;
}

/*
 *    Generates the assembly for the prelude.
 *
 *    @param k_env_t *env    The environment to generate the prelude for.
 */
void _k_assemble_prelude(k_env_t *env) {
    /*
     *     55            push rbp
     *     48 89 E5      mov  rbp, rsp
     */
    const char *prelude = "\x55\x48\x89\xE5";

    _k_append_bytecode(env, (char *)prelude, 4);
}

/*
 *    Generates assembly for memory allocation.
 *
 *    @param k_env_t *env    The environment to generate the memory allocation for.
 *    @param unsigned long   The size of the memory to allocate.
 *    @param unsigned long   The offset into the function to allocate at.
 */
void _k_assemble_allocate(k_env_t *env, unsigned long size, unsigned long offset) {
    /*
     *     48 81 EC 00   sub  rsp, size
     */
    const char *allocate = "\x48\x81\xEC\x00";
    signed long size_signed = size;

    _k_insert_bytecode(env, (char *)allocate, 3, offset);
    _k_insert_bytecode(env, (char *)&size_signed, 4, offset + 3);
}

/*
 *    Generates a paremeter store for a function.
 *
 *    @param k_env_t *env    The environment to generate the parameter store for.
 *    @param unsigned long   The offset of the parameter to store.
 *    @param unsigned long   The current parameter index.
 *    @param unsigned long   The size of the parameter to store, in bytes.
 *    @param char            flt Whether or not the parameter is a float.
 */
void _k_assemble_parameter_store(k_env_t *env, unsigned long offset, unsigned long index, unsigned long size, char flt) {
    /*
     *     48 89 BD 00 00 00 01   mov  [rbp + offset], reg
     */
    const char *store = (const char*)0x0;
    if (flt == 0) {
        /* RDI  */
        if (index == 0) {
            if (size == 8)      store = "\x48\x89\xBD";
            else if (size == 4) store = "\x89\xBD";
            else if (size == 2) store = "\x66\x89\xBD";
            else                store = "\x40\x88\xBD";
        }
        
        /* RSI  */
        else if (index == 1) {
            if (size == 8)      store = "\x48\x89\xB5";
            else if (size == 4) store = "\x89\xB5";
            else if (size == 2) store = "\x66\x89\xB5";
            else                store = "\x40\x88\xB5";
        }

        /* R8   */
        else if (index == 2) {
            if (size == 8)      store = "\x4C\x89\x85";
            else if (size == 4) store = "\x44\x89\x85";
            else if (size == 2) store = "\x66\x44\x89\x85";
            else                store = "\x40\x44\x88\x85";
        }

        /* R9   */
        else if (index == 3) {
            if (size == 8)      store = "\x4C\x89\x8D";
            else if (size == 4) store = "\x44\x89\x8D";
            else if (size == 2) store = "\x66\x44\x89\x8D";
            else                store = "\x40\x44\x88\x8D";
        }
    }

    else {
        /* XMM7  */
        if (index == 0) {
            if (size == 8) store = "\xF2\x0F\x11\xBD";
            else           store = "\xF3\x0F\x11\xBD";
        }
        
        /* XMM6  */
        else if (index == 1) {
            if (size == 8) store = "\xF2\x0F\x11\xB5";
            else           store = "\xF3\x0F\x11\xB5";
        }

        /* XMM5   */
        else if (index == 2) {
            if (size == 8) store = "\xF2\x0F\x11\xAD";
            else           store = "\xF3\x0F\x11\xAD";
        }

        /* XMM4   */
        else if (index == 3) {
            if (size == 8) store = "\xF2\x0F\x11\xA5";
            else           store = "\xF3\x0F\x11\xA5";
        }
    }

    signed long offset_signed = 0xFFFFFFFF - offset + 1;

    _k_append_bytecode(env, (char *)store, strlen(store));
    _k_append_bytecode(env, (char *)&offset_signed, 4);
}

/*
 *    Generates a parameter load for a function.
 *
 *    @param k_env_t *env    The environment to generate the parameter load for.
 *    @param unsigned long   The current parameter index.
 */
void _k_assemble_parameter_load(k_env_t *env, unsigned long index, unsigned long size, char flt) {
    /*
     *     48 8B BD 00 00 00 01   mov  reg, rax
     */
    const char *load = (const char*)0x0;

    if (flt == 0) {
        if (index == 0) {
            if (size == 8)      load = "\x48\x89\xC7";
            else if (size == 4) load = "\x89\xC7";
            else if (size == 2) load = "\x66\x89\xC7";
            else                load = "\x40\x88\xC7";
        }

        else if (index == 1) {
            if (size == 8)      load = "\x48\x89\xD7";
            else if (size == 4) load = "\x89\xD7";
            else if (size == 2) load = "\x66\x89\xD7";
            else                load = "\x40\x88\xD7";
        }

        else if (index == 2) {
            if (size == 8)      load = "\x4C\x89\xC7";
            else if (size == 4) load = "\x44\x89\xC7";
            else if (size == 2) load = "\x66\x44\x89\xC7";
            else                load = "\x40\x44\x88\xC7";
        }

        else if (index == 3) {
            if (size == 8)      load = "\x4C\x89\xD7";
            else if (size == 4) load = "\x44\x89\xD7";
            else if (size == 2) load = "\x66\x44\x89\xD7";
            else                load = "\x40\x44\x88\xD7";
        }
    }

    else {
        if (index == 0) {
            if (size == 8) load = "\xF2\x0F\x10\xC7";
            else           load = "\xF3\x0F\x10\xC7";
        }

        else if (index == 1) {
            if (size == 8) load = "\xF2\x0F\x10\xD7";
            else           load = "\xF3\x0F\x10\xD7";
        }

        else if (index == 2) {
            if (size == 8) load = "\xF2\x0F\x10\xCF";
            else           load = "\xF3\x0F\x10\xCF";
        }

        else if (index == 3) {
            if (size == 8) load = "\xF2\x0F\x10\xDF";
            else           load = "\xF3\x0F\x10\xDF";
        }
    }

    _k_append_bytecode(env, (char *)load, strlen(load));
}

/*
 *    Pops the parameters off the stack.
 *
 *    @param k_env_t *env    The environment to pop the parameters for.
 */
void _k_assemble_pop_parameters(k_env_t *env) {
    /*
     *    58    pop rax
     */
    const char *pop = "\x58";

    _k_append_bytecode(env, (char *)pop, 1);
}

/*
 *    Generates the assembly for a call.
 *
 *    @param k_env_t *env    The environment to generate the call for.
 *    @param unsigned long   The offset of the function to call.
 */
void _k_assemble_call(k_env_t *env, unsigned long offset) {
    /*
     *    E8 00 00 00 00    call offset
     */
    const char *call = "\xE8";
    signed int offset_signed = (char*)offset - (env->runtime->mem + env->runtime->size + 5 + 7);

    _k_append_bytecode(env, (char *)call, 1);
    _k_append_bytecode(env, (char *)&offset_signed, 4);
}

/*
 *    Generates assembly for an assignment.
 *
 *    @param k_env_t      *env      The environment to generate the assignment for.
 *    @param unsigned long offset   The offset of the variable to assign to.
 *    @param unsigned long size     The size of the variable to assign to, in bytes.
 *    @param char          flt      Whether or not the variable is a float.
 */
void _k_assemble_assignment(k_env_t *env, unsigned long offset, unsigned long size, char flt) {
    /*
     *     48 89 85 00 00 00 01   mov  [rbp + offset], rax
     */
    const char   *assignment    = (const char*)0x0;
    signed long   offset_signed = 0xFFFFFFFF - offset + 1;

    if (flt == 0) {
        if (size == 8)      assignment = "\x48\x89\x85";     /* int   64->64 */
        else if (size == 4) assignment = "\x89\x85";         /* int   64->32 */
        else if (size == 2) assignment = "\x66\x89\x85";     /* int   64->16 */
        else                assignment = "\x88\x85";         /* int   64->8  */
    }
    else {
        if (size == 8) assignment = "\xF2\x0F\x11\x85"; /* float 64->64  */
        else           assignment = "\xF3\x0F\x11\x85"; /* float 64->32  */
    }

    _k_append_bytecode(env, (char *)assignment, strlen(assignment));
    _k_append_bytecode(env, (char *)&offset_signed, 4);
}

/*
 *    Generates assembly for a global assignment.
 *
 *    @param k_env_t *env           The environment to generate the assignment for.
 *    @param unsigned long offset   The offset of the variable to assign to.
 */
void _k_assemble_assignment_global(k_env_t *env, unsigned long offset) {
    /*
     *     48 89 05 00 00 00 00   mov  [rip + offset], rax
     */
    const char *assignment = "\x48\x89\x05";
    signed long offset_signed = -offset - 0xe;

    _k_append_bytecode(env, (char *)assignment, 3);
    _k_append_bytecode(env, (char *)&offset_signed, 4);
}

/*
 *    Generates assembly for a move into a register.
 *
 *    @param k_env_t *env    The environment to generate the move for.
 *    @param unsigned long   The offset of the variable to move.
 *    @param unsigned long   The size of the variable to move, in bytes.
 *    @param char            flt      Whether or not the variable is a float.
 */
void _k_assemble_move(k_env_t *env, unsigned long offset, unsigned long size, char flt) {
    /*
     *     48 8B 85 00 00 00 01   mov  reg, [rbp + offset]
     */
    const char   *move          = (const char*)0x0;
    signed long   offset_signed = 0xFFFFFFFF - offset + 1;

    if (flt == 0) {
        if (size == 8)      move = "\x48\x8B\x85";     /* int   64->64 */
        else if (size == 4) move = "\x8B\x85";         /* int   32->64 */
        else if (size == 2) move = "\x66\x8B\x85";     /* int   16->64 */
        else                move = "\x8A\x85";         /* int   8->64  */
    }
    else {
        if (size == 8) move = "\xF2\x0F\x10\x85"; /* float 64->64  */
        else           move = "\xF3\x0F\x10\x85"; /* float 32->64  */
    }

    _k_append_bytecode(env, (char *)move, strlen(move));
    _k_append_bytecode(env, (char *)&offset_signed, 4);
}

/*
 *    Generates assembly for a move from a global into a register.
 *
 *    @param k_env_t *env    The environment to generate the move for.
 *    @param unsigned long   The offset of the variable to move.
 */
void _k_assemble_move_global(k_env_t *env, unsigned long offset) {
    /*
     *     48 8B 05 00 00 00 00   mov  reg, [rip + offset]
     */
    const char *move = "\x48\x8B\x05";
    signed long offset_signed = -offset - 0xe;    

    _k_append_bytecode(env, (char *)move, 3);
    _k_append_bytecode(env, (char *)&offset_signed, 4);
}

/*
 *    Generates assembly to put integer into rax.
 *
 *    @param k_env_t *env        The environment to generate assembly for.
 *    @param long     integer    The integer to put into rax.
 */
void _k_assemble_mov_integer(k_env_t *env, long integer) {
    /*
     *    48 C7 C0 00 00 00 00    mov rax, integer
     */
    const char *put = "\x48\xC7\xC0";

    _k_append_bytecode(env, (char *)put, 3);
    _k_append_bytecode(env, (char *)&integer, 4);
}

/*
 *    Generates assembly to put float into rax.
 *
 *    @param k_env_t *env    The environment to generate assembly for.
 *    @param float    flt    The float to put into rax.
 */
void _k_assemble_mov_float(k_env_t *env, float flt) {
    /*
     *    48 C7 C0 00 00 00 00    mov  rax, flt
     *    66 0F 6E C0             movq xmm0, rax
     */
    const char *put = "\x48\xC7\xC0";
    const char *move = "\x66\x0F\x6E\xC0";

    _k_append_bytecode(env, (char *)put, 3);
    _k_append_bytecode(env, (char *)&flt, 4);
    _k_append_bytecode(env, (char *)move, 4);
}

/*
 *    Generates assembly to store rcx.
 *
 *    @param k_env_t *env    The environment to generate assembly for.
 */
void _k_assemble_store_rcx(k_env_t *env) {
    /*
     *    51          push rcx
     */
    const char *store = "\x51";

    _k_append_bytecode(env, (char *)store, 1);
}

/*
 *    Generates assembly to store rax.
 *
 *    @param k_env_t *env    The environment to generate assembly for.
 */
void _k_assemble_store_rax(k_env_t *env) {
    /*
     *    50          push rax
     */
    const char *store = "\x50";

    _k_append_bytecode(env, (char *)store, 1);
}

/*
 *    Generates assembly to load rcx.
 *
 *    @param k_env_t *env    The environment to generate assembly for.
 */
void _k_assemble_load_rcx(k_env_t *env) {
    /*
     *    59          pop rcx
     */
    const char *load = "\x59";

    _k_append_bytecode(env, (char *)load, 1);
}

/*
 *    Generates assembly to mov rax into rcx.
 *
 *    @param k_env_t *env    The environment to generate assembly for.
 */
void _k_assemble_mov_rcx_rax(k_env_t *env) {
    /*
     *    48 89 C1    mov rcx, rax
     */
    const char *put = "\x48\x89\xC1";

    _k_append_bytecode(env, (char *)put, 3);
}

/*
 *    Generates assembly to swap rax and rcx.
 *
 *    @param k_env_t *env    The environment to generate assembly for.
 */
void _k_assemble_swap_rax_rcx(k_env_t *env) {
    /*
     *    48 91   xchg rax, rcx
     */
    const char *swap = "\x48\x91";

    _k_append_bytecode(env, (char *)swap, 2);
}

/*
 *    Generates assembly to move rdx to rax.
 *
 *    @param k_env_t *env    The environment to generate assembly for.
 */
void _k_assemble_mov_rdx_rax(k_env_t *env) {
    /*
     *    48 89 D0    mov rax, rdx
     */
    const char *put = "\x48\x89\xD0";

    _k_append_bytecode(env, (char *)put, 3);
}

/*
 *    Generates assembly for an addition.
 *
 *    @param k_env_t *env    The environment to generate the addition for.
 */
void _k_assemble_addition(k_env_t *env) {
    /*
     *     48 01 C0   add  rax, rcx
     */
    const char *addition = "\x48\x01\xC8";

    _k_append_bytecode(env, (char *)addition, 3);
}

/*
 *    Generates assembly for a subtraction.
 *
 *    @param k_env_t *env    The environment to generate the subtraction for.
 */
void _k_assemble_subtraction(k_env_t *env) {
    /*
     *     48 29 C8   sub  rax, rcx
     */
    const char *subtraction = "\x48\x29\xC8";

    _k_append_bytecode(env, (char *)subtraction, 3);
}

/*
 *    Generates assembly for a multiplication.
 *
 *    @param k_env_t *env    The environment to generate the multiplication for.
 */ 
void _k_assemble_multiplication(k_env_t *env) {
    /*
     *     48 0F AF C1   imul rax, rcx
     */
    const char *multiplication = "\x48\x0F\xAF\xC1";

    _k_append_bytecode(env, (char *)multiplication, 4);
}

/*
 *    Generates assembly for a division.
 *
 *    @param k_env_t *env    The environment to generate the division for.
 */
void _k_assemble_division(k_env_t *env) {
    /*
     *     48 99      cqo
     *     48 F7 F9   idiv rcx
     */
    const char *division = "\x48\x99\x48\xF7\xF9";

    _k_append_bytecode(env, (char *)division, 5);
}

/*
 *    Generates assembly for a comparison.
 *
 *    @param k_env_t      *env     The environment to generate the comparison for.
 *    @param _k_op_type_e  cmp     The comparison to generate.
 */
void _k_assemble_comparison(k_env_t *env, _k_op_type_e cmp) {
    /*
     *     48 39 C1   cmp  rcx, rasx
     */
    const char *comparison = "\x48\x39\xC1";

    _k_append_bytecode(env, (char *)comparison, 3);

    const char *set = (const char*)0x0;

    switch (cmp) {
        case _K_OP_E:
            /*
             *     0F 94 C0   sete al
             */
            set = "\x0F\x94\xC0";
            break;
        case _K_OP_NE:
            /*
             *     0F 95 C0   setne al
             */
            set = "\x0F\x95\xC0";
            break;
        case _K_OP_L:
            /*
             *     0F 9C C0   setl al
             */
            set = "\x0F\x9C\xC0";
            break;
        case _K_OP_LE:
            /*
             *     0F 9E C0   setle al
             */
            set = "\x0F\x9E\xC0";
            break;
        case _K_CMP_G:
            /*
             *     0F 9F C0   setg al
             */
            set = "\x0F\x9F\xC0";
            break;
        case _K_CMP_GE:
            /*
             *     0F 9D C0   setge al
             */
            set = "\x0F\x9D\xC0";
            break;
    }

    _k_append_bytecode(env, (char *)set, 3);

    /*
     *     48 0F B6 C0   movzx rax, al
     */
    const char *movzx = "\x48\x0F\xB6\xC0";

    _k_append_bytecode(env, (char *)movzx, 4);
}

/*
 *    Generates assembly for a while.
 *
 *    @param k_env_t *env    The environment to generate the while for.
 * 
 *    @return unsigned long  The offset of the je offset.
 */
unsigned long _k_assemble_while(k_env_t *env) {
    /*
     *    48 83 F8 00          cmp rax, 0
     *    0F 84 00 00 00 00    je  end
     */
    const char *while_start = "\x48\x83\xF8\x00\x0F\x84\x00\x00\x00\x00";
    signed long offset_signed = 0xFFFFFFFF - env->runtime->size + 1;

    _k_append_bytecode(env, (char *)while_start, 10);

    return env->runtime->size - 4;
}

/*
 *    Generates assembly for a jump.
 *
 *    @param k_env_t *env    The environment to generate the jump for.
 *    @param char *address   The address to jump to.
 */
void _k_assemble_jump(k_env_t *env, char *address) {
    /*
     *    E9 00 00 00 00    jmp address
     */
    const char *jump = "\xE9";
    signed long offset_signed = address - (env->runtime->mem + env->runtime->size + 5);

    _k_append_bytecode(env, (char *)jump, 1);
    _k_append_bytecode(env, (char *)&offset_signed, 4);
}

/*
 *    Generates assembly for a return.
 *
 *    @param k_env_t *env    The environment to generate the return for.
 */
void _k_assemble_return(k_env_t *env) {
    /*
     *    48 89 EC    mov rsp, rbp
     *    C9          pop rbp
     *    C3          ret
     */
    const char *ret = "\x48\x89\xEC\x5D\xC3";

    _k_append_bytecode(env, (char *)ret, 5);
}

/*
 *    Prints the assembly bytestream for debugging.
 *
 *    @param k_env_t *env    The environment to print the assembly for.
 * 
 *    @return char *         The assembly bytestream.
 */
char *_k_print_assembly(k_env_t *env) {
    char *assembly = malloc(env->runtime->size * 3 + 1);

    for (unsigned long i = 0; i < env->runtime->size; i++) {
        sprintf(assembly + i * 3, "%02X ", (unsigned char)env->runtime->mem[i]);
    }

    assembly[env->runtime->size * 3] = '\0';

    return assembly;
}