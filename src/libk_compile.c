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
unsigned long   _base_offset   = 0;

/*
 *     Compiles a number.
 *
 *     @param k_env_t    *env       The environment to compile the number in.
 * 
 *     @return k_compile_error_t    The error code, if any.
 */
k_compile_error_t _k_compile_number(k_env_t *env) {
    _k_assemble_mov_integer(env, atoi(env->cur_token->str));
}

/*
 *     Compiles a string.
 *
 *     @param k_env_t    *env       The environment to compile the string in.
 * 
 *     @return k_compile_error_t    The error code, if any.
 */
k_compile_error_t _k_compile_string(k_env_t *env) {
    _k_token_t *token = env->cur_token;

    /* TODO  */
    _k_assemble_mov_integer(env, 0xDEADBEEF);
}

/*
 *     Compiles an identifier.
 *
 *     @param k_env_t    *env       The environment to compile the identifier in.
 *
 *     @return k_compile_error_t    The error code, if any.
 */
k_compile_error_t _k_compile_identifier(k_env_t *env) {
    _k_variable_t *var = _k_get_var(env, env->cur_token->str);

    if (var == (_k_variable_t *)0x0) {
        env->log(_k_get_error(env, "Undefined variable or function: %s\n", env->cur_token->str));

        return K_ERROR_UNDECLARED_VARIABLE;
    }

    /* If the variable is a function, we'll compile a function call.  */
    if (var->flags & _K_VARIABLE_FLAG_FUNC) {
        _k_advance_token(env);

        /* Function call.  */
        if (env->cur_type == _K_TOKEN_TYPE_NEWEXPRESSION) {
            _k_advance_token(env);

            unsigned long param_count = 0;

            while (env->cur_type != _K_TOKEN_TYPE_ENDEXPRESSION) {
                _k_compile_expression(env);

                _k_assemble_parameter_store(env, var->offset, param_count++);

                if (env->cur_type == _K_TOKEN_TYPE_SEPARATOR) {
                    _k_advance_token(env);
                }
            }

            _k_advance_token(env);

            _k_assemble_call(env, _k_get_function(env, var->name));
        }

        /* Return the address of the function.  */
        else _k_assemble_move(env, var->offset);
    }

    /* Set value to value pointed to by identifier.  */
    else _k_assemble_move(env, var->offset);

    return K_ERROR_NONE;
}

/*
 *     Compiles a new expression.
 *
 *     @param k_env_t    *env       The environment to compile the new expression in.
 *
 *     @return k_compile_error_t    The error code, if any.
 */
k_compile_error_t _k_compile_new_expression(k_env_t *env) {
    _k_advance_token(env);

    _k_compile_expression(env);

    if (env->cur_type == _K_TOKEN_TYPE_ENDEXPRESSION) {
        _k_advance_token(env);
    } else return K_ERROR_INVALID_ENDEXPRESSION;

    return K_ERROR_NONE;
}

_k_grammar_t _expression_grammar[] = {
    { _K_TOKEN_TYPE_NUMBER, _k_compile_number },
    { _K_TOKEN_TYPE_STRING, _k_compile_string },
    { _K_TOKEN_TYPE_IDENTIFIER, _k_compile_identifier },
    { _K_TOKEN_TYPE_NEWEXPRESSION, _k_compile_new_expression },
    { _K_TOKEN_TYPE_NEWINDEX, _k_compile_new_index },
    { _K_TOKEN_TYPE_OPERATOR, _k_compile_operator },
    { _K_TOKEN_TYPE_ENDLINE, _k_compile_endline },
    { _K_TOKEN_TYPE_ENDSTATEMENT, _k_compile_endstatement },
    { _K_TOKEN_TYPE_ENDINDEX, _k_compile_endindex },
    { _K_TOKEN_TYPE_UNKNOWN, _k_compile_unknown },
}

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
 *    Adds a variable to the compiler environment.
 *
 *    @param k_env_t       *env     The environment to add the variable to.
 *    @param _k_variable_t *var     The variable to add.
 *    @param char           global  Whether or not the variable is global.
 */
