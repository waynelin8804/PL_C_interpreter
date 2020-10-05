// ToDo:
//   Parameters reference

# include <stdio.h>
# include <stdlib.h>
# include <ctype.h>
# include <string.h>
# include <iostream>
# include <string>
# include <vector>
# include <map>

# define EPSINON 0.0001
# define EOF_EXCEPTION "End of file"
# define CIN 1
# define COUT -1
# define NORMAL 0

using namespace std;

enum TokenType {
  TYPE_UNSET               = 0,
  TYPE_ID                  = 1,
  TYPE_RESERVE_WORD_INT    = 2,  // 'int'
  TYPE_RESERVE_WORD_FLOAT  = 3,  // 'float'
  TYPE_RESERVE_WORD_CHAR   = 4,  // 'char'
  TYPE_RESERVE_WORD_BOOL   = 5,  // 'bool'
  TYPE_RESERVE_WORD_STRING = 6,  // 'string'
  TYPE_RESERVE_WORD_VOID   = 7,  // 'void'
  TYPE_RESERVE_WORD_IF     = 8,  // 'if'
  TYPE_RESERVE_WORD_ELSE   = 9,  // 'else'
  TYPE_RESERVE_WORD_WHILE  = 10, // 'while'
  TYPE_RESERVE_WORD_DO     = 11, // 'do'
  TYPE_RESERVE_WORD_RETURN = 12, // 'return'
  TYPE_LEFT_SMALL_BRACKET  = 13, // '('
  TYPE_RIGHT_SMALL_BRACKET = 14, // ')'
  TYPE_LEFT_MID_BRACKET    = 15, // '['
  TYPE_RIGHT_MID_BRACKET   = 16, // ']'
  TYPE_LEFT_BIG_BRACKET    = 17, // '{'
  TYPE_RIGHT_BIG_BRACKET   = 18, // '}'
  TYPE_ADD                 = 19, // '+'
  TYPE_SUB                 = 20, // '-'
  TYPE_MUL                 = 21, // '*'
  TYPE_DIV                 = 22, // '/'
  TYPE_MOD                 = 23, // '%'
  TYPE_XOR                 = 24, // '^'
  TYPE_GT                  = 25, // '>'
  TYPE_LT                  = 26, // '<'
  TYPE_GE                  = 27, // '>='
  TYPE_LE                  = 28, // '<='
  TYPE_EQ                  = 29, // '=='
  TYPE_NE                  = 30, // '!='
  TYPE_BIT_AND             = 31, // '&'
  TYPE_BIT_OR              = 32, // '|'
  TYPE_ASSIGN              = 33, // '='
  TYPE_NOT                 = 34, // '!'
  TYPE_AND                 = 35, // '&&'
  TYPE_OR                  = 36, // '||'
  TYPE_PE                  = 37, // '+='
  TYPE_ME                  = 38, // '-='
  TYPE_TE                  = 39, // '*='
  TYPE_DE                  = 40, // '/='
  TYPE_RE                  = 41, // '%='
  TYPE_PP                  = 42, // '++'
  TYPE_MM                  = 43, // '--'
  TYPE_RS                  = 44, // '>>'
  TYPE_LS                  = 45, // '<<'
  TYPE_SEMICOLON           = 46, // ';'
  TYPE_COMMA               = 47, // ','
  TYPE_ROMCE               = 48, // '?'
  TYPE_ROMLOE              = 49, // ':'
  TYPE_CONST_INT           = 50, // 3
  TYPE_CONST_FLOAT         = 51, // 3.0
  TYPE_CONST_CHAR          = 52, // 'a'
  TYPE_CONST_BOOL          = 53, // true
  TYPE_CONST_STRING        = 54, // "String"
  TYPE_CONST_VOID          = 55
}; // TokenType

enum RunMode {
  MODE_SYNTAX_CHECK  = 0,
  MODE_EXECUTION     = 1,
  MODE_LIST_FUNCTION = 2
}; // RunMode

struct Token_s {
  string str;       // Token orginal string
  TokenType type;   // Token type
  int column;       // Column of token
  int line;         // Line of token
}; // struct Token_s

// type_specifier Identifier [ '[' Constant ']' ]
struct Token_Value_s {
  string str;
  TokenType type;       // Token data type
  bool isArray;         // Need to check index bound
  vector<string> value; // Token value
  int index;            // Get/Set token value by index, -1: Select all, others: certain index
}; // Token_value_s

typedef map<string, Token_Value_s> Table; // Token_Value_s tokenValue = Table[string key]
typedef vector<Token_s> TokenLine_t;
typedef char Str100[100];

// type_specifier [ '&' ] Identifier [ '[' Constant ']' ]
struct Parameter_s{
  string str;          // Name of parameter
  bool isReference;    // Need to pass by reference
  Token_Value_s info;  // Rest of information of parameter
  int isRefStackLayer; // Stack layer for call by reference
};

typedef map<string, Parameter_s> ParameterTable;
typedef vector<Parameter_s> ParameterList_t;
typedef vector<Token_Value_s> ArgumentList_t;

struct Function_Format_s {
  ParameterList_t parameters;   // Parameters of function
  map<int, int>   jumpTable;    // Record the source and destination for jump
  TokenType       returnType;   // Type of return value
  int             startAddress; // Starting address of function
};

struct Function_Variable_s {
  ParameterTable parameterTable;     // Parameters of function
  Table          localVariableTable; // Local variables in function
  Token_Value_s  returnValue;        // Return value
  int            returnAddress;      // Return address
};

typedef map<string, vector<Function_Format_s> > FunctionTable;

static int uIfNum;
static int uTestNum; // Test number

class TokenException {
public:
  TokenException() {
  } // TokenException()
 
  TokenException( string message ) {
    mMessage = message;
  } // TokenException()
 
  string Message() {
    return mMessage;
  } // Message()
 
protected:
  string mMessage;
}; // class TokenException

class UnrecognizedTokenException : public TokenException {
public:
  UnrecognizedTokenException( Token_s token ) {
    Str100 errorMsg;
    sprintf( errorMsg,
             "Line %d : unrecognized token with first char : \'%c\'", token.line, token.str.at( 0 ) );
    mMessage = string( errorMsg );
  } // UnrecognizedTokenException()
}; // class UnrecognizedTokenException

class UnexpectedTokenException : public TokenException {
public:
  UnexpectedTokenException( Token_s token ) {
    Str100 errorMsg;
    sprintf( errorMsg,
             "Line %d : unexpected token \'%s\'", token.line, token.str.c_str() );
    mMessage = string( errorMsg );
  } // UnexpectedTokenException()
}; // class UnexpectedTokenException

class UndefinedTokenException : public TokenException {
public:
  UndefinedTokenException( Token_s token ) {
    Str100 errorMsg;
    sprintf( errorMsg,
             "Line %d : undefined identifier \'%s\'", token.line, token.str.c_str() );
    mMessage = string( errorMsg );
  } // UndefinedTokenException()
}; // class UndefinedTokenException

class ReturnException : public TokenException {
public:
  ReturnException() {
    Str100 errorMsg;
    sprintf( errorMsg,
             "Return outside function" );
    mMessage = string( errorMsg );
  } // ReturnException()
}; // class ReturnException

class Scanner {
private:
  int mLine;           // 「下一個要讀進來的字元」所在的line number
  int mColumn;         // 「下一個要讀進來的字元」所在的column number
  int mPreviousLine;   // Line of previous char
  int mPreviousColumn; // Column of previous char
  int mLastGottenChar;
  string mUngetBuf;
  int mUngetBufIndex;
  TokenLine_t mMainMemory; // Token group
  int mPC;                 // Index of next token
  int mFunctionSegPointer; // Index of function segment's last token
  int mPreviousFunctionSegPointer; // Previous index of function segment's last token

  void Reset() {
    mLine = mColumn = 1;
    mUngetBuf = "";
    mMainMemory.clear();
    mFunctionSegPointer = -1;
  } // Reset()

  Token_s GetTokenFromInput() {
    // Get every case token
    int ch;
    Token_s token;

    // Get start char of token
    mUngetBufIndex = mUngetBuf.size() - 1;
    token.str = "";
    F2_GetNonWhiteSpaceChar( ch, token.line, token.column );
    if ( ch == EOF )
      throw EOF_EXCEPTION;
    token.str += ch;

    // Classify
    if ( ch == '_' || isalpha( ch ) )
      F3_GetIdentifier( token );
    else if ( isdigit( ch ) )
      F4_GetConstantNumber( token );
    else if ( ch == '.' ) {
      // Peek next char
      F1_GetChar( ch );
      F1_UngetChar( ch );
      if ( !isdigit( ch ) )
        throw UnexpectedTokenException( token );
      F4_GetConstantNumber( token );
    } // else if
    else if ( ch == '\"' )
      F4_GetConstantString( token );
    else if ( ch == '\'' )
      F4_GetConstantChar( token );
    else
      F5_GetSpecial( token );

    return token;
  } // GetTokenFromInput()

  void F1_GetChar( int & ch ) {
    // Get char and update line column

    // Backup line column
    mPreviousLine = mLine;
    mPreviousColumn = mColumn;

    // Get a char
    if ( mUngetBufIndex == -1 ) {
      ch = getchar();
      mUngetBuf.insert( mUngetBuf.begin(), ch );
    } // if
    else
      ch = mUngetBuf.at( mUngetBufIndex-- );

    mLastGottenChar = ch;
    if ( ch == EOF )
      return;

    // Update line column
    if ( ch != '\n' )
      mColumn++;
    else {
      mLine++;
      mColumn = 1;
    } // else
  } // F1_GetChar()

  void F1_UngetChar( int ch ) {
    // Put back char
    if ( ch == EOF )
      return;
  
    // Call standard C function to unget char
    mUngetBufIndex++;
    // Restore line column
    mLine = mPreviousLine;
    mColumn = mPreviousColumn;
  } // F1_UngetChar()

  void F2_GetNonWhiteSpaceChar( int & firstChar, int & line, int & column ) {
    // Get first non-white-space char. Need skip comment.
    int ch;
    bool ok = true;

    while ( 1 ) {
      // Get non-white-space char
      F1_GetChar( firstChar );
      if ( firstChar == EOF )
        return;
      while ( isspace( firstChar ) ) {
        F1_GetChar( firstChar );
        if ( firstChar == EOF )
          return;
      } // while
  
      // Set start line column of token
      line = mPreviousLine;
      column = mPreviousColumn;

      if ( firstChar != '/' )
        return;

      // Skip comment
      F1_GetChar( ch );
      if ( ch == EOF )
        return;
      if ( ch == '/' ) {
        // It is "//"
        F1_GetChar( ch );
        while ( ch != EOF && ch != '\n' )
          F1_GetChar( ch );
      } // if
      else if ( ch == '*' ) {
        // It is "/*"
        F1_GetChar( ch );
        while ( ch != EOF && ok ) {
          if ( ch == '*' ) {
            F1_GetChar( ch );
            if ( ch == '/' )
              ok = false;
            else
              F1_UngetChar( ch );
          } // if

          F1_GetChar( ch );
        } // while      
      } // else if
      else {
        F1_UngetChar( ch );
        return;
      } // else
    } // while
  } // F2_GetNonWhiteSpaceChar()

  void F3_GetIdentifier( Token_s & token ) {
    // Get identifier token
    int ch;

    // Put char into token
    F1_GetChar( ch );
    while ( ch != EOF && ( ch == '_' || isalpha( ch ) || isdigit( ch ) ) ) {
      token.str += ch;
      F1_GetChar( ch );
    } // while

    if ( ch != EOF )
      F1_UngetChar( ch );

    if ( strcmp( token.str.c_str(), "true" ) == 0 || strcmp( token.str.c_str(), "false" ) == 0 )
      token.type = TYPE_CONST_BOOL;
    else if ( strcmp( token.str.c_str(), "int" ) == 0 )
      token.type = TYPE_RESERVE_WORD_INT;
    else if ( strcmp( token.str.c_str(), "float" ) == 0 )
      token.type = TYPE_RESERVE_WORD_FLOAT;
    else if ( strcmp( token.str.c_str(), "char" ) == 0 )
      token.type = TYPE_RESERVE_WORD_CHAR;
    else if ( strcmp( token.str.c_str(), "bool" ) == 0 )
      token.type = TYPE_RESERVE_WORD_BOOL;
    else if ( strcmp( token.str.c_str(), "string" ) == 0 )
      token.type = TYPE_RESERVE_WORD_STRING;
    else if ( strcmp( token.str.c_str(), "void" ) == 0 )
      token.type = TYPE_RESERVE_WORD_VOID;
    else if ( strcmp( token.str.c_str(), "if" ) == 0 )
      token.type = TYPE_RESERVE_WORD_IF;
    else if ( strcmp( token.str.c_str(), "else" ) == 0 )
      token.type = TYPE_RESERVE_WORD_ELSE;
    else if ( strcmp( token.str.c_str(), "while" ) == 0 )
      token.type = TYPE_RESERVE_WORD_WHILE;
    else if ( strcmp( token.str.c_str(), "do" ) == 0 )
      token.type = TYPE_RESERVE_WORD_DO;
    else if ( strcmp( token.str.c_str(), "return" ) == 0 )
      token.type = TYPE_RESERVE_WORD_RETURN;
    else
      token.type = TYPE_ID;
  } // F3_GetIdentifier()

  void F4_GetConstantNumber( Token_s & token ) {
    // Get number token
    int ch;
    bool hasOneDot = ( token.str.at( 0 ) == '.' );

    // Put char into token
    F1_GetChar( ch );
    while ( ch != EOF && ( ( ch == '.' && !hasOneDot ) || isdigit( ch ) ) ) {
      hasOneDot = ( hasOneDot || ch == '.' );
      token.str += ch;
      F1_GetChar( ch );
    } // while

    if ( ch != EOF )
      F1_UngetChar( ch );
    token.type = hasOneDot ? TYPE_CONST_FLOAT : TYPE_CONST_INT;
  } // F4_GetConstantNumber()

  void F4_GetConstantString( Token_s & token ) {
    // Get string token
    int ch;
    token.str = "";

    // Put char into token
    F1_GetChar( ch );
    while ( ch != EOF && ch != '\n' && ch != '\"' ) {
      token.str += ch;
      if ( ch == '\\' ) {
        F1_GetChar( ch );
        if ( ch != EOF ) {
          token.str += ch;
          if ( ( token.str[token.str.size() - 2] == '\\' ) && ( token.str[token.str.size() - 1] == 'n' ) ) {
            token.str[token.str.size() - 2] = '\n';
            token.str.erase( token.str.begin() + token.str.size() - 1, token.str.end() );
          } // if
        } // if
      } // if

      F1_GetChar( ch );
    } // while

    if ( ch != EOF && ch != '\"' )
      token.str += ch;

    token.type = TYPE_CONST_STRING;
  } // F4_GetConstantString()

  void F4_GetConstantChar( Token_s & token ) {
    // Get char token
    int ch;
    token.str = "";
  
    // Put char into token
    F1_GetChar( ch );
    while ( ch != EOF && ch != '\n' && ch != '\'' ) {
      token.str += ch;
      if ( ch == '\\' ) {
        F1_GetChar( ch );
        if ( ch != EOF ) {
          token.str += ch;
          if ( ( token.str[token.str.size() - 2] == '\\' ) && ( token.str[token.str.size() - 1] == 'n' ) ) {
            token.str[token.str.size() - 2] = '\n';
            token.str.erase( token.str.begin() + token.str.size() - 1, token.str.end() );
          } // if
        } // if
      } // if

      F1_GetChar( ch );
    } // while

    if ( ch != EOF && ch != '\'' )
      token.str += ch;

    token.type = TYPE_CONST_CHAR;
  } // F4_GetConstantChar()

