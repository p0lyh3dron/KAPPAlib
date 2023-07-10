/*
 *    libk_compile.c    --    Source for KAPPA compilation
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 1, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    The KAPPA compiler is contained in this file, the stage
 *    after the parsing and lexing of the KAPPA source code.
 */
#include "libk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "builtin.h"
#include "util.h"

#include "libk_assemble.h"
#include "libk_parse.h"

unsigned long   _var_size      = 0;

char          **_locals        = (char **)0x0;
unsigned long  *_local_offsets = (unsigned long *)0x0;
unsigned long   _local_count   = 0;
unsigned long   _base_offset   = 0;

/*
 *    Deduces the size of a variable.
 *
 *    @param const char *type    The type of the variable.
 * 
 *    @return unsigned long      The size of the variable.
 */
unsigned long _k_deduce_size(const char *type) {
    for (unsigned long i = 0; i < _types_length; i++) {
        if (strcmp(type, _types[i]) == 0) {
            return _type_lengths[i];
        }
    }

    return 0;
}

/*
 *    Adds a local variable to the compiler environment.
 *
 *    @param char *name    The name of the variable.
 *
 *    @return unsigned long    The offset of the variable.
 */
unsigned long _k_add_local(const char *name) {
    for (unsigned long i = 0; i < _local_count; i++) {
        if (strcmp(_locals[i], name) == 0) {
            return _local_offsets[i];
        }
    }

    _locals        = realloc(_locals, sizeof(char *) * (_local_count + 1));
    _local_offsets = realloc(_local_offsets, sizeof(unsigned long) * (_local_count + 1));

    _locals[_local_count]          = strdup(name);
    _local_offsets[_local_count++] = _base_offset + _var_size;

    return _base_offset += _var_size;
}

/*
 *    Gets the offset of a local variable.
 *
 *    @param const char *name    The name of the variable.
 * 
 *    @return unsigned long      The offset of the variable.
 */
unsigned long _k_get_local(const char *name) {
    for (unsigned long i = 0; i < _local_count; i++) {
        if (strcmp(_locals[i], name) == 0) {
            return _local_offsets[i];
        }
    }

    return 0;
}

/*
 *    Compiles an expression.
 *
 *    @param k_env_t    *env       The environment to parse the expression in.
 *    @param const char *source    The source to parse the expression from.
 */
void _k_compile_expression(k_env_t *env, const char *source) {
    _k_token_type_e *type      = &env->cur_type;
    _k_token_t      *prev      = env->cur_token;

    do {
        prev = env->cur_token;

        if (*type == _K_TOKEN_TYPE_NUMBER) {
            /* Set LH value to integer, and advance token  */
            _k_generate_put_integer(env, atoi(_k_get_token_str(source, env->cur_token->index, env->cur_token->length)));

            _k_advance_token(env);
        }

        if (*type == _K_TOKEN_TYPE_STRING) {
            _k_advance_token(env);
            
            /* Set LH value to string.  */
        }

        if (*type == _K_TOKEN_TYPE_IDENTIFIER) {
            /* Set LH value to value pointed to by identifier.  */
            _k_generate_move(env, 'a', _k_get_local(_k_get_token_str(source, env->cur_token->index, env->cur_token->length)));

            _k_advance_token(env);
        }

        if (*type == _K_TOKEN_TYPE_NEWEXPRESSION) {
            _k_advance_token(env);
            /*
             *    If LH is an identifier, it's a function call.
             *    Otherwise, it's a parenthesized expression.
             */
            if (prev->tokenable->type == _K_TOKEN_TYPE_IDENTIFIER) {

            }
            else {
                _k_compile_expression(env, source);
                if (*type == _K_TOKEN_TYPE_ENDEXPRESSION) {
                    _k_advance_token(env);
                } else return;
            }
        }

        if (*type == _K_TOKEN_TYPE_NEWINDEX) {
            /* Array access e.g.  identifier[var1]  */
            _k_advance_token(env);

            if (prev->tokenable->type == _K_TOKEN_TYPE_IDENTIFIER) {
                _k_advance_token(env);
            } else return;
        }

        if (*type == _K_TOKEN_TYPE_OPERATOR) {
            const char *op  = _k_get_token_str(source, env->cur_token->index, env->cur_token->length);
            _k_token_t  *lh = env->cur_token - 1;
            _k_advance_token(env);

            if (strcmp(op, "=") == 0) {
                _k_compile_expression(env, source);

                /* Assembly generated should put arithmetic register into local address. */
                _k_generate_assignment(env, _k_get_local(_k_get_token_str(source, lh->index, lh->length)));
            } else if (strcmp(op, "+") == 0) {
                /* 
                 *    LH should be already in arithmetic register, 
                 *    so we'll move into another register, and then 
                 *    add to arithmetic register after compiling the next expression. 
                 */
                _k_generate_put_rax_rcx(env);
                _k_compile_expression(env, source);
                _k_generate_addition(env);
            } else if (strcmp(op, "<") == 0) {
                _k_generate_put_rax_rcx(env);
                _k_compile_expression(env, source);
                _k_generate_comparison(env, _K_CMP_L);
            }
            _k_compile_expression(env, source);
        }
    /* If it can't find anything to parse, we're probably no longer in expression land.  */
    } while (prev != env->cur_token);
}

/*
 *    Compiles a statement.
 *
 *    @param k_env_t    *env       The environment to parse the statement in.
 *    @param const char *source    The source to parse the statement from.
 */
