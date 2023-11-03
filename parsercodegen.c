/* Pharit Smitasin
 * COP 3402 Fall 2023
 * HW2
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

// These defined values are the max lengths for identifiers, numbers, and tokens
#define ID_LEN_MAX 11
#define NUM_LEN_MAX 5
#define TOKEN_LEN_MAX 1000
#define MAX_SYMBOL_TABLE_SIZE 500
#define MAX_INSTRUCTION_LENGTH MAX_SYMBOL_TABLE_SIZE

// FILE *inputFile;
// FILE *outputFile;

// list of enumerations for token types
// with skipsym starting at 1, the rest of the enumerations will be assigned values incrementing by 1
typedef enum
{
    skipsym = 1,  //  skip symbol
    identsym,     //  identifier symbol
    numbersym,    //  number symbol
    plussym,      //  '+' symbol
    minussym,     //  '-' symbol
    multsym,      //  '*' symbol
    slashsym,     //  '/' symbol
    ifelsym,      //  'if', 'else' keyword
    eqsym,        //  '=' symbol
    neqsym,       //  '<>' symbol
    lessym,       //  '<' symbol
    leqsym,       //  '<=' symbol
    gtrsym,       //  '>' symbol
    geqsym,       //  '>=' symbol
    lparentsym,   //  '(' symbol
    rparentsym,   //  ')' symbol
    commasym,     //  ',' symbol
    semicolonsym, //  ';' symbol
    periodsym,    //  '.' symbol
    becomessym,   //  ':=' symbol
    beginsym,     //  'begin' keyword
    endsym,       //  'end' keyword
    ifsym,        //  'if' keyword
    thensym,      //  'then' keyword
    whilesym,     //  'while' keyword
    dosym,        //  'do' keyword
    callsym,      //  'call' keyword
    constsym,     //  'const' keyword
    varsym,       //  'var' keyword
    procsym,      //  'procedure' keyword
    writesym,     //  'write' keyword
    readsym,      //  'read' keyword
    elsesym       //  'else' keyword
} token_type;

// token struct
typedef struct
{
    char value[TOKEN_LEN_MAX + 1];
    char lexeme[TOKEN_LEN_MAX + 1];
} token;

// list of tokens struct
typedef struct
{
    token *tokens;
    int size;
    int capacity;
} list;

// this is the list of tokens that will be used to store the tokens for the parser
list *token_list;

// reads next character from input file
char peekc()
{
    int nextChar = getc(inputFile);
    ungetc(nextChar, inputFile);

    return (char)nextChar;
}

// prints output to console and output file
void printOutput(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    int result = vprintf(format, args);
    va_end(args);

    va_start(args, format);
    int fileResult = vfprintf(outputFile, format, args);
    va_end(args);

    if (result < 0 || fileResult < 0)
    {
        printf("Error occurred while writing to output file.\n");
    }
}

// this prints the original to the console above the lexeme and in the the output file
void printOriginal()
{
    char buffer[1024];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), inputFile)) > 0)
    {
        fwrite(buffer, 1, bytesRead, stdout);
        fwrite(buffer, 1, bytesRead, outputFile);
    }

    rewind(inputFile); // Reset file pointer to the beginning of the file
}

// sets chars in the specified string to null up to the parameter index using '\0'
void cutString(char *string, int cutoff)
{
    for (int i = 0; i < cutoff; i++)
    {
        string[i] = '\0';
    }
}

// checks if the given string is a reserved word and returns its token value
// in = reserved word, out = token value
int reservedToToken(char *buffer)
{
    char *reserved_words[] = {"const", "var", "procedure", "call", "begin", "end", "if", "then", "else", "while", "do", "read", "write", "ifel"};
    int reserved_word_values[] = {constsym, varsym, procsym, callsym, beginsym, endsym, ifsym, thensym, elsesym, whilesym, dosym, readsym, writesym, ifelsym};
    int num_reserved_words = sizeof(reserved_words) / sizeof(reserved_words[0]);

    // loop through reserved words and check if buffer matches any of them
    for (int i = 0; i < num_reserved_words; i++)
    {
        if (strcmp(buffer, reserved_words[i]) == 0)
        {
            return reserved_word_values[i];
        }
    }

    return 0; // invalid word
}

// checks if the given string matches any special symbo and return its token value
// this is literally the same as the reservedToToken function but with different arrays
int specialToToken(char *buffer)
{
    char *special_symbols[] = {"+", "-", "*", "/", "(", ")", ",", ";", ".", "=", "<", ">", ":=", "<=", ">=", "<>"};
    int special_symbol_values[] = {plussym, minussym, multsym, slashsym, lparentsym, rparentsym, commasym, semicolonsym, periodsym, eqsym, lessym, gtrsym, becomessym, leqsym, geqsym, neqsym};
    int num_special_symbols = sizeof(special_symbols) / sizeof(special_symbols[0]);

    // loop through special symbols and check if buffer matches any of them
    for (int i = 0; i < num_special_symbols; i++)
    {
        if (strcmp(buffer, special_symbols[i]) == 0)
        {
            return special_symbol_values[i];
        }
    }

    return 0; // invalid symbol
}

// checks if a character is a special symbol, returning 1 for true and 0 for false
int specialSymbolCheck(char c)
{
    char special_symbols[] = "+-*/()=,.<>:;&%!?@#$^`~|";
    int num_special_symbols = sizeof(special_symbols) / sizeof(special_symbols[0]);

    for (int i = 0; i < num_special_symbols; i++)
    {
        if (c == special_symbols[i])
        {
            return 1;
        }
    }

    return 0;
}

