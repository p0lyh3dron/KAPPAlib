/*
 *    fastK_interpret.c    --    Source for KAPPA interpretation
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 1, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    For convenience, the KAPPA interpreter is contained in this
 *    file, away from the KAPPA environment and parser.
 */
#include "fastK.h"

/*
 *    Adds a function to the interpreter environment.
 *
 *    @param k_env_t      *env       The environment to add the function to.
 *    @param char         *name      The name of the function.
 *    @param unsigned long index     The token index of the start statement.
 */
void k_add_function(k_env_t *env, const char *name, unsigned long index) {
    env->functions = realloc(env->functions, sizeof(k_interp_var_t *) * (env->function_count + 1));

    env->functions[env->function_count].name  = name;
    env->functions[env->function_count].index = index;

    env->function_count++;
}

/*
 *    Adds a global variable to the interpreter environment.
 *
 *    @param k_env_t    *env       The environment to create the runtime in.
 *    @param char       *name      The name of the variable.
 *    @param char       *value     The value of the variable.
 *    @param char       *type      The type of the variable.
 */
void k_add_global(k_env_t *env, const char *name, const char *value, const char *type) {
    env->globals = realloc(env->globals, sizeof(k_interp_var_t *) * (env->global_count + 1));

    env->globals[env->global_count].name  = name;
    env->globals[env->global_count].value = value;
    env->globals[env->global_count].type  = type;

    env->global_count++;
}

/*
 *    Adds a local variable to the interpreter environment.
 *
 *    @param k_env_t    *env       The environment to create the runtime in.
 *    @param char       *name      The name of the variable.
 *    @param char       *value     The value of the variable.
 *    @param char       *type      The type of the variable.
 */
void k_add_local(k_env_t *env, const char *name, const char *value, const char *type) {
    if (env->scope == (k_interp_scope_t *)0x0) {
        env->scope            = malloc(sizeof(k_interp_scope_t));
        env->scope->next      = (k_interp_scope_t *)0x0;
        env->scope->prev      = (k_interp_scope_t *)0x0;
        env->scope->var_count = 0;
    }

    env->scope->vars = realloc(env->scope->vars, sizeof(k_interp_var_t *) * (env->scope->var_count + 1));
    
    env->scope->vars[env->scope->var_count].name  = name;
    env->scope->vars[env->scope->var_count].value = value;
    env->scope->vars[env->scope->var_count].type  = type;

    env->scope->var_count++;
}

/*
 *    Interprets an expression.
 *
 *    @param k_env_t    *env       The environment to parse the expression in.
 *    @param const char *source    The source to parse the expression from.
 */