  void F5_GetSpecial( Token_s & token ) {
    // Get special token
    int ch1;
    int ch2;

    // Read second char in advance
    F1_GetChar( ch2 );
    token.str += ch2;
  
    // Check first char and second char
    ch1 = token.str.at( 0 );
    if ( ch1 == '>' && ch2 == '=' )
      token.type = TYPE_GE;
    else if ( ch1 == '<' && ch2 == '=' )
      token.type = TYPE_LE;
    else if ( ch1 == '=' && ch2 == '=' )
      token.type = TYPE_EQ;
    else if ( ch1 == '!' && ch2 == '=' )
      token.type = TYPE_NE;
    else if ( ch1 == '&' && ch2 == '&' )
      token.type = TYPE_AND;
    else if ( ch1 == '|' && ch2 == '|' )
      token.type = TYPE_OR;
    else if ( ch1 == '+' && ch2 == '=' )
      token.type = TYPE_PE;
    else if ( ch1 == '-' && ch2 == '=' )
      token.type = TYPE_ME;
    else if ( ch1 == '*' && ch2 == '=' )
      token.type = TYPE_TE;
    else if ( ch1 == '/' && ch2 == '=' )
      token.type = TYPE_DE;
    else if ( ch1 == '%' && ch2 == '=' )
      token.type = TYPE_RE;
    else if ( ch1 == '+' && ch2 == '+' )
      token.type = TYPE_PP;
    else if ( ch1 == '-' && ch2 == '-' )
      token.type = TYPE_MM;
    else if ( ch1 == '>' && ch2 == '>' )
      token.type = TYPE_RS;
    else if ( ch1 == '<' && ch2 == '<' )
      token.type = TYPE_LS;
    else {
      F1_UngetChar( ch2 );
      token.str.erase( 1, 1 );
      if ( ch1 == '(' )
        token.type = TYPE_LEFT_SMALL_BRACKET;
      else if ( ch1 == ')' )
        token.type = TYPE_RIGHT_SMALL_BRACKET;
      else if ( ch1 == '[' )
        token.type = TYPE_LEFT_MID_BRACKET;
      else if ( ch1 == ']' )
        token.type = TYPE_RIGHT_MID_BRACKET;
      else if ( ch1 == '{' )
        token.type = TYPE_LEFT_BIG_BRACKET;
      else if ( ch1 == '}' )
        token.type = TYPE_RIGHT_BIG_BRACKET;
      else if ( ch1 == '+' )
        token.type = TYPE_ADD;
      else if ( ch1 == '-' )
        token.type = TYPE_SUB;
      else if ( ch1 == '*' )
        token.type = TYPE_MUL;
      else if ( ch1 == '/' )
        token.type = TYPE_DIV;
      else if ( ch1 == '%' )
        token.type = TYPE_MOD;
      else if ( ch1 == '^' )
        token.type = TYPE_XOR;
      else if ( ch1 == '>' )
        token.type = TYPE_GT;
      else if ( ch1 == '<' )
        token.type = TYPE_LT;
      else if ( ch1 == '&' )
        token.type = TYPE_BIT_AND;
      else if ( ch1 == '|' )
        token.type = TYPE_BIT_OR;
      else if ( ch1 == '=' )
        token.type = TYPE_ASSIGN;
      else if ( ch1 == '!' )
        token.type = TYPE_NOT;
      else if ( ch1 == ';' )
        token.type = TYPE_SEMICOLON;
      else if ( ch1 == ',' )
        token.type = TYPE_COMMA;
      else if ( ch1 == '?' )
        token.type = TYPE_ROMCE;
      else if ( ch1 == ':' )
        token.type = TYPE_ROMLOE;
      else
        throw UnrecognizedTokenException( token );
    } // else
  } // F5_GetSpecial()

public:
  Scanner() {
    Reset();
  } // Scanner()

  ~Scanner() {
    Reset();
  } // ~Scanner()

  Token_s PeekToken() {
    Token_s token;
    int line = mLine, column = mColumn;

    if ( mMainMemory.empty() || mPC >= ( int ) mMainMemory.size() ) {
      token = GetTokenFromInput();
      if ( token.type == TYPE_CONST_CHAR ) F1_UngetChar( '\'' );
      else if ( token.type == TYPE_CONST_STRING ) F1_UngetChar( '\"' );

      if ( token.type == TYPE_CONST_CHAR ) F1_UngetChar( '\'' );
      else if ( token.type == TYPE_CONST_STRING ) F1_UngetChar( '\"' );

      mLine = line;
      mColumn = column;
      return token;
    } // if

    return mMainMemory[mPC];
  } // PeekToken()

  Token_s GetToken() {
    Token_s token;

    if ( mMainMemory.empty() || mPC >= ( int ) mMainMemory.size() ) {
      token = GetTokenFromInput();
      mUngetBuf.erase( mUngetBuf.begin() + ( mUngetBufIndex + 1 ), mUngetBuf.end() );

      mMainMemory.push_back( token );
      mPC++;
      return token;
    } // if

    return mMainMemory[mPC++];
  } // GetToken()

  void JumpTo( int dst ) {
    // Invalid jump
  /*
    if ( dst < 0 || ( ( int ) mMainMemory.size() - 1 != mFunctionSegPointer &&
                      dst >= ( int ) mMainMemory.size() ) )
      throw "Error : Invalid jump";
  */

    mPC = dst;
  } // JumpTo()

  // PC jump to beginning of input segment
  void JumpToInputBegin() {
    JumpTo( mFunctionSegPointer + 1 );
  } // JumpToInputBegin()

  // Reserve memory space for function definition
  void ReserveFunction() {
    mPreviousFunctionSegPointer = mFunctionSegPointer;
    mFunctionSegPointer = mMainMemory.size() - 1;
  } // ReserveFunction()

  void ClearInputBuf() {
    JumpToInputBegin();
    mMainMemory.erase( mMainMemory.begin() + ( mFunctionSegPointer + 1 ), mMainMemory.end() );
  } // ClearInputBuf()

  int GetProgramCounter() {
    return mPC;
  } // GetProgramCounter()

  void ResetLineNum() {
    mLine = mColumn = 1;
  } // ResetLineNum()

  void RemoveOneLineWhiteSpace() {
    int ch, ch2;
    bool isDone = false, ok = true;

    mUngetBufIndex = mUngetBuf.size() - 1;
    while ( !isDone ) {
      // Get non-white-space char
      F1_GetChar( ch );
      if ( ch == EOF || ch == '\n' )
        isDone = true;
      else {
        while ( !isDone && isspace( ch ) ) {
          F1_GetChar( ch );
          if ( ch == EOF || ch == '\n' )
            isDone = true;
        } // while
  
        if ( ch != '/' ) {
          if ( ch != EOF || ch != '\n' )
            F1_UngetChar( ch );
          isDone = true;
        } // if
        else { // Skip comment
          F1_GetChar( ch2 );
          if ( ch2 == '/' ) {
            // It is "//"
            F1_GetChar( ch );
            while ( ch != EOF && ch != '\n' )
              F1_GetChar( ch );
            isDone = true;
          } // if
          else if ( ch2 == '*' ) {
            // It is "/*"
            F1_GetChar( ch );
            while ( ch != EOF && ok ) {
              if ( ch == '*' ) {
                F1_GetChar( ch );
                if ( ch == '/' )
                  ok = false;
                else
                  F1_UngetChar( ch );
              } // if

              F1_GetChar( ch );
            } // while      
          } // else if
          else {
            if ( ch != EOF && ch != '\n' )
              F1_UngetChar( ch );
            if ( ch2 != EOF && ch2 != '\n' )
              F1_UngetChar( ch2 );
            isDone = true;
          } // else
        } // else
      } // else
    } // while

    mUngetBuf.erase( mUngetBuf.begin() + ( mUngetBufIndex + 1 ), mUngetBuf.end() );
  } // RemoveOneLineWhiteSpace()

  void RemoveOneLineInput( int inputBeginPos ) {
    int ch;

    // mMainMemory.erase( mMainMemory.begin() + inputBeginPos, mMainMemory.begin() + mPC );
    ClearInputBuf();
    mUngetBufIndex = mUngetBuf.size() - 1;
    F1_GetChar( ch );
    while ( ch != EOF && ch != '\n' )
      F1_GetChar( ch );

    if ( ch == '\n' ) F1_UngetChar( ch );
    mUngetBuf.erase( mUngetBuf.begin() + ( mUngetBufIndex + 1 ), mUngetBuf.end() );
    mLine = mColumn = 1;
  } // RemoveOneLineInput()
}; // class Scanner

class Parser {
private:
  Scanner mScanner;
  RunMode mMode;
  string mIndentStr;
  bool mIsEOF;
  map<int, string> mDataTypeToStringTable;
  map<int, int> mJumpTable;           // Record the source and destination for jump
  FunctionTable mFunctionTable;       // returnType functionName(parameters);
  vector<string> mSystemFunctionList; // returnType functionName(parameters);
  vector<Function_Variable_s> mCallStack;

  void InitialDataTypeToStringTable() {
    mDataTypeToStringTable.clear();

    mDataTypeToStringTable[TYPE_UNSET]               = "";
    mDataTypeToStringTable[TYPE_CONST_INT]    = "int";
    mDataTypeToStringTable[TYPE_CONST_FLOAT]  = "float";
    mDataTypeToStringTable[TYPE_CONST_BOOL]   = "bool";
    mDataTypeToStringTable[TYPE_CONST_CHAR]   = "char";
    mDataTypeToStringTable[TYPE_CONST_STRING] = "string";
    mDataTypeToStringTable[TYPE_CONST_VOID]   = "void";
  } // InitialDataTypeToStringTable()

  void InitalizeTokenValue( Token_Value_s & tokenValue ) {
    for ( int i = 0, size = tokenValue.value.size() ; i < size ; i++ ) {
      if ( tokenValue.type == TYPE_CONST_INT )         tokenValue.value[i] = "0";
      else if ( tokenValue.type == TYPE_CONST_FLOAT )  tokenValue.value[i] = "0.0";
      else if ( tokenValue.type == TYPE_CONST_BOOL )   tokenValue.value[i] = "false";
      else if ( tokenValue.type == TYPE_CONST_CHAR )   tokenValue.value[i] = "";
      else if ( tokenValue.type == TYPE_CONST_STRING ) tokenValue.value[i] = "";
    } // for
  } // InitalizeTokenValue()

  Token_Value_s GetTokenValue( Token_s token ) {
    Token_Value_s tokenValue;
    tokenValue.type = TYPE_UNSET;
    tokenValue.index = -1;

    if ( token.type == TYPE_ID )
      tokenValue = GetIDTokenValue( token, -1 );
    else if ( IsConstant( token.type ) ) {
      tokenValue.isArray = false;
      tokenValue.type    = token.type;
      tokenValue.value.push_back( token.str );
    } // else if
    else throw "Error: Cannot get token value";

    return tokenValue;
  } // GetTokenValue()

  Table LOCAL_VARIABLES() {
    return mCallStack[mCallStack.size() - 1].localVariableTable;
  } // LOCAL_VARIABLES()

  ParameterTable PARAMETERS() {
    return mCallStack[mCallStack.size() - 1].parameterTable;
  } // PARAMETERS()

  Table GLOBAL_VARIABLES() {
    return mCallStack[0].localVariableTable;
  } // GLOBAL_VARIABLES()

  Token_Value_s GetIDTokenValue( Token_s token, int stackLayer ) {
    Table localVar;
    ParameterTable param;
    Table global = GLOBAL_VARIABLES();
    Token_s searchKey;
    Token_Value_s retValue;
    int dstStackLayer = 0;

    if ( stackLayer == -1 ) stackLayer = mCallStack.size() - 1;
    localVar = mCallStack[stackLayer].localVariableTable;
    param    = mCallStack[stackLayer].parameterTable;

    if ( IsDefinedToken( localVar, token.str ) )
      retValue = localVar[token.str];
    else if ( IsDefinedToken( param, token.str ) ) {
      if ( ( mMode == MODE_EXECUTION ) && param[token.str].isReference ) {
        dstStackLayer = param[token.str].isRefStackLayer;
        if ( ( dstStackLayer < 0 ) || ( dstStackLayer >= ( int ) mCallStack.size() ) )
          dstStackLayer = stackLayer - 1;

        searchKey.str = param[token.str].info.str;
        retValue = GetIDTokenValue( searchKey,  dstStackLayer );
        // printf( "index = [%d]\n", param[token.str].info.index );
        // printf( "array size = [%d]\n", retValue.value.size() );
        if ( ( param[token.str].info.index >= 0 ) &&
             ( param[token.str].info.index < ( int ) retValue.value.size() ) ) {
          retValue.value.erase( retValue.value.begin() + ( param[token.str].info.index + 1 ),
                                retValue.value.end() );
          retValue.value.erase( retValue.value.begin(),
                                retValue.value.begin() + param[token.str].info.index );
          retValue.isArray = false;
          retValue.index = 0;
        } // if
      } // if
      else
        retValue = param[token.str].info;
    } // else if
    else if ( IsDefinedToken( global, token.str ) )
      retValue = global[token.str];
    else throw UndefinedTokenException( token );

    retValue.str = token.str;
    return retValue;
  } // GetIDTokenValue()

  void SetIDTokenValue( Token_s token, Token_Value_s tokenValue, int index, int stackLayer ) {
    Table localVar;
    ParameterTable param;
    Table global = GLOBAL_VARIABLES();
    Token_s searchKey;
    Token_Value_s retValue;
    int dstStackLayer = 0;

    if ( stackLayer == -1 ) stackLayer = mCallStack.size() - 1;
    localVar = mCallStack[stackLayer].localVariableTable;
    param    = mCallStack[stackLayer].parameterTable;

    if ( IsDefinedToken( localVar, token.str ) ) {
      if ( ( index >= 0 ) &&
           ( index < ( int ) mCallStack[stackLayer].localVariableTable[token.str].value.size() ) )
        mCallStack[stackLayer].localVariableTable[token.str].value[index] = tokenValue.value[0];
      else
        mCallStack[stackLayer].localVariableTable[token.str].value = tokenValue.value;
    } // if
    else if ( IsDefinedToken( param, token.str ) ) {
      if ( param[token.str].isReference ) {
        dstStackLayer = param[token.str].isRefStackLayer;
        if ( ( dstStackLayer < 0 ) || ( dstStackLayer >= ( int ) mCallStack.size() ) )
          dstStackLayer = stackLayer - 1;

        searchKey.str = param[token.str].info.str;
        SetIDTokenValue( searchKey, tokenValue, param[token.str].info.index, dstStackLayer );
      } // if
      else {
        if ( ( index >= 0 ) &&
             ( index < ( int ) mCallStack[stackLayer].parameterTable[token.str].info.value.size() ) )
          mCallStack[stackLayer].parameterTable[token.str].info.value[index] = tokenValue.value[0];
        else
          mCallStack[stackLayer].parameterTable[token.str].info.value = tokenValue.value;
      } // else
    } // else if
    else if ( IsDefinedToken( global, token.str ) ) {
      if ( ( index >= 0 ) &&
           ( index < ( int ) mCallStack[0].localVariableTable[token.str].value.size() ) )
        mCallStack[0].localVariableTable[token.str].value[index] = tokenValue.value[0];
      else
        mCallStack[0].localVariableTable[token.str].value = tokenValue.value;
    } // else if
    else
      throw UndefinedTokenException( token );
  } // SetIDTokenValue()

  bool IsDefinedToken( string tokenName ) {
    Table localVar = LOCAL_VARIABLES();
    ParameterTable param = PARAMETERS();
    Table global = GLOBAL_VARIABLES();

    return IsDefinedToken( localVar, tokenName )
      || IsDefinedToken( param, tokenName )
      || IsDefinedToken( global, tokenName );
  } // IsDefinedToken()

  bool IsDefinedToken( Table table, string tokenName ) {
    return ( table.find( tokenName ) != table.end() );
  } // IsDefinedToken()

  bool IsDefinedToken( ParameterTable table, string tokenName ) {
    return ( table.find( tokenName ) != table.end() );
  } // IsDefinedToken()

  bool IsDefinedFunction( string funcName ) {
    return ( mFunctionTable.find( funcName ) != mFunctionTable.end() );
  } // IsDefinedFunction()

  bool IsSystemFunction( string funcName ) {
    for ( int i = 0, size = mSystemFunctionList.size() ; i < size ; i++ )
      if ( !mSystemFunctionList[i].compare( funcName ) )
        return true;

    return false;
  } // IsSystemFunction()

  bool IsRedefinedFunction( Token_s token, ParameterList_t parmList ) {
    ArgumentList_t arguList;
    arguList.clear();
    arguList.reserve( parmList.size() );

    for ( int i = 0, size = parmList.size() ; i < size ; i++ )
      arguList.push_back( parmList[i].info );
    
    return IsDefinedFunction( token, arguList );
  } // IsRedefinedFunction()