// creates and returns lists for tokens, properally allocating memory
list *createNewList()
{
    list *newList = malloc(sizeof(list));                        // Allocate memory for list
    newList->size = 0;                                           // initial size is zero
    newList->capacity = 10;                                      // max cap of 10
    newList->tokens = malloc(sizeof(token) * newList->capacity); // allocate memory for tokens

    return newList;
}

// completely frees memory of list (starting with tokens)
list *freeList(list *l)
{
    free(l->tokens);
    free(l);

    return NULL;
}

// Given the list and new token, adds it to the list and resizes IF size equals capacity
list *appendToken(list *list, token t)
{
    if (list->size == list->capacity)
    {
        list->capacity *= 2;
        token *new_tokens = malloc(sizeof(token) * list->capacity);
        for (int i = 0; i < list->size; i++)
        {
            new_tokens[i] = list->tokens[i];
        }
        free(list->tokens);
        list->tokens = new_tokens;
    }
    list->tokens[list->size] = t;
    list->size++;
    return list;
}

// prints all of the tokens in list to output and console
void printAllTokens(list *l)
{
    for (int i = 0; i < l->size; i++)
    {
        token t = l->tokens[i];
        printOutput("%s", t.value);
        if (strcmp(t.value, "2") == 0 || strcmp(t.value, "3") == 0)
        {
            printOutput(" %s", t.lexeme);
        }
        if (i < l->size - 1)
        {
            printOutput(" ");
        }
    }
    printOutput("\n");
}