void k_interpret_expression(k_env_t *env, const char *source) {
    k_token_type_e **type      = &env->cur_token->tokenable->type;

    if (*type == K_TOKEN_TYPE_NUMBER) {
        /* Interpret number literal as a s64, in C as long.  */
        env->ret.type = (char *)realloc(env->ret.type, sizeof(char) * (strlen("s64") + 1));
        strcpy(env->ret.type, "s64");

        env->ret.value = (char *)realloc(env->ret.value, sizeof(long));
        *(long *)env->ret.value = atol(source + env->cur_token->index);

        env->ret.size = sizeof(long);

        k_advance_token(env);
        return;
    }

    if (*type == K_TOKEN_TYPE_STRING) {
        /* Interpret string literal as ptr_u8, in C as char *.  */
        /* do later lol  */

        k_advance_token(env);
        return;
    }

    if (*type == K_TOKEN_TYPE_IDENTIFIER) {
        /* Return the identifier's value. */

        /* Check if the identifier is a global variable.  */
        for (unsigned long i = 0; i < env->global_count; ++i) {
            if (strcmp(env->globals[i].name, source + env->cur_token->index) == 0) {
                env->ret.type = (char *)realloc(env->ret.type, sizeof(char) * (strlen(env->globals[i].type) + 1));
                strcpy(env->ret.type, env->globals[i].type);

                env->ret.value = (char *)realloc(env->ret.value, sizeof(char) * (strlen(env->globals[i].value) + 1));
                strcpy(env->ret.value, env->globals[i].value);

                env->ret.size = strlen(env->globals[i].value);

                k_advance_token(env);
                return;
            }
        }

        /* Check if the identifier is a local variable.  */
        if (env->scope != (k_interp_scope_t *)0x0) {
            for (unsigned long i = 0; i < env->scope->var_count; ++i) {
                if (strcmp(env->scope->vars[i].name, source + env->cur_token->index) == 0) {
                    env->ret.type = (char *)realloc(env->ret.type, sizeof(char) * (strlen(env->scope->vars[i].type) + 1));
                    strcpy(env->ret.type, env->scope->vars[i].type);

                    env->ret.value = (char *)realloc(env->ret.value, sizeof(char) * (strlen(env->scope->vars[i].value) + 1));
                    strcpy(env->ret.value, env->scope->vars[i].value);

                    env->ret.size = strlen(env->scope->vars[i].value);

                    k_advance_token(env);
                    return;
                }
            }
        }
        k_advance_token(env);
    }

    if (*type == K_TOKEN_TYPE_NEWEXPRESSION) {
        /* Accessor for the previous identifier  */
        k_advance_token(env);
        k_interpret_expression(env, source);
        if (*type == K_TOKEN_TYPE_ENDEXPRESSION) {
            k_advance_token(env);
            return;
        } else if (*type == K_TOKEN_TYPE_SEPARATOR) {
            k_advance_token(env);
            /* Function call e.g. identifier(var1, var2)  */
            while (*type != K_TOKEN_TYPE_ENDEXPRESSION) {
                k_interpret_expression(env, source);

                if (*type == K_TOKEN_TYPE_SEPARATOR) {
                    k_advance_token(env);
                }
            }
        }
    }

    if (*type == K_TOKEN_TYPE_NEWINDEX) {
        k_advance_token(env);
        k_interpret_expression(env, source);

        if (*type == K_TOKEN_TYPE_ENDINDEX) {
            k_advance_token(env);
            return;
        }
    }

    if (*type == K_TOKEN_TYPE_OPERATOR) {
        k_advance_token(env);
        k_interpret_expression(env, source);
    }
}

/*
 *    Interprets a statement.
 *
 *    @param k_env_t    *env       The environment to parse the statement in.
 *    @param const char *source    The source to parse the statement from.
 */
void k_interpret_statement(k_env_t *env, const char *source) {
    k_token_t      **tok  = &env->cur_token;
    k_token_type_e **type = &(*tok)->tokenable->type;

    if (*type == K_TOKEN_TYPE_NEWSTATEMENT) {
        k_advance_token(env);
        k_interpret_statement(env, source);
        if (*type == K_TOKEN_TYPE_ENDSTATEMENT) {
            k_advance_token(env);
            return;
        }
    }

    if (*type == K_TOKEN_TYPE_KEYWORD) {
        if (k_token_string_matches(*tok, "return", source) == 0) {
            k_advance_token(env);
            k_interpret_expression(env, source);

            if (*type == K_TOKEN_TYPE_ENDLINE) {
                k_advance_token(env);
                return;
            }
        } else {
            k_advance_token(env);
            k_interpret_expression(env, source);
            if (*type == K_TOKEN_TYPE_NEWSTATEMENT) {
                k_interpret_statement(env, source);
                k_advance_token(env);
            }
            
            if (*type == K_TOKEN_TYPE_ENDSTATEMENT) {
                k_advance_token(env);
                return;
            }
        }
    }

    if (*type == K_TOKEN_TYPE_IDENTIFIER) {
        k_advance_token(env);
        
        if (*type == K_TOKEN_TYPE_DECLARATOR) {
            k_advance_token(env);
            /* Local declaration  */
            
            if (*type == K_TOKEN_TYPE_IDENTIFIER) {
                k_advance_token(env);
                
                if (*type == K_TOKEN_TYPE_OPERATOR) {
                    k_advance_token(env);
                    k_interpret_expression(env, source);
                }
            }
        }
    }
}

/*
 *    Interprets a global declaration.
 *
 *    @param k_env_t    *env       The environment to parse the function declaration in.
 */
void k_interpret_global_declaration(k_env_t *env, const char *source) {
    k_token_type_e **type  = &env->cur_token->tokenable->type;

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
    } else if (*type == K_TOKEN_TYPE_ENDLINE) {
        /* Global variable declaration without value.  */
        return;
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
    } else return;

    k_advance_token(env);

    if (*type == K_TOKEN_TYPE_NEWSTATEMENT) {
        k_advance_token(env);
        k_interpret_statement(env, source);
    } else return;
}