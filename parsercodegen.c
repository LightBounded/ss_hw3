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
#define MAX_SYMBOL_TABLE_SIZE 500
#define MAX_INSTRUCTION_LENGTH MAX_SYMBOL_TABLE_SIZE

// Define an enumeration for token types
typedef enum
{
  oddsym = 1,   // Represents a skip symbol
  identsym,     // Represents an identifier
  numbersym,    // Represents a number
  plussym,      // Represents the '+' symbol
  minussym,     // Represents the '-' symbol
  multsym,      // Represents the '*' symbol
  slashsym,     // Represents the '/' symbol
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
  constsym,     // Represents the 'const' keyword
  varsym,       // Represents the 'var' keyword
  writesym,     // Represents the 'write' keyword
  readsym,      // Represents the 'read' keyword
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

FILE *input_file;                           // Input file pointer
FILE *output_file;                          // Output file pointer
symbol symbol_table[MAX_SYMBOL_TABLE_SIZE]; // Global symbol table
instruction code[MAX_INSTRUCTION_LENGTH];   // Global code array
int cx = 0;                                 // Code index
int tx = 0;                                 // Symbol table index
int level = 0;                              // Current level

// Function prototypes
char peekc();
void print_both(const char *format, ...);
void print_source_code();
void clear_to_index(char *str, int index);
int handle_reserved_word(char *buffer);
int handle_special_symbol(char *buffer);
int is_special_symbol(char c);
list *create_list();
list *destroy_list(list *l);
list *append_token(list *l, token t);
void add_token(list *l, token t);
void print_lexeme_table(list *l);
void print_tokens(list *l);

// Parser/Codegen function prototypes
void get_next_token();
void emit(int op, int l, int m);
void error(int error_code);
int check_symbol_table(char *string);
void add_symbol(int kind, char *name, int val, int level, int addr);
void program();
void block();
void const_declaration();
int var_declaration();
void statement();
void condition();
void expression();
void term();
void factor();
void print_symbol_table();
void print_instructions();
void get_op_name(int op, char *name);

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

  // print_both("Source Program:\n");
  // print_source_code();
  // print_both("\n");
  // print_both("Lexeme Table:\n");
  // print_both("\n");
  // print_both("%10s %20s\n", "lexeme", "token type");

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
            // print_both("%10s %20s\n", buffer, "ERROR: NUMBER TOO LONG");
          }
          else
          {
            // Number is valid
            // print_both("%10s %20d\n", buffer, numbersym);
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
          // print_both("%10s %20d\n", buffer, numbersym);
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
            // print_both("%10s %20d\n", buffer, token_value);
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
              // print_both("%10s %20s\n", buffer, "ERROR: IDENTIFIER TOO LONG");
            }
            else
            {
              // Valid identifier
              // print_both("%10s %20d\n", buffer, identsym);
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
        {
          // All symbols are invalid
          // for (int i = 0; i < buffer_index; i++)
          //   print_both("%10c %20s\n", buffer[i], "ERROR: INVALID SYMBOL");
        }
        else
        {
          // Both symbols make a valid symbol
          // print_both("%10s %20d\n", buffer, token_value);
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
        {
          // print_both("%10c %20s\n", c, "ERROR: INVALID SYMBOL");
        }
        else
        {
          // print_both("%10s %20d\n", buffer, token_value);
          sprintf(t.value, "%d", token_value);
          strcpy(t.lexeme, buffer);
          append_token(token_list, t);
        }

        clear_to_index(buffer, buffer_index);
        buffer_index = 0;
      }
    }
  }

  // print_both("\n");
  // print_both("Token List:\n");
  // print_tokens(token_list); // Print tokens to console and output file
  // printf("\n");

  // First instruction is always JMP 0 3
  code[0].op = 7;
  code[0].l = 0;
  code[0].m = 3;
  cx++;

  // Read in tokens in the tokens list and generate code
  program();

  print_instructions();
  print_symbol_table();

  destroy_list(token_list); // Free memory used by token list
  fclose(input_file);       // Close input file
  fclose(output_file);      // Close output file
  return 0;
}

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

