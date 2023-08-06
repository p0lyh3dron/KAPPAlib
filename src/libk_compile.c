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

k_build_error_t _k_compile_expression(k_env_t*);
k_build_error_t _k_compile_statement(k_env_t*);

#define _K_COMPILE_EXP(env)   { k_build_error_t __ret = _k_compile_expression(env); \
                                if (__ret != K_ERROR_NONE) return __ret; }

#define _K_COMPILE_STMT(env)  { k_build_error_t __ret = _k_compile_statement(env); \
                                if (__ret != K_ERROR_NONE) return __ret; }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "builtin.h"
#include "util.h"

#include "libk_assemble.h"
#include "libk_operator.h"
#include "libk_parse.h"

unsigned long   _var_size       = 0;
unsigned long   _local_offset   = 0;

_k_operator_t _operator_table[] = {
    {"+", _K_OP_ADD},
    {"-", _K_OP_SUB},
    {"*", _K_OP_MUL},
    {"/", _K_OP_DIV},
    {"%", _K_OP_MOD},
    {"<", _K_OP_L},
    {"<=", _K_OP_LE},
    {">", _K_OP_G},
    {">=", _K_OP_GE},
    {"==", _K_OP_E},
    {"!=", _K_OP_NE},
    {"&&", _K_OP_AND},
    {"||", _K_OP_OR},
    {"!", _K_OP_NOT},
    {"~", _K_OP_NEG},
    {"=", _K_OP_ASSIGN},
};

/*
 *    Deduces the size of a variable.
 *
 *    @param const char *type    The type of the variable.
 * 
 *    @return unsigned long      The size of the variable.
 */
unsigned long _k_deduce_size(const char *type) {
    for (unsigned long i = 0; i < _types_length; i++) {
        if (strcmp(type, _types[i].id) == 0) {
            return _types[i].size;
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
    /* In the future, check for typedefs to other float types.  */
    if (strcmp(var->type, "f32") == 0 || strcmp(var->type, "f64") == 0)
        var->flags |= _K_VARIABLE_FLAG_FLOAT;

    if (global) {
        env->runtime->globals = realloc(env->runtime->globals, sizeof(_k_variable_t) * (env->runtime->global_count + 1));

        memcpy(&env->runtime->globals[env->runtime->global_count++], var, sizeof(_k_variable_t));
    } else {
        env->runtime->locals = realloc(env->runtime->locals, sizeof(_k_variable_t) * (env->runtime->local_count + 1));

        memcpy(&env->runtime->locals[env->runtime->local_count++], var, sizeof(_k_variable_t));
    }
}

/*
 *    Adds an operation to the operation stack.
 *
 *    @param k_env_t    *env       The environment to add the operation to.
 *    @param _k_op_type_e type     The type of the operation.
 *    @param _k_token_t *lh        The left hand side of the operation.
 *    @param _k_token_t *rh        The right hand side of the operation.
 */
void _k_add_operation(k_env_t *env, _k_op_type_e type, _k_token_t *lh, _k_token_t *rh) {
    env->runtime->operations = realloc(env->runtime->operations, sizeof(_k_operation_t) * (env->runtime->operation_count + 1));

    _k_operation_t *op = &env->runtime->operations[env->runtime->operation_count++];

    op->type = type;
    op->lh = lh;
    op->rh = rh;
}

/*
 *    Adds a parameter to the function info.
 *
 *    @param k_env_t       *env       The environment to add the parameter to.
 *    @param _k_variable_t *var       The parameter to add.
 *    @param _k_function_t *function  The function to add the parameter to.
 */
void _k_add_parameter(k_env_t *env, _k_variable_t *var, _k_function_t *function) {
    function->parameters = realloc(function->parameters, sizeof(_k_variable_t) * (function->parameter_count + 1));

    memcpy(&function->parameters[function->parameter_count++], var, sizeof(_k_variable_t));
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
 *    Places a global variable into the data section.
 *
 *    @param k_env_t       *env     The environment to place the variable in.
 *    @param _k_variable_t *var     The variable to place.
 *    @param char          *data    The data to place.
 */
void _k_place_global(k_env_t *env, _k_variable_t *var, char *data) {
    env->runtime->mem = realloc(env->runtime->mem, env->runtime->size + var->size);

    if (data != (char *)0x0)
        memcpy(env->runtime->mem + env->runtime->size, data, var->size);

    var->offset = env->runtime->size;

    env->runtime->size += var->size;

    _k_add_var(env, var, 1);
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
            return env->runtime->mem + (long)env->runtime->function_table[i].source;
        }
    }

    return (char *)0x0;
}

/*
 *     Compiles a number.
 *
 *     @param k_env_t    *env       The environment to compile the number in.
 * 
 *     @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_number(k_env_t *env) {
    if (strchr(env->cur_token->str, '.') != (char *)0x0) {
        env->runtime->cur_local.is_float = 1;
        env->runtime->cur_local.size     = 8;

        _k_assemble_mov_float(env, atof(env->cur_token->str));
    }

    else {
        env->runtime->cur_local.is_float = 0;
        env->runtime->cur_local.size     = 8;

        _k_assemble_mov_integer(env, atoi(env->cur_token->str));
    }

    _k_advance_token(env);

    return K_ERROR_NONE;
}

/*
 *     Compiles a string.
 *
 *     @param k_env_t    *env       The environment to compile the string in.
 * 
 *     @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_string(k_env_t *env) {
    _k_token_t *token = env->cur_token;

    /* TODO  */
    _k_assemble_mov_integer(env, 0xDEADBEEF);

    _k_advance_token(env);

    return K_ERROR_NONE;
}

