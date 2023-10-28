/*
    COP 3402 Systems Software
    Lexical Analyzer
    Authored by Caleb Rivera and Matthew Labrada
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#define MAX_IDENTIFIER_LENGTH 11
#define MAX_NUMBER_LENGTH 5
#define MAX_BUFFER_LENGTH 1000

FILE *input_file;
FILE *output_file;

// Define an enumeration for token types
typedef enum
{
    skipsym = 1,  // Represents a skip symbol
    identsym,     // Represents an identifier
    numbersym,    // Represents a number
    plussym,      // Represents the '+' symbol
    minussym,     // Represents the '-' symbol
    multsym,      // Represents the '*' symbol
    slashsym,     // Represents the '/' symbol
    ifelsym,      // Represents a combined 'if', 'else'
    eqsym,        // Represents the '=' symbol
    neqsym,       // Represents the '<>' symbol
    lessym,       // Represents the '<' symbol
    leqsym,       // Represents the '<=' symbol
    gtrsym,       // Represents the '>' symbol
    geqsym,       // Represents the '>=' symbol
    lparentsym,   // Represents the '(' symbol
    rparentsym,   // Represents the ')' symbol
    commasym,     // Represents the ',' symbol
    semicolonsym, // Represents the ';' symbol
    periodsym,    // Represents the '.' symbol
    becomessym,   // Represents the ':=' symbol
    beginsym,     // Represents the 'begin' keyword
    endsym,       // Represents the 'end' keyword
    ifsym,        // Represents the 'if' keyword
    thensym,      // Represents the 'then' keyword
    whilesym,     // Represents the 'while' keyword
    dosym,        // Represents the 'do' keyword
    callsym,      // Represents the 'call' keyword
    constsym,     // Represents the 'const' keyword
    varsym,       // Represents the 'var' keyword
    procsym,      // Represents the 'procedure' keyword
    writesym,     // Represents the 'write' keyword
    readsym,      // Represents the 'read' keyword
    elsesym       // Represents the 'else' keyword
} token_type;

// Struct to represent token
typedef struct
{
    char lexeme[MAX_BUFFER_LENGTH + 1]; // String representation of token (Ex: "+", "-", "end")
    char value[MAX_BUFFER_LENGTH + 1];  // Value/Type of token
} token;

typedef struct
{
    token *tokens; // Pointer to array of token structs
    int size;      // Num of tokens in list
    int capacity;  // Capacity of list
} list;

list *token_list; // Global pointer to list that holds all tokens

// Peek at the next character from the input file without consuming it
char peekc()
{
    int c = getc(input_file);
    ungetc(c, input_file);
    return (char)c;
}

// Print formatted output to both the console and the output file
void print_both(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    va_start(args, format);
    vfprintf(output_file, format, args);
    va_end(args);
}

// Print the entire source code from the input file to both the console and the output file
void print_source_code()
{
    char c;
    char lastChar = 0; // To keep track of the last character printed
    while ((c = fgetc(input_file)) != EOF)
    {
        print_both("%c", c);
        lastChar = c;
    }
    if (lastChar != '\n') // If the last character wasn't a newline, print one
        print_both("\n");
    rewind(input_file); // Reset file pointer to the beginning of the file
}

// Clear a string up to a specified index by setting characters to null
void clear_to_index(char *str, int index)
{
    for (int i = 0; i < index; i++)
        str[i] = '\0';
}

// Check if given buffer matches any reserved word, return its corresponding token value
int handle_reserved_word(char *buffer)
{
    if (strcmp(buffer, "const") == 0)
        return constsym;
    else if (strcmp(buffer, "var") == 0)
        return varsym;
    else if (strcmp(buffer, "procedure") == 0)
        return procsym;
    else if (strcmp(buffer, "call") == 0)
        return callsym;
    else if (strcmp(buffer, "begin") == 0)
        return beginsym;
    else if (strcmp(buffer, "end") == 0)
        return endsym;
    else if (strcmp(buffer, "if") == 0)
        return ifsym;
    else if (strcmp(buffer, "then") == 0)
        return thensym;
    else if (strcmp(buffer, "else") == 0)
        return elsesym;
    else if (strcmp(buffer, "while") == 0)
        return whilesym;
    else if (strcmp(buffer, "do") == 0)
        return dosym;
    else if (strcmp(buffer, "read") == 0)
        return readsym;
    else if (strcmp(buffer, "write") == 0)
        return writesym;
    else if (strcmp(buffer, "ifel") == 0)
        return ifelsym;
    return 0; // invalid reserved word
}

// Check if given buffer matches any special symbol, return its corresponding token value
int handle_special_symbol(char *buffer)
{
    if (strcmp(buffer, "+") == 0)
        return plussym;
    else if (strcmp(buffer, "-") == 0)
        return minussym;
    else if (strcmp(buffer, "*") == 0)
        return multsym;
    else if (strcmp(buffer, "/") == 0)
        return slashsym;
    else if (strcmp(buffer, "(") == 0)
        return lparentsym;
    else if (strcmp(buffer, ")") == 0)
        return rparentsym;
    else if (strcmp(buffer, ",") == 0)
        return commasym;
    else if (strcmp(buffer, ";") == 0)
        return semicolonsym;
    else if (strcmp(buffer, ".") == 0)
        return periodsym;
    else if (strcmp(buffer, "=") == 0)
        return eqsym;
    else if (strcmp(buffer, "<") == 0)
        return lessym;
    else if (strcmp(buffer, ">") == 0)
        return gtrsym;
    else if (strcmp(buffer, ":=") == 0)
        return becomessym;
    else if (strcmp(buffer, "<=") == 0)
        return leqsym;
    else if (strcmp(buffer, ">=") == 0)
        return geqsym;
    else if (strcmp(buffer, "<>") == 0)
        return neqsym;
    return 0; // invalid special symbol
}

// Check if a given character is a special symbol
int is_special_symbol(char c)
{
    // If character matches, return 1 else return 0
    return (c == '+' ||
            c == '-' ||
            c == '*' ||
            c == '/' ||
            c == '(' ||
            c == ')' ||
            c == '=' ||
            c == ',' ||
            c == '.' ||
            c == '<' ||
            c == '>' ||
            c == ':' ||
            c == ';' ||
            c == '&' ||
            c == '%' ||
            c == '!' ||
            c == '@' ||
            c == '#' ||
            c == '$' ||
            c == '?' ||
            c == '^' ||
            c == '`' ||
            c == '~' ||
            c == '|');
}

// Create and initialize new list for storing tokens
list *create_list()
{
    list *l = malloc(sizeof(list));
    l->size = 0;
    l->capacity = 10;
    l->tokens = malloc(sizeof(token) * l->capacity);
    return l;
}

// Free the memory used by a list and its tokens
list *destroy_list(list *l)
{
    free(l->tokens);
    free(l);
    return NULL;
}

// Append a token to a list, resizing the list if necessary
list *append_token(list *l, token t)
{
    if (l->size == l->capacity)
    {
        l->capacity *= 2;
        l->tokens = realloc(l->tokens, sizeof(token) * l->capacity);
    }
    l->tokens[l->size++] = t;
    return l;
}

// Add a token to a list, resizing the list if necessary
void add_token(list *l, token t)
{
    if (l->size == l->capacity)
    {
        l->capacity *= 2;
        l->tokens = realloc(l->tokens, sizeof(token) * l->capacity);
    }
    l->tokens[l->size++] = t;
}

// Print the lexeme table to both the console and output file
void print_lexeme_table(list *l)
{
    for (int i = 0; i < l->size; i++)
        print_both("%10s %20s\n", l->tokens[i].lexeme, l->tokens[i].value);
}

// Print the tokens to both the console and output file
void print_tokens(list *l)
{
    int counter; // Counter to keep track of the number of tokens printed for sake of ommitting extra new line character at end of file

    for (int i = 0; i < l->size; i++)
    {
        print_both("%s ", l->tokens[i].value);
        char identifier_value[3] = {0}, number_value[3] = {0};
        sprintf(identifier_value, "%d", identsym);
        sprintf(number_value, "%d", numbersym);

        // Check if the token value matches identifier or number and print its lexeme
        if (strcmp(l->tokens[i].value, identifier_value) == 0 || strcmp(l->tokens[i].value, number_value) == 0)
            print_both("%s ", l->tokens[i].lexeme);
        counter++;
    }

    // Print a newline if we haven't reached the last token
    if (counter < l->size - 1)
    {
        print_both("\n");
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        print_both("Usage: %s <input file> <output file>\n", argv[0]);
        return 1;
    }

    input_file = fopen(argv[1], "r");
    output_file = fopen(argv[2], "w");

    if (input_file == NULL)
    {
        print_both("Error: Could not open input file %s\n", argv[1]);
        return 1;
    }

    if (output_file == NULL)
    {
        print_both("Error: Could not open output file %s\n", argv[2]);
        return 1;
    }

    print_both("Source Program:\n");
    print_source_code();
    print_both("\n");
    print_both("Lexeme Table:\n");
    print_both("\n");
    print_both("%10s %20s\n", "lexeme", "token type");

    token_list = create_list();

    char c;
    char buffer[MAX_BUFFER_LENGTH + 1] = {0};
    int buffer_index = 0;

    while ((c = fgetc(input_file)) != EOF)
    {
        if (iscntrl(c) || isspace(c)) // Skip control characters and whitespace
        {
            c = fgetc(input_file);
        }
        if (isdigit(c)) // Handle numbers
        {
            buffer[buffer_index++] = c;
            while (1)
            {
                char nextc = peekc();
                if (isspace(nextc) || is_special_symbol(nextc)) // If next character is a space or special symbol, we've reached the end of the number
                {
                    token t;
                    if (buffer_index > MAX_NUMBER_LENGTH)
                    {
                        // Number is too long
                        print_both("%10s %20s\n", buffer, "ERROR: NUMBER TOO LONG");
                    }
                    else
                    {
                        // Number is valid
                        print_both("%10s %20d\n", buffer, numbersym);
                        sprintf(t.value, "%d", numbersym);
                        strcpy(t.lexeme, buffer);
                        append_token(token_list, t);
                    }

                    // Clear buffer and break out of loop
                    clear_to_index(buffer, buffer_index);
                    buffer_index = 0;
                    break;
                }
                else if (isdigit(nextc))
                {
                    // If next character is a digit, add it to the buffer
                    c = getc(input_file);
                    buffer[buffer_index++] = c;
                }
                else if (nextc == EOF) // This is the last character in the file
                    break;
                else if (isalpha(nextc))
                {
                    // Invalid number
                    token t;
                    print_both("%10s %20d\n", buffer, numbersym);
                    sprintf(t.value, "%d", numbersym);
                    strcpy(t.lexeme, buffer);
                    append_token(token_list, t);
                    clear_to_index(buffer, buffer_index);
                    buffer_index = 0;
                    break;
                }
            }
        }
        else if (isalpha(c)) // Handle identifiers and reserved words
        {
            buffer[buffer_index++] = c;
            while (1)
            {
                char nextc = peekc();
                if (isspace(nextc) || is_special_symbol(nextc) || nextc == EOF) // If next character is a space or special symbol, we've reached the end of the identifier
                {
                    // Check reserved words
                    int token_value = handle_reserved_word(buffer);
                    if (token_value)
                    {
                        token t;
                        print_both("%10s %20d\n", buffer, token_value);
                        sprintf(t.value, "%d", token_value);
                        strcpy(t.lexeme, buffer);
                        append_token(token_list, t);
                        clear_to_index(buffer, buffer_index);
                        buffer_index = 0;
                        break;
                    }
                    else
                    {
                        // Identifier
                        token t;
                        if (buffer_index > MAX_IDENTIFIER_LENGTH) // Check if identifier is too long
                        {
                            print_both("%10s %20s\n", buffer, "ERROR: IDENTIFIER TOO LONG");
                        }
                        else
                        {
                            // Valid identifier
                            print_both("%10s %20d\n", buffer, identsym);
                            sprintf(t.value, "%d", identsym);
                            strcpy(t.lexeme, buffer);
                            append_token(token_list, t);
                        }

                        clear_to_index(buffer, buffer_index);
                        buffer_index = 0;
                        break;
                    }
                    break;
                }
                else if (isalnum(nextc)) // If next character is a letter or digit, add it to the buffer
                {
                    c = getc(input_file);
                    buffer[buffer_index++] = c;
                }
            }
        }
        else if (is_special_symbol(c)) // Handle special symbols
        {
            buffer[buffer_index++] = c;
            char nextc = peekc();

            if (is_special_symbol(nextc)) // Current character is special and so is the next
            {
                // Handle block comments
                if (c == '/' && nextc == '*')
                {
                    // Clear buffer
                    clear_to_index(buffer, buffer_index);
                    buffer_index = 0;
                    while (1) // Consume characters until we reach the end of the block comment
                    {
                        c = getc(input_file);
                        nextc = peekc();
                        if (c == '*' && nextc == '/')
                        {
                            c = getc(input_file);
                            break;
                        }
                    }
                    continue;
                }

                // Handle single line comments
                if (c == '/' && nextc == '/')
                {
                    // Clear buffer
                    clear_to_index(buffer, buffer_index);
                    buffer_index = 0;
                    while (1) // Consume characters until we reach the end of the line
                    {
                        c = getc(input_file);
                        nextc = peekc();
                        if (c == '\n')
                        {
                            break;
                        }
                    }
                    continue;
                }

                // We have two pontentially valid symbols, so we need to check if they make a valid symbol

                c = getc(input_file);
                buffer[buffer_index++] = c;
                token t;
                int token_value = handle_special_symbol(buffer);
                if (!token_value)
                    // All symbols are invalid
                    for (int i = 0; i < buffer_index; i++)
                        print_both("%10c %20s\n", buffer[i], "ERROR: INVALID SYMBOL");
                else
                {
                    // Both symbols make a valid symbol
                    print_both("%10s %20d\n", buffer, token_value);
                    sprintf(t.value, "%d", token_value);
                    strcpy(t.lexeme, buffer);
                    append_token(token_list, t);
                }

                clear_to_index(buffer, buffer_index);
                buffer_index = 0;
            }
            else
            {
                // Handle single special symbol
                token t;
                int token_value = handle_special_symbol(buffer);
                if (!token_value)
                    print_both("%10c %20s\n", c, "ERROR: INVALID SYMBOL");
                else
                {
                    print_both("%10s %20d\n", buffer, token_value);
                    sprintf(t.value, "%d", token_value);
                    strcpy(t.lexeme, buffer);
                    append_token(token_list, t);
                }

                clear_to_index(buffer, buffer_index);
                buffer_index = 0;
            }
        }
    }

    print_both("\n");
    print_both("Token List:\n");
    print_tokens(token_list); // Print tokens to console and output file
    printf("\n");
    destroy_list(token_list); // Free memory used by token list

    return 0;
}