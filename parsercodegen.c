/* Pharit Smitasin
 * COP 3402 Fall 2023
 * HW3
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
#define SYM_TABLE_MAX 500
#define IS_LEN_MAX 500

// list of enumerations for token types
// with oddsym starting at 1, the rest of the enumerations will be assigned values incrementing by 1
typedef enum
{
    oddsym = 1,   //  skip symbol
    identsym,     //  identifier symbol
    numbersym,    //  number symbol
    plussym,      //  '+' symbol
    minussym,     //  '-' symbol
    multsym,      //  '*' symbol
    slashsym,     //  '/' symbol
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
    constsym,     //  'const' keyword
    varsym,       //  'var' keyword
    writesym,     //  'write' keyword
    readsym,      //  'read' keyword
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

typedef struct
{
    int kind;      // const = 1, var = 2, proc = 3
    char name[10]; // name up to 11 chars
    int val;       // number (ASCII value)
    int level;     // L level
    int addr;      // M address
    int mark;      // to indicate unavailable or deleted
} symbol;

typedef struct
{
    int op; // opcode
    int l;  // L
    int m;  // M
} instruction;

FILE *inputFile;
FILE *outputFile;

FILE *inputFile;
FILE *outputFile;
symbol symbol_table[SYM_TABLE_MAX]; // Global symbol table
instruction code[IS_LEN_MAX];       // Global code array
int cx = 0;                         // Code index
int tx = 0;                         // Symbol table index
int level = 0;                      // Current level

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
int handle_reserved_word(char *buffer)
{
    if (strcmp(buffer, "const") == 0)
        return constsym;
    else if (strcmp(buffer, "var") == 0)
        return varsym;
    else if (strcmp(buffer, "begin") == 0)
        return beginsym;
    else if (strcmp(buffer, "end") == 0)
        return endsym;
    else if (strcmp(buffer, "if") == 0)
        return ifsym;
    else if (strcmp(buffer, "then") == 0)
        return thensym;
    else if (strcmp(buffer, "while") == 0)
        return whilesym;
    else if (strcmp(buffer, "do") == 0)
        return dosym;
    else if (strcmp(buffer, "read") == 0)
        return readsym;
    else if (strcmp(buffer, "write") == 0)
        return writesym;
    return 0; // invalid reserved word
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

// Parser/Codegen stuff
token current_token; // Keep track of current token

// Get next token from token list
void get_next_token()
{
    current_token = token_list->tokens[0];
    for (int i = 0; i < token_list->size - 1; i++)
    {
        token_list->tokens[i] = token_list->tokens[i + 1];
    }
    token_list->size--;
}

// Emit an instruction to the code array
void emit(int op, int l, int m)
{
    if (cx > IS_LEN_MAX)
    {
        error(16);
    }
    else
    {
        code[cx].op = op;
        code[cx].l = l;
        code[cx].m = m;
        cx++;
    }
}

// Print an error message and exit
void error(int error_code)
{
    print_both("Error: ");
    switch (error_code)
    {
    case 1:
        print_both("program must end with a period\n");
        break;
    case 2:
        print_both("const, var, and read keywords must be followed by identifier\n");
        break;
    case 3:
        print_both("symbol name has already been declared\n");
        break;
    case 4:
        print_both("constants must be assigned with =\n");
        break;
    case 5:
        print_both("constants must be assigned an integer value\n");
        break;
    case 6:
        print_both("constant and variables declarations must be followed by a semicolon\n");
        break;
    case 7:
        print_both("undeclared identifier %s\n", current_token.lexeme);
        break;
    case 8:
        print_both("only variable values may be altered\n");
        break;
    case 9:
        print_both("assignment statements must use :=\n");
        break;
    case 10:
        print_both("begin must be followed by end\n");
        break;
    case 11:
        print_both("if must be followed by then\n");
        break;
    case 12:
        print_both("while must be followed by do\n");
        break;
    case 13:
        print_both("condition must contain comparison operator\n");
        break;
    case 14:
        print_both("right parenthesis must follow left parenthesis\n");
        break;
    case 15:
        print_both("arithmetic equations must contain operands, parenthesis, numbers, or symbols\n");
        break;
    case 16:
        print_both("program too long\n");
    }
    exit(0);
}

// Check if a symbol is in the symbol table
int check_symbol_table(char *string)
{
    int i;
    for (i = 0; i < SYM_TABLE_MAX; i++)
    {
        if (strcmp(string, symbol_table[i].name) == 0)
        {
            return i;
        }
    }
    return -1;
}
// Add a symbol to the symbol table
void add_symbol(int kind, char *name, int val, int level, int addr)
{
    symbol_table[tx].kind = kind;
    strcpy(symbol_table[tx].name, name);
    symbol_table[tx].val = val;
    symbol_table[tx].level = level;
    symbol_table[tx].addr = addr;
    tx++;
}

// Parse the program
void program()
{
    get_next_token();                           // Get first token
    block();                                    // Parse block
    if (atoi(current_token.value) != periodsym) // Check if program ends with a period
    {
        error(1); // Error if it doesn't
    }
    emit(9, 0, 3); // Emit halt instruction
}

void block()
{
    const_declaration();              // Parse constants
    int num_vars = var_declaration(); // Parse variables
    emit(6, 0, 3 + num_vars);         // Emit INC instruction
    statement();                      // Parse statement
}

// Parse constants
void const_declaration()
{
    char name[ID_LEN_MAX + 1]; // Track name of constant
    // Check if current token is a const
    if (atoi(current_token.value) == constsym)
    {
        do
        {
            get_next_token();
            if (atoi(current_token.value) != identsym) // Check if next token is an identifier
            {
                error(2); // Error if it isn't
            }
            strcpy(name, current_token.lexeme);                 // Save name of constant
            if (check_symbol_table(current_token.lexeme) != -1) // Check if constant has already been declared
            {
                error(3); // Error if it has
            }
            get_next_token();
            if (atoi(current_token.value) != eqsym) // Check if next token is an equals sign
            {
                error(4); // Error if it isn't
            }
            get_next_token();
            if (atoi(current_token.value) != numbersym) // Check if next token is a number
            {
                error(5); // Error if it isn't
            }
            add_symbol(1, name, atoi(current_token.lexeme), level, 0); // Add constant to symbol table
            get_next_token();
        } while (atoi(current_token.value) == commasym); // Continue parsing constants if next token is a comma
        if (atoi(current_token.value) != semicolonsym)   // Check if next token is a semicolon
        {
            error(6); // Error if it isn't
        }
        get_next_token();
    }
}

// Parse variables
int var_declaration()
{
    int num_vars = 0;                        // Track number of variables
    if (atoi(current_token.value) == varsym) // Check if current token is a var
    {
        do
        {
            num_vars++; // Increment number of variables
            get_next_token();
            if (atoi(current_token.value) != identsym) // Check if next token is an identifier
            {
                error(2);
            }
            if (check_symbol_table(current_token.lexeme) != -1) // Check if variable has already been declared
            {
                error(3); // Error if it has
            }
            add_symbol(2, current_token.lexeme, 0, 0, num_vars + 2); // Add variable to symbol table
            get_next_token();
        } while (atoi(current_token.value) == commasym); // Continue parsing variables if next token is a comma
        if (atoi(current_token.value) != semicolonsym)   // Check if next token is a semicolon
        {
            error(6); // Error if it isn't
        }
        get_next_token();
    }
    return num_vars; // Return number of variables
}

// Parse statements
void statement()
{
    if (atoi(current_token.value) == identsym) // Check if current token is an identifier
    {
        int sx = check_symbol_table(current_token.lexeme); // Check if identifier is in symbol table
        if (sx == -1)
        {
            error(7); // Error if it isn't
        }
        if (symbol_table[sx].kind != 2) // Check if identifier is a variable
        {
            error(8); // Error if it isn't
        }
        get_next_token();
        if (atoi(current_token.value) != becomessym) // Check if next token is a becomes symbol (:=)
        {
            error(9); // Error if it isn't
        }
        get_next_token();
        expression();                      // Parse expression
        emit(4, 0, symbol_table[sx].addr); // Emit STO instruction
    }
    else if (atoi(current_token.value) == beginsym) // Check if current token is a begin
    {
        do
        {
            get_next_token();
            statement();                                     // Parse statement
        } while (atoi(current_token.value) == semicolonsym); // Continue parsing statements if next token is a semicolon
        if (atoi(current_token.value) != endsym)             // Check if next token is an end
        {
            error(10); // Error if it isn't
        }
        get_next_token();
    }
    else if (atoi(current_token.value) == ifsym) // Check if current token is an if
    {
        get_next_token();
        condition();                              // Parse condition
        int jx = cx;                              // Save current code index to jump to
        emit(8, 0, 0);                            // Emit JPC instruction
        if (atoi(current_token.value) != thensym) // Check if next token is a then
        {
            error(11); // Error if it isn't
        }
        get_next_token();
        statement();     // Parse statement
        code[jx].m = cx; // Set JPC instruction's M to current code index
    }
    else if (atoi(current_token.value) == whilesym) // Check if current token is a while
    {
        get_next_token();
        int lx = cx;
        condition();                            // Parse condition
        if (atoi(current_token.value) != dosym) // Check if next token is a do
        {
            error(12); // Error if it isn't
        }
        get_next_token();
        int jx = cx;     // Save current code index to jump to
        emit(8, 0, 0);   // Emit JPC instruction
        statement();     // Parse statement
        emit(7, 0, lx);  // Emit JMP instruction
        code[jx].m = cx; // Set JPC instruction's M to current code index
    }
    else if (atoi(current_token.value) == readsym) // Check if current token is a read
    {
        get_next_token();
        if (atoi(current_token.value) != identsym) // Check if current token is an identifier
        {
            error(2); // Error if it isn't
        }
        int sx = check_symbol_table(current_token.lexeme); // Check if identifier is in symbol table
        if (sx == -1)
        {
            error(7); // Error if it isn't
        }
        if (symbol_table[sx].kind != 2) // Check if identifier is a variable
        {
            error(8); // Error if it isn't
        }
        get_next_token();
        emit(9, 0, 2);  // Emit SIO instruction
        emit(4, 0, sx); // Emit STO instruction
    }
    else if (atoi(current_token.value) == writesym) // Check if current token is a write
    {
        get_next_token();
        expression();  // Parse expression
        emit(9, 0, 1); // Emit SIO instruction
    }
}

// Parse condition
void condition()
{
    if (atoi(current_token.value) == oddsym) // Check if current token is odd
    {
        get_next_token();
        expression();   // Parse expression
        emit(2, 0, 11); // Emit ODD instruction
    }
    else
    {
        expression();                      // Parse expression
        switch (atoi(current_token.value)) // Check if current token is a comparison operator
        {
        case eqsym:
            get_next_token();
            expression();
            emit(2, 0, 5); // Emit EQL instruction
            break;
        case neqsym:
            get_next_token();
            expression();
            emit(2, 0, 6); // Emit NEQ instruction
            break;
        case lessym:
            get_next_token();
            expression();
            emit(2, 0, 7); // Emit LSS instruction
            break;
        case leqsym:
            get_next_token();
            expression();
            emit(2, 0, 8); // Emit LEQ instruction
            break;
        case gtrsym:
            get_next_token();
            expression();
            emit(2, 0, 9); // Emit GTR instruction
            break;
        case geqsym:
            get_next_token();
            expression();
            emit(2, 0, 10); // Emit GEQ instruction
            break;
        default:
            error(13); // Error if it isn't
            break;
        }
    }
}

// Parse expression
void expression()
{
    term(); // Parse term
    // Check if current token is a plus or minus
    while (atoi(current_token.value) == plussym || atoi(current_token.value) == minussym)
    {
        get_next_token();
        term();                                   // Parse term
        if (atoi(current_token.value) == plussym) // Check if current token is a plus
        {
            emit(2, 0, 1); // Emit ADD instruction
        }
        else
        {
            emit(2, 0, 2); // Emit SUB instruction
        }
    }
}

// Parse term
void term()
{
    factor(); // Parse factor
    while (atoi(current_token.value) == multsym || atoi(current_token.value) == slashsym)
    {
        if (atoi(current_token.value) == multsym) // Check if current token is a multiply
        {
            get_next_token();
            factor();      // Parse factor
            emit(2, 0, 3); // Emit MUL
        }
        else
        {
            get_next_token();
            factor();      // Parse factor
            emit(2, 0, 4); // Emit DIV
        }
    }
}

// Parse factor
void factor()
{
    if (atoi(current_token.value) == identsym) // Check if current token is an identifier
    {
        int sx = check_symbol_table(current_token.lexeme); // Check if identifier is in symbol table
        if (sx == -1)
        {
            error(7); // Error if it isn't
        }
        if (symbol_table[sx].kind == 1) // Check if identifier is a constant
        {
            emit(1, 0, symbol_table[sx].val); // Emit LIT instruction
        }
        else
        {
            emit(3, level - symbol_table[sx].level, symbol_table[sx].addr); // Emit LOD instruction
        }
        get_next_token();
    }
    else if (atoi(current_token.value) == numbersym) // Check if current token is a number
    {
        emit(1, 0, atoi(current_token.lexeme)); // Emit LIT instruction
        get_next_token();
    }
    else if (atoi(current_token.value) == lparentsym) // Check if current token is a left parenthesis
    {
        get_next_token();
        expression();                                // Parse expression
        if (atoi(current_token.value) != rparentsym) // Check if currenet token is right parenthesis
        {
            error(14); // Error if it isn't
        }
        get_next_token();
    }
    else
    {
        error(15); // Error if current token is none of the above
    }
}

// Print symbol table
void print_symbol_table()
{
    print_both("Symbol Table:\n");
    print_both("%10s %10s %10s %10s %10s\n", "kind", "name", "val", "level", "addr");
    for (int i = 0; i < tx; i++)
    {
        print_both("%10d %10s %10d %10d %10d\n", symbol_table[i].kind, symbol_table[i].name, symbol_table[i].val, symbol_table[i].level, symbol_table[i].addr);
    }
}

// Print assmebly code
void print_instructions()
{

    print_both("Assembly Code:\n");
    print_both("%10s %10s %10s %10s\n", "line", "op", "l", "m");
    for (int i = 0; i < cx; i++)
    {
        char name[4];
        get_op_name(code[i].op, name);
        print_both("%10d %10s %10d %10d\n", i, name, code[i].l, code[i].m);
    }
}

// gets operation name from the op code
void get_op_name(int op, char *name)
{
    switch (op)
    {
    case 1:
        strcpy(name, "LIT");
        break;
    case 2:
        strcpy(name, "OPR");
        break;
    case 3:
        strcpy(name, "LOD");
        break;
    case 4:
        strcpy(name, "STO");
        break;
    case 5:
        strcpy(name, "CAL");
        break;
    case 6:
        strcpy(name, "INC");
        break;
    case 7:
        strcpy(name, "JMP");
        break;
    case 8:
        strcpy(name, "JPC");
        break;
    case 9:
        strcpy(name, "SYS");
        break;
    }
}

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        print_both("Usage: %s <input file> <output file>\n", argv[0]);
        return 1;
    }

    // establishing the files to read from, and one to write to (input/output)
    inputFile = fopen(argv[1], "r");
    outputFile = fopen(argv[2], "w");

    if (inputFile == NULL)
    {
        print_both("Error: Could not open input file %s\n", argv[1]);
        return 1;
    }

    if (outputFile == NULL)
    {
        print_both("Error: Could not open output file %s\n", argv[2]);
        return 1;
    }

    // creating the list of tokens
    token_list = createNewList();

    char c;
    int buffer_index = 0;

    char buffer[TOKEN_LEN_MAX + 1] = {0};

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

                if (!token_value) {
                    // given 2 symbols are invalid
                    // for (int i = 0; i < buffer_index; i++)
                    //     printOutput("%10c %20s\n", buffer[i], "ERROR: INVALID SYMBOL");
                }
                else
                {
                    // both valid symbols
                    // printOutput("%10s %20d\n", buffer, token_value);
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

    // First instruction is always JMP 0 3
    code[0].op = 7;
    code[0].l = 0;
    code[0].m = 3;
    cx++;

    // Read in tokens in the tokens list and generate code
    program();

    print_instructions();
    print_symbol_table();

    freeList(token_list);
    fclose(inputFile);  // Close input file
    fclose(outputFile); // Close output file

    return 0; // end of program
} // end of MAIN METHOD