/*
 *     Compiles an identifier.
 *
 *     @param k_env_t    *env       The environment to compile the identifier in.
 *
 *     @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_identifier(k_env_t *env) {
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
            _k_assemble_store_rcx(env);

            unsigned long param_count = 0;

            while (env->cur_type != _K_TOKEN_TYPE_ENDEXPRESSION) {
                _K_COMPILE_EXP(env);

                _k_assemble_parameter_load(env, param_count++, var->size, var->flags & _K_VARIABLE_FLAG_FLOAT);

                if (env->cur_type == _K_TOKEN_TYPE_SEPARATOR) {
                    _k_advance_token(env);
                }
            }

            _k_assemble_call(env, _k_get_function(env, var->name));
            _k_assemble_load_rcx(env);
        }

        /* Return the address of the function.  */
        else _k_assemble_move(env, var->offset, var->size, var->flags & _K_VARIABLE_FLAG_FLOAT);
    }

    /* Set value to value pointed to by identifier.  */
    else {
        if (var->flags & _K_VARIABLE_FLAG_GLOBAL) 
            _k_assemble_move_global(env, env->runtime->size - var->offset);

        else _k_assemble_move(env, var->offset, var->size, var->flags & _K_VARIABLE_FLAG_FLOAT);
    }

    _k_advance_token(env);

    return K_ERROR_NONE;
}