  int GetFunctionFormatIndex( Token_s token, ArgumentList_t arguList ) {
    bool isDefined = false;
    ParameterList_t parmListTmp;
    vector<Function_Format_s> formatList = mFunctionTable[token.str];
    int arguListSize   = arguList.size();
    int formatListSize = formatList.size();
    int i = EOF;

    for ( i = 0 ; !isDefined && ( i < formatListSize ) ; i++ ) {
      parmListTmp = formatList[i].parameters;
      if ( arguList.size() == parmListTmp.size() ) {
        isDefined = true;
        for ( int j = 0 ; isDefined && ( j < arguListSize ) ; j++ )
          isDefined = ( arguList[j].type == parmListTmp[j].info.type );
      } // if
    } // for

    return isDefined ? i - 1 : EOF;
  } // GetFunctionFormatIndex()

  bool IsDefinedFunction( Token_s token, ArgumentList_t arguList ) {
    bool isDefined = false;
    ParameterList_t parmListTmp;
    vector<Function_Format_s> formatList = mFunctionTable[token.str];
    int arguListSize   = arguList.size();
    int formatListSize = formatList.size();

    for ( int i = 0 ; !isDefined && ( i < formatListSize ) ; i++ ) {
      parmListTmp = formatList[i].parameters;
      if ( arguList.size() == parmListTmp.size() ) {
        isDefined = true;
        for ( int j = 0 ; isDefined && ( j < arguListSize ) ; j++ )
          isDefined = ( arguList[j].type == parmListTmp[j].info.type );
      } // if
    } // for

    return isDefined;
  } // IsDefinedFunction()

  // just the names of the (global) variables,  sorted (from smallest to greatest)
  void ListAllVariables() {
    map<string, Token_Value_s>::iterator it;
    Table global = GLOBAL_VARIABLES();
    string variableName;

    for ( it = global.begin() ;
          it != global.end() ; it++ ) {
      variableName = it->first;
      if ( ( strcmp( variableName.c_str(), "cin" ) != 0 ) &&
           ( strcmp( variableName.c_str(), "cout" ) != 0 ) )
        printf( "%s\n", it->first.c_str() );
    } // for
  } // ListAllVariables()

  // just the names of the (user-defined)  functions, sorted
  void ListAllFunctions() {
    for ( map<string, vector<Function_Format_s> >::iterator it = mFunctionTable.begin() ;
          it != mFunctionTable.end() ; it++ )
      if ( !IsSystemFunction( it->first ) )
        printf( "%s()\n", it->first.c_str() );
  } // ListAllFunctions()

  // The definition of a particular variable
  void ListVariable( Token_Value_s variable ) {
    Token_s token;
    token.str = variable.value[0];
    Token_Value_s value = GetIDTokenValue( token, -1 );
    int arraySize = value.value.size();
    printf( "%s %s", mDataTypeToStringTable[value.type].c_str(), token.str.c_str() );
    if ( value.isArray ) printf( "[ %d ]", arraySize );
    printf( " ;\n" );
  } // ListVariable()

  // The definition of a particular function
  void ListFunction( Token_Value_s token ) {
    RunMode backUpMode = mMode;
    int returnPos = mScanner.GetProgramCounter();
    string funcName = token.value[0];
    bool isQuit = false;

    if ( IsSystemFunction( funcName ) || !IsDefinedFunction( funcName ) )
      throw "Error : Function cannot be listed";

    mMode = MODE_LIST_FUNCTION;
    mIndentStr = "";
    for ( int i = mFunctionTable[funcName].size() - 1 ; ( i >= 0 ) && !isQuit ; i-- ) {
      if ( mFunctionTable[funcName][i].startAddress != -1 ) {
        mScanner.JumpTo( mFunctionTable[funcName][i].startAddress );
        Command();
        isQuit = true;
        // printf( "\n" );
      } // if
    } // for

    mMode = backUpMode;
    mScanner.JumpTo( returnPos );
  } // ListFunction()

  // The definition of a particular function
  Token_Value_s ExecuteFunction( Token_s funcNameToken, ArgumentList_t & arguList ) {
    RunMode backUpMode = mMode;
    map<int, int> backUpJumpTable = mJumpTable;
    Function_Format_s functionFormat;
    ParameterList_t parameters = ParameterList_t();
    vector <Function_Format_s> functionFormatList;
    functionFormatList.clear();
    int returnPos = mScanner.GetProgramCounter();
    string funcName = funcNameToken.str;
    Token_Value_s retValue;

    retValue.type = TYPE_CONST_VOID;
    retValue.value.clear();
    retValue.isArray = false;
    retValue.index = -1;

    if ( IsSystemFunction( funcName ) || !IsDefinedFunction( funcNameToken, arguList ) )
      throw "Error : Function cannot be listed";

    // mMode = MODE_EXECUTION;
    functionFormatList = mFunctionTable[funcName];
    functionFormat = functionFormatList[GetFunctionFormatIndex( funcNameToken, arguList )];

    // Pass parameters
    parameters = functionFormat.parameters;
    for ( int i = 0, size = parameters.size() ; i < size ; i++ ) {
      parameters[i].info = arguList[i];

      // Point to actual variable for call by reference
      if ( parameters[i].isReference ) FindActualVariable( parameters[i] );
    } // for

    if ( functionFormat.startAddress == EOF ) throw "Error: Invalid function address";

    mScanner.JumpTo( functionFormat.startAddress );
    mJumpTable = functionFormat.jumpTable;
    mCallStack[mCallStack.size() - 1].returnValue.type = functionFormat.returnType;
    mCallStack[mCallStack.size() - 1].returnValue.isArray = false;
    mCallStack[mCallStack.size() - 1].returnValue.value.resize( 1 );
    mCallStack[mCallStack.size() - 1].returnValue.value[0].clear();
    mCallStack[mCallStack.size() - 1].returnValue.index = -1;
    InitalizeTokenValue( mCallStack[mCallStack.size() - 1].returnValue );

    // Move PC to '{'
    while ( mScanner.PeekToken().type != TYPE_LEFT_BIG_BRACKET )
      mScanner.GetToken();

    Compound_statement( parameters );
    retValue = mCallStack[mCallStack.size() - 1].returnValue;

    mMode = backUpMode;
    mJumpTable = backUpJumpTable;
    mScanner.JumpTo( returnPos );
    return retValue;
  } // ExecuteFunction()

  void FindActualVariable( Parameter_s & parameter ) {
    int stackLayer = 0;
    Token_s token;
    token.str = parameter.info.str;

    for ( int j = mCallStack.size() - 1 ; ( j >= 0 ) && ( stackLayer == 0 ) ; j-- ) {
      if ( IsDefinedToken( mCallStack[j].localVariableTable, token.str ) ) {
        stackLayer = j;
        parameter.info.str = token.str;
      } // if
      else if ( IsDefinedToken( mCallStack[j].parameterTable, token.str ) ) {
        // Call by value( Destination found )
        if ( !mCallStack[j].parameterTable[token.str].isReference ) {
          stackLayer = j;
          parameter.info.str = token.str;
        } // if
        // Call by reference( Keep searching )
        else {
          int k = j;
          j = mCallStack[j].parameterTable[token.str].isRefStackLayer + 1;
          token.str = mCallStack[k].parameterTable[token.str].info.str;
        } // else
      } // else if
    } // for

    parameter.isRefStackLayer = stackLayer;
  } // FindActualVariable()

  // Exit the interpreter
  void Done() {
    throw EOF_EXCEPTION;
  } // Done()

  bool IsTypeCompatible( Token_Value_s origionalValue, TokenType type, bool isArray ) {
    bool canTypeCast = false;

    if ( origionalValue.type == TYPE_CONST_STRING ) canTypeCast = true;
    else if ( origionalValue.type == TYPE_CONST_BOOL ) {
      if ( type == TYPE_CONST_BOOL ) canTypeCast = true;
      else if ( type == TYPE_CONST_STRING ) canTypeCast = true;
    } // else if
    else if ( origionalValue.type == TYPE_CONST_CHAR ) {
      if ( type == TYPE_CONST_CHAR ) canTypeCast = true;
      else if ( type == TYPE_CONST_STRING ) canTypeCast = true;
    } // else if
    else if ( origionalValue.type == TYPE_CONST_INT ) {
      if ( type == TYPE_CONST_INT ) canTypeCast = true;
      else if ( type == TYPE_CONST_FLOAT ) canTypeCast = true;
      else if ( type == TYPE_CONST_STRING ) canTypeCast = true;
    } // else if
    else if ( origionalValue.type == TYPE_CONST_FLOAT ) {
      if ( type == TYPE_CONST_INT ) canTypeCast = true;
      else if ( type == TYPE_CONST_FLOAT ) canTypeCast = true;
      else if ( type == TYPE_CONST_STRING ) canTypeCast = true;
    } // else if

    return canTypeCast;
  } // IsTypeCompatible()

  Token_Value_s TypeCast( Token_Value_s origionalValue, TokenType type, bool isArray ) {
    static char errMsgStr[100];
    Token_Value_s retValue = origionalValue;
    double floatValue = 0.0;
    int intValue = 0;
    Str100 value = "";
    string errorMsg;

    // Empty value
    if ( origionalValue.value.empty() || origionalValue.value[0].empty() )
      throw "Error : Empty value found when typecast";

    // No need to type cast
    if ( ( origionalValue.type == type ) && ( origionalValue.isArray == isArray ) )
      return origionalValue;

    // Invalid type cast
    if ( !IsTypeCompatible( origionalValue, type, isArray ) ) {
      sprintf( errMsgStr, "Error : Cannot convert from '%d' to '%d'", origionalValue.type, type );
      errorMsg = string( errMsgStr );
      throw errorMsg;
    } // if

    retValue.type = type;
    retValue.isArray = isArray;

    // string -> char[]
    if ( ( type == TYPE_CONST_CHAR ) && isArray )
      ;
    // char[] -> string
    else if ( ( origionalValue.type == TYPE_CONST_CHAR ) && origionalValue.isArray )
      ;
    else if ( type == TYPE_CONST_INT ) {
      sscanf( origionalValue.value[0].c_str(), "%d", &intValue );
      sprintf( value, "%d", intValue );
    } // else if
    else if ( type == TYPE_CONST_FLOAT ) {
      sscanf( origionalValue.value[0].c_str(), "%lf", &floatValue );
      sprintf( value, "%f", floatValue );
    } // else if
    else if ( type == TYPE_CONST_BOOL ) {
      if ( origionalValue.type == TYPE_CONST_BOOL )
        sprintf( value, "%s", origionalValue.value[0].c_str() );
    } // else if
    else if ( type == TYPE_CONST_CHAR ) {
      value[0] = origionalValue.value[0].c_str()[0];
      value[1] = '\0';
    } // else if
    else if ( type == TYPE_CONST_STRING )
      sscanf( origionalValue.value[0].c_str(), "%s", value );

    retValue.value.resize( 1 );
    retValue.value[0] = string( value );
    return retValue;
  } // TypeCast()

  Token_s GetTokenWithType( bool isExceptionEnable, TokenType type1 ) {
    TokenType typeArray[1];
    typeArray[0] = type1;
    return GetTokenWithType( isExceptionEnable, 1, typeArray );
  } // GetTokenWithType()

  Token_s GetTokenWithType( bool isExceptionEnable, TokenType type1, TokenType type2 ) {
    TokenType typeArray[2];
    typeArray[0] = type1;
    typeArray[1] = type2;
    return GetTokenWithType( isExceptionEnable, 2, typeArray );
  } // GetTokenWithType()

  Token_s GetTokenWithType( bool isExceptionEnable, TokenType type1, TokenType type2,
                            TokenType type3, TokenType type4, TokenType type5 ) {
    TokenType typeArray[5] = { type1, type2, type3, type4, type5 };
    return GetTokenWithType( isExceptionEnable, 5, typeArray );
  } // GetTokenWithType()

  Token_s GetTokenWithType( bool isExceptionEnable, int n, TokenType typeArray[5] ) {
    Token_s token = mScanner.PeekToken();

    for ( int i = 0 ; i < n ; i++ )
      if ( token.type == typeArray[i] )
        return mScanner.GetToken();

    if ( isExceptionEnable ) {
      mScanner.GetToken();
      throw UnexpectedTokenException( token );
    } // if

    token.str = "";
    return token;
  } // GetTokenWithType()

  // user_input : ( definition | statement ) { definition | statement }
  Token_Value_s Command() {
    Token_s token = mScanner.PeekToken();
    Token_Value_s retMsg = ( IsType_specifier( token.type ) ||
                             ( token.type == TYPE_RESERVE_WORD_VOID ) ) ? Definition() : Statement();

    return retMsg;
  } // Command()

  // definition :           VOID Identifier function_definition_without_ID |
  //              type_specifier Identifier function_definition_or_declarators
  Token_Value_s Definition() {
    Token_s token, idToken, dataTypeToken = mScanner.GetToken();

    // VOID || type_specifier
    if ( mMode == MODE_LIST_FUNCTION )
      printf( "%s ", dataTypeToken.str.c_str() );

    // Identifier
    idToken = GetTokenWithType( true, TYPE_ID );
    if ( mMode == MODE_LIST_FUNCTION ) {
      printf( "%s", idToken.str.c_str() );
      token = mScanner.PeekToken();
      if ( ( token.type != TYPE_COMMA ) && ( token.type != TYPE_LEFT_SMALL_BRACKET ) &&
           ( token.type != TYPE_LEFT_MID_BRACKET ) &&
           ( token.type != TYPE_PP ) && ( token.type != TYPE_MM ) )
        printf( " " );
    } // if

    // function_definition_without_ID || function_definition_or_declarators
    return ( dataTypeToken.type == TYPE_RESERVE_WORD_VOID ) ?
      Function_definition_without_ID( dataTypeToken, idToken ) :
      Function_definition_or_declarators( dataTypeToken, idToken );
  } // Definition()

  bool IsType_specifier( TokenType type ) {
    return ( ( type == TYPE_RESERVE_WORD_INT ) ||
             ( type == TYPE_RESERVE_WORD_CHAR ) ||
             ( type == TYPE_RESERVE_WORD_FLOAT ) ||
             ( type == TYPE_RESERVE_WORD_STRING ) ||
             ( type == TYPE_RESERVE_WORD_BOOL ) );
  } // IsType_specifier()

  // type_specifier : INT | CHAR | FLOAT | STRING | BOOL
  Token_s Type_specifier() {
    Token_s token = GetTokenWithType( true, TYPE_RESERVE_WORD_INT, TYPE_RESERVE_WORD_CHAR,
                                      TYPE_RESERVE_WORD_FLOAT, TYPE_RESERVE_WORD_STRING,
                                      TYPE_RESERVE_WORD_BOOL );
    if ( mMode == MODE_LIST_FUNCTION )
      printf( "%s ", token.str.c_str() );
    return token;
  } // Type_specifier()

  // function_definition_or_declarators : function_definition_without_ID | rest_of_declarators
  Token_Value_s Function_definition_or_declarators( Token_s dataTypeToken, Token_s & IDToken ) {
    return ( mScanner.PeekToken().type == TYPE_LEFT_SMALL_BRACKET ) ?
      Function_definition_without_ID( dataTypeToken, IDToken ) :
      Rest_of_declarators( dataTypeToken, IDToken );
  } // Function_definition_or_declarators()

  // rest_of_declarators : array_const { ',' Identifier array_const } ';'
  Token_Value_s Rest_of_declarators( Token_s dataTypeToken, Token_s IDToken ) {
    Token_s token;
    Token_Value_s retMsg;
    retMsg.value.resize( 1 );
    retMsg.value[0] = VariableDeclare( dataTypeToken.type, IDToken.str, Array_const() );

    // { ',' Identifier array_const }
    while ( mScanner.PeekToken().type == TYPE_COMMA ) {
      token = mScanner.GetToken(); // ','
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      token = GetTokenWithType( true, TYPE_ID );
      retMsg.value[0] += "\n" + VariableDeclare( dataTypeToken.type,
                                                      token.str, Array_const() );
      if ( mMode == MODE_LIST_FUNCTION ) {
        printf( "%s", token.str.c_str() );
        token = mScanner.PeekToken();
        if ( ( token.type != TYPE_COMMA ) && ( token.type != TYPE_LEFT_SMALL_BRACKET ) &&
             ( token.type != TYPE_LEFT_MID_BRACKET ) &&
             ( token.type != TYPE_PP ) && ( token.type != TYPE_MM ) )
          printf( " " );
      } // if
    } // while

    // ';'
    token = GetTokenWithType( true, TYPE_SEMICOLON );
    if ( mMode == MODE_LIST_FUNCTION ) {
      printf( "%s", token.str.c_str() );
      if ( mScanner.PeekToken().type != TYPE_RIGHT_BIG_BRACKET )
        printf( "\n%s", mIndentStr.c_str() );
    } // if
    
    return retMsg;
  } // Rest_of_declarators()