// Parser/Codegen stuff
token current_token;
void get_next_token()
{
  current_token = token_list->tokens[0];
  for (int i = 0; i < token_list->size - 1; i++)
  {
    token_list->tokens[i] = token_list->tokens[i + 1];
  }
  token_list->size--;
}

void emit(int op, int l, int m)
{
  if (cx > MAX_INSTRUCTION_LENGTH)
  {
    print_both("Error: Program is too long\n");
    exit(0);
  }
  else
  {
    code[cx].op = op;
    code[cx].l = l;
    code[cx].m = m;
    cx++;
  }
}

void error(int error_code)
{
  printf("Error: ");
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
  }

  exit(0);
}

int check_symbol_table(char *string)
{
  int i;
  for (i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++)
  {
    if (strcmp(string, symbol_table[i].name) == 0)
    {
      return i;
    }
  }
  return -1;
}

void add_symbol(int kind, char *name, int val, int level, int addr)
{
  symbol_table[tx].kind = kind;
  strcpy(symbol_table[tx].name, name);
  symbol_table[tx].val = val;
  symbol_table[tx].level = level;
  symbol_table[tx].addr = addr;
  tx++;
}

void program()
{
  get_next_token();
  block();
  if (atoi(current_token.value) != periodsym)
  {
    error(1);
  }
  emit(9, 0, 3);
}

void block()
{
  const_declaration();
  int num_vars = var_declaration();
  emit(6, 0, 3 + num_vars);
  statement();
}

void const_declaration()
{
  if (atoi(current_token.value) == constsym)
  {
    do
    {
      get_next_token();
      if (atoi(current_token.value) != identsym)
      {
        error(2);
      }
      if (check_symbol_table(current_token.lexeme) != -1)
      {
        error(3);
      }
      get_next_token();
      if (atoi(current_token.value) != eqsym)
      {
        error(4);
      }
      get_next_token();
      if (atoi(current_token.value) != numbersym)
      {
        error(5);
      }
      add_symbol(1, current_token.lexeme, atoi(current_token.lexeme), level, 0);
      get_next_token();
    } while (atoi(current_token.value) == commasym);
    if (atoi(current_token.value) != semicolonsym)
    {
      error(6);
    }
    get_next_token();
  }
}

int var_declaration()
{
  int num_vars = 0;
  if (atoi(current_token.value) == varsym)
  {
    do
    {
      num_vars++;
      get_next_token();
      if (atoi(current_token.value) != identsym)
      {
        error(4);
      }
      if (check_symbol_table(current_token.lexeme) != -1)
      {
        error(3);
      }
      add_symbol(2, current_token.lexeme, 0, 0, num_vars + 2);
      get_next_token();
    } while (atoi(current_token.value) == commasym);
    if (atoi(current_token.value) != semicolonsym)
    {
      error(5);
    }
    get_next_token();
  }
  return num_vars;
}

