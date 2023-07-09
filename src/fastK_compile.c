/*
 *    fastK_compile.c    --    Source for KAPPA compilation
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 1, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    The KAPPA compiler is contained in this file, the stage
 *    after the parsing and lexing of the KAPPA source code.
 */
#include "fastK.h"

#include <sys/mman.h>

#include "builtin.h"
#include "util.h"

#include "fastK_assemble.h"

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
unsigned long k_deduce_size(const char *type) {
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
unsigned long k_add_local(const char *name) {
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
unsigned long k_get_local(const char *name) {
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
void k_compile_expression(k_env_t *env, const char *source) {
    k_token_type_e *type      = &env->cur_type;
    k_token_t      *prev      = env->cur_token;

    do {
        prev = env->cur_token;

        if (*type == K_TOKEN_TYPE_NUMBER) {
            /* Set LH value to integer, and advance token  */
            k_generate_put_integer(env, atoi(k_get_token_str(source, env->cur_token->index, env->cur_token->length)));

            k_advance_token(env);
        }

        if (*type == K_TOKEN_TYPE_STRING) {
            k_advance_token(env);
            
            /* Set LH value to string.  */
        }

        if (*type == K_TOKEN_TYPE_IDENTIFIER) {
            /* Set LH value to value pointed to by identifier.  */
            k_generate_move(env, 'a', k_get_local(k_get_token_str(source, env->cur_token->index, env->cur_token->length)));

            k_advance_token(env);
        }

        if (*type == K_TOKEN_TYPE_NEWEXPRESSION) {
            k_advance_token(env);
            /*
             *    If LH is an identifier, it's a function call.
             *    Otherwise, it's a parenthesized expression.
             */
            if (prev->tokenable->type == K_TOKEN_TYPE_IDENTIFIER) {

            }
            else {
                k_compile_expression(env, source);
                if (*type == K_TOKEN_TYPE_ENDEXPRESSION) {
                    k_advance_token(env);
                } else return;
            }
        }

        if (*type == K_TOKEN_TYPE_NEWINDEX) {
            /* Array access e.g.  identifier[var1]  */
            k_advance_token(env);

            if (prev->tokenable->type == K_TOKEN_TYPE_IDENTIFIER) {
                k_advance_token(env);
            } else return;
        }

        if (*type == K_TOKEN_TYPE_OPERATOR) {
            const char *op  = k_get_token_str(source, env->cur_token->index, env->cur_token->length);
            k_token_t  *lh = env->cur_token - 1;
            k_advance_token(env);

            if (strcmp(op, "=") == 0) {
                k_compile_expression(env, source);

                /* Assembly generated should put arithmetic register into local address. */
                k_generate_assignment(env, k_get_local(k_get_token_str(source, lh->index, lh->length)));
            } else if (strcmp(op, "+") == 0) {
                /* 
                 *    LH should be already in arithmetic register, 
                 *    so we'll move into another register, and then 
                 *    add to arithmetic register after compiling the next expression. 
                 */
                k_generate_put_rax_rcx(env);
                k_compile_expression(env, source);
                k_generate_addition(env);
            } else if (strcmp(op, "<") == 0) {
                k_generate_put_rax_rcx(env);
                k_compile_expression(env, source);
                k_generate_comparison(env, K_CMP_L);
            }
            k_compile_expression(env, source);
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
void k_compile_statement(k_env_t *env, const char *source) {
    k_token_t      **tok  = &env->cur_token;
    k_token_type_e *type  = &env->cur_type;

    k_advance_token(env);

    do {
        if (*type == K_TOKEN_TYPE_NEWSTATEMENT) {
            k_compile_statement(env, source);
        }

        if (*type == K_TOKEN_TYPE_KEYWORD) {
            if (k_token_string_matches(*tok, "return", source) == 0) {
                k_advance_token(env);
                k_compile_expression(env, source);
                k_generate_return(env);
            } else if (k_token_string_matches(*tok, "while", source) == 0) {
                k_advance_token(env);
                char *start = env->cur_function->source + env->cur_function->size;
                k_compile_expression(env, source);
                char *offset = k_generate_while(env);
                k_compile_statement(env, source);
                k_generate_jump(env, start);
                char *address = (env->cur_function->source + env->cur_function->size - 0x25) - start;
                memcpy(offset, &address, 4);
            }
        }

        if (*type == K_TOKEN_TYPE_IDENTIFIER) {
            _var_size = k_deduce_size(k_get_token_str(source, (*tok)->index, (*tok)->length));

            k_advance_token(env);
            
            if (*type == K_TOKEN_TYPE_DECLARATOR) {
                k_advance_token(env);

                k_add_local(k_get_token_str(source, (*tok)->index, (*tok)->length));
                
                if (*type == K_TOKEN_TYPE_IDENTIFIER) {
                    k_advance_token(env);
                    
                    if (*type == K_TOKEN_TYPE_OPERATOR) {
                        k_advance_token(env);
                        k_compile_expression(env, source);
                    }
                }
            } else k_revert_token(env);
        }

        k_compile_expression(env, source);

        if (*type == K_TOKEN_TYPE_ENDLINE) {
            k_advance_token(env);
        } else return;
    } while (*type != K_TOKEN_TYPE_ENDSTATEMENT);

    k_advance_token(env);
}

/*
 *    Compiles a global declaration.
 *
 *    @param k_env_t    *env       The environment to parse the function declaration in.
 * 
 *    @return k_compile_error_t    The error code, if any.
 */
k_compile_error_t k_compile_global_declaration(k_env_t *env, const char *source) {
    unsigned long  i       = 0;
    k_token_type_e *type   = &env->cur_type;

    while (*type == K_TOKEN_TYPE_ENDLINE) {
        k_advance_token(env);
    }

    if (*type == K_TOKEN_TYPE_IDENTIFIER) {
        k_advance_token(env);
    } else return K_ERROR_UNEXPECTED_TOKEN;

    if (*type == K_TOKEN_TYPE_DECLARATOR) {
        k_advance_token(env);
    } else return K_ERROR_UNEXPECTED_TOKEN;

    if (*type == K_TOKEN_TYPE_IDENTIFIER) {
        k_advance_token(env);
    } else return K_ERROR_UNEXPECTED_TOKEN;

    if (*type == K_TOKEN_TYPE_OPERATOR) {
        /* Global variable declaration with value.  */
        k_advance_token(env);
        k_compile_expression(env, source);

        if (*type == K_TOKEN_TYPE_ENDLINE) {
            k_advance_token(env);
        } else return K_ERROR_UNEXPECTED_TOKEN;
    } else if (*type == K_TOKEN_TYPE_ENDLINE) {
        /* Global variable declaration without value.  */
        k_advance_token(env);
    } else if (*type == K_TOKEN_TYPE_NEWEXPRESSION) {
        /* Global function declaration.  */
        k_advance_token(env);
        
        while (*type != K_TOKEN_TYPE_ENDEXPRESSION) {
            if (*type == K_TOKEN_TYPE_IDENTIFIER) {
                k_advance_token(env);
            } else return K_ERROR_UNEXPECTED_TOKEN;

            if (*type == K_TOKEN_TYPE_DECLARATOR) {
                k_advance_token(env);
            } else return K_ERROR_UNEXPECTED_TOKEN;

            if (*type == K_TOKEN_TYPE_IDENTIFIER) {
                k_advance_token(env);
            } else return K_ERROR_UNEXPECTED_TOKEN;

            if (*type == K_TOKEN_TYPE_SEPARATOR) {
                k_advance_token(env);
            }
        }

        env->runtime->function_table = realloc(env->runtime->function_table, sizeof(k_function_t) * (env->runtime->function_count + 1));

        env->cur_function = &env->runtime->function_table[env->runtime->function_count++];

        env->cur_function->name   = "TODO";
        env->cur_function->source = (char *)0x0;
        env->cur_function->size   = 0;

        k_generate_prelude(env);

        k_compile_statement(env, source);

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
void k_compile(k_env_t *env, const char *source) {
    k_create_runtime(env, source);

    env->cur_token = &env->lexer->tokens[0];
    env->cur_type  =  env->cur_token->tokenable->type;

    while (env->cur_token->tokenable->type != K_TOKEN_TYPE_EOF) {
        k_compile_error_t ret = k_compile_global_declaration(env, source);

        switch (ret) {
            case K_ERROR_NONE:
                break;
            case K_ERROR_UNEXPECTED_TOKEN:
                char *token = k_get_token_str(source, env->cur_token->index, env->cur_token->length);
                env->log(k_get_error(env, "Unexpected token %s", token));
                k_advance_token(env);
                break;
        }
    }

    printf("%s\n", k_print_assembly(env));
}