/*
 *     Compiles a new expression.
 *
 *     @param k_env_t    *env       The environment to compile the new expression in.
 *
 *     @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_new_expression(k_env_t *env) {
    _k_advance_token(env);

    k_build_error_t ret = _k_compile_expression(env);

    if (ret != K_ERROR_NONE) return ret;

    if (env->cur_type != _K_TOKEN_TYPE_ENDEXPRESSION) {
        env->log(_k_get_error(env, "Expected end of expression, got %s\n", env->cur_token->str));

        return K_ERROR_INVALID_ENDEXPRESSION;
    }

    _k_advance_token(env);

    return ret;
}

/*
 *    Compiles a token.
 *
 *    @param k_env_t    *env       The environment to compile the token in.
 *    @param _k_token_t *token     The token to compile.
 * 
 *    @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_token(k_env_t *env, _k_token_t *token) {
    _k_token_t *old = env->cur_token;

    env->cur_token = token;
    env->cur_type  = env->cur_token->tokenable->type;

    k_build_error_t ret = K_ERROR_NONE;

    switch (token->tokenable->type) {
        case _K_TOKEN_TYPE_NUMBER:
            ret = _k_compile_number(env);
            break;

        case _K_TOKEN_TYPE_STRING:
            ret = _k_compile_string(env);
            break;

        case _K_TOKEN_TYPE_IDENTIFIER:
            ret = _k_compile_identifier(env);
            break;

        case _K_TOKEN_TYPE_NEWEXPRESSION:
            _k_advance_token(env);

            ret = _k_compile_expression(env);

            if (ret != K_ERROR_NONE) {
                return ret;
            }

            if (env->cur_type != _K_TOKEN_TYPE_ENDEXPRESSION) {
                env->log(_k_get_error(env, "Expected end of expression, got %s\n", env->cur_token->str));

                return K_ERROR_INVALID_ENDEXPRESSION;
            }
            break;

        default:
            env->log(_k_get_error(env, "Unexpected token in operation: %s\n", env->cur_token->str));

            ret = K_ERROR_UNEXPECTED_TOKEN;
    }

    env->cur_token = old;
    env->cur_type  = env->cur_token->tokenable->type;

    return ret;
}

/*
 *    Compiles the operator prelude.
 *
 *    @param k_env_t    *env       The environment to compile the operator prelude in.
 *    @param _k_token_t *lh        The left hand side of the operator.
 *    @param _k_token_t *rh        The right hand side of the operator.
 * 
 *    @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_operator_prelude(k_env_t *env, _k_token_t *lh, _k_token_t *rh) {
    if (lh == (_k_token_t *)0x0) {
        _k_assemble_load_rcx(env);
        _k_assemble_swap_rax_rcx(env);
    }

    else _k_compile_token(env, lh);

    env->runtime->aux_local = env->runtime->cur_local;

    _k_assemble_mov_rcx_rax(env);

    if (rh == (_k_token_t *)0x0) {
        _k_assemble_load_rcx(env);
        _k_assemble_swap_rax_rcx(env);
    }

    else _k_compile_token(env, rh);

    return K_ERROR_NONE;
}

/*
 *    Compiles an operator postlude.
 *
 *    @param k_env_t       *env       The environment to compile the operator postlude in.
 *    @param unsigned long  index     The index of the operator.
 * 
 *    @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_operator_postlude(k_env_t *env, unsigned long index) {
    _k_assemble_store_rax(env);

    if (index > 0)
        env->runtime->operations[index-1].rh = (_k_token_t *)0x0;
    
    if (index < env->runtime->operation_count - 1)
        env->runtime->operations[index+1].lh = (_k_token_t *)0x0;

    return K_ERROR_NONE;
}

/*
 *    Compiles an operator.
 *
 *    @param k_env_t    *env       The environment to compile the operator in.
 *
 *    @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_operator(k_env_t *env) {
    _k_op_type_e type = _K_TOKEN_TYPE_UNKNOWN;
    _k_token_t  *lh   = (_k_token_t *)0x0;
    _k_token_t  *rh   = (_k_token_t *)0x0;

    for (unsigned long j = 0; j <= 3; ++j) {
        for (unsigned long i = 0; i < env->runtime->operation_count; ++i) {
            type = env->runtime->operations[i].type;
            lh   = env->runtime->operations[i].lh;
            rh   = env->runtime->operations[i].rh;

            if (type == _K_OP_ASSIGN && j == 3) {
                if (rh == (_k_token_t *)0x0) {
                    _k_assemble_load_rcx(env);
                    _k_assemble_swap_rax_rcx(env);
                }

                else _k_compile_token(env, rh);

                _k_variable_t *var = _k_get_var(env, lh->str);

                /* Assembly generated should put arithmetic register into local address. */
                if (var->flags & _K_VARIABLE_FLAG_GLOBAL)
                    _k_assemble_assignment_global(env, env->runtime->size - var->offset);

                else _k_assemble_assignment(env, var->offset, var->size, var->flags & _K_VARIABLE_FLAG_FLOAT);

                break;
            }

            else {
                for (unsigned long k = 0; k < _op_list_size; ++k) {
                    if (_op_list[k] == type && _op_hierarchy_list[k] == j) {
                        _k_compile_operator_prelude(env, lh, rh);

                        k_build_error_t ret = _op_compile_list[k](env, type);

                        if (ret != K_ERROR_NONE) return ret;

                        _k_compile_operator_postlude(env, i);

                        break;
                    }
                }
            }
        }
    }

    return K_ERROR_NONE;
}

_k_grammar_t _expression_grammar[] = {
    {_K_TOKEN_TYPE_NUMBER, _k_compile_number},
    {_K_TOKEN_TYPE_STRING, _k_compile_string},
    {_K_TOKEN_TYPE_IDENTIFIER, _k_compile_identifier},
    {_K_TOKEN_TYPE_NEWEXPRESSION, _k_compile_new_expression},
};