int main(int argc, char *argv[])
{
    // establishing the files to read from, and one to write to (input/output)
    inputFile = fopen(argv[1], "r");
    outputFile = fopen(argv[2], "w");

    // beginning of printing for top of output to beginning of lexeme table
    printOutput("Source Program:\n");
    printOriginal();
    printOutput("\n");

    printOutput("Lexeme Table:\n");
    printOutput("\n");
    printOutput("%10s %20s\n", "lexeme", "token type");

    // creating the list of tokens
    token_list = createNewList();

    char c;

    char buffer[TOKEN_LEN_MAX + 1] = {0};
    int buffer_index = 0;

    // loop through the input file and check for tokens until end of file (EOF)
    // this loop goes char by char
    while ((c = fgetc(inputFile)) != EOF)
    {

        if (iscntrl(c) || isspace(c)) // Skip control characters and whitespace
        {
            c = fgetc(inputFile);
        }

        // this loop handles all digits
        if (isdigit(c))
        {
            buffer[buffer_index++] = c;

            while (1)
            {

                char nextc = peekc();

                if (isspace(nextc) || specialSymbolCheck(nextc)) // If next character is a space or special symbol, we've reached the end of the number
                {
                    token t;
                    if (buffer_index > NUM_LEN_MAX)
                    {
                        // given number is larger than max length
                        printOutput("%10s %20s\n", buffer, "ERROR: NUMBER TOO LONG");
                    }
                    else
                    {
                        // given number is VALID
                        printOutput("%10s %20d\n", buffer, numbersym);
                        sprintf(t.value, "%d", numbersym);
                        strcpy(t.lexeme, buffer);
                        appendToken(token_list, t);
                    }

                    // cut the buffer up to buffer index and then reset index
                    cutString(buffer, buffer_index);
                    buffer_index = 0;

                    break; // done with loop, break
                }
                else if (isdigit(nextc))
                {
                    // adds digit to buffet
                    c = getc(inputFile);
                    buffer[buffer_index++] = c;
                }
                else if (nextc == EOF)
                    break; // end of file reached, break from loop
                else if (isalpha(nextc))
                {
                    // next char is invalid number
                    token tokenOfInvalid;

                    printOutput("%10s %20d\n", buffer, numbersym);

                    sprintf(tokenOfInvalid.value, "%d", numbersym);
                    strcpy(tokenOfInvalid.lexeme, buffer);

                    appendToken(token_list, tokenOfInvalid);
                    cutString(buffer, buffer_index);

                    buffer_index = 0; // reset index to start of buffer
                    break;
                }
            }
        }
        else if (isalpha(c))
        // idents and reserve words
        {
            buffer[buffer_index++] = c;
            while (1)
            {
                char nextc = peekc();
                // end of the ident will be a special symb/space
                if (isspace(nextc) || specialSymbolCheck(nextc) || nextc == EOF)
                {
                    // handling for reserved words, checks first
                    int token_value = reservedToToken(buffer);
                    if (token_value)
                    {
                        token reserve;
                        printOutput("%10s %20d\n", buffer, token_value);
                        sprintf(reserve.value, "%d", token_value);
                        strcpy(reserve.lexeme, buffer);
                        appendToken(token_list, reserve);
                        cutString(buffer, buffer_index);
                        buffer_index = 0;
                        break;
                    }
                    else
                    {
                        // identifier handling
                        token ident;

                        if (buffer_index > ID_LEN_MAX) // Check if identifier is too long
                        {
                            printOutput("%10s %20s\n", buffer, "ERROR: IDENTIFIER TOO LONG");
                        }

                        else
                        {
                            printOutput("%10s %20d\n", buffer, identsym);
                            sprintf(ident.value, "%d", identsym);
                            strcpy(ident.lexeme, buffer);
                            appendToken(token_list, ident);
                        }

                        cutString(buffer, buffer_index);
                        buffer_index = 0;
                        break;
                    }
                    break;
                }
                else if (isalnum(nextc))
                // if char or int, add to buffer
                {
                    c = getc(inputFile);
                    buffer[buffer_index++] = c;
                }
            }
        }
        else if (specialSymbolCheck(c))
        // for handling special symbols
        {
            buffer[buffer_index++] = c;
            char nextc = peekc();

            if (specialSymbolCheck(nextc))
            {
                // for handling multi line comments
                if (c == '/' && nextc == '*')
                {
                    // cut string until buffer index and reset index
                    cutString(buffer, buffer_index);
                    buffer_index = 0;

                    while (1)
                    // gets characters until the end of the block comment
                    {
                        c = getc(inputFile);
                        nextc = peekc();
                        if (c == '*' && nextc == '/')
                        {
                            c = getc(inputFile);
                            break;
                        }
                    }
                    continue;
                }

                // single line comment handling
                if (c == '/' && nextc == '/')
                {
                    // cut buffer until buffer index and reset index
                    cutString(buffer, buffer_index);
                    buffer_index = 0;

                    while (1)
                    // until the newline, gets characters
                    {
                        c = getc(inputFile);
                        nextc = peekc();
                        if (c == '\n')
                        {
                            break;
                        }
                    }
                    continue;
                }

                c = getc(inputFile);

                buffer[buffer_index++] = c;
                token doubleSpecial;
                int token_value = specialToToken(buffer);

                if (!token_value)
                    // given 2 symbols are invalid
                    for (int i = 0; i < buffer_index; i++)
                        printOutput("%10c %20s\n", buffer[i], "ERROR: INVALID SYMBOL");
                else
                {
                    // both valid symbols
                    printOutput("%10s %20d\n", buffer, token_value);
                    sprintf(doubleSpecial.value, "%d", token_value);
                    strcpy(doubleSpecial.lexeme, buffer);
                    appendToken(token_list, doubleSpecial);
                }

                cutString(buffer, buffer_index);
                buffer_index = 0;
            }
            else
            {
                // for the case of single special symbols
                token singleSpecial;
                int token_value = specialToToken(buffer);
                if (!token_value)
                    printOutput("%10c %20s\n", c, "ERROR: INVALID SYMBOL");
                else
                {
                    printOutput("%10s %20d\n", buffer, token_value);
                    sprintf(singleSpecial.value, "%d", token_value);
                    strcpy(singleSpecial.lexeme, buffer);
                    appendToken(token_list, singleSpecial);
                }

                cutString(buffer, buffer_index);
                buffer_index = 0;
            }
        }
    }

    // this is where all the printing and freeing happens for list
    printOutput("\n");
    printOutput("Token List:\n");

    printAllTokens(token_list);
    printf("\n");

    freeList(token_list);

    return 0; // end of program
} // end of MAIN METHOD