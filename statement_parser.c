//
// Copyright (c) 2015 Zhang Hai <Dreaming.in.Code.ZH@Gmail.com>
// All Rights Reserved.
//

#include "statement_parser.h"

#include "expression_parser.h"
#include "lexical_parser.h"
#include "syntactic_parser_utils.h"

/*
 * AST:
 * TODO
 *
 * GRAMMAR:
 * NONSTANDARD: No EmptyStatement or WithStatement
 * Statement :
 *     Block
 *     VariableStatement
 *     IfStatement
 *     DoWhileStatement
 *     WhileStatement
 *     ForStatement|ForEachStatement
 *     ContinueStatement
 *     BreakStatement
 *     ReturnStatement
 *     LabelledStatement
 *     SwitchStatement
 *     ThrowStatement
 *     TryStatement
 *     DebuggerStatement
 *     // Moved down for ambiguity
 *     ExpressionStatement
 * STANDARD:
 * Statement :
 *     Block
 *     VariableStatement
 *     EmptyStatement
 *     ExpressionStatement
 *     IfStatement
 *     IterationStatement
 *     ContinueStatement
 *     BreakStatement
 *     ReturnStatement
 *     WithStatement
 *     LabelledStatement
 *     SwitchStatement
 *     ThrowStatement
 *     TryStatement
 *     DebuggerStatement
 */

token_t *statement(GPtrArray *input, gsize *position_p) {

    tokenize_and_return_if_is_first(input, position_p, block)
    tokenize_and_return_if_is_first(input, position_p, variable_statement)
    // NONSTANDARD: No EmptyStatement.
    //tokenize_and_return_if_is_first(input, position_p, empty_statement)
    tokenize_and_return_if_is_first(input, position_p, if_statement)
    tokenize_and_return_if_is_first(input, position_p, do_while_statement)
    tokenize_and_return_if_is_first(input, position_p, while_statement)
    tokenize_and_return_if_is_first(input, position_p, for_statement)
    tokenize_and_return_if_is_first(input, position_p, continue_statement)
    tokenize_and_return_if_is_first(input, position_p, break_statement)
    tokenize_and_return_if_is_first(input, position_p, return_statement)
    tokenize_and_return_if_is_first(input, position_p, labeled_statement)
    tokenize_and_return_if_is_first(input, position_p, switch_statement)
    // TODO

    // TODO: Error!
    return NULL;
}

/*
 * AST:
 * Block - Statement*
 *
 * GRAMMAR:
 * Block :
 *     { Statement* }
 */

gboolean block_is_first(GPtrArray *input, gsize position) {
    return token_is_first_punctuator(input, position,
                                     PUNCTUATOR_CURLY_BRACE_LEFT);
}

token_t *block(GPtrArray *input, gsize *position_p) {

    match_punctuator_or_return_error(input, position_p,
                                     PUNCTUATOR_CURLY_BRACE_LEFT,
                                     ERROR_STATEMENT_BLOCK_CURLY_BRACE_LEFT)

    token_t *block_token = token_new_no_data(TOKEN_STATEMENT_BLOCK);
    while (!token_match_punctuator(input, position_p,
                                   PUNCTUATOR_CURLY_BRACE_RIGHT)) {

        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, statement, block_token)
    }

    return block_token;
}

/*
 * AST:
 * VariableStatement - VariableDeclaration+
 *
 * GRAMMAR:
 * VariableStatement :
 *     var VariableDeclaration (, VariableDeclaration)* ;
 */

gboolean variable_statement_is_first(GPtrArray *input, gsize position) {
    return token_is_first_keyword(input, position, KEYWORD_VAR);
}

