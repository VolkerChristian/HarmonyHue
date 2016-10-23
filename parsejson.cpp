/*
 * Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include <stdlib.h>
#include <string.h>

#include <jansson.h>

int main3(int);

int main2(const char* text) {
    size_t i;

    json_t *root;
    json_error_t error;

    root = json_loads(text, 0, &error);

    if(root) {
        json_t *activityStatus;

        activityStatus = json_object_get(root, "activityStatus");

        if(json_is_integer(activityStatus)) {
			main3(json_integer_value(activityStatus));
        }
    }

    json_decref(root);
    return 0;
}

