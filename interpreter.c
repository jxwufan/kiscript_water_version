//
// Created by fan wu on 1/5/16.
//

#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include "interpreter.h"
#include "parser.h"
#include "lexical_parser.h"
#include "variable.h"
#include "syntactic_parser.h"

return_struct_t *evaluate_token(token_t *token, activation_record_t *AR_parent) {
    if (token->id == TOKEN_LEXICAL_KEYWORD && *((keyword_id_t *) token->data) == KEYWORD_THIS) {
        token_t *this_token = GC_malloc(sizeof(token_t));
        GString *this_string = GC_malloc(sizeof(GString));
        this_string->str = GC_malloc(sizeof(gchar) * 10);
        strcpy(this_string->str, "this");
        this_token->data = this_string;
        this_token->id = TOKEN_LEXICAL_IDENTIFIER;
        this_token->children = token->children;
        token = this_token;
    }

    if (is_lexical(token)) {
        return evaluate_lexicial(token, AR_parent);
    } else if (is_expression(token)) {
        return evaluate_expression(token, AR_parent);
    } else if (is_statement(token)) {
        return evaluate_statement(token, AR_parent);
    } else if (is_function(token)) {
        return evaluate_function(token, AR_parent);
    }

    printf("%s\n", token_to_string(token)->str);
    printf("Error token!");
    return NULL;
}

return_struct_t *evaluate_program(token_t *program_token, activation_record_t *AR_parent) {
    return_struct_t *return_struct = return_struct_new();
    activation_record_t *AR = activation_record_new(NULL, NULL);
    AR->static_link = AR;
    init_builtin(AR);

    data_tokens_list = NULL;

    for (guint i  = 0; i < program_token->children->len; ++i) {
        return_struct = evaluate_token(token_get_child(program_token, i), AR);
        if (return_struct->mid_variable != NULL) {
//            printf("mid: %s ", variable_to_string(return_struct->mid_variable));
        }
        if (return_struct->end_variable != NULL) {
//            printf("end: %s\n", variable_to_string(return_struct->end_variable));
        } else {
//            printf("\n");
        }
    }

    activation_record_reach_end_of_scope(AR);

    GList *list = data_tokens_list;

    while (data_tokens_list != NULL) {
        token_free((token_t**)&(data_tokens_list->data));
        data_tokens_list = data_tokens_list->next;
    }

    g_list_free(list);

    return_struct->status = STAUS_NORMAL;
    return return_struct;
}

return_struct_t *evaluate_eval_call(token_t *program_token, activation_record_t *AR_parent) {
    return_struct_t *return_struct = return_struct_new();
    activation_record_t *AR = AR_parent;

    for (guint i  = 0; i < program_token->children->len; ++i) {
        return_struct = evaluate_token(token_get_child(program_token, i), AR);
        if (return_struct->mid_variable != NULL) {
//            printf("mid: %s ", variable_to_string(return_struct->mid_variable));
        }
        if (return_struct->end_variable != NULL) {
//            printf("end: %s\n", variable_to_string(return_struct->end_variable));
        } else {
//            printf("\n");
        }
    }

    return_struct->status = STAUS_NORMAL;
    return return_struct;
}

return_struct_t *evaluate_block(token_t *block_token, activation_record_t *AR_parent) {
    return_struct_t *return_struct = return_struct_new();
    activation_record_t *AR = activation_record_new(AR_parent, AR_parent->static_link);
    for (guint i  = 0; i < block_token->children->len; ++i) {
        return_struct_t *return_struct;
        return_struct = evaluate_token(token_get_child(block_token, i), AR);
        if (return_struct->mid_variable != NULL) {
//            printf("%s\n", variable_to_string(return_struct->mid_variable));
        }

        if (need_return_to_invoker(return_struct)) {
            activation_record_reach_end_of_scope(AR);

            return return_struct;
        }
    }

    activation_record_reach_end_of_scope(AR);

    return_struct->status = STAUS_NORMAL;
    return return_struct;
}

return_struct_t *return_struct_new() {
    return_struct_t *return_struct = GC_malloc(sizeof(return_struct_t));
    return_struct->status = STAUS_UNDEFINED;
    return_struct->mid_variable = NULL;
    return_struct->end_variable = NULL;
    return return_struct;
}

gboolean is_lexical(token_t *token) {
    return token->id == TOKEN_LEXICAL_BOOLEAN_LITERAL   ||
           token->id == TOKEN_LEXICAL_IDENTIFIER        ||
           token->id == TOKEN_LEXICAL_NULL_LITERAL      ||
           token->id == TOKEN_LEXICAL_NUMERIC_LITERAL   ||
           token->id == TOKEN_LEXICAL_STRING_LITERAL;
}

gboolean is_expression(token_t *token) {
    return token->id == TOKEN_EXPRESSION_ADDITIVE_EXPRESSION             ||
           token->id == TOKEN_EXPRESSION_MULTIPLICATIVE_EXPRESSION       ||
           token->id == TOKEN_EXPRESSION_EQUALITY_EXPRESSION             ||
           token->id == TOKEN_EXPRESSION_CALL_EXPRESSION                 ||
           token->id == TOKEN_EXPRESSION_NEW_EXPRESSION                  ||
           token->id == TOKEN_EXPRESSION_SHIFT_EXPRESSION                ||
           token->id == TOKEN_EXPRESSION_EQUALITY_EXPRESSION             ||
           token->id == TOKEN_EXPRESSION_UNARY_EXPRESSION                ||
           token->id == TOKEN_EXPRESSION_POSTFIX_EXPRESSION              ||
           token->id == TOKEN_EXPRESSION_RELATIONAL_EXPRESSION           ||
           token->id == TOKEN_EXPRESSION_CONDITIONAL_EXPRESSION          ||
           token->id == TOKEN_EXPRESSION_LOGICAL_AND_EXPRESSION          ||
           token->id == TOKEN_EXPRESSION_LOGICAL_OR_EXPRESSION           ||
           token->id == TOKEN_EXPRESSION_BITWISE_AND_EXPRESSION          ||
           token->id == TOKEN_EXPRESSION_BITWISE_OR_EXPRESSION           ||
           token->id == TOKEN_EXPRESSION_BITWISE_XOR_EXPRESSION          ||
           token->id == TOKEN_EXPRESSION_OBJECT_LITERAL                  ||
           token->id == TOKEN_EXPRESSION_ARRAY_LITERAL                   ||
           token->id == TOKEN_EXPRESSION_PROPERTY_ACCESSOR               ||
           token->id == TOKEN_EXPRESSION_ASSIGNMENT_EXPRESSION;
}

gboolean is_statement(token_t *token) {
    return  token->id == TOKEN_STATEMENT_EXPRESSION_STATEMENT       ||
            token->id == TOKEN_STATEMENT_VARIABLE_DECLARATION       ||
            token->id == TOKEN_STATEMENT_IF_STATEMENT               ||
            token->id == TOKEN_STATEMENT_WHILE_STATEMENT            ||
            token->id == TOKEN_STATEMENT_DO_WHILE_STATEMENT         ||
            token->id == TOKEN_STATEMENT_VARIABLE_DECLARATION_LIST  ||
            token->id == TOKEN_STATEMENT_FOR_STATEMENT              ||
            token->id == TOKEN_STATEMENT_RETURN_STATEMENT           ||
            token->id == TOKEN_STATEMENT_BLOCK                      ||
            token->id == TOKEN_STATEMENT_BREAK_STATEMENT            ||
            token->id == TOKEN_STATEMENT_CONTINUE_STATEMENT         ||
            token->id == TOKEN_STATEMENT_VARIABLE_STATEMENT;
}

return_struct_t *evaluate_lexicial(token_t *lexical_token, activation_record_t *AR_parent) {
    return_struct_t *return_struct = return_struct_new();

    if (lexical_token->id == TOKEN_LEXICAL_BOOLEAN_LITERAL) {
        return_struct->status = STAUS_NORMAL;
        return_struct->mid_variable = variable_bool_new(boolean_literal_get_value(lexical_token));

        return return_struct;
    } else if (lexical_token->id == TOKEN_LEXICAL_IDENTIFIER) {
        if (strcmp(identifier_get_value(lexical_token)->str,"undefined")==0) {
            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = variable_undefined_new();
            return return_struct;
        }

        return_struct->mid_variable = activation_record_lookup(AR_parent, identifier_get_value(lexical_token)->str);
        if (return_struct->mid_variable != NULL) {
            return_struct->status = STAUS_NORMAL;
        } else {
            return_struct->status = STAUS_THROW;
            // TODO: return exception
            printf("No varialbe named %s\n", identifier_get_value(lexical_token)->str);
            exit(-1);
        }

        return return_struct;
    } else if (lexical_token->id == TOKEN_LEXICAL_NULL_LITERAL) {
        return_struct->status = STAUS_NORMAL;
        return_struct->mid_variable = variable_null_new();

        return return_struct;
    } else if (lexical_token->id == TOKEN_LEXICAL_NUMERIC_LITERAL) {
        return_struct->status = STAUS_NORMAL;
        return_struct->mid_variable = variable_numerical_new(numeric_literal_get_value(lexical_token));

        return return_struct;
    } else if (lexical_token->id == TOKEN_LEXICAL_STRING_LITERAL) {
        return_struct->status = STAUS_NORMAL;
        return_struct->mid_variable = variable_string_new(string_literal_get_value(lexical_token)->str);

        return return_struct;
    }

    return_struct->status = STAUS_THROW;
    return NULL;
}

gboolean is_function(token_t *token) {
    return token->id == TOKEN_FUNCTION_FUNCTION_DECLARATION ||
           token->id == TOKEN_FUNCTION_FUNCTION_EXPRESSION;
    return FALSE;
}