token_t *variable_statement(GPtrArray *input, gsize *position_p) {

    match_keyword_or_return_error(input, position_p, KEYWORD_VAR,
                                  ERROR_STATEMENT_VARIABLE_STATEMENT_VAR);

    token_t *variable_statement_token =
            token_new_no_data(TOKEN_STATEMENT_VARIABLE_STATEMENT);

    do {
        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, variable_declaration, variable_statement_token)

    } while (token_match_punctuator(input, position_p, PUNCTUATOR_COMMA));

    match_punctuator_or_free_and_return_error(input, position_p,
            PUNCTUATOR_SEMICOLON, variable_statement_token,
            ERROR_STATEMENT_VARIABLE_STATEMENT_SEMICOLON)

    return variable_statement_token;
}

/*
 * AST:
 * VariableDeclaration - Identifier Initializer?
 *
 * GRAMMAR:
 * VariableDeclaration :
 *     Identifier Initializer?
 */

token_t *variable_declaration(GPtrArray *input, gsize *position_p) {

    token_t *variable_declaration_token =
            token_new_no_data(TOKEN_STATEMENT_VARIABLE_DECLARATION);

    match_token_id_clone_and_add_child_or_free_parent_and_return_error(input,
            position_p, TOKEN_LEXICAL_IDENTIFIER, variable_declaration_token,
            ERROR_STATEMENT_VARIABLE_DECLARATION_IDENTIFIER)

    if (initializer_is_first(input, *position_p)) {
        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, initializer, variable_declaration_token)
    }

    return variable_declaration_token;
}

/*
 * AST:
 * Initialiser = AssignmentExpression
 *
 * GRAMMAR:
 * Initialiser :
 *     = AssignmentExpression
 */

gboolean initializer_is_first(GPtrArray *input, gsize position) {
    return token_is_first_punctuator(input, position, PUNCTUATOR_EQUALS_SIGN);
}

token_t *initializer(GPtrArray *input, gsize *position_p) {

    match_punctuator_or_return_error(input, position_p, PUNCTUATOR_EQUALS_SIGN,
                                     ERROR_STATEMENT_INITIALIZER_EQUALS_SIGN)

    return assignment_expression(input, position_p);
}

/*
 * AST:
 * EmptyStatement = EmptyStatement
 *
 * GRAMMAR:
 * EmptyStatement :
 * ;
 */

gboolean empty_statement_is_first(GPtrArray *input, gsize position) {
    return token_is_first_punctuator(input, position, PUNCTUATOR_SEMICOLON);
}

token_t *empty_statement(GPtrArray *input, gsize *position_p) {

    match_punctuator_or_return_error(input, position_p, PUNCTUATOR_SEMICOLON,
                                     ERROR_STATEMENT_EMPTY_STATEMENT_SEMICOLON)

    return token_new_no_data(TOKEN_STATEMENT_EMPTY_STATEMENT);
}

/*
 * AST:
 * IfStatement - Expression Block Block?
 *
 * GRAMMAR:
 * NONSTANDARD:
 * Force Block instead of Statement.
 * IfStatement :
 *     if ( Expression ) Block (else Block)?
 * STANDARD:
 * IfStatement :
 *     if ( Expression ) Statement (else Statement)?
 */

gboolean if_statement_is_first(GPtrArray *input, gsize position) {
    return token_is_first_keyword(input, position, KEYWORD_IF);
}

token_t *if_statement(GPtrArray *input, gsize *position_p) {

    match_keyword_or_return_error(input, position_p, KEYWORD_IF,
                                  ERROR_STATEMENT_IF_STATEMENT_IF)

    match_punctuator_or_return_error(input, position_p,
            PUNCTUATOR_PARENTHESIS_LEFT,
            ERROR_STATEMENT_IF_STATEMENT_PARENTHESIS_LEFT)

    token_t *if_statement_token =
            token_new_no_data(TOKEN_STATEMENT_IF_STATEMENT);

    tokenize_and_add_child_or_free_parent_and_return_error(input, position_p,
                                                           expression,
                                                           if_statement_token)

    match_punctuator_or_free_and_return_error(input, position_p,
            PUNCTUATOR_PARENTHESIS_RIGHT, if_statement_token,
            ERROR_STATEMENT_IF_STATEMENT_PARENTHESIS_RIGHT)

    tokenize_and_add_child_or_free_parent_and_return_error(input, position_p,
                                                           block,
                                                           if_statement_token)

    if (token_match_keyword(input, position_p, KEYWORD_ELSE)) {
        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, block, if_statement_token)
    }

    return if_statement_token;
}

