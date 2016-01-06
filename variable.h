//
// Created by fan wu on 1/5/16.
//

#ifndef KISCRIPT_VARIABLE_H
#define KISCRIPT_VARIABLE_H

#include <glib.h>
#include "activation_record.h"
#define TO_STRING_LEN 255
#define INTEGER_THRESHOLD 0.001

typedef enum {
    VARIABLE_UNDEFINED,
    VARIABLE_NULL,
    VARIABLE_BOOL,
    VARIABLE_NUMERICAL,
    VARIABLE_STRING,
    VARIABLE_OBJECT,
    VARIABLE_EXCEPTION,
    VARIABLE_FUNC
} variable_type_t;

typedef struct {
    variable_type_t variable_type;
    gpointer        variable_data;
    GHashTable      *attribute_hash_table;
    activation_record_t *AR;
} variable_t;

variable_t  *variable_new            (variable_type_t variable_type, gpointer *variable_data, GHashTable *attribute_hash_table, activation_record_t *AR);
variable_t  *variable_clone          (variable_t *variable);

void        variable_on_destory     (variable_t *variable);
gboolean    variable_free           (variable_t *variable);

gchar*      variable_to_string      (variable_t *variable);
gdouble     variable_to_numerical   (variable_t *variable);



void variable_out_of_scope(variable_t *variable);


#endif //KISCRIPT_VARIABLE_H
