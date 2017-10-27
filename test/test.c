#include "jc.h"
#include <stdlib.h>
#include <stdio.h>

#define MAX_TEST_FILE_SIZE 4096
#define MAX_TOKEN_CONTENTS_SIZE 256

int main(int argc, char const * argv[])
{
    jc_state jc;
    jc_token token;
    jc_result result;
    size_t token_len = 0;
    size_t token_buf_len = 0;
    char token_buf[MAX_TOKEN_CONTENTS_SIZE] = "";
    FILE * src_file = NULL;
    size_t src_size = 0;
    char src[MAX_TEST_FILE_SIZE] = "";

    if (argc < 2) {
        printf("Usage: ./test <case-file-path>\n");
        abort();
    }

    src_file = fopen(argv[1], "rb");
    src_size = fread(src, sizeof(*src), MAX_TEST_FILE_SIZE, src_file);

    if (ferror(src_file)) {
        perror("Error while reading test case file");
        abort();
    }

    fclose(src_file);
    src[src_size] = '\0';

    jc_init(&jc, src);
    while ((result = jc_next_token(&jc, &token)) != JC_RESULT_EOF) {
        if (result != JC_RESULT_OK) {
            printf("E 0x%03X\n", result);
            break;
        }

        memset(token_buf, 0, sizeof(token_buf));
        token_len = token.end - token.start;
        token_buf_len = token_len > sizeof(token_buf) - 1
                      ? sizeof(token_buf) - 1
                      : token_len;
        strncpy(token_buf, src + token.start, token_buf_len);
        token_buf[token_buf_len + 1] = '\0';

        printf("T 0x%03X @ (%03ld, %03ld) [ %s ]\n", token.type, token.start,
                token.end, token_buf);
    }

    return 0;
}