void _k_add_var(k_env_t *env, _k_variable_t *var, char global) {
    if (global) {
        env->runtime->globals = realloc(env->runtime->globals, sizeof(_k_variable_t) * (env->runtime->global_count + 1));

        memcpy(&env->runtime->globals[env->runtime->global_count++], var, sizeof(_k_variable_t));
    } else {
        env->runtime->locals = realloc(env->runtime->locals, sizeof(_k_variable_t) * (env->runtime->local_count + 1));

        memcpy(&env->runtime->locals[env->runtime->local_count++], var, sizeof(_k_variable_t));
    }
}

/*
 *    Gets a variable from the compiler environment.
 *
 *    @param k_env_t    *env     The environment to get the variable from.
 *    @param const char *name    The name of the variable.
 * 
 *    @return _k_variable_t *    The variable.
 */
_k_variable_t *_k_get_var(k_env_t *env, const char *name) {
    for (unsigned long i = 0; i < env->runtime->local_count; i++) {
        if (strcmp(env->runtime->locals[i].name, name) == 0) {
            return &env->runtime->locals[i];
        }
    }

    for (unsigned long i = 0; i < env->runtime->global_count; i++) {
        if (strcmp(env->runtime->globals[i].name, name) == 0) {
            return &env->runtime->globals[i];
        }
    }

    return (void *)0x0;
}

/*
 *    Gets the address of a function.
 *
 *    @param k_env_t    *env    The environment to get the function from.
 *    @param const char *name    The name of the function.
 *
 *    @return char *      The address of the function.
 */
char *_k_get_function(k_env_t *env, const char *name) {
    /* TODO: Add built in functions.  */

    for (unsigned long i = 0; i < env->runtime->function_count; i++) {
        if (strcmp(env->runtime->function_table[i].name, name) == 0) {
            return env->runtime->function_table[i].source;
        }
    }

    return (char *)0x0;
}

/*
 *    Compiles an expression.
 *
 *    @param k_env_t    *env       The environment to parse the expression in.
 */