/*
 * AST:
 * DoWhileStatement - Block Expression
 *
 * GRAMMAR:
 * NONSTANDARD:
 * Force Block instead of Statement; No trailing semicolon.
 * DoWhileStatement :
 *     do Block while ( Expression )
 * STANDARD:
 * DoWhileStatement :
 *     do Statement while ( Expression );
 */

gboolean do_while_statement_is_first(GPtrArray *input, gsize position) {
    return token_is_first_keyword(input, position, KEYWORD_DO);
}

token_t *do_while_statement(GPtrArray *input, gsize *position_p) {

    match_keyword_or_return_error(input, position_p, KEYWORD_DO,
                                  ERROR_STATEMENT_DO_WHILE_STATEMENT_DO)

    token_t *do_while_statement_token =
            token_new_no_data(TOKEN_STATEMENT_DO_WHILE_STATEMENT);

    tokenize_and_add_child_or_free_parent_and_return_error(input, position_p,
            block, do_while_statement_token)

    match_keyword_or_free_and_return_error(input, position_p, KEYWORD_WHILE,
            do_while_statement_token, ERROR_STATEMENT_DO_WHILE_STATEMENT_WHILE)

    match_punctuator_or_free_and_return_error(input, position_p,
            PUNCTUATOR_PARENTHESIS_LEFT, do_while_statement_token,
            ERROR_STATEMENT_DO_WHILE_STATEMENT_PARENTHESIS_LEFT)

    tokenize_and_add_child_or_free_parent_and_return_error(input, position_p,
            expression, do_while_statement_token)

    match_punctuator_or_free_and_return_error(input, position_p,
            PUNCTUATOR_PARENTHESIS_RIGHT, do_while_statement_token,
            ERROR_STATEMENT_DO_WHILE_STATEMENT_PARENTHESIS_RIGHT)

    return do_while_statement_token;
}

/*
 * AST:
 * WhileStatement - Expression Block
 *
 * GRAMMAR:
 * NONSTANDARD:
 * Force Block instead of Statement.
 * WhileStatement :
 *     while ( Expression ) Block
 * STANDARD:
 * WhileStatement :
 *     if ( Expression ) Statement
 */

gboolean while_statement_is_first(GPtrArray *input, gsize position) {
    return token_is_first_keyword(input, position, KEYWORD_WHILE);
}

token_t *while_statement(GPtrArray *input, gsize *position_p) {

    match_keyword_or_return_error(input, position_p, KEYWORD_WHILE,
                                  ERROR_STATEMENT_WHILE_STATEMENT_WHILE)

    match_punctuator_or_return_error(input, position_p,
            PUNCTUATOR_PARENTHESIS_LEFT,
            ERROR_STATEMENT_WHILE_STATEMENT_PARENTHESIS_LEFT)

    token_t *while_statement_token =
            token_new_no_data(TOKEN_STATEMENT_WHILE_STATEMENT);

    tokenize_and_add_child_or_free_parent_and_return_error(input, position_p,
            expression, while_statement_token)

    match_punctuator_or_free_and_return_error(input, position_p,
            PUNCTUATOR_PARENTHESIS_RIGHT, while_statement_token,
            ERROR_STATEMENT_WHILE_STATEMENT_PARENTHESIS_RIGHT)

    tokenize_and_add_child_or_free_parent_and_return_error(input, position_p,
            block, while_statement_token)

    return while_statement_token;
}