return_struct_t *evaluate_statement(token_t *statement_token, activation_record_t *AR_parent) {
    return_struct_t *return_struct = return_struct_new();

    if (statement_token->id == TOKEN_STATEMENT_EXPRESSION_STATEMENT) {
        return evaluate_token(token_get_child(statement_token, 0), AR_parent);
    } else if (statement_token->id == TOKEN_STATEMENT_VARIABLE_STATEMENT) {
        return evaluate_token(token_get_child(statement_token, 0), AR_parent);
    } else if (statement_token->id == TOKEN_STATEMENT_VARIABLE_DECLARATION) {
        g_assert(token_get_child(statement_token, 0)->id == TOKEN_LEXICAL_IDENTIFIER);
        gchar *identifier_str = identifier_get_value(token_get_child(statement_token, 0))->str;

        activation_record_declare(AR_parent, identifier_str);
        if (statement_token->children->len > 1) {
            return_struct_t *return_struct;
            return_struct = evaluate_token(token_get_child(statement_token, 1), AR_parent);
            if (return_struct->status == STAUS_NORMAL) {
                activation_record_insert(AR_parent, identifier_str, return_struct->mid_variable);
                return return_struct;
            } else if (return_struct->status == STAUS_THROW) {
                // TODO: handel exception
            }
        } else {
            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = NULL;
            return return_struct;
        }

    } else if (statement_token->id == TOKEN_STATEMENT_IF_STATEMENT) {
        g_assert(statement_token->children->len >= 2);
        g_assert(statement_token->children->len <= 3);
        return_struct_t *condition_return_struct = evaluate_token(token_get_child(statement_token, 0), AR_parent);
        if (condition_return_struct->status != STAUS_NORMAL) {
            return condition_return_struct;
        } else if (condition_return_struct->mid_variable->variable_type != VARIABLE_BOOL) {
            return_struct->status = STAUS_THROW;
            // TODO: handel exception
            return return_struct;
        } else {
            gboolean *condition = condition_return_struct->mid_variable->variable_data;
            if (*condition) {
                return evaluate_block(token_get_child(statement_token, 1), AR_parent);
            } else {
                if (statement_token->children->len == 3 && token_get_child(statement_token, 2) != NULL) {
                    return evaluate_block(token_get_child(statement_token, 2), AR_parent);
                }
            }
            return_struct->status = STAUS_NORMAL;
            return return_struct;
        }
    } else if (statement_token->id == TOKEN_STATEMENT_BLOCK) {
        return evaluate_block(statement_token, AR_parent);
    } else if (statement_token->id == TOKEN_STATEMENT_RETURN_STATEMENT) {
        return_struct_t *return_value_return_struct = evaluate_token(token_get_child(statement_token, 0), AR_parent);

        return_struct->mid_variable = return_value_return_struct->mid_variable;
        return_struct->status = STAUS_RETURN;
        return return_struct;
    } else if (statement_token->id == TOKEN_STATEMENT_CONTINUE_STATEMENT) {
        return_struct->status = STAUS_CONTINUE;
        return return_struct;
    } else if (statement_token->id == TOKEN_STATEMENT_BREAK_STATEMENT) {
        return_struct->status = STAUS_BREAK;
        return return_struct;
    } else if (statement_token->id == TOKEN_STATEMENT_WHILE_STATEMENT) {
        activation_record_t *AR = activation_record_new(AR_parent, AR_parent->static_link);
        token_t *for_init_token         = NULL;
        token_t *for_condition_token    = token_get_child(statement_token, 0);
        token_t *for_loop_end_token     = NULL;
        token_t *for_block_token        = token_get_child(statement_token, 1);

        return_struct_t *for_return_struct;
        if (for_init_token != NULL) {
            for_return_struct = evaluate_token(for_init_token, AR);
            if (for_return_struct->status != STAUS_NORMAL) {
                // TODO: handel abnormal status
            }
        }
        while (TRUE) {
            if (for_condition_token != NULL) {
                for_return_struct = evaluate_token(for_condition_token, AR);
                if (for_return_struct->status != STAUS_NORMAL) {
                    // TODO: handel abnormal status
                    printf("For condition abnormal");
                    exit(-1);
                }
                if (for_return_struct->mid_variable->variable_type != VARIABLE_BOOL) {
                    // TODO: handel abnormal status
                    printf("For condition type abnormal");
                    exit(-1);
                }
                gboolean *condition = (gboolean *) for_return_struct->mid_variable->variable_data;
//                printf("%s\n", variable_to_string(for_return_struct->mid_variable));
//                printf("%s\n", token_to_string(for_condition_token)->str);
                if (!*condition)
                    break;
            }
            for_return_struct = evaluate_block(for_block_token, AR);
            if (need_return_to_invoker(for_return_struct)) {
                if (for_return_struct->status == STAUS_RETURN) {
                    activation_record_reach_end_of_scope(AR);
                    return for_return_struct;
                } else if (for_return_struct->status == STAUS_BREAK) {
                    break;
                } else if (for_return_struct->status == STAUS_CONTINUE) {
                    if (for_loop_end_token != NULL) {
                        for_return_struct = evaluate_token(for_loop_end_token, AR);
                        if (for_return_struct->status != STAUS_NORMAL) {
                            // TODO: handel abnormal status
                        }
                    }
                    continue;
                }
            }

            if (for_loop_end_token != NULL) {
                for_return_struct = evaluate_token(for_loop_end_token, AR);
                if (for_return_struct->status != STAUS_NORMAL) {
                    // TODO: handel abnormal status
                }
            }
        }

        activation_record_reach_end_of_scope(AR);

        return_struct->status = STAUS_NORMAL;
        return return_struct;
    } else if (statement_token->id == TOKEN_STATEMENT_DO_WHILE_STATEMENT) {
        activation_record_t *AR = activation_record_new(AR_parent, AR_parent->static_link);
        token_t *for_init_token         = NULL;
        token_t *for_condition_token    = token_get_child(statement_token, 1);
        token_t *for_loop_end_token     = NULL;
        token_t *for_block_token        = token_get_child(statement_token, 0);

        return_struct_t *for_return_struct;
        if (for_init_token != NULL) {
            for_return_struct = evaluate_token(for_init_token, AR);
            if (for_return_struct->status != STAUS_NORMAL) {
                // TODO: handel abnormal status
            }
        }
        while (TRUE) {
            for_return_struct = evaluate_block(for_block_token, AR);
            if (need_return_to_invoker(for_return_struct)) {
                if (for_return_struct->status == STAUS_RETURN) {
                    activation_record_reach_end_of_scope(AR);
                    return for_return_struct;
                } else if (for_return_struct->status == STAUS_BREAK) {
                    break;
                } else if (for_return_struct->status == STAUS_CONTINUE) {
                    if (for_loop_end_token != NULL) {
                        for_return_struct = evaluate_token(for_loop_end_token, AR);
                        if (for_return_struct->status != STAUS_NORMAL) {
                            // TODO: handel abnormal status
                        }
                    }
                    continue;
                }
            }

            if (for_loop_end_token != NULL) {
                for_return_struct = evaluate_token(for_loop_end_token, AR);
                if (for_return_struct->status != STAUS_NORMAL) {
                    // TODO: handel abnormal status
                }
            }
            if (for_condition_token != NULL) {
                for_return_struct = evaluate_token(for_condition_token, AR);
                if (for_return_struct->status != STAUS_NORMAL) {
                    // TODO: handel abnormal status
                    printf("For condition abnormal");
                    exit(-1);
                }
                if (for_return_struct->mid_variable->variable_type != VARIABLE_BOOL) {
                    // TODO: handel abnormal status
                    printf("For condition type abnormal");
                    exit(-1);
                }
                gboolean *condition = (gboolean *) for_return_struct->mid_variable->variable_data;
//                printf("%s\n", variable_to_string(for_return_struct->mid_variable));
//                printf("%s\n", token_to_string(for_condition_token)->str);
                if (!*condition)
                    break;
            }
        }

        activation_record_reach_end_of_scope(AR);

        return_struct->status = STAUS_NORMAL;
        return return_struct;
    } else if (statement_token->id == TOKEN_STATEMENT_FOR_STATEMENT) {
        activation_record_t *AR = activation_record_new(AR_parent, AR_parent->static_link);
        token_t *for_init_token         = token_get_child(statement_token, 0);
        token_t *for_condition_token    = token_get_child(statement_token, 1);
        token_t *for_loop_end_token     = token_get_child(statement_token, 2);
        token_t *for_block_token        = token_get_child(statement_token, 3);

        return_struct_t *for_return_struct;
        if (for_init_token != NULL) {
            for_return_struct = evaluate_token(for_init_token, AR);
            if (for_return_struct->status != STAUS_NORMAL) {
                // TODO: handel abnormal status
            }
        }
        while (TRUE) {
            if (for_condition_token != NULL) {
                for_return_struct = evaluate_token(for_condition_token, AR);
                if (for_return_struct->status != STAUS_NORMAL) {
                    // TODO: handel abnormal status
                    printf("For condition abnormal");
                    exit(-1);
                }
                if (for_return_struct->mid_variable->variable_type != VARIABLE_BOOL) {
                    // TODO: handel abnormal status
                    printf("For condition type abnormal");
                    exit(-1);
                }
                gboolean *condition = (gboolean *) for_return_struct->mid_variable->variable_data;
//                printf("%s\n", variable_to_string(for_return_struct->mid_variable));
//                printf("%s\n", token_to_string(for_condition_token)->str);
                if (!*condition)
                    break;
            }
            for_return_struct = evaluate_block(for_block_token, AR);
            if (need_return_to_invoker(for_return_struct)) {
                if (for_return_struct->status == STAUS_RETURN) {
                    activation_record_reach_end_of_scope(AR);
                    return for_return_struct;
                } else if (for_return_struct->status == STAUS_BREAK) {
                    break;
                } else if (for_return_struct->status == STAUS_CONTINUE) {
                    if (for_loop_end_token != NULL) {
                        for_return_struct = evaluate_token(for_loop_end_token, AR);
                        if (for_return_struct->status != STAUS_NORMAL) {
                            // TODO: handel abnormal status
                        }
                    }
                    continue;
                }
            }

            if (for_loop_end_token != NULL) {
                for_return_struct = evaluate_token(for_loop_end_token, AR);
                if (for_return_struct->status != STAUS_NORMAL) {
                    // TODO: handel abnormal status
                }
            }
        }

        activation_record_reach_end_of_scope(AR);

        return_struct->status = STAUS_NORMAL;
        return return_struct;
    } else if (statement_token->id == TOKEN_STATEMENT_VARIABLE_DECLARATION_LIST) {
        for (guint i  = 0; i < statement_token->children->len; ++i) {
            return_struct_t *return_struct;
            return_struct = evaluate_token(token_get_child(statement_token, i), AR_parent);
            if (return_struct->mid_variable != NULL) {
//                printf("INIT: %s\n", variable_to_string(return_struct->mid_variable));
            }

            if (need_return_to_invoker(return_struct)) {
                return return_struct;
            }
        }

        return_struct->status = STAUS_NORMAL;
        return return_struct;
    }

    return NULL;
}