/*
 *    Compiles an expression.
 *
 *    @param k_env_t    *env       The environment to parse the expression in.
 *
 *    @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_expression(k_env_t *env) {
    _k_token_type_e *type      = &env->cur_type;
    _k_token_t      *prev      = (_k_token_t *)0x0;
    _k_token_t      *lh        = (_k_token_t *)0x0;
    _k_token_t      *rh        = (_k_token_t *)0x0;

    _k_operation_t *old       = env->runtime->operations;
    unsigned long   old_count = env->runtime->operation_count;

    env->runtime->operations      = (_k_operation_t *)0x0;
    env->runtime->operation_count = 0;

    do {
        prev = env->cur_token;

        /* Unary operator.  */
        if (*type == _K_TOKEN_TYPE_OPERATOR) {
            _k_advance_token(env);
        }

        lh = env->cur_token;

        if (*type != _K_TOKEN_TYPE_OPERATOR) {
            /* Iterate through the grammar table and try to find a match.  */
            for (unsigned long i = 0; i < ARRAY_SIZE(_expression_grammar); i++) {
                if (lh->tokenable->type == _expression_grammar[i].type) {
                    k_build_error_t ret = _expression_grammar[i].compile(env);

                    if (ret != K_ERROR_NONE) return ret;

                    break;
                }
            }
        }

        for (unsigned long i = 0; i < ARRAY_SIZE(_operator_table); i++) {
            if (strcmp(env->cur_token->str, _operator_table[i].operator) == 0) {
                _k_advance_token(env);

                rh = env->cur_token;

                _k_add_operation(env, _operator_table[i].type, lh, rh);

                break;
            }
        }
    /* If it can't find anything to parse, we're probably no longer in expression land.  */
    } while (prev != env->cur_token);

    _k_compile_operator(env);

    free(env->runtime->operations);

    env->runtime->operations      = old;
    env->runtime->operation_count = old_count;

    return K_ERROR_NONE;
}

/*
 *    Compiles a new statement.
 *
 *    @param k_env_t    *env       The environment to compile the new statement in.
 * 
 *    @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_new_statement(k_env_t *env) {
    _k_advance_token(env);

    _K_COMPILE_STMT(env);

    return K_ERROR_NONE;
}

/*
 *    Compiles a keyword.
 *
 *    @param k_env_t    *env       The environment to compile the keyword in.
 * 
 *    @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_keyword(k_env_t *env) {
    char *keyword = _k_get_token_str(env);

    _k_advance_token(env);

    if (strcmp(keyword, "if") == 0) {
        /* Hold on to the start address.  */
        unsigned long old = env->runtime->size;

        /* Create while condition.  */
        _K_COMPILE_EXP(env);

        unsigned long condition = env->runtime->size;
        
        /* Address to write jump into after statement.  */
        unsigned long offset = _k_assemble_while(env);

        /* Write statement and jump to check condition again. */
        _K_COMPILE_STMT(env);

        /* Update initial condition bytecode with exit address.  */
        long int address = (env->runtime->size - condition) - 0xa;
        memcpy(env->runtime->mem + offset, &address, 4);
    }

    else if (strcmp(keyword, "else") == 0) {

    }

    else if (strcmp(keyword, "while") == 0) {
        /* Hold on to the start address.  */
        unsigned long old = env->runtime->size;

        /* Create while condition.  */
        _K_COMPILE_EXP(env);

        unsigned long condition = env->runtime->size;
        
        /* Address to write jump into after statement.  */
        unsigned long offset = _k_assemble_while(env);

        /* Write statement and jump to check condition again. */
        _K_COMPILE_STMT(env);
        _k_assemble_jump(env, env->runtime->mem + old);

        /* Update initial condition bytecode with exit address.  */
        long int address = (env->runtime->size - condition) - 0xa;
        memcpy(env->runtime->mem + offset, &address, 4);
    }

    else if (strcmp(keyword, "return") == 0) {
        /* The final expression will already be in RAX, easy as pie.  */
        _K_COMPILE_EXP(env);
        _k_assemble_return(env);

        if (env->cur_type == _K_TOKEN_TYPE_ENDLINE) {
            _k_advance_token(env);
        }

        else {
            env->log(_k_get_error(env, "Expected endline after return statement, got %s", _k_get_token_str(env)));

            _k_advance_token(env);

            return K_ERROR_UNEXPECTED_TOKEN;
        }
    }

    return K_ERROR_NONE;
}