/*
 * AST:
 * ForStatement - (Expression|VariableDeclarationList|null) (Expression|null)
 *         (Expression|null) Block
 * ForEachStatement - (LeftHandSideExpression|VariableDeclaration) Expression
 *         Block
 *
 * GRAMMAR:
 * NONSTANDARD:
 * Force Block instead of Statement.
 * ForStatement :
 *     for ( (Expression|var VariableDeclarationList)? ; Expression? ;
 *             Expression? ) Block
 *     for ( (LeftHandSideExpression|var VariableDeclaration) : Expression)
 *             Block
 * VariableDeclarationList:
 *     VariableDeclaration (, VariableDeclaration)*
 * STANDARD:
 * ForStatement :
 *     for ( ExpressionNoIn? ; Expressionopt ; Expression? ) Statement
 *     for ( var VariableDeclarationListNoIn ; Expression? ; Expression? )
 *             Statement
 *     for ( LeftHandSideExpression in Expression ) Statement
 *     for ( var VariableDeclarationNoIn in Expression ) Statement
 */

gboolean for_statement_is_first(GPtrArray *input, gsize position) {
    return token_is_first_keyword(input, position, KEYWORD_FOR);
}

token_t *for_statement(GPtrArray *input, gsize *position_p) {

    match_keyword_or_return_error(input, position_p, KEYWORD_FOR,
                                  ERROR_STATEMENT_FOR_STATEMENT_FOR)

    match_punctuator_or_return_error(input, position_p,
            PUNCTUATOR_PARENTHESIS_LEFT,
            ERROR_STATEMENT_FOR_STATEMENT_PARENTHESIS_LEFT)

    token_t *for_statement_token =
            token_new_no_data(TOKEN_STATEMENT_FOR_STATEMENT);

    gboolean is_for_each = FALSE;

    if (token_match_keyword(input, position_p, KEYWORD_VAR)) {

        token_t *first_variable_declaration_token = variable_declaration(input,
                position_p);
        return_and_free_if_error(first_variable_declaration_token,
                                 for_statement_token);

        if (token_is_first_punctuator(input, *position_p, PUNCTUATOR_COMMA)) {

            token_t *variable_declaration_list_token = token_new_no_data(
                            TOKEN_STATEMENT_VARIABLE_DECLARATION_LIST);
            token_add_child(for_statement_token,
                            variable_declaration_list_token);
            while (token_match_punctuator(input, position_p,
                                          PUNCTUATOR_COMMA)) {
                tokenize_and_add_child_or_free_grand_parent_and_return_error(
                        input, position_p, variable_declaration,
                        for_statement_token, variable_declaration_list_token)
            }

        } else {

            if (token_is_first_punctuator(input, *position_p,
                                          PUNCTUATOR_COLON)) {
                token_add_child(for_statement_token,
                                first_variable_declaration_token);
                is_for_each = TRUE;

            } else {

                token_t *variable_declaration_list_token = token_new_no_data(
                        TOKEN_STATEMENT_VARIABLE_DECLARATION_LIST);
                token_add_child(variable_declaration_list_token,
                                first_variable_declaration_token);
                token_add_child(for_statement_token,
                                variable_declaration_list_token);
            }
        }

    } else {

        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, expression, for_statement_token)

        if (token_is_first_punctuator(input, *position_p, PUNCTUATOR_COLON)) {
            token_t *expression_token = token_get_child(for_statement_token, 0);
            if (!left_hand_side_expression_is_left_hand_side_expression(
                    expression_token)) {
                token_free(&for_statement_token);
                return error_new_syntactic(
                   ERROR_STATEMENT_FOR_EACH_STATEMENT_LEFT_HAND_SIDE_EXPRESSION,
                        *position_p);
            }
            is_for_each = TRUE;
        }
    }

    if (is_for_each) {

        for_statement_token->id = TOKEN_STATEMENT_FOR_EACH_STATEMENT;

        // Already tested with token_is_first_punctuator().
        token_consume(input, position_p);

        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, expression, for_statement_token)

        match_punctuator_or_free_and_return_error(input, position_p,
                PUNCTUATOR_PARENTHESIS_RIGHT, for_statement_token,
                ERROR_STATEMENT_FOR_EACH_STATEMENT_PARENTHESIS_RIGHT);

        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, block, for_statement_token)

    } else {

        match_punctuator_or_free_and_return_error(input, position_p,
                PUNCTUATOR_SEMICOLON, for_statement_token,
                ERROR_STATEMENT_FOR_STATEMENT_FIRST_SEMICOLON)

        if (!token_match_punctuator(input, position_p, PUNCTUATOR_SEMICOLON)) {

            tokenize_and_add_child_or_free_parent_and_return_error(input,
                    position_p, expression, for_statement_token)

            match_punctuator_or_free_and_return_error(input, position_p,
                    PUNCTUATOR_SEMICOLON, for_statement_token,
                    ERROR_STATEMENT_FOR_STATEMENT_SECOND_SEMICOLON)

        } else {
            token_add_child(for_statement_token, NULL);
        }

        if (!token_match_punctuator(input, position_p,
                                    PUNCTUATOR_PARENTHESIS_RIGHT)) {

            tokenize_and_add_child_or_free_parent_and_return_error(input,
                    position_p, expression, for_statement_token)

            match_punctuator_or_free_and_return_error(input, position_p,
                    PUNCTUATOR_PARENTHESIS_RIGHT, for_statement_token,
                    ERROR_STATEMENT_FOR_STATEMENT_PARENTHESIS_RIGHT)

        } else {
            token_add_child(for_statement_token, NULL);
        }

        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, block, for_statement_token)
    }

    return for_statement_token;
}

