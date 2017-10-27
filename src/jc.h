/*
 * Copyright 2017 Noa Technologies GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * Distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * Limitations under the License.
 */

#ifndef JC_H
#define JC_H

/*
 * jc is a simple constant-memory JSON tokenizer suitable for
 * resource-restricted embedded environments. It's written in C89.
 *
 * Current Limitations:
 *
 *     - Supports only ASCII-strings. If you intend to use Unicode, make sure
 *     that characters are encoded using `\uXXXX` notation.
 *     - Supports object and array nesting only up to JC_MAX_NESTING_LEVEL
 *     - Does not check JSON strings for validity, e.g. they may contain
 *     unsupported escape sequences like '\a'. It's up to you to check it.
 *     - Does not check JSON number for validity, e.g. "++-0EE" is considered a
 *     valid number. It's up to you to check it.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <ctype.h>
#include <string.h>

/*
 * Max level of object and array nesting supported
 * For example, '{"foo": [{"bar": 0}, 2]}' has max nesting level of 3: the first
 * is from an outermost object itself, the second is from the array in "foo"
 * field, the third is from the object in the array.
 */
#ifndef JC_MAX_NESTING_LEVEL
#define JC_MAX_NESTING_LEVEL 8
#endif

/*
 * All available jc token types.
 * They mostly correspond to `value` forms of JSON grammar, except that:
 * - `object` and `array` forms split into start/end tokens to keep them atomic
 * - `field_name` token is introduced; it's a `string` form that is placed right
 *   after `object_start` or `comma` if current nesting is in object.
 */
typedef enum {
    JC_TOKEN_TYPE_NUMBER        = 0x001,
    JC_TOKEN_TYPE_STRING        = 0x002,
    JC_TOKEN_TYPE_TRUE          = 0x004,
    JC_TOKEN_TYPE_FALSE         = 0x008,
    JC_TOKEN_TYPE_NULL          = 0x010,
    JC_TOKEN_TYPE_ARRAY_START   = 0x020,
    JC_TOKEN_TYPE_ARRAY_END     = 0x040,
    JC_TOKEN_TYPE_OBJECT_START  = 0x080,
    JC_TOKEN_TYPE_OBJECT_END    = 0x100,
    JC_TOKEN_TYPE_FIELD_NAME    = 0x200,
    JC_TOKEN_TYPE_COMMA         = 0x400,
    JC_TOKEN_TYPE_COLON         = 0x800
} jc_token_type;

/*
 * Contains the type of token and its boundaries in the source string.
 *
 * In case of `string` and `field_name` tokens, start and end position WON'T
 * include wrapping double quotes
 */
typedef struct {
    jc_token_type type;
    size_t start;
    size_t end;
} jc_token;

/*
 * Contains all possible result and error codes
 */
typedef enum {
    JC_RESULT_OK                        = 0x001,
    JC_RESULT_EOF                       = 0x002,
    JC_RESULT_ERR_CANT_INIT             = 0x004,
    JC_RESULT_ERR_UNEXPECTED_TOKEN      = 0x008,
    JC_RESULT_ERR_UNEXPECTED_EOF        = 0x010,
    JC_RESULT_ERR_GARBAGE               = 0x020,
    JC_RESULT_ERR_MAX_NESTING_REACHED   = 0x040,
    JC_RESULT_ERR_CORRUPTED_STATE       = 0x080
} jc_result;

typedef struct jc_state_s jc_state;

/*
 * Given a jc_state structure and null-terminated ASCII-encoded source JSON
 * string, initializes the state structure.
 *
 * Returns:
 *  - JC_RESULT_OK if everything went fine
 *  - JC_RESULT_ERR_CANT_INIT if `state` or `source` is null
 */
jc_result jc_init(jc_state * state, char const * const source);

/*
 * Given a state and token objects, parses the next token from the source string
 * set by `jc_init` into the token object if it is supplied, and returns result
 * code.
 *
 * Returns:
 *  - JC_RESULT_OK if everything went fine
 *  - JC_RESULT_EOF if reached the end of the source string AND no token was
 *      expected
 *  - JC_RESULT_ERR_UNEXPECTED_TOKEN if an unexpected token was encountered
 *  - JC_RESULT_ERR_UNEXPECTED_EOF if a source was null-terminated in the
 *      middle of a token
 *  - JC_RESULT_ERR_GARBAGE if no token was expected, but the source did not
 *      terminate with NULL
 *  - JC_RESULT_ERR_MAX_NESTING_REACHED if maximum level of nesting was reached
 *  - JC_RESULT_ERR_CORRUPTED_STATE if state was NULL or somehow corrupted
 */
