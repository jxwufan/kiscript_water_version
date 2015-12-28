//
// Copyright (c) 2015 Zhang Hai <Dreaming.in.Code.ZH@Gmail.com>
// All Rights Reserved.
//

#ifndef KISCRIPT_PARSER_H
#define KISCRIPT_PARSER_H

#include <glib.h>

typedef enum {
    TOKEN_LEXICAL_INPUT_ELEMENT_DIV,
    TOKEN_LEXICAL_WHITE_SPACE,
    TOKEN_LEXICAL_LINE_TERMINATOR,
    TOKEN_LEXICAL_LINE_TERMINATOR_SEQUENCE,
    TOKEN_LEXICAL_COMMENT,
    TOKEN_LEXICAL_MULTI_LINE_COMMENT,
    TOKEN_LEXICAL_SINGLE_LINE_COMMENT,
    TOKEN_LEXICAL_IDENTIFIER,
    TOKEN_LEXICAL_IDENTIFIER_NAME,
    TOKEN_LEXICAL_PUNCTUATOR,
    TOKEN_LEXICAL_DIV_PUNCTUATOR,
    TOKEN_LEXICAL_NULL_LITERAL,
    TOKEN_LEXICAL_BOOLEAN_LITERAL,
    TOKEN_LEXICAL_NUMERIC_LITERAL,
    TOKEN_LEXICAL_STRING_LITERAL
} token_id_t;

typedef void (*token_data_free_func_t)(gpointer data);

typedef struct {
    token_id_t id;
    gchar *text;
    gpointer data;
    token_data_free_func_t data_free_func;
} token_t;

void token_init(token_t *token, token_id_t id, gchar *text, gpointer data,
                token_data_free_func_t data_free_func);

token_t * token_new(token_id_t id, gchar *text, gpointer data,
                    token_data_free_func_t data_free_func);

token_t *token_new_strndup(token_id_t id, gchar *text, gchar *text_end,
                           gpointer data,
                           token_data_free_func_t data_free_func);

token_t *token_new_strndup_gstring(token_id_t id, gchar *text, gchar *text_end,
                                   GString *string);

token_t * token_new_no_data(token_id_t id, gchar *text);

token_t * token_new_strndup_no_data(token_id_t id, gchar *text,
                                    gchar *text_end);

token_t *token_new_buffer_no_data(token_id_t id, GString *buffer);

void token_final(token_t *token);

void token_free(token_t *token);


gboolean char_is_first(gchar *input, gchar char0);

gboolean char_is_match(gunichar input, gchar char0);

void char_consume(gchar **input_p);

void char_consume_save_char(gchar **input_p, gunichar *char_p);

void char_consume_save_other(gchar **input_p, GString *buffer, gunichar char0);

void char_consume_save(gchar **input_p, GString *buffer);

gboolean char_match(gchar **input_p, gchar char0);

gboolean char_match_save_char(gchar **input_p, gchar char0, gunichar *char_p);

gboolean char_match_save_other(gchar **input_p, gchar char0, GString *buffer,
                               gunichar char1);

gboolean char_match_save(gchar **input_p, gchar char0, GString *buffer);

gboolean char_match_not(gchar **input_p, gchar char0);

gboolean char_match_not_2(gchar **input_p, gchar char0, gchar char1);

gboolean chars_is_first(gchar *input, gchar *chars);

gboolean chars_match(gchar **input_p, gchar *chars);

gboolean chars_match_save_char(gchar **input_p, gchar *chars, gunichar *char_p);

gboolean chars_is_match(gunichar input, gchar *chars);

gboolean text_is_first(gchar *input, gchar *text);

void text_consume(gchar **input_p, gchar *text);

void text_consume_save(gchar **input_p, gchar *text, GString *buffer);

gboolean text_match(gchar **input_p, gchar *text);

gboolean text_match_save(gchar **input_p, gchar *text, GString *buffer);

gboolean text_is_match(gchar *input, gchar *text);

#define DEFINE_MATCH_ANY_FUNC(match_func) \
    void match_func##_any(gchar **input_p) { \
        while (match_func(input_p)) {} \
    }

#define DEFINE_MATCH_ANY_SAVE_FUNC(match_func_name, match_func) \
    void match_func_name(gchar **input_p, GString *buffer) { \
        while (match_func(input_p, buffer)) {} \
    }

#define DEFINE_MATCH_MULTIPLE_FUNC(match_func) \
    gboolean match_func##_multiple(gchar **input_p) { \
        if (match_func(input_p)) { \
            while (match_func(input_p)) {} \
            return TRUE; \
        } else { \
            return FALSE; \
        } \
    }

#define DEFINE_MATCH_MULTIPLE_SAVE_FUNC(match_func_name, match_func) \
    gboolean match_func_name(gchar **input_p, GString *buffer) { \
        if (match_func(input_p, buffer)) { \
            while (match_func(input_p, buffer)) {} \
            return TRUE; \
        } else { \
            return FALSE; \
        } \
    }

#define try_match_and_return_token(input_p, match, token_id) \
    { \
        char *input_old = *(input_p); \
        if ((match)) { \
            return token_new_strndup_no_data((token_id), input_old, \
                                             *(input_p)); \
        } \
    }

#define try_match_char_and_return_token(input_p, token_char, token_id) \
    try_match_and_return_token((input_p), char_match((input_p), (token_char)), \
                               (token_id))

#define try_match_text_and_return_token(input_p, token_text, token_id) \
    try_match_and_return_token((input_p), text_match((input_p), (token_text)), \
                               (token_id))


void normalize_input(gchar **input_p);

#endif //KISCRIPT_PARSER_H