/*
 *    Compiles a declaration.
 *
 *    @param k_env_t    *env       The environment to compile the declaration in.
 * 
 *    @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_declaration(k_env_t *env) {
    char          *type = _k_get_token_str(env);
    unsigned long  size = _k_deduce_size(type);

    _k_advance_token(env);

    if (env->cur_type != _K_TOKEN_TYPE_DECLARATOR) {
        _k_revert_token(env);

        /* If there is no declaration, then it must be an expression.  */
        _K_COMPILE_EXP(env);

        return K_ERROR_NONE;
    }

    /* Declaration.  */
    _k_advance_token(env);

    if (env->cur_type != _K_TOKEN_TYPE_IDENTIFIER) {
        env->log(_k_get_error(env, "Expected identifier after declarator, got %s", _k_get_token_str(env)));

        return K_ERROR_INVALID_DECLARATION;
    }

    _k_variable_t var = {
        .name   = _k_get_token_str(env),
        .type   = type,
        .offset = _local_offset += size,
        .flags  = 0x0,
        .size   = size,
    };

    _k_add_var(env, &var, 0);
    _k_advance_token(env);

    /* Check for variable definition.  */
    if (env->cur_type == _K_TOKEN_TYPE_OPERATOR &&
        strcmp(_k_get_token_str(env), "=") == 0) {
        _k_advance_token(env);
        _K_COMPILE_EXP(env);
        _k_assemble_assignment(env, var.offset, var.size, var.flags & _K_VARIABLE_FLAG_FLOAT);
    }

    else if (env->cur_type != _K_TOKEN_TYPE_ENDLINE) {
        env->log(_k_get_error(env, "Expected assignment operator, got %s", _k_get_token_str(env)));

        _k_advance_token(env);

        return K_ERROR_JUNK_AFTER_DECLARATION;
    }

    return K_ERROR_NONE;
}

/*
 *    Compiles an endline.
 *
 *    @param k_env_t    *env       The environment to compile the endline in.
 * 
 *    @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_endline(k_env_t *env) {
    _k_advance_token(env);

    return K_ERROR_NONE;
}

_k_grammar_t _statement_grammar[] = {
    {_K_TOKEN_TYPE_NEWSTATEMENT, _k_compile_new_statement},
    {_K_TOKEN_TYPE_KEYWORD, _k_compile_keyword},
    {_K_TOKEN_TYPE_IDENTIFIER, _k_compile_declaration},
    {_K_TOKEN_TYPE_ENDLINE, _k_compile_endline},
    {_K_TOKEN_TYPE_NEWEXPRESSION, _k_compile_expression},
};

/*
 *    Compiles a statement.
 *
 *    @param k_env_t    *env       The environment to parse the statement in.
 * 
 *    @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_statement(k_env_t *env) {
    _k_token_t      **tok   = &env->cur_token;
    _k_token_type_e *type   = &env->cur_type;
    unsigned long    parsed = 0;

    _k_advance_token(env);

    do {
        parsed = 0;

        /* Iterate through the grammar table and try to find a match.  */
        for (unsigned long i = 0; i < ARRAY_SIZE(_statement_grammar); i++) {
            if (*type == _statement_grammar[i].type) {
                parsed = 1;

                k_build_error_t ret = _statement_grammar[i].compile(env);

                if (ret != K_ERROR_NONE) return ret;

                break;
            }
        }

        /* If we've hit junk, we'll return an error.  */
        if (parsed == 0) {
            env->log(_k_get_error(env, "Invalid statement starter: %s", _k_get_token_str(env)));
            return K_ERROR_UNEXPECTED_TOKEN;
        }
    } while (*type != _K_TOKEN_TYPE_ENDSTATEMENT);

    _k_advance_token(env);

    return K_ERROR_NONE;
}

