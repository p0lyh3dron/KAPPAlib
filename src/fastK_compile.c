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

/*
 *    Compiles an expression.
 *
 *    @param k_env_t    *env       The environment to parse the expression in.
 *    @param const char *source    The source to parse the expression from.
 */
void k_compile_expression(k_env_t *env, const char *source) {
    k_token_type_e *type      = &env->cur_type;

    if (*type == K_TOKEN_TYPE_NUMBER) {
        k_advance_token(env);
        return;
    }

    if (*type == K_TOKEN_TYPE_STRING) {
        k_advance_token(env);
        return;
    }

    if (*type == K_TOKEN_TYPE_NEWEXPRESSION) {
        k_advance_token(env);
        k_compile_expression(env, source);
        if (*type == K_TOKEN_TYPE_ENDEXPRESSION) {
            k_advance_token(env);
            return;
        }
    }

    if (*type == K_TOKEN_TYPE_IDENTIFIER) {
        k_advance_token(env);

        if (*type == K_TOKEN_TYPE_NEWEXPRESSION) {
            /* Function call e.g. identifier(var1, var2)  */
            k_advance_token(env);
            
        }

        if (*type == K_TOKEN_TYPE_NEWINDEX) {
            /* Array access e.g.  identifier[var1]  */
            k_advance_token(env);
            
        }
        return;
    }
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

    do {
        if (*type == K_TOKEN_TYPE_NEWSTATEMENT) {
            k_advance_token(env);
            k_compile_statement(env, source);
        }

        if (*type == K_TOKEN_TYPE_KEYWORD) {
            if (k_token_string_matches(*tok, "return", source) == 0) {
                k_advance_token(env);
                k_compile_expression(env, source);
            } else {
                k_advance_token(env);
                k_compile_expression(env, source);
                k_compile_statement(env, source);
            }
        }

        if (*type == K_TOKEN_TYPE_IDENTIFIER) {
            k_advance_token(env);
            
            if (*type == K_TOKEN_TYPE_DECLARATOR) {
                k_advance_token(env);
                
                if (*type == K_TOKEN_TYPE_IDENTIFIER) {
                    k_advance_token(env);
                    
                    if (*type == K_TOKEN_TYPE_OPERATOR) {
                        k_advance_token(env);
                        k_compile_expression(env, source);
                    }
                }
            }
        }

        if (*type == K_TOKEN_TYPE_ENDLINE) {
            k_advance_token(env);
        } else return;
    } while (*type != K_TOKEN_TYPE_ENDSTATEMENT);
}

/*
 *    Compiles a global declaration.
 *
 *    @param k_env_t    *env       The environment to parse the function declaration in.
 */
void k_compile_global_declaration(k_env_t *env, const char *source) {
    unsigned long  i       = 0;
    k_token_type_e *type   = &env->cur_type;

    if (*type == K_TOKEN_TYPE_ENDLINE) {
        k_advance_token(env);
    }

    if (*type == K_TOKEN_TYPE_IDENTIFIER) {
        k_advance_token(env);
    } else return;

    if (*type == K_TOKEN_TYPE_DECLARATOR) {
        k_advance_token(env);
    } else return;

    if (*type == K_TOKEN_TYPE_IDENTIFIER) {
        k_advance_token(env);
    } else return;

    if (*type == K_TOKEN_TYPE_OPERATOR) {
        /* Global variable declaration with value.  */
        k_advance_token(env);
        k_compile_expression(env, source);

        if (*type == K_TOKEN_TYPE_ENDLINE) {
            k_advance_token(env);
        } else return;
    } else if (*type == K_TOKEN_TYPE_ENDLINE) {
        /* Global variable declaration without value.  */
        k_advance_token(env);
    } else if (*type == K_TOKEN_TYPE_NEWEXPRESSION) {
        /* Global function declaration.  */
        k_advance_token(env);
        
        while (*type != K_TOKEN_TYPE_ENDEXPRESSION) {
            if (*type == K_TOKEN_TYPE_IDENTIFIER) {
                k_advance_token(env);
            } else return;

            if (*type == K_TOKEN_TYPE_DECLARATOR) {
                k_advance_token(env);
            } else return;

            if (*type == K_TOKEN_TYPE_IDENTIFIER) {
                k_advance_token(env);
            } else return;

            if (*type == K_TOKEN_TYPE_SEPARATOR) {
                k_advance_token(env);
            }
        }

        k_advance_token(env);

        if (*type == K_TOKEN_TYPE_NEWSTATEMENT) {
            k_advance_token(env);
            k_compile_statement(env, source);
        } else return;
    } else return;
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
        k_compile_global_declaration(env, source);

        k_advance_token(env);
    }
}