/*
 * AST:
 * ContinueStatement - Identifier?
 *
 * GRAMMAR:
 * NONSTANDARD: Allow LineTerminator.
 * ContinueStatement :
 *     continue Identifier? ;
 * STANDARD:
 * ContinueStatement :
 *     continue ([no LineTerminator here] Identifier)? ;
 */

gboolean continue_statement_is_first(GPtrArray *input, gsize position) {
    return token_is_first_keyword(input, position, KEYWORD_CONTINUE);
}

token_t *continue_statement(GPtrArray *input, gsize *position_p) {

    match_keyword_or_return_error(input, position_p, KEYWORD_CONTINUE,
                                  ERROR_STATEMENT_CONTINUE_STATEMENT_CONTINUE)

    token_t *continue_statement_token = token_new_no_data(
            TOKEN_STATEMENT_CONTINUE_STATEMENT);

    if (!token_match_punctuator(input, position_p, PUNCTUATOR_SEMICOLON)) {

        match_token_id_clone_and_add_child_or_free_parent_and_return_error(
                input, position_p, TOKEN_LEXICAL_IDENTIFIER,
                continue_statement_token,
                ERROR_STATEMENT_CONTINUE_STATEMENT_IDENTIFIER)

        match_punctuator_or_free_and_return_error(input, position_p,
                PUNCTUATOR_SEMICOLON, continue_statement_token,
                ERROR_STATEMENT_CONTINUE_STATEMENT_SEMICOLON)
    }

    return continue_statement_token;
}

/*
 * AST:
 * BreakStatement - Identifier?
 *
 * GRAMMAR:
 * NONSTANDARD: Allow LineTerminator.
 * BreakStatement :
 *     break Identifier? ;
 * STANDARD:
 * BreakStatement :
 *     break ([no LineTerminator here] Identifier)? ;
 */

gboolean break_statement_is_first(GPtrArray *input, gsize position) {
    return token_is_first_keyword(input, position, KEYWORD_BREAK);
}

token_t *break_statement(GPtrArray *input, gsize *position_p) {

    match_keyword_or_return_error(input, position_p, KEYWORD_BREAK,
                                  ERROR_STATEMENT_BREAK_STATEMENT_BREAK)

    token_t *break_statement_token = token_new_no_data(
            TOKEN_STATEMENT_BREAK_STATEMENT);

    if (!token_match_punctuator(input, position_p, PUNCTUATOR_SEMICOLON)) {

        match_token_id_clone_and_add_child_or_free_parent_and_return_error(
                input, position_p, TOKEN_LEXICAL_IDENTIFIER,
                break_statement_token,
                ERROR_STATEMENT_BREAK_STATEMENT_IDENTIFIER)

        match_punctuator_or_free_and_return_error(input, position_p,
                PUNCTUATOR_SEMICOLON, break_statement_token,
                ERROR_STATEMENT_BREAK_STATEMENT_SEMICOLON)
    }

    return break_statement_token;
}