void _k_compile_statement(k_env_t *env, const char *source) {
    _k_token_t      **tok  = &env->cur_token;
    _k_token_type_e *type  = &env->cur_type;

    _k_advance_token(env);

    do {
        if (*type == _K_TOKEN_TYPE_NEWSTATEMENT) {
            _k_compile_statement(env, source);
        }

        if (*type == _K_TOKEN_TYPE_KEYWORD) {
            if (_k_token_string_matches(*tok, "return", source) == 0) {
                _k_advance_token(env);
                _k_compile_expression(env, source);
                _k_generate_return(env);
            } else if (_k_token_string_matches(*tok, "while", source) == 0) {
                _k_advance_token(env);
                char *start = env->cur_function->source + env->cur_function->size;
                _k_compile_expression(env, source);
                char *offset = _k_generate_while(env);
                _k_compile_statement(env, source);
                _k_generate_jump(env, start);
                long int address = (env->cur_function->source + env->cur_function->size - 0x25) - start;
                memcpy(offset, &address, 4);
            }
        }

        if (*type == _K_TOKEN_TYPE_IDENTIFIER) {
            _var_size = _k_deduce_size(_k_get_token_str(source, (*tok)->index, (*tok)->length));

            _k_advance_token(env);
            
            if (*type == _K_TOKEN_TYPE_DECLARATOR) {
                _k_advance_token(env);

                _k_add_local(_k_get_token_str(source, (*tok)->index, (*tok)->length));
                
                if (*type == _K_TOKEN_TYPE_IDENTIFIER) {
                    _k_advance_token(env);
                    
                    if (*type == _K_TOKEN_TYPE_OPERATOR) {
                        _k_advance_token(env);
                        _k_compile_expression(env, source);
                    }
                }
            } else _k_revert_token(env);
        }

        _k_compile_expression(env, source);

        if (*type == _K_TOKEN_TYPE_ENDLINE) {
            _k_advance_token(env);
        } else return;
    } while (*type != _K_TOKEN_TYPE_ENDSTATEMENT);

    _k_advance_token(env);
}

/*
 *    Compiles a global declaration.
 *
 *    @param k_env_t    *env       The environment to parse the function declaration in.
 * 
 *    @return k_compile_error_t    The error code, if any.
 */
k_compile_error_t _k_compile_global_declaration(k_env_t *env, const char *source) {
    unsigned long  i       = 0;
    _k_token_type_e *type   = &env->cur_type;

    while (*type == _K_TOKEN_TYPE_ENDLINE) {
        _k_advance_token(env);
    }

    if (*type == _K_TOKEN_TYPE_IDENTIFIER) {
        _k_advance_token(env);
    } else return K_ERROR_UNEXPECTED_TOKEN;

    if (*type == _K_TOKEN_TYPE_DECLARATOR) {
        _k_advance_token(env);
    } else return K_ERROR_UNEXPECTED_TOKEN;

    if (*type == _K_TOKEN_TYPE_IDENTIFIER) {
        _k_advance_token(env);
    } else return K_ERROR_UNEXPECTED_TOKEN;

    if (*type == _K_TOKEN_TYPE_OPERATOR) {
        /* Global variable declaration with value.  */
        _k_advance_token(env);
        _k_compile_expression(env, source);

        if (*type == _K_TOKEN_TYPE_ENDLINE) {
            _k_advance_token(env);
        } else return K_ERROR_UNEXPECTED_TOKEN;
    } else if (*type == _K_TOKEN_TYPE_ENDLINE) {
        /* Global variable declaration without value.  */
        _k_advance_token(env);
    } else if (*type == _K_TOKEN_TYPE_NEWEXPRESSION) {
        /* Global function declaration.  */
        _k_advance_token(env);
        
        while (*type != _K_TOKEN_TYPE_ENDEXPRESSION) {
            if (*type == _K_TOKEN_TYPE_IDENTIFIER) {
                _k_advance_token(env);
            } else return K_ERROR_UNEXPECTED_TOKEN;

            if (*type == _K_TOKEN_TYPE_DECLARATOR) {
                _k_advance_token(env);
            } else return K_ERROR_UNEXPECTED_TOKEN;

            if (*type == _K_TOKEN_TYPE_IDENTIFIER) {
                _k_advance_token(env);
            } else return K_ERROR_UNEXPECTED_TOKEN;

            if (*type == _K_TOKEN_TYPE_SEPARATOR) {
                _k_advance_token(env);
            }
        }

        env->runtime->function_table = realloc(env->runtime->function_table, sizeof(_k_function_t) * (env->runtime->function_count + 1));

        env->cur_function = &env->runtime->function_table[env->runtime->function_count++];

        env->cur_function->name   = "TODO";
        env->cur_function->source = (char *)0x0;
        env->cur_function->size   = 0;

        _k_generate_prelude(env);

        _k_compile_statement(env, source);

        void *exec = mmap(0, env->cur_function->size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        memcpy(exec, env->cur_function->source, env->cur_function->size);

        env->cur_function->source = exec;

    } else return K_ERROR_UNEXPECTED_TOKEN;

    return K_ERROR_NONE;
}

/*
 *    Compiles a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to compile the source in.
 *    @param const char *source    The source to compile.
 */
void _k_compile(k_env_t *env, const char *source) {
    _k_create_runtime(env, source);

    env->cur_token = &env->lexer->tokens[0];
    env->cur_type  =  env->cur_token->tokenable->type;

    while (env->cur_token->tokenable->type != _K_TOKEN_TYPE_EOF) {
        k_compile_error_t ret = _k_compile_global_declaration(env, source);

        switch (ret) {
            case K_ERROR_NONE:
                break;
            case K_ERROR_UNEXPECTED_TOKEN:
                char *token = _k_get_token_str(source, env->cur_token->index, env->cur_token->length);
                env->log(_k_get_error(env, "Unexpected token %s", token));
                _k_advance_token(env);
                break;
        }
    }

    printf("%s\n", _k_print_assembly(env));
}