void _k_compile_expression(k_env_t *env) {
    _k_token_type_e *type      = &env->cur_type;
    _k_token_t      *prev      = env->cur_token;

    do {
        prev = env->cur_token;

        switch (*type) {
            case _K_TOKEN_TYPE_NUMBER:
                /* Set LH value to integer, and advance token  */
                _k_assemble_mov_integer(env, atoi(_k_get_token_str(env)));
                _k_advance_token(env);
                break;

            case _K_TOKEN_TYPE_STRING:
                _k_advance_token(env);
                /* Set LH value to string.  */
                break;

            case _K_TOKEN_TYPE_IDENTIFIER:
                _k_variable_t *var = _k_get_var(env, _k_get_token_str(env));

                if (var->flags & _K_VARIABLE_FLAG_FUNC) {
                    /* Function call.  */
                    _k_advance_token(env);

                    if (*type == _K_TOKEN_TYPE_NEWEXPRESSION) {
                        _k_advance_token(env);
                    } else return;

                    unsigned long i = 0;

                    while (*type != _K_TOKEN_TYPE_ENDEXPRESSION) {
                        _k_compile_expression(env, source);

                        _k_assemble_parameter_store(env, var->offset, i++);

                        if (*type == _K_TOKEN_TYPE_SEPARATOR) {
                            _k_advance_token(env);
                        }
                    }

                    _k_advance_token(env);

                    _k_assemble_call(env, _k_get_function(env, var->name));
                } else {
                    /* Set LH value to value pointed to by identifier.  */
                    _k_assemble_move(env, _k_get_var(env, var->name)->offset);
                    _k_advance_token(env);
                }
                break;

            case _K_TOKEN_TYPE_NEWEXPRESSION:
                _k_advance_token(env);
                
                _k_compile_expression(env, source);

                if (*type == _K_TOKEN_TYPE_ENDEXPRESSION) {
                    _k_advance_token(env);
                } else return;
                break;

            case _K_TOKEN_TYPE_NEWINDEX:
                /* Array access e.g.  identifier[var1]  */
                _k_advance_token(env);
                if (prev->tokenable->type == _K_TOKEN_TYPE_IDENTIFIER) {
                    _k_advance_token(env);
                } else {
                    return;
                }
                break;

            case _K_TOKEN_TYPE_OPERATOR:
                const char *op  = _k_get_token_str(env);
                _k_token_t *lh  = env->cur_token - 1;
                _k_advance_token(env);

                if (strcmp(op, "=") == 0) {
                    _k_compile_expression(env, source);
                    
                    /* Assembly generated should put arithmetic register into local address. */
                    _k_assemble_assignment(env, _k_get_var(env, lh->str)->offset);
                } 
                
                else if (strcmp(op, "+") == 0) {
                    /* 
                    *    LH should be already in arithmetic register, 
                    *    so we'll move into another register, and then 
                    *    add to arithmetic register after compiling the next expression. 
                    */
                    _k_assemble_mov_rcx_rax(env);
                    _k_compile_expression(env, source);
                    _k_assemble_addition(env);
                } 
                
                else if (strcmp(op, "<") == 0) {
                    _k_assemble_mov_rcx_rax(env);
                    _k_compile_expression(env, source);
                    _k_assemble_comparison(env, _K_CMP_L);
                }

                _k_compile_expression(env, source);

                break;

            default:
                /* Handle unknown token type */
                break;
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
        switch (*type) {
            case _K_TOKEN_TYPE_NEWSTATEMENT:
                _k_compile_statement(env, source);
                break;

            case _K_TOKEN_TYPE_KEYWORD:
                char *keyword = _k_get_token_str(env);

                if (strcmp(keyword, "return") == 0) {
                    _k_advance_token(env);

                    /* The final expression will already be in RAX, easy as pie.  */
                    _k_compile_expression(env, source);
                    _k_assemble_return(env);
                } 
                
                else if (strcmp(keyword, "while") == 0) {
                    _k_advance_token(env);

                    /* Hold on to the start address.  */
                    unsigned long old = env->cur_function->size;

                    /* Create while condition.  */
                    _k_compile_expression(env, source);
                    
                    /* Address to write jump into after statement.  */
                    char *offset = _k_assemble_while(env);

                    /* Write statement and jump to check condition again. */
                    _k_compile_statement(env, source);
                    _k_assemble_jump(env, env->cur_function->source + old);

                    /* Update initial condition bytecode with exit address.  */
                    long int address = (env->cur_function->size - 0x25) - old;
                    memcpy(offset, &address, 4);

                    continue;
                }

                break;

            case _K_TOKEN_TYPE_IDENTIFIER:
                /* Get size for stack allocation.  */
                char *type = _k_get_token_str(env);
                _var_size  = _k_deduce_size(type);

                _k_advance_token(env);

                if (*type == _K_TOKEN_TYPE_DECLARATOR) {
                    _k_advance_token(env);

                    /* Hold on to stack offsets.  */
                    _k_variable_t var = {
                        .name   = _k_get_token_str(env),
                        .type   = type,
                        .offset = _base_offset += _var_size,
                        .flags  = 0x0,
                        .size   = _var_size,
                    };

                    _k_add_var(env, &var, 0);

                    /* Check for variable declaration.  */
                    if (*type == _K_TOKEN_TYPE_IDENTIFIER) {
                        _k_advance_token(env);

                        if (*type == _K_TOKEN_TYPE_OPERATOR) {
                            if (strncmp(_k_get_token_str(env), "=", 2) != 0) {
                                env->log(_k_get_error(env, "Expected assignment operator, got %s", _k_get_token_str(env)));
                                return;
                            }

                            _k_advance_token(env);
                            _k_compile_expression(env, source);
                            _k_assemble_assignment(env, var.offset);
                        }
                    }
                } 
                
                /* Wasn't a declaration, prepare to compile expression.  */
                else _k_revert_token(env);

            default:
            type = &env->cur_type;
                _k_compile_expression(env, source);

                if (*type == _K_TOKEN_TYPE_ENDLINE)
                    _k_advance_token(env);
                    
                else return;

                break;
        }
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
    unsigned long    i      = 0;
    _k_token_type_e *type   = &env->cur_type;
    char            *id     = (char *)0x0;
    char            *param  = (char *)0x0;

    while (*type == _K_TOKEN_TYPE_ENDLINE)
        _k_advance_token(env);

    if (*type == _K_TOKEN_TYPE_IDENTIFIER) {
        _k_advance_token(env);
    } else return K_ERROR_UNEXPECTED_TOKEN;

    if (*type == _K_TOKEN_TYPE_DECLARATOR) {
        _k_advance_token(env);
    } else return K_ERROR_UNEXPECTED_TOKEN;

    if (*type == _K_TOKEN_TYPE_IDENTIFIER) {
        id = _k_get_token_str(env);

        _k_advance_token(env);
    } else return K_ERROR_UNEXPECTED_TOKEN;

    if (*type == _K_TOKEN_TYPE_OPERATOR) {
        /* Global variable declaration with value.  */
        _k_advance_token(env);
        _k_compile_expression(env, source);

        if (*type == _K_TOKEN_TYPE_ENDLINE) {
            _k_advance_token(env);
        } else return K_ERROR_UNEXPECTED_TOKEN;

    } 
    
    else if (*type == _K_TOKEN_TYPE_ENDLINE) {
        /* Global variable declaration without value.  */
        _k_advance_token(env);
    } 
    
    else if (*type == _K_TOKEN_TYPE_NEWEXPRESSION) {
        /* Global function declaration.  */
        _k_advance_token(env);
        
        while (*type != _K_TOKEN_TYPE_ENDEXPRESSION) {
            if (*type == _K_TOKEN_TYPE_IDENTIFIER) {
                _var_size = _k_deduce_size(_k_get_token_str(env));

                _k_advance_token(env);
            } else return K_ERROR_UNEXPECTED_TOKEN;

            if (*type == _K_TOKEN_TYPE_DECLARATOR) {
                _k_advance_token(env);
            } else return K_ERROR_UNEXPECTED_TOKEN;

            if (*type == _K_TOKEN_TYPE_IDENTIFIER) {
                _k_variable_t var = {
                    .name   = _k_get_token_str(env),
                    .type   = "param",
                    .offset = _base_offset += _var_size,
                    .flags  = 0x0,
                    .size   = _var_size,
                };

                _k_add_var(env, &var, 0);
                
                _k_advance_token(env);
            } else return K_ERROR_UNEXPECTED_TOKEN;

            if (*type == _K_TOKEN_TYPE_SEPARATOR) {
                _k_advance_token(env);
            }
        }

        _k_variable_t var = {
            .name   = id,
            .type   = "function",
            .offset = 0x0,
            .flags  = _K_VARIABLE_FLAG_FUNC,
            .size   = 0x0,
        };

        _k_advance_token(env);

        _k_add_var(env, &var, 1);

        env->runtime->function_table = realloc(env->runtime->function_table, sizeof(_k_function_t) * (env->runtime->function_count + 1));

        env->cur_function = &env->runtime->function_table[env->runtime->function_count++];

        env->cur_function->name   = id;
        env->cur_function->source = (char *)0x0;
        env->cur_function->size   = 0;

        _k_assemble_prelude(env);

        for (i = 0; i < env->runtime->local_count; i++) {
            _k_variable_t *param = &env->runtime->locals[i];

            _k_assemble_parameter_store(env, param->offset, i);
        }

        _k_compile_statement(env, source);

        void *exec = mmap(0, env->cur_function->size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        memcpy(exec, env->cur_function->source, env->cur_function->size);

        env->cur_function->source = exec;

        if (env->runtime->local_count > 0) {
            free(env->runtime->locals);
        }

        env->runtime->locals = (_k_function_t *)0x0;

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
                char *token = _k_get_token_str(env);
                env->log(_k_get_error(env, "Unexpected token %s", token));
                _k_advance_token(env);
                break;
        }
    }

    printf("%s\n", _k_print_assembly(env));
}