return_struct_t *evaluate_expression(token_t *expression_token, activation_record_t *AR_parent) {
    return_struct_t *return_struct = return_struct_new();
    if (expression_token->id == TOKEN_EXPRESSION_ASSIGNMENT_EXPRESSION) {
        gchar *identifier;
        GHashTable *storage_hash_table;
        resolve_assignment_identifier(token_get_child(expression_token, 0), AR_parent, &identifier,
                                      &storage_hash_table);

        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 2), AR_parent);
        // TODO: check return status
        if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_EQUALS_SIGN) {
//        printf("assignment rhs: %s\n", variable_to_string(return_struct_rhs->mid_variable));
            if (storage_hash_table != NULL) {
                g_hash_table_insert(storage_hash_table, identifier, return_struct_rhs->mid_variable);
                if (return_struct_rhs->mid_variable->variable_type == VARIABLE_OBJECT || return_struct_rhs->mid_variable->variable_type == VARIABLE_FUNC) {
                    g_hash_table_ref(return_struct_rhs->mid_variable->variable_data);
                }
            } else {
                activation_record_insert(AR_parent, identifier, return_struct_rhs->mid_variable);
            }

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = return_struct_rhs->mid_variable;
            return return_struct;
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_MULTIPLY_ASSIGNMENT) {
            // TODO: check existence
            variable_t *lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent)->mid_variable;
            variable_t *rhs = evaluate_token(token_get_child(expression_token, 2), AR_parent)->mid_variable;
            gdouble operand_value = variable_to_numerical(lhs) * variable_to_numerical(rhs);
            variable_t *new_operand = variable_numerical_new(&operand_value);
            if (storage_hash_table != NULL) {
                g_hash_table_insert(storage_hash_table, identifier, new_operand);
            } else {
                activation_record_insert(AR_parent, identifier, new_operand);
            }

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = new_operand;
            return return_struct;
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_DIVIDE_ASSIGNMENT) {
            // TODO: check existence
            variable_t *lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent)->mid_variable;
            variable_t *rhs = evaluate_token(token_get_child(expression_token, 2), AR_parent)->mid_variable;
            gdouble operand_value = variable_to_numerical(lhs) / variable_to_numerical(rhs);
            variable_t *new_operand = variable_numerical_new(&operand_value);
            if (storage_hash_table != NULL) {
                g_hash_table_insert(storage_hash_table, identifier, new_operand);
            } else {
                activation_record_insert(AR_parent, identifier, new_operand);
            }

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = new_operand;
            return return_struct;
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_MODULO_ASSIGNMENT) {
            // TODO: check existence
            variable_t *lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent)->mid_variable;
            variable_t *rhs = evaluate_token(token_get_child(expression_token, 2), AR_parent)->mid_variable;
            gdouble operand_value = ((int)variable_to_numerical(lhs)) % ((int)variable_to_numerical(rhs));
            variable_t *new_operand = variable_numerical_new(&operand_value);
            if (storage_hash_table != NULL) {
                g_hash_table_insert(storage_hash_table, identifier, new_operand);
            } else {
                activation_record_insert(AR_parent, identifier, new_operand);
            }

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = new_operand;
            return return_struct;
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_ADD_ASSIGNMENT) {
            // TODO: check existence
            variable_t *lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent)->mid_variable;
            variable_t *rhs = evaluate_token(token_get_child(expression_token, 2), AR_parent)->mid_variable;
            if ((lhs->variable_type == VARIABLE_NUMERICAL
                 || lhs->variable_type == VARIABLE_NULL
                 || lhs->variable_type == VARIABLE_BOOL)
                && (rhs->variable_type == VARIABLE_NUMERICAL
                    || rhs->variable_type == VARIABLE_NULL
                    || rhs->variable_type == VARIABLE_BOOL)) {
                gdouble operand_value = variable_to_numerical(lhs) + variable_to_numerical(rhs);
                variable_t *new_operand = variable_numerical_new(&operand_value);
                if (storage_hash_table != NULL) {
                    g_hash_table_insert(storage_hash_table, identifier, new_operand);
                } else {
                    activation_record_insert(AR_parent, identifier, new_operand);
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = new_operand;
                return return_struct;
            } else if (lhs->variable_type == VARIABLE_STRING || rhs->variable_type == VARIABLE_STRING) {
                gchar *lhs_str = variable_to_string(lhs);
                gchar *rhs_str = variable_to_string(rhs);
                strcat(lhs_str, rhs_str);
                variable_t *new_operand = variable_string_new(lhs_str);
                if (storage_hash_table != NULL) {
                    g_hash_table_insert(storage_hash_table, identifier, new_operand);
                } else {
                    activation_record_insert(AR_parent, identifier, new_operand);
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = new_operand;
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_SUBTRACT_ASSIGNMENT) {
            // TODO: check existence
            variable_t *lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent)->mid_variable;
            variable_t *rhs = evaluate_token(token_get_child(expression_token, 2), AR_parent)->mid_variable;
            gdouble operand_value = variable_to_numerical(lhs) - variable_to_numerical(rhs);
            variable_t *new_operand = variable_numerical_new(&operand_value);
            if (storage_hash_table != NULL) {
                g_hash_table_insert(storage_hash_table, identifier, new_operand);
            } else {
                activation_record_insert(AR_parent, identifier, new_operand);
            }

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = new_operand;
            return return_struct;
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_LEFT_SHIFT_ASSIGNMENT) {
            // TODO: check existence
            variable_t *lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent)->mid_variable;
            variable_t *rhs = evaluate_token(token_get_child(expression_token, 2), AR_parent)->mid_variable;
            gdouble operand_value = ((int)variable_to_numerical(lhs)) << ((int)variable_to_numerical(rhs));
            variable_t *new_operand = variable_numerical_new(&operand_value);
            if (storage_hash_table != NULL) {
                g_hash_table_insert(storage_hash_table, identifier, new_operand);
            } else {
                activation_record_insert(AR_parent, identifier, new_operand);
            }

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = new_operand;
            return return_struct;
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_SIGNED_RIGHT_SHIFT_ASSIGNMENT) {
            // TODO: check existence
            variable_t *lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent)->mid_variable;
            variable_t *rhs = evaluate_token(token_get_child(expression_token, 2), AR_parent)->mid_variable;
            gdouble operand_value = ((int)variable_to_numerical(lhs)) >> ((int)variable_to_numerical(rhs));
            variable_t *new_operand = variable_numerical_new(&operand_value);
            if (storage_hash_table != NULL) {
                g_hash_table_insert(storage_hash_table, identifier, new_operand);
            } else {
                activation_record_insert(AR_parent, identifier, new_operand);
            }

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = new_operand;
            return return_struct;
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_UNSIGNED_RIGHT_SHIFT_ASSIGNMENT) {
            // TODO: check existence
            variable_t *lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent)->mid_variable;
            variable_t *rhs = evaluate_token(token_get_child(expression_token, 2), AR_parent)->mid_variable;
            gdouble operand_value = ((gint64)variable_to_numerical(lhs)) >> ((int)variable_to_numerical(rhs));
            if (operand_value < 0) {
                operand_value = ((gint64)(2147483648ll * 2 + variable_to_numerical(lhs))) >> ((int)variable_to_numerical(rhs));
            }
            variable_t *new_operand = variable_numerical_new(&operand_value);
            if (storage_hash_table != NULL) {
                g_hash_table_insert(storage_hash_table, identifier, new_operand);
            } else {
                activation_record_insert(AR_parent, identifier, new_operand);
            }

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = new_operand;
            return return_struct;
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_BITWISE_AND_ASSIGNMENT) {
            // TODO: check existence
            variable_t *lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent)->mid_variable;
            variable_t *rhs = evaluate_token(token_get_child(expression_token, 2), AR_parent)->mid_variable;
            gdouble operand_value = ((int)variable_to_numerical(lhs)) & ((int)variable_to_numerical(rhs));
            variable_t *new_operand = variable_numerical_new(&operand_value);
            if (storage_hash_table != NULL) {
                g_hash_table_insert(storage_hash_table, identifier, new_operand);
            } else {
                activation_record_insert(AR_parent, identifier, new_operand);
            }

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = new_operand;
            return return_struct;
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_BITWISE_XOR_ASSIGNMENT) {
            // TODO: check existence
            variable_t *lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent)->mid_variable;
            variable_t *rhs = evaluate_token(token_get_child(expression_token, 2), AR_parent)->mid_variable;
            gdouble operand_value = ((int)variable_to_numerical(lhs)) ^ ((int)variable_to_numerical(rhs));
            variable_t *new_operand = variable_numerical_new(&operand_value);
            if (storage_hash_table != NULL) {
                g_hash_table_insert(storage_hash_table, identifier, new_operand);
            } else {
                activation_record_insert(AR_parent, identifier, new_operand);
            }

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = new_operand;
            return return_struct;
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_BITWISE_OR_ASSIGNMENT) {
            // TODO: check existence
            variable_t *lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent)->mid_variable;
            variable_t *rhs = evaluate_token(token_get_child(expression_token, 2), AR_parent)->mid_variable;
            gdouble operand_value = ((int)variable_to_numerical(lhs)) | ((int)variable_to_numerical(rhs));
            variable_t *new_operand = variable_numerical_new(&operand_value);
            if (storage_hash_table != NULL) {
                g_hash_table_insert(storage_hash_table, identifier, new_operand);
            } else {
                activation_record_insert(AR_parent, identifier, new_operand);
            }

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = new_operand;
            return return_struct;
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_CONDITIONAL_EXPRESSION) {
        token_t *condition_token = token_get_child(expression_token, 0);
        token_t *true_token = token_get_child(expression_token, 1);
        token_t *false_token = token_get_child(expression_token, 2);

        return_struct = evaluate_token(condition_token, AR_parent);

        if (return_struct->status != STAUS_NORMAL) {
            return return_struct;
        }
        if (return_struct->mid_variable->variable_type != VARIABLE_BOOL) {
            return_struct->status = STAUS_THROW;
            // TODO: exception
            exit(-1);
        }
        gboolean *condition = return_struct->mid_variable->variable_data;

        if (*condition) {
            return evaluate_token(true_token, AR_parent);
        } else {
            return evaluate_token(false_token, AR_parent);
        }

    } else if (expression_token->id == TOKEN_EXPRESSION_NEW_EXPRESSION) {
        variable_t *constructor;
        variable_t *new_object_variable = variable_object_new();
        return_struct = evaluate_token(token_get_child(expression_token, 0), AR_parent);
        if (return_struct->status != STAUS_NORMAL) {
            return return_struct;
        }

        g_assert(return_struct->mid_variable->variable_type == VARIABLE_FUNC);
        constructor = return_struct->mid_variable;

        activation_record_t *AR = activation_record_new(constructor->AR, constructor->AR->static_link);
        activation_record_declare(AR, "this");
        activation_record_insert(AR, "this", new_object_variable);
        g_hash_table_ref(new_object_variable->variable_data);

        token_t *call_argument_value_list       = token_get_child(expression_token, 1);
        token_t *call_argument_identifier_list = token_get_child((token_t *)constructor->function_token, 1);

        variable_t *arguments = variable_object_new();
        gdouble arguments_length = call_argument_value_list->children->len;

        g_hash_table_insert(arguments->variable_data, "length", variable_numerical_new(&arguments_length));

        for (guint i = 0; i < call_argument_value_list->children->len; ++i) {
            gchar *index_str = GC_malloc(sizeof(gchar) * 10);
            return_struct = evaluate_token(token_get_child(call_argument_value_list, i), AR_parent);
            if (return_struct->status != STAUS_NORMAL) {
                //TODO: exception
                exit(-1);
            }

            sprintf(index_str, "%d", i);
            g_hash_table_insert(arguments->variable_data, index_str, return_struct->mid_variable);
        }
        activation_record_declare(AR, "arguments");
        activation_record_insert(AR, "arguments", arguments);

        for (guint i = 0; i < MIN(call_argument_value_list->children->len, call_argument_identifier_list->children->len); ++i) {
            return_struct = evaluate_token(token_get_child(call_argument_value_list, i), AR_parent);
            if (return_struct->status != STAUS_NORMAL) {
                //TODO: exception
                exit(-1);
            }
            activation_record_declare(AR, identifier_get_value(token_get_child(call_argument_identifier_list, i))->str);
            activation_record_insert(AR, identifier_get_value(token_get_child(call_argument_identifier_list, i))->str, return_struct->mid_variable);
        }

        return_struct = evaluate_call_function(token_get_child((token_t *)constructor->function_token, 2), AR);
        if (return_struct->status != STAUS_NORMAL) {
            //TODO: exception
            exit(-1);
        }

        activation_record_reach_end_of_scope(AR);

        g_hash_table_insert(new_object_variable->variable_data, "__proto__", g_hash_table_lookup(constructor->variable_data, "prototype"));
        g_hash_table_ref(((variable_t *) g_hash_table_lookup(constructor->variable_data, "prototype"))->variable_data);
        g_hash_table_insert(new_object_variable->variable_data, "constructor", constructor);
        g_hash_table_ref(constructor->variable_data);
        return_struct->status = STAUS_NORMAL;
        return_struct->mid_variable = new_object_variable;

        return return_struct;
    } else if (expression_token->id == TOKEN_EXPRESSION_CALL_EXPRESSION) {
        token_t *func_token = token_get_child(expression_token, 0);
        if (func_token->id == TOKEN_LEXICAL_IDENTIFIER && strcmp("Log", identifier_get_value(func_token)->str) == 0) {
            return_struct = evaluate_token(token_get_child(token_get_child(expression_token, 1), 0), AR_parent);
            if (return_struct->status != STAUS_NORMAL) {
                return return_struct;
            }
            printf("Log: %s\n", variable_to_string(return_struct->mid_variable));
            return return_struct;
        }
        if (func_token->id == TOKEN_LEXICAL_IDENTIFIER && strcmp("eval", identifier_get_value(func_token)->str) == 0) {
            return evaluate_eval(string_literal_get_value(token_get_child(token_get_child(expression_token, 1), 0))->str,
                                 AR_parent);
        }
        if (func_token->id == TOKEN_LEXICAL_IDENTIFIER && strcmp("REPL", identifier_get_value(func_token)->str) == 0) {
            while (TRUE) {
                gchar *line = g_malloc(sizeof(gchar) * 300);
                printf(">>>");
                fgets(line, 300, stdin);
                if (strcmp(line, "exit();\n") == 0) {
                    break;
                }
                evaluate_eval(line, AR_parent);
            }
            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = NULL;
            return return_struct;
        }

        variable_t *callee_variable;
        variable_t *caller_variable;
        return_struct = evaluate_token(token_get_child(expression_token, 0), AR_parent);
        if (return_struct->status != STAUS_NORMAL) {
            return return_struct;
        }

        g_assert(return_struct->mid_variable->variable_type == VARIABLE_FUNC);
        callee_variable = return_struct->mid_variable;

        if (return_struct->end_variable != NULL) {
            caller_variable = return_struct->end_variable;
        } else {
            caller_variable = variable_new(VARIABLE_OBJECT, AR_parent->AR_hash_table, NULL);
        }
        g_assert(caller_variable->variable_type == VARIABLE_OBJECT || caller_variable->variable_type == VARIABLE_FUNC);

        activation_record_t *AR = activation_record_new(callee_variable->AR, callee_variable->AR->static_link);

        activation_record_declare(AR, "this");
        activation_record_insert(AR, "this", caller_variable);
        g_hash_table_ref(caller_variable->variable_data);

        token_t *call_argument_value_list       = token_get_child(expression_token, 1);
        token_t *call_argument_identifier_list  = token_get_child((token_t *)callee_variable->function_token, 1);

        variable_t *arguments = variable_object_new();
        gdouble arguments_length = call_argument_value_list->children->len;

        g_hash_table_insert(arguments->variable_data, "length", variable_numerical_new(&arguments_length));

        for (guint i = 0; i < call_argument_value_list->children->len; ++i) {
            gchar *index_str = GC_malloc(sizeof(gchar) * 10);
            return_struct = evaluate_token(token_get_child(call_argument_value_list, i), AR_parent);
            if (return_struct->status != STAUS_NORMAL) {
                //TODO: exception
                exit(-1);
            }

            sprintf(index_str, "%d", i);
            g_hash_table_insert(arguments->variable_data, index_str, return_struct->mid_variable);
            if (return_struct->mid_variable->variable_type == VARIABLE_FUNC || return_struct->mid_variable->variable_type == VARIABLE_OBJECT) {
                if (return_struct->mid_variable->new_flag) {
                    return_struct->mid_variable->new_flag = FALSE;
                } else {
                    g_hash_table_ref(return_struct->mid_variable->variable_data);
                }
            }
        }
        activation_record_declare(AR, "arguments");
        activation_record_insert(AR, "arguments", arguments);

        for (guint i = 0; i < MIN(call_argument_value_list->children->len, call_argument_identifier_list->children->len); ++i) {
            return_struct = evaluate_token(token_get_child(call_argument_value_list, i), AR_parent);
            if (return_struct->status != STAUS_NORMAL) {
                //TODO: exception
                exit(-1);
            }
            activation_record_declare(AR, identifier_get_value(token_get_child(call_argument_identifier_list, i))->str);
            activation_record_insert(AR, identifier_get_value(token_get_child(call_argument_identifier_list, i))->str, return_struct->mid_variable);
        }

        return_struct = evaluate_call_function(token_get_child((token_t *)callee_variable->function_token, 2), AR);
        if (return_struct->status != STAUS_NORMAL) {
            //TODO: exception
            exit(-1);
        }
        activation_record_reach_end_of_scope(AR);

        return return_struct;
    } else if (expression_token->id == TOKEN_EXPRESSION_PROPERTY_ACCESSOR) {
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent);

        if (return_struct_lhs->status == STAUS_NORMAL) {
            if (return_struct_lhs->mid_variable->variable_type == VARIABLE_STRING) {
                if (token_get_child(expression_token, 1)->id == TOKEN_LEXICAL_IDENTIFIER) {
                    if (strcmp(identifier_get_value(token_get_child(expression_token, 1))->str, "length") == 0) {
                        gdouble len = strlen(return_struct_lhs->mid_variable->variable_data);
                        return_struct->status = STAUS_NORMAL;
                        return_struct->mid_variable = variable_numerical_new(&len);

                        return return_struct;
                    } else {
                        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 1),
                                                                            AR_parent);

                        if (return_struct_rhs->status != STAUS_NORMAL) {
                            //TODO: exception
                            exit(-1);
                        } else {
                            variable_t *rhs = return_struct_rhs->mid_variable;
                            if (rhs->variable_type == VARIABLE_NUMERICAL) {
                                gdouble len = strlen(return_struct_lhs->mid_variable->variable_data);
                                gdouble index = *(gdouble *) rhs->variable_data;
                                if (index < 0 || index >= len) {
                                    printf("Index out of range");
                                    exit(-1);
                                }

                                gchar *str = return_struct_lhs->mid_variable->variable_data;
                                gchar ret_str[2];
                                ret_str[0] = str[(guint) index];
                                ret_str[1] = '\0';

                                return_struct->status = STAUS_NORMAL;
                                return_struct->mid_variable = variable_string_new(ret_str);
                                return return_struct;
                            } else {
                                // TODO: exception
                                exit(-1);
                            }
                        }
                    }
                } else {
                    return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 1),
                                                                        AR_parent);

                    if (return_struct_rhs->status != STAUS_NORMAL) {
                        //TODO: exception
                        exit(-1);
                    } else {
                        variable_t *rhs = return_struct_rhs->mid_variable;
                        if (rhs->variable_type == VARIABLE_NUMERICAL) {
                            gdouble len = strlen(return_struct_lhs->mid_variable->variable_data);
                            gdouble index = *(gdouble *) rhs->variable_data;
                            if (index < 0 || index >= len) {
                                printf("Index out of range");
                                exit(-1);
                            }

                            gchar *str = return_struct_lhs->mid_variable->variable_data;
                            gchar ret_str[2];
                            ret_str[0] = str[(guint) index];
                            ret_str[1] = '\0';

                            return_struct->status = STAUS_NORMAL;
                            return_struct->mid_variable = variable_string_new(ret_str);
                            return return_struct;
                        } else {
                            // TODO: exception
                            exit(-1);
                        }
                    }
                }
            }
        }

        if (return_struct_lhs->status == STAUS_NORMAL) {
            g_assert(return_struct_lhs->mid_variable->variable_type == VARIABLE_OBJECT || return_struct_lhs->mid_variable->variable_type == VARIABLE_FUNC );

            return_struct->mid_variable = variable_object_lookup(return_struct_lhs->mid_variable, token_get_child(expression_token, 1), AR_parent);
            if (return_struct->mid_variable != NULL) {
                return_struct->status = STAUS_NORMAL;
                return_struct->end_variable = return_struct_lhs->mid_variable;
                return return_struct;
            } else {
                // TODO: Return undefined
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_undefined_new();
                return return_struct;
            }
        } else if (return_struct_lhs->status == STAUS_THROW) {
            // TODO: handel exception
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_ARRAY_LITERAL) {
        variable_t *array_varialbe = variable_object_new();

        gdouble *array_length = GC_malloc(sizeof(gdouble));
        *array_length = expression_token->children->len;
        g_hash_table_insert(array_varialbe->variable_data, "length", variable_numerical_new(array_length));
        g_hash_table_insert(array_varialbe->variable_data, "__proto__", Array_prototype);
        g_hash_table_ref(Array_prototype->variable_data);

        for (guint i = 0; i < expression_token->children->len; ++i) {
            gchar *index_str = GC_malloc(sizeof(gchar) * 10);
            token_t *array_item_token = token_get_child(expression_token, i);
            sprintf(index_str, "%d", i);

            return_struct = evaluate_token(array_item_token, AR_parent);
            if (return_struct->status != STAUS_NORMAL) {
                return return_struct;
            }

            g_hash_table_insert(array_varialbe->variable_data, index_str, return_struct->mid_variable);
            if (return_struct->mid_variable->variable_type == VARIABLE_FUNC || return_struct->mid_variable->variable_type == VARIABLE_OBJECT) {
                if (return_struct->mid_variable->new_flag) {
                    return_struct->mid_variable->new_flag = FALSE;
                } else {
                    g_hash_table_ref(return_struct->mid_variable->variable_data);
                }
            }
        }

        return_struct->status = STAUS_NORMAL;
        return_struct->mid_variable = array_varialbe;

        return return_struct;
    } else if (expression_token->id == TOKEN_EXPRESSION_OBJECT_LITERAL) {
        variable_t *object_varialbe = variable_object_new();

        for (guint i = 0; i < expression_token->children->len; ++i) {
            token_t *attribute_token = token_get_child(expression_token, i);

            token_t *lhs = token_get_child(attribute_token, 0);
            return_struct_t *return_struct_rhs = evaluate_token(token_get_child(attribute_token, 1), AR_parent);

            if (return_struct_rhs->status == STAUS_NORMAL) {
                variable_object_insert(object_varialbe, lhs, return_struct_rhs->mid_variable);
            } else {
                // TODO: handel exception
            }
        }

        return_struct->status = STAUS_NORMAL;
        return_struct->mid_variable = object_varialbe;

        return return_struct;
    } else if (expression_token->id == TOKEN_EXPRESSION_ADDITIVE_EXPRESSION) {
        g_assert(expression_token->children->len == 3);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 2), AR_parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if ((lhs->variable_type == VARIABLE_NUMERICAL
             || lhs->variable_type == VARIABLE_NULL
             || lhs->variable_type == VARIABLE_BOOL)
            && (rhs->variable_type == VARIABLE_NUMERICAL
             || rhs->variable_type == VARIABLE_NULL
             || rhs->variable_type == VARIABLE_BOOL)) {
            gdouble lhs_double = variable_to_numerical(lhs);
            gdouble rhs_double = variable_to_numerical(rhs);
            gdouble result;
            if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_PLUS) {
                result = lhs_double + rhs_double;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_MINUS) {
                result = lhs_double - rhs_double;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            }
        } else if (lhs->variable_type == VARIABLE_STRING || rhs->variable_type == VARIABLE_STRING) {
            if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_PLUS) {
                gchar* lhs_str = variable_to_string(lhs);
                gchar* rhs_str = variable_to_string(rhs);
                strcat(lhs_str, rhs_str);
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_string_new(lhs_str);
                return return_struct;
            } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_MINUS) {
                gdouble lhs_double = variable_to_numerical(lhs);
                gdouble rhs_double = variable_to_numerical(rhs);
                gdouble result;
                result = lhs_double - rhs_double;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            }
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_MULTIPLICATIVE_EXPRESSION) {
        g_assert(expression_token->children->len == 3);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 2), AR_parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if ((lhs->variable_type == VARIABLE_NUMERICAL
             || lhs->variable_type == VARIABLE_NULL
             || lhs->variable_type == VARIABLE_BOOL
             || lhs->variable_type == VARIABLE_STRING)
            && (rhs->variable_type == VARIABLE_NUMERICAL
                || rhs->variable_type == VARIABLE_NULL
                || rhs->variable_type == VARIABLE_BOOL
                || rhs->variable_type == VARIABLE_STRING)) {
            gdouble lhs_double = variable_to_numerical(lhs);
            gdouble rhs_double = variable_to_numerical(rhs);
            gdouble result;
            if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_SLASH) {
                result = lhs_double / rhs_double;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_ASTERISK) {
                result = lhs_double * rhs_double;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_PERCENT) {
                gint lhs_int = (int)(lhs_double);
                gint rhs_int = (int)(rhs_double);

                result = lhs_int % rhs_int;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            }
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_SHIFT_EXPRESSION) {
        g_assert(expression_token->children->len == 3);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 2), AR_parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if ((lhs->variable_type == VARIABLE_NUMERICAL
             || lhs->variable_type == VARIABLE_NULL
             || lhs->variable_type == VARIABLE_BOOL
             || lhs->variable_type == VARIABLE_STRING)
            && (rhs->variable_type == VARIABLE_NUMERICAL
                || rhs->variable_type == VARIABLE_NULL
                || rhs->variable_type == VARIABLE_BOOL
                || rhs->variable_type == VARIABLE_STRING)) {
            gdouble *lhs_value = lhs->variable_data;
            gdouble *rhs_value = rhs->variable_data;
            gint lhs_value_int = (int)(*lhs_value);
            gint rhs_value_int = (int)(*rhs_value);
            gdouble result;
            if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_LEFT_SHIFT) {
                result = lhs_value_int << rhs_value_int;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_SIGNED_RIGHT_SHIFT) {
                result = lhs_value_int >> rhs_value_int;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_UNSIGNED_RIGHT_SHIFT) {
                result = lhs_value_int >> rhs_value_int;
                if (result<0) {
                    result = ((gint64)(2147483648ll * 2 + variable_to_numerical(lhs))) >> ((int)variable_to_numerical(rhs));
                }
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            }
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_EQUALITY_EXPRESSION) {
        g_assert(expression_token->children->len == 3);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 2), AR_parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_EQUALS) {
             if ((lhs->variable_type == VARIABLE_NUMERICAL
                    || lhs->variable_type == VARIABLE_STRING
                    || lhs->variable_type == VARIABLE_BOOL
                    || lhs->variable_type == VARIABLE_UNDEFINED)
                && (rhs->variable_type == VARIABLE_NUMERICAL
                    || rhs->variable_type == VARIABLE_STRING
                    || rhs->variable_type == VARIABLE_BOOL
                    || rhs->variable_type == VARIABLE_UNDEFINED)) {
                gchar* lhs_str = variable_to_string(lhs);
                gchar* rhs_str = variable_to_string(rhs);
                gboolean result;

                if (g_strcmp0(lhs_str, rhs_str)==0) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_DOES_NOT_EQUAL) {
            if ((lhs->variable_type == VARIABLE_NUMERICAL
                 || lhs->variable_type == VARIABLE_STRING
                 || lhs->variable_type == VARIABLE_BOOL
                 || lhs->variable_type == VARIABLE_UNDEFINED)
                && (rhs->variable_type == VARIABLE_NUMERICAL
                    || rhs->variable_type == VARIABLE_STRING
                    || rhs->variable_type == VARIABLE_BOOL
                    || rhs->variable_type == VARIABLE_UNDEFINED)) {
                gchar* lhs_str = variable_to_string(lhs);
                gchar* rhs_str = variable_to_string(rhs);
                gboolean result;

                if (g_strcmp0(lhs_str, rhs_str)==0) {
                    result = FALSE;
                } else {
                    result = TRUE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_STRICT_EQUALS) {
            if ((lhs->variable_type == VARIABLE_NUMERICAL
                 || lhs->variable_type == VARIABLE_STRING
                 || lhs->variable_type == VARIABLE_BOOL
                 || lhs->variable_type == VARIABLE_UNDEFINED)
                && (rhs->variable_type == VARIABLE_NUMERICAL
                    || rhs->variable_type == VARIABLE_STRING
                    || rhs->variable_type == VARIABLE_BOOL
                    || rhs->variable_type == VARIABLE_UNDEFINED)
                && lhs->variable_type == rhs->variable_type) {
                gchar* lhs_str = variable_to_string(lhs);
                gchar* rhs_str = variable_to_string(rhs);
                gboolean result;

                if (g_strcmp0(lhs_str, rhs_str)==0) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_STRICT_DOES_NOT_EQUAL) {
            if ((lhs->variable_type == VARIABLE_NUMERICAL
                 || lhs->variable_type == VARIABLE_STRING
                 || lhs->variable_type == VARIABLE_BOOL
                 || lhs->variable_type == VARIABLE_UNDEFINED)
                && (rhs->variable_type == VARIABLE_NUMERICAL
                    || rhs->variable_type == VARIABLE_STRING
                    || rhs->variable_type == VARIABLE_BOOL
                    || rhs->variable_type == VARIABLE_UNDEFINED)) {
                gchar* lhs_str = variable_to_string(lhs);
                gchar* rhs_str = variable_to_string(rhs);
                gboolean result;

                if (g_strcmp0(lhs_str, rhs_str)==0 && lhs->variable_type == rhs->variable_type) {
                    result = FALSE;
                } else {
                    result = TRUE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_UNARY_EXPRESSION) {
        g_assert(expression_token->children->len == 2);
//        // TODO: Is the child 1 always a identifier?   Nope. e.g. -true;  -zyy
//        g_assert(token_get_child(expression_token, 1)->id == TOKEN_LEXICAL_IDENTIFIER);

        return_struct_t *return_struct_operand = evaluate_token(token_get_child(expression_token, 1), AR_parent);
        variable_t *operand = return_struct_operand->mid_variable;
        // TODO: check return status
        if (*punctuator_get_id(token_get_child(expression_token, 0)) == PUNCTUATOR_INCREMENT) {
            if (token_get_child(expression_token, 1)->id == TOKEN_EXPRESSION_PROPERTY_ACCESSOR) {
                gchar *identifier;
                GHashTable *storage_hash_table;
                resolve_assignment_identifier(token_get_child(expression_token, 1), AR_parent, &identifier,
                                              &storage_hash_table);

                // TODO: check existence
                variable_t *operand = evaluate_token(token_get_child(expression_token, 1), AR_parent)->mid_variable;
                gdouble operand_value = variable_to_numerical(operand) + 1;
                variable_t *new_operand = variable_numerical_new(&operand_value);
                if (storage_hash_table != NULL) {
                    g_hash_table_insert(storage_hash_table, identifier, new_operand);
                } else {
                    activation_record_insert(AR_parent, identifier, new_operand);
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = new_operand;
                return return_struct;
            }

            if (operand->variable_type == VARIABLE_NUMERICAL
                || operand->variable_type == VARIABLE_NULL
                || operand->variable_type == VARIABLE_BOOL
                || operand->variable_type == VARIABLE_STRING) {
                gdouble operand_double = variable_to_numerical(operand);
                gdouble result;
                result = (operand_double) + 1;

                variable_t *result_variable = variable_numerical_new(&result);

                activation_record_insert(AR_parent, identifier_get_value(token_get_child(expression_token, 1))->str, result_variable);

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = result_variable;
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 0)) == PUNCTUATOR_DECREMENT) {
            if (token_get_child(expression_token, 1)->id == TOKEN_EXPRESSION_PROPERTY_ACCESSOR) {
                gchar *identifier;
                GHashTable *storage_hash_table;
                resolve_assignment_identifier(token_get_child(expression_token, 1), AR_parent, &identifier,
                                              &storage_hash_table);

                // TODO: check existence
                variable_t *operand = evaluate_token(token_get_child(expression_token, 1), AR_parent)->mid_variable;
                gdouble operand_value = variable_to_numerical(operand) - 1;
                variable_t *new_operand = variable_numerical_new(&operand_value);
                if (storage_hash_table != NULL) {
                    g_hash_table_insert(storage_hash_table, identifier, new_operand);
                } else {
                    activation_record_insert(AR_parent, identifier, new_operand);
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = new_operand;
                return return_struct;
            }

            if (operand->variable_type == VARIABLE_NUMERICAL
                || operand->variable_type == VARIABLE_NULL
                || operand->variable_type == VARIABLE_BOOL
                || operand->variable_type == VARIABLE_STRING) {
                gdouble operand_double = variable_to_numerical(operand);
                gdouble result;
                result = (operand_double) - 1;

                variable_t *result_variable = variable_numerical_new(&result);

                activation_record_insert(AR_parent, identifier_get_value(token_get_child(expression_token, 1))->str, result_variable);

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 0)) == PUNCTUATOR_PLUS) {
            if (operand->variable_type == VARIABLE_NUMERICAL
                || operand->variable_type == VARIABLE_NULL
                || operand->variable_type == VARIABLE_BOOL
                || operand->variable_type == VARIABLE_STRING) {
                gdouble operand_double = variable_to_numerical(operand);
                gdouble result;
                result = operand_double;

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 0)) == PUNCTUATOR_MINUS) {
            if (operand->variable_type == VARIABLE_NUMERICAL
                || operand->variable_type == VARIABLE_NULL
                || operand->variable_type == VARIABLE_BOOL
                || operand->variable_type == VARIABLE_STRING) {
                gdouble operand_double = variable_to_numerical(operand);
                gdouble result;
                result = -operand_double;

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 0)) == PUNCTUATOR_TILDE) { //TODO: from here
            if (operand->variable_type == VARIABLE_NUMERICAL
                || operand->variable_type == VARIABLE_NULL
                || operand->variable_type == VARIABLE_BOOL
                || operand->variable_type == VARIABLE_STRING) {
                gdouble operand_double = variable_to_numerical(operand);
                gdouble result;
                gint operand_int = (int)(operand_double);
                result = ~operand_int;

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 0)) == PUNCTUATOR_EXCLAMATION) {
            if (operand->variable_type == VARIABLE_NUMERICAL
                || operand->variable_type == VARIABLE_STRING
                || operand->variable_type == VARIABLE_NULL) {
                gdouble operand_double = variable_to_numerical(operand);
                gdouble result;
                result = !(operand_double);

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            } else if (operand->variable_type == VARIABLE_BOOL) {
                gboolean  *operand_value = operand->variable_data;
                gboolean result;
                result = !(*operand_value);

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_POSTFIX_EXPRESSION) {
        g_assert(expression_token->children->len == 2);
        if (token_get_child(expression_token, 0)->id == TOKEN_EXPRESSION_PROPERTY_ACCESSOR) {
            gchar *identifier;
            GHashTable *storage_hash_table;
            resolve_assignment_identifier(token_get_child(expression_token, 0), AR_parent, &identifier,
                                          &storage_hash_table);

            // TODO: check existence
            variable_t *operand = evaluate_token(token_get_child(expression_token, 0), AR_parent)->mid_variable;
            gdouble operand_value = 0.0;
            if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_INCREMENT) {
                operand_value = variable_to_numerical(operand) + 1;
            } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_DECREMENT) {
                operand_value = variable_to_numerical(operand) - 1;
            }

            variable_t *new_operand = variable_numerical_new(&operand_value);
            if (storage_hash_table != NULL) {
                g_hash_table_insert(storage_hash_table, identifier, new_operand);
            } else {
                activation_record_insert(AR_parent, identifier, new_operand);
            }

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = operand;
            return return_struct;
        }

        return_struct_t *return_struct_operand = evaluate_token(token_get_child(expression_token, 0), AR_parent);
        variable_t *operand = return_struct_operand->mid_variable;
        // TODO: check return status
        if (operand->variable_type == VARIABLE_NUMERICAL
            || operand->variable_type == VARIABLE_NULL
            || operand->variable_type == VARIABLE_BOOL
            || operand->variable_type == VARIABLE_STRING) {
            gdouble operand_value = variable_to_numerical(operand);
            gdouble result;
            if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_INCREMENT) {
                result = operand_value + 1;

                variable_t *result_variable = variable_numerical_new(&result);

                activation_record_insert(AR_parent, identifier_get_value(token_get_child(expression_token, 0))->str, result_variable);

                result = result - 1;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_DECREMENT) {
                result = operand_value - 1;

                variable_t *result_variable = variable_numerical_new(&result);

                activation_record_insert(AR_parent, identifier_get_value(token_get_child(expression_token, 0))->str, result_variable);

                result = result + 1;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_numerical_new(&result);
                return return_struct;
            }
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_RELATIONAL_EXPRESSION) {
        g_assert(expression_token->children->len == 3);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 2), AR_parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_ANGLE_BRACKET_LEFT) {

            if (lhs->variable_type == VARIABLE_STRING
                && rhs->variable_type == VARIABLE_STRING) {
                gchar* lhs_str = variable_to_string(lhs);
                gchar* rhs_str = variable_to_string(rhs);
                gboolean result;

                if (g_strcmp0(lhs_str, rhs_str)<0) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            } else if ((lhs->variable_type == VARIABLE_NUMERICAL
                        || lhs->variable_type == VARIABLE_STRING
                        || lhs->variable_type == VARIABLE_BOOL)
                       && (rhs->variable_type == VARIABLE_NUMERICAL
                           || rhs->variable_type == VARIABLE_STRING
                           || rhs->variable_type == VARIABLE_BOOL)) {
                gdouble lhs_double = variable_to_numerical(lhs);
                gdouble rhs_double = variable_to_numerical(rhs);
                gboolean result;

                if (lhs_double < rhs_double) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            } else if (lhs->variable_type == VARIABLE_UNDEFINED
                       && rhs->variable_type == VARIABLE_UNDEFINED)
            {
                gboolean result = FALSE;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_ANGLE_BRACKET_RIGHT) {
            if (lhs->variable_type == VARIABLE_STRING
                && rhs->variable_type == VARIABLE_STRING) {
                gchar* lhs_str = variable_to_string(lhs);
                gchar* rhs_str = variable_to_string(rhs);
                gboolean result;

                if (g_strcmp0(lhs_str, rhs_str)>0) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            } else if ((lhs->variable_type == VARIABLE_NUMERICAL
                        || lhs->variable_type == VARIABLE_STRING
                        || lhs->variable_type == VARIABLE_BOOL)
                       && (rhs->variable_type == VARIABLE_NUMERICAL
                           || rhs->variable_type == VARIABLE_STRING
                           || rhs->variable_type == VARIABLE_BOOL)) {
                gdouble lhs_double = variable_to_numerical(lhs);
                gdouble rhs_double = variable_to_numerical(rhs);
                gboolean result;

                if (lhs_double > rhs_double) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            } else if (lhs->variable_type == VARIABLE_UNDEFINED
                      && rhs->variable_type == VARIABLE_UNDEFINED)
            {
                gboolean result = FALSE;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_LESS_THAN_OR_EQUAL) {
            if (lhs->variable_type == VARIABLE_STRING
                && rhs->variable_type == VARIABLE_STRING) {
                gchar* lhs_str = variable_to_string(lhs);
                gchar* rhs_str = variable_to_string(rhs);
                gboolean result;

                if (g_strcmp0(lhs_str, rhs_str)<=0) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            } else if ((lhs->variable_type == VARIABLE_NUMERICAL
                        || lhs->variable_type == VARIABLE_STRING
                        || lhs->variable_type == VARIABLE_BOOL)
                       && (rhs->variable_type == VARIABLE_NUMERICAL
                           || rhs->variable_type == VARIABLE_STRING
                           || rhs->variable_type == VARIABLE_BOOL)) {
                gdouble lhs_double = variable_to_numerical(lhs);
                gdouble rhs_double = variable_to_numerical(rhs);
                gboolean result;

                if (lhs_double <= rhs_double) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            } else if (lhs->variable_type == VARIABLE_UNDEFINED
                       && rhs->variable_type == VARIABLE_UNDEFINED)
            {
                gboolean result = FALSE;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        } else if (*punctuator_get_id(token_get_child(expression_token, 1)) == PUNCTUATOR_GREATER_THAN_OR_EQUAL) {
            if (lhs->variable_type == VARIABLE_STRING
                && rhs->variable_type == VARIABLE_STRING) {
                gchar* lhs_str = variable_to_string(lhs);
                gchar* rhs_str = variable_to_string(rhs);
                gboolean result;

                if (g_strcmp0(lhs_str, rhs_str)>=0) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            } else if ((lhs->variable_type == VARIABLE_NUMERICAL
                        || lhs->variable_type == VARIABLE_STRING
                        || lhs->variable_type == VARIABLE_BOOL)
                       && (rhs->variable_type == VARIABLE_NUMERICAL
                           || rhs->variable_type == VARIABLE_STRING
                           || rhs->variable_type == VARIABLE_BOOL)) {
                gdouble lhs_double = variable_to_numerical(lhs);
                gdouble rhs_double = variable_to_numerical(rhs);
                gboolean result;

                if (lhs_double >= rhs_double) {
                    result = TRUE;
                } else {
                    result = FALSE;
                }

                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            } else if (lhs->variable_type == VARIABLE_UNDEFINED
                       && rhs->variable_type == VARIABLE_UNDEFINED)
            {
                gboolean result = FALSE;
                return_struct->status = STAUS_NORMAL;
                return_struct->mid_variable = variable_bool_new(&result);
                return return_struct;
            }
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_LOGICAL_AND_EXPRESSION) {
        g_assert(expression_token->children->len == 2);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 1), AR_parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if ((lhs->variable_type == VARIABLE_NUMERICAL
             || lhs->variable_type == VARIABLE_BOOL
             || lhs->variable_type == VARIABLE_NULL
             || lhs->variable_type == VARIABLE_STRING)
            && (rhs->variable_type == VARIABLE_NUMERICAL
                || rhs->variable_type == VARIABLE_BOOL
                || rhs->variable_type == VARIABLE_NULL
                || rhs->variable_type == VARIABLE_STRING)) {
            gdouble  lhs_value = variable_to_numerical(lhs);
            gdouble  rhs_value = variable_to_numerical(rhs);
            gboolean result;
            result = (lhs_value) && (rhs_value);

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = variable_bool_new(&result);
            return return_struct;
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_LOGICAL_OR_EXPRESSION) {
        g_assert(expression_token->children->len == 2);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 1), AR_parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if ((lhs->variable_type == VARIABLE_NUMERICAL
             || lhs->variable_type == VARIABLE_BOOL
             || lhs->variable_type == VARIABLE_NULL
             || lhs->variable_type == VARIABLE_STRING)
            && (rhs->variable_type == VARIABLE_NUMERICAL
                || rhs->variable_type == VARIABLE_BOOL
                || rhs->variable_type == VARIABLE_NULL
                || rhs->variable_type == VARIABLE_STRING)) {
            gdouble  lhs_value = variable_to_numerical(lhs);
            gdouble  rhs_value = variable_to_numerical(rhs);
            gboolean result;
            result = (lhs_value) || (rhs_value);

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = variable_bool_new(&result);
            return return_struct;
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_BITWISE_AND_EXPRESSION) {
        g_assert(expression_token->children->len == 2);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 1), AR_parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if ((lhs->variable_type == VARIABLE_NUMERICAL
             || lhs->variable_type == VARIABLE_BOOL
             || lhs->variable_type == VARIABLE_NULL
             || lhs->variable_type == VARIABLE_STRING)
            && (rhs->variable_type == VARIABLE_NUMERICAL
                || rhs->variable_type == VARIABLE_BOOL
                || rhs->variable_type == VARIABLE_NULL
                || rhs->variable_type == VARIABLE_STRING)) {
            gdouble  lhs_value = variable_to_numerical(lhs);
            gdouble  rhs_value = variable_to_numerical(rhs);
            gint lhs_int = (int)(lhs_value);
            gint rhs_int = (int)(rhs_value);
            gdouble result;
            result = lhs_int & rhs_int;

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = variable_numerical_new(&result);
            return return_struct;
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_BITWISE_OR_EXPRESSION) {
        g_assert(expression_token->children->len == 2);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 1), AR_parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if ((lhs->variable_type == VARIABLE_NUMERICAL
             || lhs->variable_type == VARIABLE_BOOL
             || lhs->variable_type == VARIABLE_NULL
             || lhs->variable_type == VARIABLE_STRING)
            && (rhs->variable_type == VARIABLE_NUMERICAL
                || rhs->variable_type == VARIABLE_BOOL
                || rhs->variable_type == VARIABLE_NULL
                || rhs->variable_type == VARIABLE_STRING)) {
            gdouble  lhs_value = variable_to_numerical(lhs);
            gdouble  rhs_value = variable_to_numerical(rhs);
            gint lhs_int = (int)(lhs_value);
            gint rhs_int = (int)(rhs_value);
            gdouble result;
            result = lhs_int | rhs_int;

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = variable_numerical_new(&result);
            return return_struct;
        }
    } else if (expression_token->id == TOKEN_EXPRESSION_BITWISE_XOR_EXPRESSION) {
        g_assert(expression_token->children->len == 2);
        return_struct_t *return_struct_lhs = evaluate_token(token_get_child(expression_token, 0), AR_parent);
        variable_t *lhs = return_struct_lhs->mid_variable;
        // TODO: check return status
        return_struct_t *return_struct_rhs = evaluate_token(token_get_child(expression_token, 1), AR_parent);
        variable_t *rhs = return_struct_rhs->mid_variable;
        // TODO: check return status
        if ((lhs->variable_type == VARIABLE_NUMERICAL
             || lhs->variable_type == VARIABLE_BOOL
             || lhs->variable_type == VARIABLE_NULL
             || lhs->variable_type == VARIABLE_STRING)
            && (rhs->variable_type == VARIABLE_NUMERICAL
                || rhs->variable_type == VARIABLE_BOOL
                || rhs->variable_type == VARIABLE_NULL
                || rhs->variable_type == VARIABLE_STRING)) {
            gdouble  lhs_value = variable_to_numerical(lhs);
            gdouble  rhs_value = variable_to_numerical(rhs);
            gint lhs_int = (int)(lhs_value);
            gint rhs_int = (int)(rhs_value);
            gdouble result;
            result = lhs_int ^ rhs_int;

            return_struct->status = STAUS_NORMAL;
            return_struct->mid_variable = variable_numerical_new(&result);
            return return_struct;
        }
    }

    return_struct->status = STAUS_THROW;
    // TODO: handel exception
    return return_struct;
}

return_struct_t *resolve_assignment_identifier(token_t *lhs_token, activation_record_t *AR, gchar **identifier,
                                               GHashTable **storage_hash_table) {
    return_struct_t *return_struct = return_struct_new();

    if (lhs_token->id == TOKEN_LEXICAL_IDENTIFIER) {
        *identifier = identifier_get_value(lhs_token)->str;
        *storage_hash_table = NULL;

        return_struct->status = STAUS_NORMAL;
        return return_struct;
    } else if (lhs_token->id == TOKEN_EXPRESSION_PROPERTY_ACCESSOR) {
        token_t *object_token = token_get_child(lhs_token, 0);
        token_t *identifier_token = token_get_child(lhs_token, 1);
        return_struct_t *object_return_struct = evaluate_token(object_token, AR);
        if (object_return_struct->status == STAUS_NORMAL) {
            g_assert(object_return_struct->mid_variable->variable_type == VARIABLE_OBJECT || object_return_struct->mid_variable->variable_type == VARIABLE_FUNC);

            if (identifier_token->id == TOKEN_LEXICAL_IDENTIFIER) {
                *identifier = identifier_get_value(identifier_token)->str;
            } else {
                return_struct_t *identifier_return_struct = evaluate_token(identifier_token, AR);
                if (identifier_return_struct->status == STAUS_NORMAL) {
                    *identifier = variable_to_string(identifier_return_struct->mid_variable);
                } else {
                    return_struct->status = STAUS_THROW;
                    // TODO: handel exception
                    return return_struct;
                }
            }
            *storage_hash_table = object_return_struct->mid_variable->variable_data;
        }
    }

    return_struct->status = STAUS_THROW;
    // TODO: handel exception
    return return_struct;
}

gboolean need_return_to_invoker(return_struct_t *return_struct) {
    return return_struct->status == STAUS_THROW     ||
           return_struct->status == STAUS_CONTINUE  ||
           return_struct->status == STAUS_BREAK     ||
           return_struct->status == STAUS_RETURN;
}

variable_t *generate_function_variable(token_t *function_token, activation_record_t *AR_parent) {
    variable_t *function_variable = variable_function_new(function_token, AR_parent);
    g_hash_table_ref(AR_parent->AR_hash_table);

    return function_variable;
}

return_struct_t *evaluate_function(token_t *function_token, activation_record_t *AR_parent) {
    return_struct_t *return_struct = return_struct_new();
    variable_t *function_variable = generate_function_variable(function_token, AR_parent);

    return_struct->status = STAUS_NORMAL;
    return_struct->mid_variable = function_variable;

    if (function_token->id == TOKEN_FUNCTION_FUNCTION_DECLARATION) {
        activation_record_insert(AR_parent, identifier_get_value(token_get_child(function_token, 0))->str, function_variable);
    }

    return return_struct;
}

return_struct_t *evaluate_call_function(token_t *function_body_token, activation_record_t *AR_parent) {
    return_struct_t *return_struct = return_struct_new();
    for (guint i  = 0; i < function_body_token->children->len; ++i) {
        return_struct = evaluate_token(token_get_child(function_body_token, i), AR_parent);
        if (return_struct->mid_variable != NULL) {
//            printf("%s\n", variable_to_string(return_struct->mid_variable));
        }

        if (return_struct->status == STAUS_RETURN)
            break;
        if (return_struct->status != STAUS_NORMAL)
            return return_struct;
    }

    return_struct->status = STAUS_NORMAL;
    return return_struct;
}

void init_builtin(activation_record_t *AR) {
    Object      = NULL;
    Function    = NULL;
    Array       = NULL;
    Objcet_prototype    = NULL;
    Function_prototype  = NULL;
    Array_prototype     = NULL;

    FILE *fi = fopen("./builtin.js", "r");

    char *input = (char *) g_malloc(sizeof(char) * 5000);
    size_t input_length = 0;
    int data;
    while((data = fgetc(fi)) != EOF) {
        input[input_length++] = data;
    }
    input[input_length] = '\0';

    if (errno) {
        perror("getline");
        g_free(input);
    } else {
        char *input_malloc = input;
        input = g_strdup(input);
        free(input_malloc);
        input_malloc = NULL;
    }

    if (!lexical_parse_normalize_input(&input)) {
        fprintf(stderr, "lexical_parse_normalize_input: error");
        g_free(input);
    }

    token_t *lexical_error = NULL;
    GPtrArray *token_list = lexical_parse(input, &lexical_error);
    g_free(input);
    if (lexical_error) {
        GString *error_string = token_to_string(lexical_error);
        fprintf(stderr, "lexical_parse: %s", error_string->str);
        g_string_free(error_string, TRUE);
        token_free(&lexical_error);
    }

    token_t *program_or_error = syntactic_parse(token_list);
    token_list_free(&token_list);
    if (error_is_error(program_or_error)) {
        GString *error_string = token_to_string(program_or_error);
        fprintf(stderr, "syntactic_parse: %s", error_string->str);
        g_string_free(error_string, TRUE);
        token_free(&program_or_error);
    }

    evaluate_init(program_or_error, AR);

    data_tokens_list = g_list_append(data_tokens_list, program_or_error);

    Object      = activation_record_lookup(AR, "Object");
    Function    = activation_record_lookup(AR, "Function");
    Array       = activation_record_lookup(AR, "Array");

//    printf("%p\n", Object);
//    printf("%p\n", Function);
//    printf("%p\n", Array);

    Objcet_prototype     = g_hash_table_lookup(Object->variable_data,   "prototype");
    Function_prototype   = g_hash_table_lookup(Function->variable_data, "prototype");
    Array_prototype      = g_hash_table_lookup(Array->variable_data,    "prototype");

    g_hash_table_insert(Object->variable_data, "__proto__", NULL);
    g_hash_table_insert(Objcet_prototype->variable_data, "__proto__", NULL);

    g_hash_table_insert(Function->variable_data, "__proto__", NULL);
    g_hash_table_insert(Function_prototype->variable_data, "__proto__", Objcet_prototype);
    g_hash_table_ref(Objcet_prototype->variable_data);

    g_hash_table_insert(Array->variable_data, "__proto__", Function);
    g_hash_table_insert(Array_prototype->variable_data, "__proto__", Objcet_prototype);
    g_hash_table_ref(Function->variable_data);
    g_hash_table_ref(Objcet_prototype->variable_data);

//    printf("%s\n", variable_to_string(Object));
//    printf("%s\n", variable_to_string(Function));
//    printf("%s\n", variable_to_string(Array));
//
//    exit(-1);
}

return_struct_t *evaluate_init(token_t *program_token, activation_record_t *AR_parent) {
    return_struct_t *return_struct = return_struct_new();
    activation_record_t *AR = AR_parent;

    for (guint i  = 0; i < program_token->children->len; ++i) {
        return_struct_t *return_struct;
        return_struct = evaluate_token(token_get_child(program_token, i), AR);
        if (return_struct->mid_variable != NULL) {
//            printf("mid: %s ", variable_to_string(return_struct->mid_variable));
        }
        if (return_struct->end_variable != NULL) {
//            printf("end: %s\n", variable_to_string(return_struct->end_variable));
        } else {
//            printf("\n");
        }
    }

    return_struct->status = STAUS_NORMAL;
    return return_struct;
}

return_struct_t *evaluate_eval(gchar *eval_code, activation_record_t *AR_parent) {
    token_t *lexical_error = NULL;
    GPtrArray *token_list = lexical_parse(eval_code, &lexical_error);
    if (lexical_error) {
        GString *error_string = token_to_string(lexical_error);
        fprintf(stderr, "lexical_parse: %s", error_string->str);
        g_string_free(error_string, TRUE);
        token_free(&lexical_error);
    }

    token_t *program_or_error = syntactic_parse(token_list);
    token_list_free(&token_list);
    if (error_is_error(program_or_error)) {
        GString *error_string = token_to_string(program_or_error);
        fprintf(stderr, "syntactic_parse: %s", error_string->str);
        g_string_free(error_string, TRUE);
        token_free(&program_or_error);
    }

    return_struct_t *return_struct = evaluate_eval_call(program_or_error, AR_parent);
    data_tokens_list = g_list_append(data_tokens_list, program_or_error);

    return return_struct;
}