  string VariableDeclare( TokenType dataType, string IDTokenName, Token_s arraySizeToken ) {
    Token_Value_s variable;
    string retMsgStr;
    Table localVar = LOCAL_VARIABLES();

    /*
      if ( mMode == MODE_SYNTAX_CHECK ) {
    */
    if ( IsDefinedToken( localVar, IDTokenName ) ) {
      /*
      if ( localVar[IDTokenName].type == dataType )
        throw "Error : Redefined variable";
      */
      retMsgStr = "New definition of ";
    } // if
    else retMsgStr = "Definition of ";

    variable.str = IDTokenName;
    variable.isArray = ( arraySizeToken.type != TYPE_UNSET );
    variable.type = TokenType( dataType + TYPE_CONST_INT - TYPE_RESERVE_WORD_INT );
    variable.value.resize( variable.isArray ?
                           atoi( arraySizeToken.str.c_str() ) : 1 );
    variable.index = -1;
    InitalizeTokenValue( variable );

    mCallStack[mCallStack.size() - 1].localVariableTable[IDTokenName] = variable;
    /*
      } // if
    */

    return retMsgStr + IDTokenName + " entered ...";
  } // VariableDeclare()

  // array_const: [ '[' Constant ']' ]
  Token_s Array_const() {
    Token_s arraySizeToken, token;
    arraySizeToken.type = TYPE_UNSET;

    // '['
    if ( mScanner.PeekToken().type == TYPE_LEFT_MID_BRACKET ) {
      token = mScanner.GetToken();
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      arraySizeToken = Constant(); // Constant

      token = GetTokenWithType( true, TYPE_RIGHT_MID_BRACKET ); // ']'
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );
    } // if