jc_result jc_next_token(jc_state * state, jc_token * token);

/* --- Move along sir. This is private property. --- */

#define JC_NO_TOKENS_EXPECTED   (0)
#define JC_NO_NESTING_LEVEL     (-1)

#define JC_TOKEN_TYPE_VALUE \
    ( JC_TOKEN_TYPE_NUMBER \
    | JC_TOKEN_TYPE_STRING \
    | JC_TOKEN_TYPE_TRUE \
    | JC_TOKEN_TYPE_FALSE \
    | JC_TOKEN_TYPE_NULL \
    | JC_TOKEN_TYPE_ARRAY_START \
    | JC_TOKEN_TYPE_OBJECT_START \
    )

#define JC_CHAR_NULL            '\0'
#define JC_CHAR_OBJECT_START    '{'
#define JC_CHAR_OBJECT_END      '}'
#define JC_CHAR_ARRAY_START     '['
#define JC_CHAR_ARRAY_END       ']'
#define JC_CHAR_COMMA           ','
#define JC_CHAR_COLON           ':'
#define JC_CHAR_DQUOTE          '"'
#define JC_CHAR_BACKSLASH       '\\'

#define JC_LIT_TRUE     "true"
#define JC_LIT_FALSE    "false"
#define JC_LIT_NULL     "null"

#define JC_VALID_CHARS_IN_NUMBER "0123456789-+eE."

typedef enum {
    JC_NESTING_TYPE_OBJECT,
    JC_NESTING_TYPE_ARRAY
} jc_nesting_type;

struct jc_state_s {
    char const * source;
    size_t source_pos;
    jc_nesting_type nesting_stack[JC_MAX_NESTING_LEVEL];
    int nesting_level;
    size_t expected_token_types;
};

jc_result jc_init(jc_state * state, char const * const source)
{
    if (state == NULL || source == NULL) {
        return JC_RESULT_ERR_CANT_INIT;
    }

    state->source = source;
    state->source_pos = 0;
    state->nesting_level = JC_NO_NESTING_LEVEL;
    state->expected_token_types = JC_TOKEN_TYPE_VALUE;
    return JC_RESULT_OK;
}

char const * jc_current_source(jc_state * state)
{
    return state->source + state->source_pos;
}

void jc_make_token(jc_state * state, jc_token * token, jc_token_type type,
                   size_t len)
{
    if (token != NULL) {
        token->type = type;
        token->start = state->source_pos;
        token->end = state->source_pos + len;
    }
}

void jc_advance_source_pos(jc_state * state, size_t num_of_chars)
{
    state->source_pos += num_of_chars;
}

void jc_expect_next(jc_state * state, size_t expected_token_types)
{
    state->expected_token_types = expected_token_types;
}

int jc_is_expected(jc_state * state, jc_token_type type)
{
    if (type == JC_NO_TOKENS_EXPECTED) {
        return state->expected_token_types == type;
    } else {
        return (state->expected_token_types & type) > 0;
    }
}

void jc_skip_whitespace(jc_state * state)
{
    while (isspace(state->source[state->source_pos])) {
        ++(state->source_pos);
    }
}

jc_result jc_nest(jc_state * state, jc_nesting_type type)
{
    if (state->nesting_level >= JC_MAX_NESTING_LEVEL - 1) {
        return JC_RESULT_ERR_MAX_NESTING_REACHED;
    }

    state->nesting_level += 1;
    state->nesting_stack[state->nesting_level] = type;
    return JC_RESULT_OK;
}

jc_result jc_unnest(jc_state * state)
{
    if (state->nesting_level <= JC_NO_NESTING_LEVEL) {
        return JC_RESULT_ERR_CORRUPTED_STATE;
    }

    state->nesting_level -= 1;
    return JC_RESULT_OK;
}

jc_token_type jc_get_end_token_type_of_current_nesting(jc_state * state)
{
    if (JC_NO_NESTING_LEVEL < state->nesting_level
            && state->nesting_level < JC_MAX_NESTING_LEVEL) {
        if (state->nesting_stack[state->nesting_level]
                == JC_NESTING_TYPE_OBJECT) {
            return JC_TOKEN_TYPE_COMMA | JC_TOKEN_TYPE_OBJECT_END;
        } else {
            return JC_TOKEN_TYPE_COMMA | JC_TOKEN_TYPE_ARRAY_END;
        }
    } else {
        return JC_NO_TOKENS_EXPECTED;
    }
}

