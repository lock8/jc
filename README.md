# jc

jc is a simple constant-memory JSON tokenizer suitable for resource-restricted
embedded environments. It's a one-header library written in C89; it doesn't have
any external dependencies except of a couple of stdlib string functions.

## Current Limitations:

- Supports only ASCII-strings. If you intend to use Unicode, make sure
that characters are encoded using `\uXXXX` notation.
- Supports object and array nesting only up to `JC_MAX_NESTING_LEVEL`
- Does not check JSON strings for validity, e.g. they may contain
unsupported escape sequences like `\a`. It's up to you to check it.
- Does not check JSON number for validity, e.g. `++-0EE` is considered a
valid number. It's up to you to check it.

## Installation

Copy [src/jc.h](src/jc.h) into your project.

## API

Public API consists of:

- `enum jc_token_type` containing all possible token types
- `enum jc_result` containing all possible result and error codes
- `struct jc_token` containing the type of token and its boundaries in the
  source string
- `struct jc_state` holding the state of tokenizer
- `jc_result jc_init(jc_state *, char const *)` function that initializes
  tokenizer with a source string
- `jc_result jc_next_token(jc_state *, jc_token *)` function that fetches next
  token from the source string
- `JC_MAX_NESTING_LEVEL` definition that sets desired maximum level of object
  and array nesting. Default is `8`.

## Examples

An example of a tokenizer that prints parts of JSON object supplied as its first
argument can be found in `examples` directory.

## License

Apache License Version 2