    return arraySizeToken;
  } // Array_const()

  int Array_expression( Token_Value_s IDTokenValue ) {
    Token_s token = GetTokenWithType( true, TYPE_LEFT_MID_BRACKET );
    Token_Value_s tempTokenValue;
    int index = 0;
    if ( mMode == MODE_LIST_FUNCTION )
      printf( "%s ", token.str.c_str() );

    // expression
    tempTokenValue = Expression();
    if ( mMode == MODE_EXECUTION ) {
      tempTokenValue = TypeCast( tempTokenValue, TYPE_CONST_INT, false );
      sscanf( tempTokenValue.value[0].c_str(), "%d", &index );
      if ( IDTokenValue.str.compare( "w" ) == 0 && tempTokenValue.str.compare( "i" ) == 0 )
        index = index;
      if ( ( index < 0 ) || ( index > ( int ) IDTokenValue.value.size() - 1 ) )
        throw "Error: Array index out of range";
    } // if

    token = GetTokenWithType( true, TYPE_RIGHT_MID_BRACKET );
    if ( mMode == MODE_LIST_FUNCTION )
      printf( "%s ", token.str.c_str() );

    return index;
  } // Array_expression()

  bool IsConstant( TokenType type ) {
    return ( ( type == TYPE_CONST_INT ) ||
             ( type == TYPE_CONST_FLOAT ) ||
             ( type == TYPE_CONST_CHAR ) ||
             ( type == TYPE_CONST_BOOL ) ||
             ( type == TYPE_CONST_STRING ) );
  } // IsConstant()

  // Constant : CONST_INT | CONST_FLOAT | CONST_CHAR | CONST_BOOL | CONST_STRING
  Token_s Constant() {
    Token_s token = GetTokenWithType( true, TYPE_CONST_INT, TYPE_CONST_FLOAT,
                                      TYPE_CONST_CHAR, TYPE_CONST_BOOL, TYPE_CONST_STRING );
    if ( mMode == MODE_LIST_FUNCTION )
      printf( "%s ", token.str.c_str() );
    return token;
  } // Constant()

  // function_definition_without_ID : '(' [ VOID | formal_parameter_list ] ')' compound_statement
  Token_Value_s Function_definition_without_ID( Token_s returnTypeToken, Token_s IDToken ) {
    Function_Format_s functionFormat;
    ParameterList_t parmList;
    Token_s token;
    int currentPos;
    Token_Value_s retMsg;
    retMsg.value.clear();

    // '('
    token = GetTokenWithType( true, TYPE_LEFT_SMALL_BRACKET );
    if ( mMode == MODE_LIST_FUNCTION )
      printf( "%s ", token.str.c_str() );

    // [ VOID | formal_parameter_list ]
    token = mScanner.PeekToken();
    if ( token.type == TYPE_RESERVE_WORD_VOID ) {
      mScanner.GetToken();
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );
    } // if
    else if ( IsType_specifier( token.type ) )
      parmList = Formal_parameter_list();

    // ')'
    token = GetTokenWithType( true, TYPE_RIGHT_SMALL_BRACKET );

    if ( mMode == MODE_LIST_FUNCTION )
      printf( "%s ", token.str.c_str() );

    // compound_statement
    if ( mMode != MODE_EXECUTION )
      Compound_statement( parmList );

    if ( mMode == MODE_SYNTAX_CHECK ) {
      functionFormat.parameters   = parmList;
      functionFormat.returnType   = TokenType( returnTypeToken.type +
                                               TYPE_CONST_INT - TYPE_RESERVE_WORD_INT );
      currentPos = mScanner.GetProgramCounter();
      mScanner.JumpToInputBegin();
      functionFormat.startAddress = mScanner.GetProgramCounter();
      mScanner.ReserveFunction();
      mScanner.JumpTo( currentPos );
      if ( !IsDefinedFunction( IDToken.str ) )
        mFunctionTable[IDToken.str].clear();
        /*
          else
            if ( IsRedefinedFunction( IDToken, parmList ) )
              throw "Error : Redefined function";
        */

      functionFormat.jumpTable = mJumpTable;
      mFunctionTable[IDToken.str].insert( mFunctionTable[IDToken.str].begin(), functionFormat );
    } // if

    if ( mFunctionTable[IDToken.str].size() == 1 )
      retMsg.value.push_back( "Definition of " );
    else
      retMsg.value.push_back( "New definition of " );

    retMsg.value[retMsg.value.size() - 1] += IDToken.str + "() entered ..." ;
    return retMsg;
  } // Function_definition_without_ID()

  // formal_parameter_list : formal_parameter { ',' formal_parameter }
  ParameterList_t Formal_parameter_list() {
    Token_s token;
    ParameterList_t parameterList;
    parameterList.clear();
    parameterList.push_back( Formal_parameter() );

    // ','
    while ( mScanner.PeekToken().type == TYPE_COMMA ) {
      token = mScanner.GetToken(); // ','
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      parameterList.push_back( Formal_parameter() );  // formal_parameter
    } // while

    return parameterList;
  } // Formal_parameter_list()

  // formal_parameter : type_specifier [ '&' ] Identifier array_const
  Parameter_s Formal_parameter() {
    Token_s token, arraySizeToken;
    Parameter_s parameter;
    parameter.info.type = TokenType( Type_specifier().type + TYPE_CONST_INT - TYPE_RESERVE_WORD_INT );
    parameter.info.index = -1;
    parameter.isReference = ( mScanner.PeekToken().type == TYPE_BIT_AND );
    parameter.isRefStackLayer = -1;
    token = GetTokenWithType( false, TYPE_BIT_AND );            // [ '&' ]
    if ( mMode == MODE_LIST_FUNCTION && !token.str.empty() )
      printf( "%s ", token.str.c_str() );

    token = GetTokenWithType( true, TYPE_ID );                  // Identifier
    parameter.str = token.str;
    if ( mMode == MODE_LIST_FUNCTION ) {
      printf( "%s", token.str.c_str() );
      token = mScanner.PeekToken();
      if ( ( token.type != TYPE_COMMA ) && ( token.type != TYPE_LEFT_SMALL_BRACKET ) &&
           ( token.type != TYPE_LEFT_MID_BRACKET ) &&
           ( token.type != TYPE_PP ) && ( token.type != TYPE_MM ) )
        printf( " " );
    } // if

    arraySizeToken = Array_const();
    parameter.info.isArray = ( arraySizeToken.type != TYPE_UNSET ); // array_const
    parameter.isReference  = ( ( parameter.isReference ) || ( parameter.info.isArray ) );
    parameter.info.value.resize( parameter.info.isArray ?
                                 atoi( arraySizeToken.str.c_str() ) : 1 );

    InitalizeTokenValue( parameter.info );
    return parameter;
  } // Formal_parameter()

  // compound_statement : '{' { declaration | statement } '}'
  void Compound_statement() {
    Token_s token, arraySizeToken;
    Function_Variable_s variables;
    ParameterTable parameters = PARAMETERS();
    map<string, Parameter_s>::iterator itParm;
    map<string, Token_Value_s>::iterator it;
    Table localVar = LOCAL_VARIABLES();

    // '{'
    token = GetTokenWithType( true, TYPE_LEFT_BIG_BRACKET );
    if ( mMode == MODE_LIST_FUNCTION ) {
      printf( "%s\n", token.str.c_str() );
      mIndentStr += "  ";
      printf( "%s", mIndentStr.c_str() );
    } // if

    variables.localVariableTable.clear();
    variables.parameterTable.clear();
    if ( mCallStack.size() != 1 ) {
      variables.parameterTable = PARAMETERS();
      for ( it = localVar.begin() ; it != localVar.end() ; it++ ) {
        variables.parameterTable[it->first].info = it->second;
        variables.parameterTable[it->first].isReference = true;
        variables.parameterTable[it->first].isRefStackLayer = mCallStack.size() - 1;
      } // for
    } // if

    mCallStack.push_back( variables );

    // { declaration | statement }
    token = mScanner.PeekToken();
    while ( token.type != TYPE_RIGHT_BIG_BRACKET ) {
      if ( IsType_specifier( token.type ) )
        Declaration();
      else
        Statement();
      token = mScanner.PeekToken();
    } // while

    // '}'
    token = GetTokenWithType( true, TYPE_RIGHT_BIG_BRACKET );
    if ( mMode == MODE_LIST_FUNCTION ) {
      mIndentStr.erase( mIndentStr.end() - 2, mIndentStr.end() );
      printf( "\n%s%s", mIndentStr.c_str(), token.str.c_str() );
      if ( mScanner.PeekToken().type != TYPE_RIGHT_BIG_BRACKET )
        printf( "\n%s", mIndentStr.c_str() );
    } // if

    if ( mCallStack.size() != 1 )
      mCallStack.pop_back();
  } // Compound_statement()

  void Compound_statement( ParameterList_t & parmList ) {
    Token_s token, arraySizeToken;
    Function_Variable_s variables;
    ParameterTable parameters = PARAMETERS();
    map<string, Token_Value_s>::iterator it;

    // '{'
    token = GetTokenWithType( true, TYPE_LEFT_BIG_BRACKET );
    if ( mMode == MODE_LIST_FUNCTION ) {
      printf( "%s\n", token.str.c_str() );
      mIndentStr += "  ";
      printf( "%s", mIndentStr.c_str() );
    } // if

    variables.localVariableTable.clear();
    variables.parameterTable.clear();

    for ( int i = 0, size = parmList.size() ; i < size ; i++ )
      variables.parameterTable[parmList[i].str] = parmList[i];

    mCallStack.push_back( variables );

    // { declaration | statement }
    try {
      token = mScanner.PeekToken();
      while ( token.type != TYPE_RIGHT_BIG_BRACKET ) {
        if ( IsType_specifier( token.type ) )
          Declaration();
        else
          Statement();
        token = mScanner.PeekToken();
      } // while

      // '}'
      token = GetTokenWithType( true, TYPE_RIGHT_BIG_BRACKET );
      if ( mMode == MODE_LIST_FUNCTION ) {
        mIndentStr.erase( mIndentStr.end() - 2, mIndentStr.end() );
        printf( "\n%s%s", mIndentStr.c_str(), token.str.c_str() );
        if ( mScanner.PeekToken().type != TYPE_RIGHT_BIG_BRACKET )
          printf( "\n%s", mIndentStr.c_str() );
      } // if
    } // try
    catch ( ReturnException &re ) {
      ;
    } // catch()

    if ( mCallStack.size() != 1 ) {
    /*
      if ( mMode == MODE_EXECUTION )
        for ( int i = 0, size = parmList.size() ; i < size ; i++ )
          parmList[i] = mCallStack[mCallStack.size() - 1].parameterTable[parmList[i].str];
    */
      mCallStack.pop_back();
    } // if
  } // Compound_statement()

  // declaration : type_specifier Identifier rest_of_declarators
  void Declaration() {
    Token_s token, idToken, dataTypeToken = Type_specifier(); // type_specifier

    idToken = GetTokenWithType( true, TYPE_ID );    // Identifier
    if ( mMode == MODE_LIST_FUNCTION ) {
      printf( "%s", idToken.str.c_str() );
      token = mScanner.PeekToken();
      if ( ( token.type != TYPE_COMMA ) && ( token.type != TYPE_LEFT_SMALL_BRACKET ) &&
           ( token.type != TYPE_LEFT_MID_BRACKET ) &&
           ( token.type != TYPE_PP ) && ( token.type != TYPE_MM ) )
        printf( " " );
    } // if

    Rest_of_declarators( dataTypeToken, idToken );     // rest_of_declarators
  } // Declaration()

  // statement : ';'     // the null statement
  //           | IF '(' expression ')' statement [ ELSE statement ]
  //           | WHILE '(' expression ')' statement
  //           | DO statement WHILE '(' expression ')' ';'
  //           | RETURN [ expression ] ';'
  //           | compound_statement
  //           | expression ';'  /* expression here should not be empty */
  Token_Value_s Statement() {
    ParameterList_t parmList;
    bool needIndent = false;
    int startPos, startConditionPos;
    Token_s token = mScanner.PeekToken();
    Token_Value_s retMsg, condition;
    retMsg.value.clear();
    parmList.clear();

    // ';'
    if ( token.type == TYPE_SEMICOLON ) {
      token = GetTokenWithType( true, TYPE_SEMICOLON );
      if ( mMode == MODE_LIST_FUNCTION ) {
        printf( "%s", token.str.c_str() );
        if ( mScanner.PeekToken().type != TYPE_RIGHT_BIG_BRACKET )
          printf( "\n%s", mIndentStr.c_str() );
      } // if
    } // if

    // IF '(' expression ')' statement [ ELSE statement ]
    else if ( token.type == TYPE_RESERVE_WORD_IF ) {
      if ( mMode == MODE_SYNTAX_CHECK ) uIfNum++;
      token = GetTokenWithType( true, TYPE_RESERVE_WORD_IF );     // 'if'
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      token = GetTokenWithType( true, TYPE_LEFT_SMALL_BRACKET );  // '('
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      condition = Expression();

      token = GetTokenWithType( true, TYPE_RIGHT_SMALL_BRACKET ); // ')'
      if ( mMode == MODE_LIST_FUNCTION ) {
        needIndent = ( mScanner.PeekToken().type != TYPE_LEFT_BIG_BRACKET );
        printf( "%s ", token.str.c_str() );
        if ( needIndent ) {
          mIndentStr += "  ";
          printf( "\n%s", mIndentStr.c_str() );
        } // if
      } // if

      if ( mMode != MODE_EXECUTION ) {
        startPos = mScanner.GetProgramCounter();
        Statement();
      } // if
      else {
        if ( strcmp( TypeCast( condition, TYPE_CONST_BOOL, false ).value[0].c_str(), "true" ) == 0 )
          retMsg = Statement();

        mScanner.JumpTo( mJumpTable[mScanner.GetProgramCounter()] );
      } // else if

      if ( mMode == MODE_LIST_FUNCTION && needIndent )
        mIndentStr.erase( mIndentStr.end() - 2, mIndentStr.end() );

      // Condition is false
      if ( mMode == MODE_SYNTAX_CHECK ) {
        mJumpTable[startPos] = mScanner.GetProgramCounter();
        startPos = mScanner.GetProgramCounter();
      } // if

      // [ ELSE statement ]
      if ( ( mMode != MODE_EXECUTION ) ||
           ( strcmp( TypeCast( condition, TYPE_CONST_BOOL, false ).value[0].c_str(), "false" ) == 0 ) ) {
        token = mScanner.PeekToken();
        if ( token.type == TYPE_RESERVE_WORD_ELSE ) {
          token = GetTokenWithType( true, TYPE_RESERVE_WORD_ELSE ); // 'else'
          if ( mMode == MODE_LIST_FUNCTION ) {
            needIndent = ( mScanner.PeekToken().type != TYPE_LEFT_BIG_BRACKET );
            printf( "%s ", token.str.c_str() );
            if ( needIndent ) {
              mIndentStr += "  ";
              printf( "\n%s", mIndentStr.c_str() );
            } // if
          } // if

          Statement();

          if ( mMode == MODE_LIST_FUNCTION && needIndent )
            mIndentStr.erase( mIndentStr.end() - 2, mIndentStr.end() );
        } // if
      } // if

      // Condition is true
      if ( mMode == MODE_SYNTAX_CHECK )
        mJumpTable[startPos] = mScanner.GetProgramCounter();
    } // else if

    // WHILE '(' expression ')' statement
    else if ( token.type == TYPE_RESERVE_WORD_WHILE ) {
      token = GetTokenWithType( true, TYPE_RESERVE_WORD_WHILE );  // 'while'
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      token = GetTokenWithType( true, TYPE_LEFT_SMALL_BRACKET );  // '('
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      condition.isArray = false;
      condition.type = TYPE_CONST_BOOL;
      condition.value.resize( 1 );
      condition.value[0] = "true";
      condition.index = -1;
      while ( strcmp( TypeCast( condition, TYPE_CONST_BOOL, false ).value[0].c_str(), "true" ) == 0 ) {
        if ( mMode == MODE_SYNTAX_CHECK )
          startConditionPos = mScanner.GetProgramCounter();

        condition = Expression();

        token = GetTokenWithType( true, TYPE_RIGHT_SMALL_BRACKET ); // ')'
        if ( mMode == MODE_LIST_FUNCTION ) {
          needIndent = ( mScanner.PeekToken().type != TYPE_LEFT_BIG_BRACKET );
          printf( "%s ", token.str.c_str() );
          if ( needIndent ) {
            mIndentStr += "  ";
            printf( "\n%s", mIndentStr.c_str() );
          } // if
        } // if

        if ( mMode == MODE_SYNTAX_CHECK )
          startPos = mScanner.GetProgramCounter();
        if ( mMode != MODE_EXECUTION )
          Statement();
        else {
          if ( strcmp( TypeCast( condition, TYPE_CONST_BOOL, false ).value[0].c_str(), "true" ) == 0 )
            retMsg = Statement();

          mScanner.JumpTo( mJumpTable[mScanner.GetProgramCounter()] );
        } // else if

        if ( mMode == MODE_SYNTAX_CHECK ) {
          mJumpTable[startPos] = mScanner.GetProgramCounter();          // Condition is false
          mJumpTable[mScanner.GetProgramCounter()] = startConditionPos; // Condition is true
        } // if

        if ( mMode != MODE_EXECUTION ) {
          condition.isArray = false;
          condition.type = TYPE_CONST_BOOL;
          condition.value.resize( 1 );
          condition.value[0] = "false";
          condition.index = -1;
        } // if
      } // while

      if ( mMode == MODE_LIST_FUNCTION && needIndent )
        mIndentStr.erase( mIndentStr.end() - 2, mIndentStr.end() );
    } // else if

    // DO statement WHILE '(' expression ')' ';'
    else if ( token.type == TYPE_RESERVE_WORD_DO ) {
      token = GetTokenWithType( true, TYPE_RESERVE_WORD_DO );     // 'do'
      if ( mMode == MODE_LIST_FUNCTION ) {
        needIndent = ( mScanner.PeekToken().type != TYPE_LEFT_BIG_BRACKET );
        printf( "%s ", token.str.c_str() );
        if ( needIndent ) {
          mIndentStr += "  ";
          printf( "\n%s", mIndentStr.c_str() );
        } // if
      } // if

      Statement();

      if ( mMode == MODE_LIST_FUNCTION && needIndent )
        mIndentStr.erase( mIndentStr.end() - 2, mIndentStr.end() );

      token = GetTokenWithType( true, TYPE_RESERVE_WORD_WHILE );  // 'while'
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      token = GetTokenWithType( true, TYPE_LEFT_SMALL_BRACKET );  // '('
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      Expression();

      token = GetTokenWithType( true, TYPE_RIGHT_SMALL_BRACKET ); // ')'
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      token = GetTokenWithType( true, TYPE_SEMICOLON );           // ';'
      if ( mMode == MODE_LIST_FUNCTION ) {
        printf( "%s", token.str.c_str() );
        if ( mScanner.PeekToken().type != TYPE_RIGHT_BIG_BRACKET )
          printf( "\n%s", mIndentStr.c_str() );
      } // if
    } // else if

    // RETURN [ expression ] ';'
    else if ( token.type == TYPE_RESERVE_WORD_RETURN ) {
      token = GetTokenWithType( true, TYPE_RESERVE_WORD_RETURN ); // 'return'
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      token = mScanner.PeekToken();
      if ( ( token.type == TYPE_LEFT_SMALL_BRACKET ) ||
           ( token.type == TYPE_ID ) ||
           ( token.type == TYPE_PP ) ||
           ( token.type == TYPE_MM ) ||
           Sign( token ) || IsConstant( token.type ) )
        retMsg = Expression();

      token = GetTokenWithType( true, TYPE_SEMICOLON );           // ';'
      if ( mMode == MODE_LIST_FUNCTION ) {
        printf( "%s", token.str.c_str() );
        if ( mScanner.PeekToken().type != TYPE_RIGHT_BIG_BRACKET )
          printf( "\n%s", mIndentStr.c_str() );
      } // if

      // Set return value and exit
      if ( mMode == MODE_EXECUTION ) {
        if ( mCallStack.size() > 1 )
          mCallStack[mCallStack.size() - 2].returnValue = retMsg;

        throw ReturnException();
      } // if
    } // else if

    // compound_statement
    else if ( token.type == TYPE_LEFT_BIG_BRACKET )
      Compound_statement();

    // expression ';'  /* expression here should not be empty */
    else {
      Expression();
      token = GetTokenWithType( true, TYPE_SEMICOLON ); // ';'
      if ( mMode == MODE_LIST_FUNCTION ) {
        printf( "%s", token.str.c_str() );
        if ( mScanner.PeekToken().type != TYPE_RIGHT_BIG_BRACKET )
          printf( "\n%s", mIndentStr.c_str() );
      } // if
    } // else

    retMsg.value.push_back( "Statement executed ..." );
    return retMsg;
  } // Statement()

  // expression : basic_expression { ',' basic_expression }
  Token_Value_s Expression() {
    Token_s token;
    Token_Value_s retValue = Basic_expression(); // basic_expression

    while ( mScanner.PeekToken().type == TYPE_COMMA ) {
      token = mScanner.GetToken(); // ','
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );
      retValue = Basic_expression();  // basic_expression
    } // while

    return retValue;
  } // Expression()

  // basic_expression :             Identifier rest_of_Identifier_started_basic_exp
  //                  | ( PP | MM ) Identifier rest_of_PPMM_Identifier_started_basic_exp
  //                  |    sign { sign } signed_unary_exp romce_and_romloe
  //                  | ( Constant | '(' expression ')' ) romce_and_romloe
  Token_Value_s Basic_expression() {
    Token_Value_s retValue, value;
    Token_s idToken, token = mScanner.PeekToken();
    int intValue;
    double floatValue = 0.0;
    Str100 result = "";
    vector<char> signVec;

    value.index = -999;

    // Identifier rest_of_Identifier_started_basic_exp
    if ( token.type == TYPE_ID ) {
      idToken = mScanner.GetToken(); // Identifier
      if ( mScanner.PeekToken().type == TYPE_LEFT_SMALL_BRACKET ) {
        if ( !IsDefinedFunction( token.str ) )
          throw UndefinedTokenException( token );
      } // if
      else GetIDTokenValue( token, -1 );

      if ( mMode == MODE_LIST_FUNCTION ) {
        printf( "%s", token.str.c_str() );
        token = mScanner.PeekToken();
        if ( ( token.type != TYPE_COMMA ) && ( token.type != TYPE_LEFT_SMALL_BRACKET ) &&
             ( token.type != TYPE_LEFT_MID_BRACKET ) &&
             ( token.type != TYPE_PP ) && ( token.type != TYPE_MM ) )
          printf( " " );
      } // if

      retValue = Rest_of_Identifier_started_basic_exp( idToken ); // Rest_of_Identifier_started_basic_exp
    } // if

    // ( PP | MM ) Identifier rest_of_PPMM_Identifier_started_basic_exp
    else if ( ( token.type == TYPE_PP ) || ( token.type == TYPE_MM ) ) {
      mScanner.GetToken();                         // ( PP | MM )
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      idToken = GetTokenWithType( true, TYPE_ID );        // Identifier
      if ( mScanner.PeekToken().type == TYPE_LEFT_SMALL_BRACKET ) {
        if ( !IsDefinedFunction( idToken.str ) )
          throw UndefinedTokenException( idToken );
      } // if
      else retValue = value = GetIDTokenValue( idToken, -1 );

      if ( mMode == MODE_LIST_FUNCTION ) {
        printf( "%s", idToken.str.c_str() );
        token = mScanner.PeekToken();
        if ( ( token.type != TYPE_COMMA ) && ( token.type != TYPE_LEFT_SMALL_BRACKET ) &&
             ( token.type != TYPE_LEFT_MID_BRACKET ) &&
             ( token.type != TYPE_PP ) && ( token.type != TYPE_MM ) )
          printf( " " );
      } // if

      // rest_of_PPMM_Identifier_started_basic_exp
      // ToDo: value is not initialized?
      // if ( value.index == -999 )
      //    intValue = intValue;
      retValue = Rest_of_PPMM_Identifier_started_basic_exp( value, token.type, idToken );
    } // else if

    // ( sign { sign } signed_unary_exp | Constant | '(' expression ')' ) romce_and_romloe
    else {
      signVec.clear();
      if ( Sign( token ) ) {                                    // ( sign { sign } signed_unary_exp
        while ( Sign( token ) ) {
          mScanner.GetToken();
          signVec.push_back( token.str[0] );
          if ( mMode == MODE_LIST_FUNCTION )
            printf( "%s ", token.str.c_str() );

          token = mScanner.PeekToken();
        } // while

        retValue = Signed_unary_exp(); // signed_unary_exp

        if ( mMode == MODE_EXECUTION )
          for ( int i = signVec.size() - 1 ; i >= 0 ; i-- ) {
            if ( signVec[i] == '-' ) {
              if ( retValue.type == TYPE_CONST_INT ) {
                sscanf( retValue.value[0].c_str(), "%d", &intValue );
                sprintf( result, "%d", -intValue );
              } // if
              else if ( retValue.type == TYPE_CONST_FLOAT ) {
                sscanf( retValue.value[0].c_str(), "%lf", &floatValue );
                sprintf( result, "%f", -floatValue );
              } // else if

              retValue.value[0] = string( result );
            } // if
            else if ( signVec[i] == '!' ) {
              retValue.value[0] = ! ( strcmp( TypeCast( retValue, TYPE_CONST_BOOL, false ).value[0].c_str(),
                                              "true" ) == 0 ) ? "true" : "false";
            } // else if
          } // for
      } // if
      else if ( token.type == TYPE_LEFT_SMALL_BRACKET ) {       // '(' expression ')'
        mScanner.GetToken();
        if ( mMode == MODE_LIST_FUNCTION )
          printf( "%s ", token.str.c_str() );

        retValue = Expression();

        token = GetTokenWithType( true, TYPE_RIGHT_SMALL_BRACKET );
        if ( mMode == MODE_LIST_FUNCTION )
          printf( "%s ", token.str.c_str() );
      } // if
      else if ( IsConstant( token.type ) ) {
        retValue.value.push_back( token.str );
        retValue.type = token.type;
        retValue.isArray = false;
        retValue.index = -1;
        token = mScanner.GetToken(); // Constant
        if ( mMode == MODE_LIST_FUNCTION )
          printf( "%s ", token.str.c_str() );
      } // else if
      else throw UnexpectedTokenException( token );

      retValue = Romce_and_romloe( retValue ); // romce_and_romloe
    } // else

    return retValue;
  } // Basic_expression()

  Token_Value_s PPMM( Token_s idToken, TokenType ppMMType, int index ) {
    int intValue = 0;
    double floatValue = 0.0;
    Str100 result = "";
    Token_Value_s idTokenValue = GetIDTokenValue( idToken, -1 );

    if ( idTokenValue.type == TYPE_CONST_FLOAT ) {
      sscanf( idTokenValue.value[index].c_str(), "%lf", &floatValue );
      floatValue = ( ppMMType == TYPE_PP ) ? floatValue + 1 : floatValue - 1;
      sprintf( result, "%f", floatValue );
    } // if
    else {
      sscanf( idTokenValue.value[index].c_str(), "%d", &intValue );
      ( ppMMType == TYPE_PP ) ? intValue++ : intValue--;
      sprintf( result, "%d", intValue );
    } // else

    idTokenValue.value[index] = string( result );
    SetIDTokenValue( idToken, idTokenValue, -1, -1 );

    return idTokenValue;
  } // PPMM()

  Token_Value_s FunctionCall( Token_s IDToken, ArgumentList_t & arguList ) {
    Token_Value_s retValue;

    retValue.type = TYPE_CONST_STRING;
    retValue.value.resize( 1 );
    retValue.value[0] = "0";
    retValue.index = -1;
    Str100 funcName = "";
    strcpy( funcName, IDToken.str.c_str() );

    if ( ( strcmp( funcName, "ListAllVariables" ) == 0 ) && arguList.empty() )
      ListAllVariables();
    else if ( ( strcmp( funcName, "ListAllFunctions" ) == 0 ) && arguList.empty() )
      ListAllFunctions();
    else if ( ( strcmp( funcName, "ListVariable" ) == 0 ) &&
              ( arguList.size() == 1 ) )
      ListVariable( arguList[0] );
    else if ( ( strcmp( funcName, "ListFunction" ) == 0 ) &&
              ( arguList.size() == 1 ) )
      ListFunction( arguList[0] );
    else if ( strcmp( funcName, "Done" ) == 0 )
      Done();
    else retValue = ExecuteFunction( IDToken, arguList );
    return retValue;
  } // FunctionCall()

  // rest_of_Identifier_started_basic_exp :
  //   [ array_expression ] ( assignment_operator basic_expression | [ PP | MM ] romce_and_romloe )
  // | '(' [ actual_parameter_list ] ')' romce_and_romloe
  Token_Value_s Rest_of_Identifier_started_basic_exp( Token_s IDToken ) {
    Token_Value_s retValue, value;
    Token_Value_s idTokenValue, tempTokenValue;
    ArgumentList_t arguList;
    int intValue = 0, index = 0;
    double floatValue = 0.0;
    Str100 result = "";
    Token_s token = mScanner.PeekToken();

    value.index = -999;

    // '(' [ actual_parameter_list ] ')' romce_and_romloe
    if ( token.type == TYPE_LEFT_SMALL_BRACKET ) {
      mScanner.GetToken();
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      token = mScanner.PeekToken();
      if ( ( token.type == TYPE_LEFT_SMALL_BRACKET ) ||
           ( token.type == TYPE_ID ) ||
           ( token.type == TYPE_PP ) ||
           ( token.type == TYPE_MM ) ||
           Sign( token ) || IsConstant( token.type ) )
        arguList = Actual_parameter_list();

      token = GetTokenWithType( true, TYPE_RIGHT_SMALL_BRACKET );
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );
      else if ( mMode == MODE_EXECUTION )
        value = FunctionCall( IDToken, arguList );

      // ToDo: value is not initialized?
      if ( value.index == -999 )
        intValue = intValue;
      retValue = Romce_and_romloe( value );
    } // if

    // [ array_expression ] ( assignment_operator basic_expression | [ PP | MM ] romce_and_romloe )
    else {
      static int count = 0;
      if ( IDToken.str.compare( "w" ) == 0 ) {
        if ( ++count == 14 )
          count = count; // In GetIDTokenValue: param[token.str].info.index is not initialized
      } // if

      value = idTokenValue = GetIDTokenValue( IDToken, -1 );
      // if ( IDToken.str.compare( "w" ) == 0 )
      //  printf(" *** %d: %d\n", count, idTokenValue.value.size() );

      // [ array_expression ]
      if ( token.type == TYPE_LEFT_MID_BRACKET ) {
        index = Array_expression( idTokenValue );

        if ( mMode == MODE_EXECUTION ) {
          value.value.erase( value.value.begin() + ( index + 1 ), value.value.end() );
          value.value.erase( value.value.begin(), value.value.begin() + index );
          value.isArray = false;
          value.index = index;
        } // if
      } // if

      // ( assignment_operator basic_expression | [ PP | MM ] romce_and_romloe )
      token = mScanner.PeekToken();
      if ( Assignment_operator( token.type ) ) {
        mScanner.GetToken();
        if ( mMode == MODE_LIST_FUNCTION )
          printf( "%s ", token.str.c_str() );
        retValue = Basic_expression();
      } // if
      else {
        if ( ( token.type == TYPE_PP ) || ( token.type == TYPE_MM ) ) {
          mScanner.GetToken();
          if ( mMode == MODE_LIST_FUNCTION )
            printf( "%s ", token.str.c_str() );
        } // if

        retValue = Romce_and_romloe( value );

        if ( ( mMode == MODE_EXECUTION ) &&
             ( ( token.type == TYPE_PP ) || ( token.type == TYPE_MM ) ) )
          idTokenValue = PPMM( IDToken, token.type, index );
      } // else

      if ( mMode == MODE_EXECUTION ) {
        idTokenValue = GetIDTokenValue( IDToken, -1 );
        value.isArray = false;
        value.type = idTokenValue.type;
        value.value.resize( 1 );
        value.value[0] = idTokenValue.value[index];
        value.index = -1;
        if ( token.type == TYPE_ASSIGN ) { // =
          idTokenValue.value[index] = retValue.value[0];
          SetIDTokenValue( IDToken, idTokenValue, -1, -1 );
        } // if
        else if ( token.type == TYPE_PE ) { // +=
          retValue = AddSub( value, retValue, true );
          idTokenValue.value[index] = retValue.value[0];
          SetIDTokenValue( IDToken, idTokenValue, -1, -1 );
        } // else if
        else if ( token.type == TYPE_ME ) { // -=
          retValue = AddSub( value, retValue, false );
          idTokenValue.value[index] = retValue.value[0];
          SetIDTokenValue( IDToken, idTokenValue, -1, -1 );
        } // else if
        else if ( token.type == TYPE_TE ) { // *=
          retValue = Mul( value, retValue );
          idTokenValue.value[index] = retValue.value[0];
          SetIDTokenValue( IDToken, idTokenValue, -1, -1 );
        } // else if
        else if ( token.type == TYPE_DE ) { // /=
          retValue = Div( value, retValue );
          idTokenValue.value[index] = retValue.value[0];
          SetIDTokenValue( IDToken, idTokenValue, -1, -1 );
        } // else if
        else if ( token.type == TYPE_RE ) { // %=
          retValue = Mod( value, retValue );
          idTokenValue.value[index] = retValue.value[0];
          SetIDTokenValue( IDToken, idTokenValue, -1, -1 );
        } // else if
      } // if
    } // else

    return retValue;
  } // Rest_of_Identifier_started_basic_exp()

  // rest_of_PPMM_Identifier_started_basic_exp : [ array_expression ] romce_and_romloe 
  Token_Value_s Rest_of_PPMM_Identifier_started_basic_exp( Token_Value_s value, TokenType ppMMType,
                                                           Token_s idToken ) {
    Token_Value_s tempTokenValue;
    Token_s token = mScanner.PeekToken();
    int intValue = 0, index = 0;
    double floatValue = 0.0;
    Str100 result = "";

    // [ array_expression ]
    if ( token.type == TYPE_LEFT_MID_BRACKET )
      index = Array_expression( value );

    if ( mMode == MODE_EXECUTION ) {
      value = PPMM( idToken, ppMMType, index );

      value.value.erase( value.value.begin() + ( index + 1 ), value.value.end() );
      value.value.erase( value.value.begin(), value.value.begin() + index );
      value.isArray = false;
      value.index = index;
    } // if

    // romce_and_romloe
    return Romce_and_romloe( value );
  } // Rest_of_PPMM_Identifier_started_basic_exp()

  // sign : '+' | '-' | '!'
  bool Sign( Token_s token ) {
    return ( token.type == TYPE_ADD ) ||
      ( token.type == TYPE_SUB ) ||
      ( token.type == TYPE_NOT );
  } // Sign()

  // actual_parameter_list : basic_expression { ',' basic_expression }
  ArgumentList_t Actual_parameter_list() {
    Token_s token;
    ArgumentList_t arguList;
    arguList.clear();

    token = mScanner.PeekToken();
    arguList.push_back( Basic_expression() );
    if ( mMode == MODE_EXECUTION )
      arguList[arguList.size() - 1].str = token.str;

    while ( mScanner.PeekToken().type == TYPE_COMMA ) {
      token = mScanner.GetToken();
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );
    
      token = mScanner.PeekToken();
      arguList.push_back( Basic_expression() );
      if ( mMode == MODE_EXECUTION )
        arguList[arguList.size() - 1].str = token.str;
    } // while

    return arguList;
  } // Actual_parameter_list()

  // assignment_operator : '=' | TE | DE | RE | PE | ME
  bool Assignment_operator( TokenType type ) {
    return ( type == TYPE_ASSIGN ) ||
      ( type == TYPE_TE ) ||
      ( type == TYPE_DE ) ||
      ( type == TYPE_RE ) ||
      ( type == TYPE_PE ) ||
      ( type == TYPE_ME );
  } // Assignment_operator()

  // rest_of_maybe_conditional_exp_and_rest_of_maybe_logical_OR_exp // 即romce_and_romloe
  // : rest_of_maybe_logical_OR_exp [ '?' basic_expression ':' basic_expression ]
  Token_Value_s Romce_and_romloe( Token_Value_s value ) {
    Token_s token;
    int startPos;
    Token_Value_s retValue, condition;
    retValue = condition = Rest_of_maybe_logical_OR_exp( value ); // rest_of_maybe_logical_OR_exp

    // [ '?' basic_expression ':' basic_expression ]
    if ( mScanner.PeekToken().type == TYPE_ROMCE ) {
      token = mScanner.GetToken();              // '?'
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      if ( mMode != MODE_EXECUTION ) {
        startPos = mScanner.GetProgramCounter();
        retValue = Basic_expression();
      } // if
      else {
        if ( strcmp( TypeCast( condition, TYPE_CONST_BOOL, false ).value[0].c_str(), "true" ) == 0 )
          retValue = Basic_expression();                       // basic_expression
        mScanner.JumpTo( mJumpTable[mScanner.GetProgramCounter()] );
      } // else

      // Condition is false
      if ( mMode == MODE_SYNTAX_CHECK ) {
        mJumpTable[startPos] = mScanner.GetProgramCounter();
        startPos = mScanner.GetProgramCounter();
      } // if

      if ( ( mMode != MODE_EXECUTION ) ||
           ( strcmp( TypeCast( condition, TYPE_CONST_BOOL, false ).value[0].c_str(), "false" ) == 0 ) ) {
        token = GetTokenWithType( true, TYPE_ROMLOE ); // ':'
        if ( mMode == MODE_LIST_FUNCTION )
          printf( "%s ", token.str.c_str() );

        retValue = Basic_expression();                       // basic_expression
      } // if

      // Condition is true
      if ( mMode == MODE_SYNTAX_CHECK )
        mJumpTable[startPos] = mScanner.GetProgramCounter();
    } // if

    return retValue;
  } // Romce_and_romloe()

  Token_Value_s AndOr( Token_Value_s value1, Token_Value_s value2, bool isAnd ) {
    bool boolValue1 = false, boolValue2 = false;
    Token_Value_s retValue = value1;

    boolValue1 = ( strcmp( TypeCast( retValue, TYPE_CONST_BOOL, false ).value[0].c_str(), "true" ) == 0 );
    boolValue2 = ( strcmp( TypeCast( value2, TYPE_CONST_BOOL, false ).value[0].c_str(), "true" ) == 0 );
    retValue.isArray  = false;
    retValue.type     = TYPE_CONST_BOOL;
    if ( ( isAnd  && ( boolValue1 && boolValue2 ) ) ||
         ( !isAnd && ( boolValue1 || boolValue2 ) ) )
      retValue.value[0] = "true";
    else
      retValue.value[0] = "false";

    retValue.str = "";
    return retValue;
  } // AndOr()

  // rest_of_maybe_logical_OR_exp : rest_of_maybe_logical_AND_exp { OR maybe_logical_AND_exp }
  Token_Value_s Rest_of_maybe_logical_OR_exp( Token_Value_s value ) {
    Token_s token;
    Token_Value_s retValue = Rest_of_maybe_logical_AND_exp( value ), value2; // rest_of_maybe_logical_AND_exp

    // { OR maybe_logical_AND_exp }
    while ( mScanner.PeekToken().type == TYPE_OR ) {
      token = mScanner.GetToken();
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      value.type = TYPE_UNSET;
      value2 = Maybe_logical_AND_exp( value );

      if ( mMode == MODE_EXECUTION )
        retValue = AndOr( retValue, value2, false );
    } // while

    return retValue;
  } // Rest_of_maybe_logical_OR_exp()

  // maybe_logical_AND_exp : maybe_bit_OR_exp { AND maybe_bit_OR_exp }
  Token_Value_s Maybe_logical_AND_exp( Token_Value_s value ) {
    Token_s token;
    Token_Value_s retValue = Maybe_bit_OR_exp( value ), value2; // maybe_bit_OR_exp

    // { AND maybe_bit_OR_exp }
    while ( mScanner.PeekToken().type == TYPE_AND ) {
      token = GetTokenWithType( true, TYPE_AND );
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      value.type = TYPE_UNSET;
      value2 = Maybe_bit_OR_exp( value );
      if ( mMode == MODE_EXECUTION )
        retValue = AndOr( retValue, value2, true );
    } // while

    return retValue;
  } // Maybe_logical_AND_exp()

  // rest_of_maybe_logical_AND_exp : rest_of_maybe_bit_OR_exp { AND maybe_bit_OR_exp }
  Token_Value_s Rest_of_maybe_logical_AND_exp( Token_Value_s value ) {
    Token_s token;
    Token_Value_s retValue = Rest_of_maybe_bit_OR_exp( value ), value2; // rest_of_maybe_bit_OR_exp

    // { AND maybe_bit_OR_exp }
    while ( mScanner.PeekToken().type == TYPE_AND ) {
      token = GetTokenWithType( true, TYPE_AND );
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      value.type = TYPE_UNSET;
      value2 = Maybe_bit_OR_exp( value );
      if ( mMode == MODE_EXECUTION )
        retValue = AndOr( retValue, value2, true );
    } // while

    return retValue;
  } // Rest_of_maybe_logical_AND_exp()

  Token_Value_s BitOperation( Token_Value_s value1, Token_Value_s value2, TokenType type ) {
    int intValue = 0, intValue2 = 0;
    Str100 result = "";
    Token_Value_s retValue = value1;

    sscanf( TypeCast( retValue, TYPE_CONST_INT, false ).value[0].c_str(), "%d", &intValue );
    sscanf( TypeCast( value2, TYPE_CONST_INT, false ).value[0].c_str(), "%d", &intValue2 );
    /*
      if ( type == TYPE_BIT_AND )
        sprintf( result, "%d", intValue & intValue2 );
      else if ( type == TYPE_BIT_OR )
        sprintf( result, "%d", intValue | intValue2 );
      else if ( type == TYPE_XOR )
        sprintf( result, "%d", intValue ^ intValue2 );
    */

    retValue.isArray  = false;
    retValue.type     = TYPE_CONST_INT;
    retValue.value[0] = string( result );
    retValue.str = "";
    return retValue;
  } // BitOperation()

  // maybe_bit_OR_exp : maybe_bit_ex_OR_exp { '|' maybe_bit_ex_OR_exp }
  Token_Value_s Maybe_bit_OR_exp( Token_Value_s value ) {
    Token_s token;
    Token_Value_s retValue = Maybe_bit_ex_OR_exp( value ), value2; // maybe_bit_ex_OR_exp

    // { '|' maybe_bit_ex_OR_exp }
    while ( mScanner.PeekToken().type == TYPE_BIT_OR ) {
      token = GetTokenWithType( true, TYPE_BIT_OR );
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      value.type = TYPE_UNSET;
      value2 = Maybe_bit_ex_OR_exp( value );
      if ( mMode == MODE_EXECUTION )
        retValue = BitOperation( retValue, value2, token.type );
    } // while

    return retValue;
  } // Maybe_bit_OR_exp()

  // rest_of_maybe_bit_OR_exp : rest_of_maybe_bit_ex_OR_exp { '|' maybe_bit_ex_OR_exp }
  Token_Value_s Rest_of_maybe_bit_OR_exp( Token_Value_s value ) {
    Token_s token;
    Token_Value_s retValue = Rest_of_maybe_bit_ex_OR_exp( value ), value2; // rest_of_maybe_bit_ex_OR_exp

    // { '|' maybe_bit_ex_OR_exp }
    while ( mScanner.PeekToken().type == TYPE_BIT_OR ) {
      token = GetTokenWithType( true, TYPE_BIT_OR );
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      value.type = TYPE_UNSET;
      value2 = Maybe_bit_ex_OR_exp( value );
      if ( mMode == MODE_EXECUTION )
        retValue = BitOperation( retValue, value2, token.type );
    } // while

    return retValue;
  } // Rest_of_maybe_bit_OR_exp()

  // maybe_bit_ex_OR_exp : maybe_bit_AND_exp { '^' maybe_bit_AND_exp }
  Token_Value_s Maybe_bit_ex_OR_exp( Token_Value_s value ) {
    Token_s token;
    Token_Value_s retValue = Maybe_bit_AND_exp( value ), value2; // maybe_bit_AND_exp

    // { '^' maybe_bit_AND_exp }
    while ( mScanner.PeekToken().type == TYPE_XOR ) {
      token = GetTokenWithType( true, TYPE_XOR );
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      value.type = TYPE_UNSET;
      value2 = Maybe_bit_AND_exp( value );
      if ( mMode == MODE_EXECUTION )
        retValue = BitOperation( retValue, value2, token.type );
    } // while

    return retValue;
  } // Maybe_bit_ex_OR_exp()

  // rest_of_maybe_bit_ex_OR_exp : rest_of_maybe_bit_AND_exp { '^' maybe_bit_AND_exp }
  Token_Value_s Rest_of_maybe_bit_ex_OR_exp( Token_Value_s value ) {
    Token_s token;
    Token_Value_s retValue = Rest_of_maybe_bit_AND_exp( value ), value2; // rest_of_maybe_bit_AND_exp

    // { '^' maybe_bit_AND_exp }
    while ( mScanner.PeekToken().type == TYPE_XOR ) {
      token = GetTokenWithType( true, TYPE_XOR );
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      value.type = TYPE_UNSET;
      value2 = Maybe_bit_AND_exp( value );
      if ( mMode == MODE_EXECUTION )
        retValue = BitOperation( retValue, value2, token.type );
    } // while

    return retValue;
  } // Rest_of_maybe_bit_ex_OR_exp()

  // maybe_bit_AND_exp : maybe_equality_exp { '&' maybe_equality_exp }
  Token_Value_s Maybe_bit_AND_exp( Token_Value_s value ) {
    Token_s token;
    Token_Value_s retValue = Maybe_equality_exp( value ), value2; // maybe_equality_exp

    // { '&' maybe_equality_exp }
    while ( mScanner.PeekToken().type == TYPE_BIT_AND ) {
      token = GetTokenWithType( true, TYPE_BIT_AND );
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      value.type = TYPE_UNSET;
      value2 = Maybe_equality_exp( value );
      if ( mMode == MODE_EXECUTION )
        retValue = BitOperation( retValue, value2, token.type );
    } // while

    return retValue;
  } // Maybe_bit_AND_exp()

  // rest_of_maybe_bit_AND_exp : rest_of_maybe_equality_exp { '&' maybe_equality_exp }
  Token_Value_s Rest_of_maybe_bit_AND_exp( Token_Value_s value ) {
    Token_s token;
    Token_Value_s retValue = Rest_of_maybe_equality_exp( value ), value2; // rest_of_maybe_equality_exp

    // { '&' maybe_equality_exp }
    while ( mScanner.PeekToken().type == TYPE_BIT_AND ) {
      token = GetTokenWithType( true, TYPE_BIT_AND );
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      value.type = TYPE_UNSET;
      value2 = Maybe_equality_exp( value );
      if ( mMode == MODE_EXECUTION )
        retValue = BitOperation( retValue, value2, token.type );
    } // while

    return retValue;
  } // Rest_of_maybe_bit_AND_exp()

  Token_Value_s BoolCompare( Token_Value_s value1, Token_Value_s value2, TokenType type ) {
    double floatValue = 0.0, floatValue2 = 0.0;
    Token_Value_s retValue = value1;

    sscanf( TypeCast( retValue, TYPE_CONST_FLOAT, false ).value[0].c_str(), "%lf", &floatValue );
    sscanf( TypeCast( value2, TYPE_CONST_FLOAT, false ).value[0].c_str(), "%lf", &floatValue2 );
    floatValue -= floatValue2;

    // EQ
    if ( ( floatValue >= -EPSINON && floatValue <= EPSINON ) &&
         ( ( type == TYPE_EQ ) || ( type == TYPE_GE ) || ( type == TYPE_LE ) ) )
      retValue.value[0] = "true";
    // GT
    else if ( ( floatValue > EPSINON ) &&
              ( ( type == TYPE_NE ) || ( type == TYPE_GE ) || ( type == TYPE_GT ) ) )
      retValue.value[0] = "true";
    // LT
    else if ( ( floatValue < -EPSINON ) &&
              ( ( type == TYPE_NE ) || ( type == TYPE_LE ) || ( type == TYPE_LT ) ) )
      retValue.value[0] = "true";
    else retValue.value[0] = "false";

    retValue.isArray  = false;
    retValue.type     = TYPE_CONST_BOOL;
    retValue.str = "";
    return retValue;
  } // BoolCompare()

  // maybe_equality_exp : maybe_relational_exp { ( EQ | NEQ ) maybe_relational_exp}
  Token_Value_s Maybe_equality_exp( Token_Value_s value ) {
    Token_s token;
    Token_Value_s retValue = Maybe_relational_exp( value ), value2; // maybe_relational_exp

    // { ( EQ | NEQ ) maybe_relational_exp}
    token = mScanner.PeekToken();
    while ( ( token.type == TYPE_EQ ) || ( token.type == TYPE_NE ) ) {
      mScanner.GetToken();
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      value.type = TYPE_UNSET;
      value2 = Maybe_relational_exp( value );
      if ( mMode == MODE_EXECUTION )
        retValue = BoolCompare( retValue, value2, token.type );

      token = mScanner.PeekToken();
    } // while

    return retValue;
  } // Maybe_equality_exp()

  // rest_of_maybe_equality_exp : rest_of_maybe_relational_exp { ( EQ | NEQ ) maybe_relational_exp }
  Token_Value_s Rest_of_maybe_equality_exp( Token_Value_s value ) {
    Token_s token;
    double floatValue = 0.0, floatValue2 = 0.0;
    Token_Value_s retValue = Rest_of_maybe_relational_exp( value ), value2; // rest_of_maybe_relational_exp

    // { ( EQ | NEQ ) maybe_relational_exp }
    token = mScanner.PeekToken();
    while ( ( token.type == TYPE_EQ ) || ( token.type == TYPE_NE ) ) {
      mScanner.GetToken();
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      value.type = TYPE_UNSET;
      value2 = Maybe_relational_exp( value );
      if ( mMode == MODE_EXECUTION )
        retValue = BoolCompare( retValue, value2, token.type );

      token = mScanner.PeekToken();
    } // while

    return retValue;
  } // Rest_of_maybe_equality_exp()

  // maybe_relational_exp : maybe_shift_exp { ( '<' | '>' | LE | GE ) maybe_shift_exp }
  Token_Value_s Maybe_relational_exp( Token_Value_s value ) {
    Token_s token;
    double floatValue = 0.0, floatValue2 = 0.0;
    Token_Value_s retValue = Maybe_shift_exp( value ), value2; // maybe_shift_exp

    // { ( '<' | '>' | LE | GE ) maybe_shift_exp }
    token = mScanner.PeekToken();
    while ( ( token.type == TYPE_LT ) || ( token.type == TYPE_GT ) ||
            ( token.type == TYPE_LE ) || ( token.type == TYPE_GE ) ) {
      mScanner.GetToken();
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      value.type = TYPE_UNSET;
      value2 = Maybe_shift_exp( value );
      if ( mMode == MODE_EXECUTION )
        retValue = BoolCompare( retValue, value2, token.type );

      token = mScanner.PeekToken();
    } // while

    return retValue;
  } // Maybe_relational_exp()

  // rest_of_maybe_relational_exp : rest_of_maybe_shift_exp { ( '<' | '>' | LE | GE ) maybe_shift_exp }
  Token_Value_s Rest_of_maybe_relational_exp( Token_Value_s value ) {
    Token_s token;
    double floatValue = 0.0, floatValue2 = 0.0;
    Token_Value_s retValue = Rest_of_maybe_shift_exp( value ), value2; // rest_of_maybe_shift_exp

    // { ( '<' | '>' | LE | GE ) maybe_shift_exp }
    token = mScanner.PeekToken();
    while ( ( token.type == TYPE_LT ) || ( token.type == TYPE_GT ) ||
            ( token.type == TYPE_LE ) || ( token.type == TYPE_GE ) ) {
      mScanner.GetToken();
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );
        /*
          if ( ( mMode == MODE_SYNTAX_CHECK ) && ( uTestNum == 2 ) && ( uIfNum == 2 ) ) {
            PrintStack();
            throw EOF_EXCEPTION;
          } // if
        */

      value.type = TYPE_UNSET;
      value2 = Maybe_shift_exp( value );
      if ( mMode == MODE_EXECUTION )
        retValue = BoolCompare( retValue, value2, token.type );

      token = mScanner.PeekToken();
    } // while

    return retValue;
  } // Rest_of_maybe_relational_exp()

  Token_Value_s Shift( Token_Value_s value1, Token_Value_s value2, bool isLeftShift ) {
    int intValue = 0, intValue2 = 0;
    Str100 result = "";
    Token_Value_s retValue = value1;

    sscanf( retValue.value[0].c_str(), "%d", &intValue );
    sscanf( value2.value[0].c_str(), "%d", &intValue2 );
    sprintf( result, "%d", isLeftShift ? intValue << intValue2 : intValue >> intValue2 );

    retValue.isArray  = false;
    retValue.type     = TYPE_CONST_INT;
    retValue.value[0] = string( result );
    retValue.str = "";
    return retValue;
  } // Shift()

  // maybe_shift_exp : maybe_additive_exp { ( LS | RS ) maybe_additive_exp }
  Token_Value_s Maybe_shift_exp( Token_Value_s value ) {
    Token_s token;
    int cinCoutFactor;
    Str100 result = "";
    Token_Value_s retValue, value2;
    bool isLeftShift;

    if ( !value.value.empty() && ( strcmp( value.value[0].c_str(), "cin" ) == 0 ) )
      cinCoutFactor = CIN;
    else if ( !value.value.empty() && ( strcmp( value.value[0].c_str(), "cout" ) == 0 ) )
      cinCoutFactor = COUT;
    else
      cinCoutFactor = NORMAL;

    if ( cinCoutFactor == NORMAL )
      retValue = Maybe_additive_exp( value ); // maybe_additive_exp
    else {
      retValue.type = TYPE_CONST_STRING;
      retValue.value.resize( 1 );
      retValue.index = -1;
    } // else

    // { ( LS | RS ) maybe_additive_exp }
    token = mScanner.PeekToken();
    while ( token.type == TYPE_LS || token.type == TYPE_RS ) {
      if ( cinCoutFactor == CIN )
        GetTokenWithType( true, TYPE_RS );
      else if ( cinCoutFactor == COUT )
        GetTokenWithType( true, TYPE_LS );
      else mScanner.GetToken();

      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      value.type = TYPE_UNSET;
      value2 = Maybe_additive_exp( value );

      if ( cinCoutFactor == COUT ) {
        if ( !value2.value.empty() )
          if ( ( mMode == MODE_EXECUTION ) && ( cinCoutFactor == COUT ) )
            // if ( cinCoutFactor == COUT )
            printf( "%s", value2.value[0].c_str() );
      } // if
      else if ( cinCoutFactor == CIN ) {
        ;
      } // else if
      else if ( mMode == MODE_EXECUTION ) {
        if ( retValue.type != TYPE_CONST_INT ) {
          sprintf( result, "Error: Left of '%s' must be integer", token.str.c_str() );
          throw string( result );
        } // if

        if ( value2.type != TYPE_CONST_INT ) {
          sprintf( result, "Error: Right of '%s' must be integer", token.str.c_str() );
          throw string( result );
        } // if

        isLeftShift = ( token.type == TYPE_LS );
        retValue = Shift( retValue, value2, isLeftShift );
      } // else if

      token = mScanner.PeekToken();
    } // while

    return retValue;
  } // Maybe_shift_exp()

  // rest_of_maybe_shift_exp : rest_of_maybe_additive_exp { ( LS | RS ) maybe_additive_exp }
  Token_Value_s Rest_of_maybe_shift_exp( Token_Value_s value ) {
    Token_s token;
    int cinCoutFactor;
    Str100 result = "";
    Token_Value_s retValue, value2;
    bool isLeftShift;

    if ( !value.value.empty() && ( strcmp( value.value[0].c_str(), "cin" ) == 0 ) )
      cinCoutFactor = CIN;
    else if ( !value.value.empty() && ( strcmp( value.value[0].c_str(), "cout" ) == 0 ) )
      cinCoutFactor = COUT;
    else
      cinCoutFactor = NORMAL;

    if ( cinCoutFactor == NORMAL )
      retValue = Rest_of_maybe_additive_exp( value ); // rest_of_maybe_additive_exp
    else {
      retValue.type = TYPE_CONST_STRING;
      retValue.value.resize( 1 );
      retValue.index = -1;
    } // else

    // { ( LS | RS ) maybe_additive_exp }
    token = mScanner.PeekToken();
    while ( token.type == TYPE_LS || token.type == TYPE_RS ) {
      if ( cinCoutFactor == CIN )
        GetTokenWithType( true, TYPE_RS );
      else if ( cinCoutFactor == COUT )
        GetTokenWithType( true, TYPE_LS );
      else mScanner.GetToken();

      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      value.type = TYPE_UNSET;
      value2 = Maybe_additive_exp( value );
      if ( cinCoutFactor == COUT ) {
        if ( !value2.value.empty() )
          if ( ( mMode == MODE_EXECUTION ) && ( cinCoutFactor == COUT ) )
            // if ( cinCoutFactor == COUT )
            printf( "%s", value2.value[0].c_str() );
      } // if
      else if ( cinCoutFactor == CIN ) {
        ;
      } // else if
      else if ( mMode == MODE_EXECUTION ) {
        if ( retValue.type != TYPE_CONST_INT ) {
          sprintf( result, "Error: Left of '%s' must be integer", token.str.c_str() );
          throw string( result );
        } // if

        if ( value2.type != TYPE_CONST_INT ) {
          sprintf( result, "Error: Right of '%s' must be integer", token.str.c_str() );
          throw string( result );
        } // if

        isLeftShift = ( token.type == TYPE_LS );
        retValue = Shift( retValue, value2, isLeftShift );
      } // else if

      token = mScanner.PeekToken();
    } // while

    return retValue;
  } // Rest_of_maybe_shift_exp()

  Token_Value_s AddSub( Token_Value_s value1, Token_Value_s value2, bool isAdd ) {
    double floatValue = 0.0, floatValue2 = 0.0;
    int intValue = 0, intValue2 = 0;
    Str100 result = "";
    Token_Value_s retValue = value1;

    if ( retValue.type == TYPE_CONST_STRING || value2.type == TYPE_CONST_STRING ) {
      if ( !isAdd ) throw "Error: Cannot substract string";

      retValue.value[0] += value2.value[0];
      retValue.type = TYPE_CONST_STRING;
      retValue.index = -1;
    } // if
    else {
      if ( retValue.type == TYPE_CONST_FLOAT || value2.type == TYPE_CONST_FLOAT ) {
        sscanf( TypeCast( retValue, TYPE_CONST_FLOAT, false ).value[0].c_str(), "%lf",  &floatValue );
        sscanf( TypeCast( value2, TYPE_CONST_FLOAT, false ).value[0].c_str(), "%lf",  &floatValue2 );
        isAdd ? floatValue += floatValue2 : floatValue -= floatValue2;

        retValue.type = TYPE_CONST_FLOAT;
        sprintf( result, "%f", floatValue );
      } // else if
      else {
        sscanf( TypeCast( retValue, TYPE_CONST_INT, false ).value[0].c_str(), "%d",  &intValue );
        sscanf( TypeCast( value2, TYPE_CONST_INT, false ).value[0].c_str(), "%d",  &intValue2 );
        isAdd ? intValue += intValue2 : intValue -= intValue2;

        retValue.type = TYPE_CONST_INT;
        sprintf( result, "%d", intValue );
      } // else

      retValue.value[0] = string( result ); 
      retValue.isArray  = false;
    } // else

    retValue.str = "";
    return retValue;
  } // AddSub()

  // maybe_additive_exp : maybe_mult_exp { ( '+' | '-' ) maybe_mult_exp }
  Token_Value_s Maybe_additive_exp( Token_Value_s value ) {
    Token_s token;
    Token_Value_s retValue = Maybe_mult_exp( value ), value2; // maybe_mult_exp
    bool isAdd;

    // { ( '+' | '-' ) maybe_mult_exp }
    token = mScanner.PeekToken();
    while ( token.type == TYPE_ADD || token.type == TYPE_SUB ) {
      mScanner.GetToken();
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      value.type = TYPE_UNSET;
      value2 = Maybe_mult_exp( value );
      isAdd  = ( token.type == TYPE_ADD );
      if ( mMode == MODE_EXECUTION )
        retValue = AddSub( retValue, value2, isAdd );

      token = mScanner.PeekToken();
    } // while

    return retValue;
  } // Maybe_additive_exp()

  // rest_of_maybe_additive_exp : rest_of_maybe_mult_exp { ( '+' | '-' ) maybe_mult_exp }
  Token_Value_s Rest_of_maybe_additive_exp( Token_Value_s value ) {
    Token_s token;
    Token_Value_s retValue = Rest_of_maybe_mult_exp( value ), value2; // rest_of_maybe_mult_exp
    bool isAdd;

    // { ( '+' | '-' ) maybe_mult_exp }
    token = mScanner.PeekToken();
    while ( token.type == TYPE_ADD || token.type == TYPE_SUB ) {
      mScanner.GetToken();
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      value.type = TYPE_UNSET;
      value2 = Maybe_mult_exp( value );
      isAdd = ( token.type == TYPE_ADD );
      if ( mMode == MODE_EXECUTION )
        retValue = AddSub( retValue, value2, isAdd );

      token = mScanner.PeekToken();
    } // while

    return retValue;
  } // Rest_of_maybe_additive_exp()

  Token_Value_s Mul( Token_Value_s value1, Token_Value_s value2 ) {
    Token_Value_s retValue = value1;
    double floatValue = 0.0, floatValue2 = 0.0;
    int intValue = 0, intValue2 = 0;
    Str100 result = "";

    if ( ( retValue.type == TYPE_CONST_FLOAT ) || ( value2.type == TYPE_CONST_FLOAT ) ) {
      sscanf( TypeCast( retValue, TYPE_CONST_FLOAT, false ).value[0].c_str(), "%lf", &floatValue );
      sscanf( TypeCast( value2, TYPE_CONST_FLOAT, false ).value[0].c_str(), "%lf", &floatValue2 );
      floatValue *= floatValue2;

      retValue.type = TYPE_CONST_FLOAT;
      sprintf( result, "%f", floatValue );
    } // if
    else {
      sscanf( TypeCast( retValue, TYPE_CONST_INT, false ).value[0].c_str(), "%d", &intValue );
      sscanf( TypeCast( value2, TYPE_CONST_INT, false ).value[0].c_str(), "%d", &intValue2 );
      intValue *= intValue2;

      retValue.type = TYPE_CONST_INT;
      sprintf( result, "%d", intValue );
    } // else

    retValue.isArray = false;
    retValue.value[0] = string( result );
    retValue.str = "";
    return retValue;
  } // Mul()

  Token_Value_s Div( Token_Value_s value1, Token_Value_s value2 ) {
    Token_Value_s retValue = value1;
    double floatValue = 0.0, floatValue2 = 0.0;
    int intValue = 0, intValue2 = 0;
    Str100 result = "";

    if ( strcmp( TypeCast( value2, TYPE_CONST_INT, false ).value[0].c_str(), "0" ) == 0 )
      throw "Error: Divided by zero";


    if ( ( retValue.type == TYPE_CONST_FLOAT ) || ( value2.type == TYPE_CONST_FLOAT ) ) {
      sscanf( TypeCast( retValue, TYPE_CONST_FLOAT, false ).value[0].c_str(), "%lf", &floatValue );
      sscanf( TypeCast( value2, TYPE_CONST_FLOAT, false ).value[0].c_str(), "%lf", &floatValue2 );
      floatValue /= floatValue2;

      retValue.type = TYPE_CONST_FLOAT;
      sprintf( result, "%f", floatValue );
    } // if
    else {
      sscanf( TypeCast( retValue, TYPE_CONST_INT, false ).value[0].c_str(), "%d", &intValue );
      sscanf( TypeCast( value2, TYPE_CONST_INT, false ).value[0].c_str(), "%d", &intValue2 );
      intValue /= intValue2;

      retValue.type = TYPE_CONST_INT;
      sprintf( result, "%d", intValue );
    } // else

    retValue.isArray = false;
    retValue.value[0] = string( result );
    retValue.str = "";
    return retValue;
  } // Div()

  Token_Value_s Mod( Token_Value_s value1, Token_Value_s value2 ) {
    Token_Value_s retValue = value1;
    int intValue = 0, intValue2 = 0;
    Str100 result = "";

    if ( value2.type != TYPE_CONST_INT )
      throw "Error: Right of '%' must be an integer";
    if ( strcmp( TypeCast( value2, TYPE_CONST_INT, false ).value[0].c_str(), "0" ) == 0 )
      throw "Error: Divided by zero";

    sscanf( TypeCast( retValue, TYPE_CONST_INT, false ).value[0].c_str(), "%d", &intValue );
    sscanf( TypeCast( value2, TYPE_CONST_INT, false ).value[0].c_str(), "%d", &intValue2 );
    intValue %= intValue2;
    sprintf( result, "%d", intValue );

    retValue.isArray = false;
    retValue.type = TYPE_CONST_INT;
    retValue.value[0] = string( result );
    retValue.str = "";
    return retValue;
  } // Mod()

  // maybe_mult_exp : unary_exp rest_of_maybe_mult_exp
  Token_Value_s Maybe_mult_exp( Token_Value_s value ) {
    Token_Value_s retValue = Unary_exp( value ); // unary_exp
    return Rest_of_maybe_mult_exp( retValue ); // rest_of_maybe_mult_exp
  } // Maybe_mult_exp()

  // rest_of_maybe_mult_exp : { ( '*' | '/' | '%' ) unary_exp }  /* could be empty ! */
  Token_Value_s Rest_of_maybe_mult_exp( Token_Value_s value ) {
    Token_s token = mScanner.PeekToken();
    Token_Value_s retValue = value, value2;

    // { ( '*' | '/' | '%' ) unary_exp }
    while ( ( token.type == TYPE_MUL ) || ( token.type == TYPE_DIV ) || ( token.type == TYPE_MOD ) ) {
      mScanner.GetToken();
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );
      if ( ( mMode == MODE_EXECUTION ) && ( token.type == TYPE_MOD ) && ( retValue.type != TYPE_CONST_INT ) )
        throw "Error: Left of '%' must be an integer";

      value.type = TYPE_UNSET;
      value2 = Unary_exp( value );
      if ( mMode == MODE_EXECUTION ) {
        if ( token.type == TYPE_MUL )
          retValue = Mul( retValue, value2 );
        else if ( token.type == TYPE_DIV )
          retValue = Div( retValue, value2 );
        else
          retValue = Mod( retValue, value2 );
      } // if

      token = mScanner.PeekToken();
    } // while

    return retValue;
  } // Rest_of_maybe_mult_exp()

  // unary_exp : sign { sign } signed_unary_exp | unsigned_unary_exp |
  //             ( PP | MM ) Identifier [ array_expression ]
  Token_Value_s Unary_exp( Token_Value_s value ) {
    Token_s idToken, ppMMToken, token = mScanner.PeekToken();
    Token_Value_s retValue = value, tempTokenValue;
    vector<char> signVec;
    Str100 result = "";
    int index = 0;
    double floatValue = 0.0;

    // sign { sign } signed_unary_exp
    signVec.clear();
    if ( Sign( token ) ) {
      while ( Sign( token ) ) {
        mScanner.GetToken();
        signVec.push_back( token.str[0] );
        if ( mMode == MODE_LIST_FUNCTION )
          printf( "%s ", token.str.c_str() );

        token = mScanner.PeekToken();
      } // while

      retValue = Signed_unary_exp();
      if ( mMode == MODE_EXECUTION )
        for ( int i = signVec.size() - 1 ; i >= 0 ; i-- ) {
          if ( ( signVec[i] == '-' ) && ( ( retValue.type == TYPE_CONST_INT ) ||
                                          ( retValue.type == TYPE_CONST_FLOAT ) ) ) {
            sscanf( retValue.value[0].c_str(), "%lf", &floatValue );
            floatValue = -floatValue;
            sprintf( result, "%f", floatValue );
            retValue.value[0] = string( result ); 
          } // if
          else if ( signVec[i] == '!' ) {
            retValue.value[0] = ! ( strcmp( TypeCast( retValue, TYPE_CONST_BOOL, false ).value[0].c_str(),
                                            "true" ) == 0 ) ? "true" : "false";
          } // else if
        } // for
    } // if
    // ( PP | MM ) Identifier [ array_expression ]
    else if ( ( token.type == TYPE_PP ) || ( token.type == TYPE_MM ) ) {
      ppMMToken = mScanner.GetToken();
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      idToken = GetTokenWithType( true, TYPE_ID ); // Identifier
      if ( mScanner.PeekToken().type == TYPE_LEFT_SMALL_BRACKET ) {
        if ( !IsDefinedFunction( idToken.str ) )
          throw UndefinedTokenException( idToken );
      } // if
      else retValue = GetIDTokenValue( idToken, -1 );

      if ( mMode == MODE_LIST_FUNCTION ) {
        printf( "%s", idToken.str.c_str() );
        token = mScanner.PeekToken();
        if ( ( token.type != TYPE_COMMA ) && ( token.type != TYPE_LEFT_SMALL_BRACKET ) &&
             ( token.type != TYPE_LEFT_MID_BRACKET ) &&
             ( token.type != TYPE_PP ) && ( token.type != TYPE_MM ) )
          printf( " " );
      } // if

      // [ array_expression ]
      if ( mScanner.PeekToken().type == TYPE_LEFT_MID_BRACKET ) {
        index = Array_expression( retValue );
      } // if

      if ( mMode == MODE_EXECUTION ) retValue = PPMM( idToken, ppMMToken.type, index );
    } // else if
    // unsigned_unary_exp
    else
      retValue = Unsigned_unary_exp();

    return retValue;
  } // Unary_exp()

  // signed_unary_exp   :
  // Identifier [ '(' [ actual_parameter_list ] ')' | array_expression ]
  // | Constant | '(' expression ')'
  // unsigned_unary_exp :
  // Identifier [ '(' [ actual_parameter_list ] ')' | [ array_expression ] [ ( PP | MM ) ] ]
  // | Constant | '(' expression ')'
  Token_Value_s Signed_unary_exp() {
    return Basic_unary_exp( true );
  } // Signed_unary_exp()

  Token_Value_s Unsigned_unary_exp() {
    return Basic_unary_exp( false );
  } // Unsigned_unary_exp()

  Token_Value_s Basic_unary_exp( bool isSigned ) {
    ArgumentList_t arguList;
    Token_s idToken, token = mScanner.PeekToken();
    Token_Value_s retValue, value, tempTokenValue;
    int index = 0, intValue = 0;
    double floatValue = 0.0;
    Str100 result = "";

    value.index = -999;
    value.str = "XXX";

    // Constant
    if ( IsConstant( token.type ) ) retValue = GetTokenValue( Constant() );
    
    // '(' expression ')'
    else if ( token.type == TYPE_LEFT_SMALL_BRACKET ) {
      mScanner.GetToken();                                        // '('
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );

      retValue = Expression();                                    // expression
      token = GetTokenWithType( true, TYPE_RIGHT_SMALL_BRACKET ); // ')'
      if ( mMode == MODE_LIST_FUNCTION )
        printf( "%s ", token.str.c_str() );
    } // else if

    // signed_unary_exp   : Identifier [ '(' [ actual_parameter_list ] ')' | array_expression ]
    // unsigned_unary_exp : Identifier [ '(' [ actual_parameter_list ] ')' |
    //                      [ array_expression ] [ ( PP | MM ) ] ]
    else {
      idToken = GetTokenWithType( true, TYPE_ID ); // Identifier
      if ( mScanner.PeekToken().type == TYPE_LEFT_SMALL_BRACKET ) {
        if ( !IsDefinedFunction( idToken.str ) )
          throw UndefinedTokenException( idToken );
          // ToDo: else --- value is not initialized
        else
          intValue = intValue;
      } // if
      else value = retValue = GetIDTokenValue( idToken, -1 );

      if ( mMode == MODE_LIST_FUNCTION ) {
        printf( "%s", token.str.c_str() );
        token = mScanner.PeekToken();
        if ( ( token.type != TYPE_COMMA ) && ( token.type != TYPE_LEFT_SMALL_BRACKET ) &&
             ( token.type != TYPE_LEFT_MID_BRACKET ) &&
             ( token.type != TYPE_PP ) && ( token.type != TYPE_MM ) )
          printf( " " );
      } // if

      // '(' [ actual_parameter_list ] ')'
      token = mScanner.PeekToken();
      if ( token.type == TYPE_LEFT_SMALL_BRACKET ) {
        mScanner.GetToken();
        if ( mMode == MODE_LIST_FUNCTION )
          printf( "%s ", token.str.c_str() );
        token = mScanner.PeekToken();
        if ( ( token.type == TYPE_LEFT_SMALL_BRACKET ) ||
             ( token.type == TYPE_ID ) ||
             ( token.type == TYPE_PP ) ||
             ( token.type == TYPE_MM ) ||
             Sign( token ) || IsConstant( token.type ) )
          arguList = Actual_parameter_list();

        token = GetTokenWithType( true, TYPE_RIGHT_SMALL_BRACKET );
        if ( mMode == MODE_LIST_FUNCTION )
          printf( "%s ", token.str.c_str() );
        else if ( mMode == MODE_EXECUTION )
          retValue = FunctionCall( idToken, arguList );
      } // if

      // array_expression
      else if ( isSigned ) {
        token = mScanner.PeekToken();
        if ( token.type == TYPE_LEFT_MID_BRACKET ) {
          index = Array_expression( retValue );

          if ( mMode == MODE_EXECUTION ) {
            retValue.value.erase( retValue.value.begin() + ( index + 1 ), retValue.value.end() );
            retValue.value.erase( retValue.value.begin(), retValue.value.begin() + index );
            retValue.isArray = false;
            retValue.index = index;
          } // if
        } // if
      } // else if

      // [ array_expression ] [ ( PP | MM ) ]
      else {
        if ( token.type == TYPE_LEFT_MID_BRACKET )
          index = Array_expression( retValue );

        token = GetTokenWithType( false, TYPE_PP, TYPE_MM );
        if ( !token.str.empty() ) {
          if ( mMode == MODE_LIST_FUNCTION )
            printf( "%s ", token.str.c_str() );
          else if ( mMode == MODE_EXECUTION ) value = PPMM( idToken, token.type, index );
        } // if

        if ( mMode == MODE_EXECUTION ) {
          retValue.value.erase( retValue.value.begin() + ( index + 1 ), retValue.value.end() );
          retValue.value.erase( retValue.value.begin(), retValue.value.begin() + index );
          retValue.isArray = false;
          retValue.index = index;
        } // if
      } // else
    } // else

    return retValue;
  } // Basic_unary_exp()