jc_token_type jc_get_token_type_after_comma_of_current_nesting(jc_state * state)
{
    if (JC_NO_NESTING_LEVEL < state->nesting_level
            && state->nesting_level < JC_MAX_NESTING_LEVEL) {
        if (state->nesting_stack[state->nesting_level]
                == JC_NESTING_TYPE_OBJECT) {
            return JC_TOKEN_TYPE_FIELD_NAME;
        } else {
            return JC_TOKEN_TYPE_VALUE;
        }
    } else {
        return JC_NO_TOKENS_EXPECTED;
    }
}

char const * jc_search_dquote(char const * str)
{
    char const * c = str;

    while (*c != JC_CHAR_DQUOTE) {
        if (*c == JC_CHAR_NULL) {
            c = NULL;
            break;
        } else if (*c == JC_CHAR_BACKSLASH && *(c + 1) != JC_CHAR_NULL) {
            c += 2;
        } else {
            c += 1;
        }
    }

    return c;
}

jc_result jc_parse_string_or_field_name(jc_state * state, jc_token * token)
{
    size_t token_len = 0;
    jc_token_type token_type = JC_TOKEN_TYPE_STRING;
    char const * end_dquote_ptr = NULL;

    end_dquote_ptr = jc_search_dquote(jc_current_source(state) + 1);
    if (end_dquote_ptr == NULL) {
        return JC_RESULT_ERR_UNEXPECTED_EOF;
    }
    token_len = end_dquote_ptr - jc_current_source(state) - 1;

    if (jc_is_expected(state, JC_TOKEN_TYPE_FIELD_NAME)) {
        token_type = JC_TOKEN_TYPE_FIELD_NAME;
    }

    jc_advance_source_pos(state, 1);
    jc_make_token(state, token, token_type, token_len);
    jc_advance_source_pos(state, token_len + 1);

    if (token_type == JC_TOKEN_TYPE_FIELD_NAME) {
        jc_expect_next(state, JC_TOKEN_TYPE_COLON);
    } else {
        jc_expect_next(state,
            jc_get_end_token_type_of_current_nesting(state));
    }
    return JC_RESULT_OK;
}

jc_result jc_parse_number(jc_state * state, jc_token * token)
{
    size_t token_len = 0;
    while (strchr(JC_VALID_CHARS_IN_NUMBER,
                  *(jc_current_source(state) + token_len)) != NULL) {
        ++token_len;
    }

    jc_make_token(state, token, JC_TOKEN_TYPE_NUMBER, token_len);
    jc_advance_source_pos(state, token_len);
    jc_expect_next(state, jc_get_end_token_type_of_current_nesting(state));
    return JC_RESULT_OK;
}

jc_result jc_parse_literal(jc_state * state, jc_token * token,
                           jc_token_type token_type, char const * literal)
{
    size_t token_len = strlen(literal);
    if (strncmp(jc_current_source(state), literal, token_len) != 0) {
        return JC_RESULT_ERR_UNEXPECTED_TOKEN;
    }

    jc_make_token(state, token, token_type, token_len);
    jc_advance_source_pos(state, token_len);
    jc_expect_next(state, jc_get_end_token_type_of_current_nesting(state));
    return JC_RESULT_OK;
}