/*
 * AST:
 * ReturnStatement - Identifier?
 *
 * GRAMMAR:
 * NONSTANDARD: Allow LineTerminator.
 * ReturnStatement :
 *     return Identifier? ;
 * STANDARD:
 * ReturnStatement :
 *     return ([no LineTerminator here] Identifier)? ;
 */

gboolean return_statement_is_first(GPtrArray *input, gsize position) {
    return token_is_first_keyword(input, position, KEYWORD_RETURN);
}

token_t *return_statement(GPtrArray *input, gsize *position_p) {

    match_keyword_or_return_error(input, position_p, KEYWORD_RETURN,
                                  ERROR_STATEMENT_RETURN_STATEMENT_RETURN)

    token_t *return_statement_token = token_new_no_data(
            TOKEN_STATEMENT_RETURN_STATEMENT);

    if (!token_match_punctuator(input, position_p, PUNCTUATOR_SEMICOLON)) {

        match_token_id_clone_and_add_child_or_free_parent_and_return_error(
                input, position_p, TOKEN_LEXICAL_IDENTIFIER,
                return_statement_token,
                ERROR_STATEMENT_RETURN_STATEMENT_IDENTIFIER)

        match_punctuator_or_free_and_return_error(input, position_p,
                PUNCTUATOR_SEMICOLON, return_statement_token,
                ERROR_STATEMENT_RETURN_STATEMENT_SEMICOLON)
    }

    return return_statement_token;
}

/*
 * AST:
 * LabeledStatement - Identifier Statement
 *
 * GRAMMAR:
 * LabeledStatement :
 *     Identifier : Statement
 */

gboolean labeled_statement_is_first(GPtrArray *input, gsize position) {
    return token_match_id(input, &position, TOKEN_LEXICAL_IDENTIFIER)
           && token_is_first_punctuator(input, position, PUNCTUATOR_SEMICOLON);
}

token_t *labeled_statement(GPtrArray *input, gsize *position_p) {

    token_t *labeled_statement_token = token_new_no_data(
            TOKEN_STATEMENT_LABELED_STATEMENT);

    match_token_id_clone_and_add_child_or_free_parent_and_return_error(input,
            position_p, TOKEN_LEXICAL_IDENTIFIER, labeled_statement_token,
            ERROR_STATEMENT_LABELED_STATEMENT_IDENTIFIER)

    match_punctuator_or_free_and_return_error(input, position_p,
            PUNCTUATOR_COLON, labeled_statement_token,
            ERROR_STATEMENT_LABELED_STATEMENT_COLON)

    tokenize_and_add_child_or_free_parent_and_return_error(input, position_p,
            statement, labeled_statement_token)

    return labeled_statement_token;
}

/*
 * AST:
 * SwitchStatement - Expression CaseClause* DefaultClause CaseClause*
 *
 * GRAMMAR:
 * SwitchStatement :
 *     switch ( Expression ) { CaseClause* DefaultClause CaseClause* }
 */

gboolean switch_statement_is_first(GPtrArray *input, gsize position) {
    return token_is_first_keyword(input, position, KEYWORD_SWITCH);
}