/*
 *    Prepares the compiler environment for function compilation.
 *
 *    @param k_env_t    *env       The environment to prepare.
 *    @param char       *id        The name of the function.
 * 
 *    @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_prepare_function_compile(k_env_t *env, char *id) {
    env->runtime->function_table = realloc(env->runtime->function_table, sizeof(_k_function_t) * (env->runtime->function_count + 1));

    env->cur_function = &env->runtime->function_table[env->runtime->function_count++];

    env->cur_function->name   = id;
    env->cur_function->source = env->runtime->size;
    env->cur_function->size   = 0;

    env->cur_function->parameters      = (_k_variable_t *)0x0;
    env->cur_function->parameter_count = 0;

    env->cur_function->flags = 0;

    return K_ERROR_NONE;
}

/*
 *    Compiles a function declaration.
 *
 *    @param k_env_t    *env       The environment to parse the function declaration in.
 *
 *    @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_function_declaration(k_env_t *env) {
    _k_advance_token(env);

    while (env->cur_type != _K_TOKEN_TYPE_ENDEXPRESSION) {
        if (env->cur_type != _K_TOKEN_TYPE_IDENTIFIER) {
            env->log(_k_get_error(env, "Expected identifier, got %s", _k_get_token_str(env)));
            return K_ERROR_UNEXPECTED_TOKEN;
        }

        char          *type       = _k_get_token_str(env);
        unsigned long  param_size = _k_deduce_size(_k_get_token_str(env));

        _k_advance_token(env);

        if (env->cur_type != _K_TOKEN_TYPE_DECLARATOR) {
            env->log(_k_get_error(env, "Expected declarator, got %s", _k_get_token_str(env)));
            return K_ERROR_UNEXPECTED_TOKEN;
        }

        _k_advance_token(env);

        if (env->cur_type != _K_TOKEN_TYPE_IDENTIFIER) {
            env->log(_k_get_error(env, "Expected identifier, got %s", _k_get_token_str(env)));
            return K_ERROR_UNEXPECTED_TOKEN;
        }

        _k_variable_t var = {
            .name   = _k_get_token_str(env),
            .type   = type,
            .offset = _local_offset += param_size,
            .flags  = 0x0,
            .size   = param_size,
        };

        _k_advance_token(env);

        if (env->cur_type == _K_TOKEN_TYPE_SEPARATOR) {
            _k_advance_token(env);
        }

        _k_add_var(env, &var, 0);
        _k_add_parameter(env, &var, env->cur_function);
    }

    _k_advance_token(env);

    _k_assemble_prelude(env);

    unsigned long old = env->runtime->size;

    for (unsigned long i = 0; i < env->runtime->local_count; i++) {
        _k_variable_t *param = &env->runtime->locals[i];

        _k_assemble_parameter_store(env, param->offset, i, param->size, param->flags & _K_VARIABLE_FLAG_FLOAT);
    }

    _K_COMPILE_STMT(env);

    _k_assemble_allocate(env, _local_offset, old);

    if (env->runtime->local_count > 0) {
        free(env->runtime->locals);
    }

    env->runtime->locals      = (_k_function_t *)0x0;
    env->runtime->local_count = 0;

    _local_offset = 0;

    return K_ERROR_NONE;
}

/*
 *    Compiles a global declaration.
 *
 *    @param k_env_t    *env       The environment to parse the global declaration in.
 * 
 *    @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_global_declaration(k_env_t *env) {
    /* Get info about return type.  */
    char *type = _k_get_token_str(env);

    _k_advance_token(env);

    if (env->cur_type != _K_TOKEN_TYPE_DECLARATOR) {
        env->log(_k_get_error(env, "Expected declarator after type, got %s", _k_get_token_str(env)));
        return K_ERROR_INVALID_DECLARATION;
    }

    _k_advance_token(env);

    if (env->cur_type != _K_TOKEN_TYPE_IDENTIFIER) {
        env->log(_k_get_error(env, "Expected identifier after declarator, got %s", _k_get_token_str(env)));
        return K_ERROR_INVALID_DECLARATION;
    }

    _k_variable_t var = {
        .name   = _k_get_token_str(env),
        .type   = type,
        .offset = 0,
        .flags  = _K_VARIABLE_FLAG_GLOBAL,
        .size   = _k_deduce_size(type),
    };

    _k_advance_token(env);

    if (env->cur_type == _K_TOKEN_TYPE_NEWEXPRESSION) {
        var.flags |= _K_VARIABLE_FLAG_FUNC;
        var.size   = 0;

        _k_add_var(env, &var, 1);

        _k_prepare_function_compile(env, var.name);

        env->cur_function->flags = strcmp(type, "f64") == 0 || strcmp(type, "f32") == 0 ? _K_VARIABLE_FLAG_FLOAT : 0x0;

        return _k_compile_function_declaration(env);
    }

    if (env->cur_type == _K_TOKEN_TYPE_OPERATOR) {
        if (strncmp(_k_get_token_str(env), "=", 1) != 0) {
            env->log(_k_get_error(env, "Expected assignment operator, got %s", _k_get_token_str(env)));
            return K_ERROR_EXPECTED_ASSIGNMENT;
        } 

        _k_advance_token(env);
        
        /* Initialize global and assign constant.  */
        if (env->cur_type == _K_TOKEN_TYPE_NUMBER) {
            unsigned long value = atoi(_k_get_token_str(env));

            _k_place_global(env, &var, (char *)&value);
            _k_advance_token(env);

            return K_ERROR_NONE;
        }

        if (env->cur_type == _K_TOKEN_TYPE_STRING) {
            char *value = _k_get_token_str(env);

            _k_place_global(env, &var, value);
            _k_advance_token(env);

            return K_ERROR_NONE;
        }

        if (env->cur_type == _K_TOKEN_TYPE_IDENTIFIER) {
            _k_variable_t *id = _k_get_var(env, _k_get_token_str(env));

            if (id == (_k_variable_t *)0x0) {
                env->log(_k_get_error(env, "Undefined variable: %s", _k_get_token_str(env)));
                return K_ERROR_UNDECLARED_VARIABLE;
            }

            _k_place_global(env, &var, env->runtime->mem + id->offset);
            _k_advance_token(env);

            return K_ERROR_NONE;
        }

        env->log(_k_get_error(env, "Expected constant after assignment operator, got %s", _k_get_token_str(env)));
        return K_ERROR_EXPECTED_CONSTANT;
    }

    else if (env->cur_type != _K_TOKEN_TYPE_ENDLINE) {
        env->log(_k_get_error(env, "Expected assignment operator or endline, got %s", _k_get_token_str(env)));
        return K_ERROR_JUNK_AFTER_DECLARATION;
    }

    return K_ERROR_NONE;
}