jc_result jc_next_token(jc_state * state, jc_token * token)
{
    char current_char = '\0';

    if (state == NULL) {
        return JC_RESULT_ERR_CORRUPTED_STATE;
    }

    jc_skip_whitespace(state);
    current_char = state->source[state->source_pos];

    /* Check for EOF */

    if (jc_is_expected(state, JC_NO_TOKENS_EXPECTED)) {
        return (current_char == JC_CHAR_NULL)
            ? JC_RESULT_EOF
            : JC_RESULT_ERR_GARBAGE;
    } else if (current_char == JC_CHAR_NULL) {
        return JC_RESULT_ERR_UNEXPECTED_EOF;
    }

    /* Parse object start */

    if (jc_is_expected(state, JC_TOKEN_TYPE_OBJECT_START)
            && current_char == JC_CHAR_OBJECT_START) {
        jc_make_token(state, token, JC_TOKEN_TYPE_OBJECT_START, 1);
        jc_advance_source_pos(state, 1);
        jc_expect_next(state,
            JC_TOKEN_TYPE_FIELD_NAME | JC_TOKEN_TYPE_OBJECT_END);
        return jc_nest(state, JC_NESTING_TYPE_OBJECT);
    }

    /* Parse array start */

    if (jc_is_expected(state, JC_TOKEN_TYPE_ARRAY_START)
            && current_char == JC_CHAR_ARRAY_START) {
        jc_make_token(state, token, JC_TOKEN_TYPE_ARRAY_START, 1);
        jc_advance_source_pos(state, 1);
        jc_expect_next(state, JC_TOKEN_TYPE_VALUE | JC_TOKEN_TYPE_ARRAY_END);
        return jc_nest(state, JC_NESTING_TYPE_ARRAY);
    }

    /* Parse object end */

    if (jc_is_expected(state, JC_TOKEN_TYPE_OBJECT_END)
            && current_char == JC_CHAR_OBJECT_END) {
        jc_make_token(state, token, JC_TOKEN_TYPE_OBJECT_END, 1);
        jc_advance_source_pos(state, 1);

        if (jc_unnest(state) != JC_RESULT_OK) {
            return JC_RESULT_ERR_CORRUPTED_STATE;
        }

        jc_expect_next(state, jc_get_end_token_type_of_current_nesting(state));
        return JC_RESULT_OK;
    }

    /* Parse array end */

    if (jc_is_expected(state, JC_TOKEN_TYPE_ARRAY_END)
            && current_char == JC_CHAR_ARRAY_END) {
        jc_make_token(state, token, JC_TOKEN_TYPE_ARRAY_END, 1);
        jc_advance_source_pos(state, 1);

        if (jc_unnest(state) != JC_RESULT_OK) {
            return JC_RESULT_ERR_CORRUPTED_STATE;
        }

        jc_expect_next(state, jc_get_end_token_type_of_current_nesting(state));
        return JC_RESULT_OK;
    }

    /* Parse colon */

    if (jc_is_expected(state, JC_TOKEN_TYPE_COLON)
            && current_char == JC_CHAR_COLON) {
        jc_make_token(state, token, JC_TOKEN_TYPE_COLON, 1);
        jc_advance_source_pos(state, 1);
        jc_expect_next(state, JC_TOKEN_TYPE_VALUE);
        return JC_RESULT_OK;
    }

    /* Parse comma */

    if (jc_is_expected(state, JC_TOKEN_TYPE_COMMA)
            && current_char == JC_CHAR_COMMA) {
        jc_make_token(state, token, JC_TOKEN_TYPE_COMMA, 1);
        jc_advance_source_pos(state, 1);
        jc_expect_next(state,
            jc_get_token_type_after_comma_of_current_nesting(state));
        return JC_RESULT_OK;
    }

    /* Parse field name or string */

    if (jc_is_expected(state, JC_TOKEN_TYPE_FIELD_NAME | JC_TOKEN_TYPE_STRING)
            && current_char == JC_CHAR_DQUOTE) {
        return jc_parse_string_or_field_name(state, token);
    }

    /* Parse number */

    if (jc_is_expected(state, JC_TOKEN_TYPE_NUMBER)
            && strchr(JC_VALID_CHARS_IN_NUMBER, current_char) != NULL) {
        return jc_parse_number(state, token);
    }

    /* Parse literals */

    if (jc_is_expected(state, JC_TOKEN_TYPE_TRUE)
            && strchr(JC_LIT_TRUE, current_char) != NULL) {
        return jc_parse_literal(state, token, JC_TOKEN_TYPE_TRUE, JC_LIT_TRUE);
    }

    if (jc_is_expected(state, JC_TOKEN_TYPE_FALSE)
            && strchr(JC_LIT_FALSE, current_char) != NULL) {
        return jc_parse_literal(state, token, JC_TOKEN_TYPE_FALSE, JC_LIT_FALSE);
    }

    if (jc_is_expected(state, JC_TOKEN_TYPE_NULL)
            && strchr(JC_LIT_NULL, current_char) != NULL) {
        return jc_parse_literal(state, token, JC_TOKEN_TYPE_NULL, JC_LIT_NULL);
    }

    return JC_RESULT_ERR_UNEXPECTED_TOKEN;
}

#ifdef __cplusplus
}
#endif

#endif /* JC_H */
