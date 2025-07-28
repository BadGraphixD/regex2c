# regex2c

Converts a regex string into c code, which parses the given regular expression.
For exact syntax specifications, look at the comment in regex2c.c.

# Build the project

Run `make` to build the regex2c executable. The executable outputs either c code, or debug info.
To change that, change the preprocessor-definition `PRINT_DEBUG` in common.h, a value of 0 prints the c code, larger values print more debug info.

There also exists the build script build_pattern_matcher.sh, which expects a regex string as the first argument. It then compiles the project,
uses regex2c to create pattern.c, which it then also compiles and links with pattern_matcher.c. The resulting executable pattern_matcher accepts
strings from stdin which match the given regex.

# How it works

The executable expects a non-empty regex-string from stdin and prints c code to stdout. It uses the following steps to convert the expression:

1. Parse the regex and convert to an AST
2. Convert the AST to a NFA (structures such as repititions and optionals are converted by inserting epsilon-transitions)
3. Convert the NFA to a DFA (making it deterministic)
4. Minimize the DFA (using Moore's algorithm)
5. Convert the DFA into c code, which can be compiled and linked with other code
