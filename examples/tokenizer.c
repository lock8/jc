#include "jc.h"
#include <stdio.h>

void print_usage()
{
    printf("Usage: ./tokenizer <json>\n");
}

int main(int argc, char const * argv[])
{
    jc_state jc;
    jc_token token;
    jc_result result;
    size_t i = 0;
    size_t token_len = 0;
    size_t buf_len = 0;
    char buf[64] = "";
    char const * src = NULL;

    if (argc < 2) {
        print_usage();
        return 0;
    }

    src = argv[1];
    jc_init(&jc, src);

    while ((result = jc_next_token(&jc, &token)) != JC_RESULT_EOF) {
        if (result != JC_RESULT_OK) {
            printf("Error: 0x%03X\n", result);
            printf("Expected Token: 0x%03lX\n", jc.expected_token_types);
            printf("Nesting Level: %d\n", jc.nesting_level + 1);
            break;
        }

        memset(buf, 0, sizeof(buf));
        token_len = token.end - token.start;
        buf_len = token_len > sizeof(buf) ? sizeof(buf) : token_len;
        strncpy(buf, src + token.start, buf_len);
        buf[sizeof(buf) - 1] = '\0';

        printf("Token %02ld of type 0x%03X @ (%02ld, %02ld) [ %s ]\n", i, token.type,
                token.start, token.end, buf);
        ++i;
    }

    return 0;
}