void statement()
{
  if (atoi(current_token.value) == identsym)
  {
    int sx = check_symbol_table(current_token.lexeme);
    if (sx == -1)
    {
      error(7);
    }
    if (symbol_table[sx].kind != 2)
    {
      error(8);
    }
    get_next_token();
    if (atoi(current_token.value) != becomessym)
    {
      error(9);
    }
    get_next_token();
    expression();
    emit(4, 0, symbol_table[sx].addr);
  }
  else if (atoi(current_token.value) == beginsym)
  {
    do
    {
      get_next_token();
      statement();
    } while (atoi(current_token.value) == semicolonsym);
    if (atoi(current_token.value) != endsym)
    {
      error(10);
    }
    get_next_token();
  }
  else if (atoi(current_token.value) == ifsym)
  {
    get_next_token();
    condition();
    int jx = cx;
    emit(8, 0, 0);
    if (atoi(current_token.value) != thensym)
    {
      error(11);
    }
    get_next_token();
    statement();
    code[jx].m = cx;
  }
  else if (atoi(current_token.value) == whilesym)
  {
    get_next_token();
    int lx = cx;
    condition();
    if (atoi(current_token.value) != dosym)
    {
      error(12);
    }
    get_next_token();
    int jx = cx;
    emit(8, 0, 0);
    statement();
    emit(7, 0, lx);
    code[jx].m = cx;
  }
  else if (atoi(current_token.value) == readsym)
  {
    get_next_token();
    if (atoi(current_token.value) != identsym)
    {
      error(2);
    }
    int sx = check_symbol_table(current_token.lexeme);
    if (sx == -1)
    {
      error(7);
    }
    if (symbol_table[sx].kind != 2)
    {
      error(8);
    }
    get_next_token();
    emit(9, 0, 2);
    emit(4, 0, sx);
  }
  else if (atoi(current_token.value) == writesym)
  {
    get_next_token();
    expression();
    emit(9, 0, 1);
  }
}

void condition()
{
  if (atoi(current_token.value) == oddsym)
  {
    get_next_token();
    expression();
    emit(2, 0, 11);
  }
  else
  {
    expression();
    switch (atoi(current_token.value))
    {
    case eqsym:
      get_next_token();
      expression();
      emit(2, 0, 5);
      break;
    case neqsym:
      get_next_token();
      expression();
      emit(2, 0, 6);
      break;
    case lessym:
      get_next_token();
      expression();
      emit(2, 0, 7);
      break;
    case leqsym:
      get_next_token();
      expression();
      emit(2, 0, 8);
      break;
    case gtrsym:
      get_next_token();
      expression();
      emit(2, 0, 9);
      break;
    case geqsym:
      get_next_token();
      expression();
      emit(2, 0, 10);
      break;
    default:
      error(13);
      break;
    }
  }
}

// expression ::=  term { ("+"|"-") term}.
void expression()
{
  term();
  while (atoi(current_token.value) == plussym || atoi(current_token.value) == minussym)
  {
    get_next_token();
    term();
    if (atoi(current_token.value) == plussym)
    {
      emit(2, 0, 1);
    }
    else
    {
      emit(2, 0, 2);
    }
  }
}

void term()
{
  factor();
  while (atoi(current_token.value) == multsym || atoi(current_token.value) == slashsym)
  {
    if (atoi(current_token.value) == multsym)
    {
      get_next_token();
      factor();
      emit(2, 0, 3);
    }
    else
    {
      get_next_token();
      factor();
      emit(2, 0, 4);
    }
  }
}

void factor()
{
  if (atoi(current_token.value) == identsym)
  {
    int sx = check_symbol_table(current_token.lexeme);
    if (sx == -1)
    {
      error(7);
    }
    if (symbol_table[sx].kind == 1)
    {
      emit(1, 0, symbol_table[sx].val);
    }
    else
    {
      emit(3, level - symbol_table[sx].level, symbol_table[sx].addr);
    }
    get_next_token();
  }
  else if (atoi(current_token.value) == numbersym)
  {
    emit(1, 0, atoi(current_token.lexeme));
    get_next_token();
  }
  else if (atoi(current_token.value) == lparentsym)
  {
    get_next_token();
    expression();
    if (atoi(current_token.value) != rparentsym)
    {
      error(14);
    }
    get_next_token();
  }
  else
  {
    error(15);
  }
}

void print_symbol_table()
{
  print_both("Symbol Table:\n");
  print_both("%10s %10s %10s %10s %10s\n", "kind", "name", "val", "level", "addr");
  for (int i = 0; i < tx; i++)
  {
    print_both("%10d %10s %10d %10d %10d\n", symbol_table[i].kind, symbol_table[i].name, symbol_table[i].val, symbol_table[i].level, symbol_table[i].addr);
  }
}

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