/*
 *    Compiles a global keyword.
 *
 *    @param k_env_t    *env       The environment to parse the global keyword in.
 * 
 *    @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_global_keyword(k_env_t *env) {
    /* Allow some global keywords like import, etc.  */
    _k_advance_token(env);

    return K_ERROR_NONE;
}

_k_grammar_t _loop_grammar[] = {
    {_K_TOKEN_TYPE_IDENTIFIER, _k_compile_global_declaration},
    {_K_TOKEN_TYPE_KEYWORD,    _k_compile_global_keyword},
    {_K_TOKEN_TYPE_ENDLINE,    _k_compile_endline},
};

/*
 *    Compilation loop.
 *
 *    @param k_env_t    *env       The environment to parse the function declaration in.
 * 
 *    @return k_build_error_t      The error code, if any.
 */

k_build_error_t _k_compile_loop(k_env_t *env) {
    unsigned long parsed;

    while (env->cur_type != _K_TOKEN_TYPE_EOF) {
        parsed = 0;

        for (unsigned long i = 0; i < ARRAY_SIZE(_loop_grammar); i++) {
            if (env->cur_type == _loop_grammar[i].type) {
                parsed = 1;

                k_build_error_t ret = _loop_grammar[i].compile(env);

                if (ret != K_ERROR_NONE) return ret;

                break;
            }
        }

        if (parsed == 0) {
            env->log(_k_get_error(env, "Invalid global starter: %s", _k_get_token_str(env)));
            return K_ERROR_UNEXPECTED_TOKEN;
        }
    }
    
    return K_ERROR_NONE;
}

/*
 *    Compiles a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to compile the source in.
 *    @param const char *source    The source to compile.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile(k_env_t *env, const char *source) {
    _k_create_runtime(env, source);

    env->cur_token = &env->lexer->tokens[0];
    env->cur_type  =  env->cur_token->tokenable->type;

    k_build_error_t ret = _k_compile_loop(env);

    char *old  = env->runtime->mem;
    void *exec = mmap(0, env->runtime->size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    memcpy(exec, env->runtime->mem, env->runtime->size);
    env->runtime->mem = exec;
    free(old);

    printf("%s\n", _k_print_assembly(env));

    return ret;
}