public:
  Parser() {
    Token_Value_s value;
    value.isArray  = false;
    value.type     = TYPE_ID;
    value.value.resize( 1 );
    InitialDataTypeToStringTable();
    mFunctionTable.clear();
    mJumpTable.clear();

    // Call stack
    mCallStack.clear();
    Function_Variable_s variables;
    variables.parameterTable.clear();
    variables.localVariableTable.clear();
    value.value[0] = "cin";
    variables.localVariableTable["cin"] = value;
    value.value[0] = "cout";
    variables.localVariableTable["cout"] = value;
    variables.returnAddress = mScanner.GetProgramCounter();
    mCallStack.push_back( variables );

    // System function
    Parameter_s parm;
    parm.str = "name";
    parm.isReference  = true;
    parm.info.isArray = true;
    parm.info.type    = TYPE_CONST_CHAR;
    parm.info.value.clear();
    parm.isRefStackLayer = -1;

    Function_Format_s format;
    format.returnType = TYPE_CONST_VOID;
    format.startAddress = EOF;
    format.parameters.push_back( parm );
    mFunctionTable.clear();
    mSystemFunctionList.clear();

    mFunctionTable["ListVariable"].push_back( format );
    mSystemFunctionList.push_back( "ListVariable" );
    mFunctionTable["ListFunction"].push_back( format );
    mSystemFunctionList.push_back( "ListFunction" );

    format.parameters.clear();
    mFunctionTable["ListAllFunctions"].push_back( format );
    mSystemFunctionList.push_back( "ListAllFunctions" );
    mFunctionTable["ListAllVariables"].push_back( format );
    mSystemFunctionList.push_back( "ListAllVariables" );
    mFunctionTable["Done"].push_back( format );
    mSystemFunctionList.push_back( "Done" );
  } // Parser()

  ~Parser() {
    mFunctionTable.clear();
    mJumpTable.clear();
    mCallStack.clear();
  } // ~Parser()

  void PrintStack() {
    Table t = LOCAL_VARIABLES();
    vector<string> value;
    map<string, Token_Value_s>::iterator it;

    for ( it = t.begin() ; it != t.end() ; it++ ) {
      value = it->second.value;
      printf( "%d %s: ", it->second.type, it->first.c_str() );
      for ( int i = 0, size = value.size() ; i < size ; i++ )
        printf( "[%s], ", value[i].c_str() );

      printf( "\n" );
    } // for
  } // PrintStack()

  void ParseToken() {
    Token_Value_s ret;
    int inputBeginPos;
    vector<Function_Variable_s> callStackBackUp;

    printf( "Our-C running ...\n" );
    mIsEOF = false;
    while ( !mIsEOF ) {
      try {
        printf( "> " );
        mScanner.RemoveOneLineWhiteSpace();
        mScanner.ResetLineNum();

        // Pass1
        mMode = MODE_SYNTAX_CHECK;
        mScanner.ClearInputBuf();
        inputBeginPos = mScanner.GetProgramCounter();
        callStackBackUp = mCallStack;
        mJumpTable.clear();
        Command();

        // Pass2
        mMode = MODE_EXECUTION;
        mScanner.JumpTo( inputBeginPos );
        mCallStack = callStackBackUp;
        ret = Command();
        printf( "%s\n", ret.value[0].c_str() );
      } // try
      catch ( TokenException &te ) {
        printf( "%s\n", te.Message().c_str() );
        if ( strstr( te.Message().c_str(), "undefined" ) != NULL )
          mCallStack = callStackBackUp;
        mCallStack = callStackBackUp;
        mScanner.RemoveOneLineInput( inputBeginPos );
      } // catch
      catch ( const char * e ) {
        if ( strcmp( e, EOF_EXCEPTION ) == 0 )
          mIsEOF = true;
        else {
          printf( "%s\n", e );
          mCallStack = callStackBackUp;
          mScanner.RemoveOneLineInput( inputBeginPos );
        } // else
      } // catch
      catch ( string se ) {
        if ( strcmp( se.c_str(), EOF_EXCEPTION ) == 0 )
          mIsEOF = true;
        else {
          printf( "%s\n", se.c_str() );
          mCallStack = callStackBackUp;
          mScanner.RemoveOneLineInput( inputBeginPos );
        } // else
      } // catch

      // Pop function variables(callStack[0] is Main())
      mCallStack.erase( mCallStack.begin() + 1, mCallStack.end() );
    } // while

    printf( "Our-C exited ...\n" );
  } // ParseToken()
}; // class Parser

int main() {
  Parser parser;
  char ch;

  if ( scanf( "%d", &uTestNum ) == EOF )
    return 0;

  uIfNum = 0;
  scanf( "%c", &ch );
  parser.ParseToken();

  return 0;
} // main()