token_t *switch_statement(GPtrArray *input, gsize *position_p) {

    match_keyword_or_return_error(input, position_p, KEYWORD_SWITCH,
                                  ERROR_STATEMENT_SWITCH_STATEMENT_SWITCH)

    match_punctuator_or_return_error(input, position_p,
            PUNCTUATOR_PARENTHESIS_LEFT,
            ERROR_STATEMENT_SWITCH_STATEMENT_PARENTHESIS_LEFT)

    token_t *switch_statement_token = token_new_no_data(
            TOKEN_STATEMENT_SWITCH_STATEMENT);

    tokenize_and_add_child_or_free_parent_and_return_error(input, position_p,
            expression, switch_statement_token)

    match_punctuator_or_free_and_return_error(input, position_p,
            PUNCTUATOR_PARENTHESIS_RIGHT, switch_statement_token,
            ERROR_STATEMENT_SWITCH_STATEMENT_PARENTHESIS_RIGHT)

    match_punctuator_or_return_error(input, position_p,
            PUNCTUATOR_CURLY_BRACE_LEFT,
            ERROR_STATEMENT_SWITCH_STATEMENT_CURLY_BRACE_LEFT)

    while (TRUE) {

        if (case_clause_is_first(input, *position_p)) {
            tokenize_and_add_child_or_free_parent_and_return_error(input,
                    position_p, case_clause, switch_statement_token)

        } else if (default_clause_is_first(input, *position_p)) {
            tokenize_and_add_child_or_free_parent_and_return_error(input,
                    position_p, default_clause, switch_statement_token)

        } else {

            match_punctuator_or_return_error(input, position_p,
                    PUNCTUATOR_CURLY_BRACE_RIGHT,
                    ERROR_STATEMENT_SWITCH_STATEMENT_CURLY_BRACE_RIGHT)
            break;
        }
    }

    return switch_statement_token;
}

/*
 * AST:
 * CaseClause - Identifier Statement
 *
 * GRAMMAR:
 * CaseClause :
 *     case Expression : Statement* [lookahead ∉ case, default, } ]
 */

gboolean case_clause_is_first(GPtrArray *input, gsize position) {
    return token_is_first_keyword(input, position, KEYWORD_CASE);
}

token_t *case_clause(GPtrArray *input, gsize *position_p) {

    match_keyword_or_return_error(input, position_p, KEYWORD_CASE,
                                  ERROR_STATEMENT_CASE_CLAUSE_CASE)

    token_t *case_clause_token = token_new_no_data(TOKEN_STATEMENT_CASE_CLAUSE);

    tokenize_and_add_child_or_free_parent_and_return_error(input, position_p,
                                                           expression,
                                                           case_clause_token)

    match_punctuator_or_free_and_return_error(input, position_p,
            PUNCTUATOR_COLON, case_clause_token,
            ERROR_STATEMENT_CASE_CLAUSE_COLON)

    while (!case_clause_is_first(input, *position_p)
           && !default_clause_is_first(input, *position_p)
           && !token_is_first_punctuator(input, *position_p,
                                         PUNCTUATOR_CURLY_BRACE_RIGHT)) {
        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, statement, case_clause_token)
    }

    return case_clause_token;
}

/*
 * AST:
 * DefaultClause - Statement*
 *
 * GRAMMAR:
 * DefaultClause :
 *     default : Statement* [lookahead ∉ case, default, } ]
 */

gboolean default_clause_is_first(GPtrArray *input, gsize position) {
    return token_is_first_keyword(input, position, KEYWORD_DEFAULT);
}

token_t *default_clause(GPtrArray *input, gsize *position_p) {

    match_keyword_or_return_error(input, position_p, KEYWORD_DEFAULT,
                                  ERROR_STATEMENT_DEFAULT_CLAUSE_CASE)

    match_punctuator_or_return_error(input, position_p, PUNCTUATOR_COLON,
                                     ERROR_STATEMENT_DEFAULT_CLAUSE_COLON)

    token_t *default_clause_token = token_new_no_data(
            TOKEN_STATEMENT_DEFAULT_CLAUSE);

    while (!case_clause_is_first(input, *position_p)
           && !default_clause_is_first(input, *position_p)
           && !token_is_first_punctuator(input, *position_p,
                                         PUNCTUATOR_CURLY_BRACE_RIGHT)) {
        tokenize_and_add_child_or_free_parent_and_return_error(input,
                position_p, statement, default_clause_token)
    }

    return default_clause_token;
}
