#define _GNU_SOURCE
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//　トークンの種類
typedef enum {
  TK_RESERVED, //　文字
  TK_IDENT,    //　識別子
  TK_NUM,      //　整数トークン
  TK_EOF,      //　入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

//　トークン型
struct Token {
  TokenKind kind; //　トークンの型
  Token *next;    //　次の入力トークン
  int val;        //　kindがTK_NUMの場合、その数値
  char *str;      //　トークン文字列
  int len;        //　トークンの長さ
};

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD,      // +
  ND_SUB,      // -
  ND_MUL,      // *
  ND_DIV,      // /
  ND_EQ,       // ==
  ND_NEQ,      // !=
  ND_GT,       // <
  ND_GTE,      // <=
  ND_LT,       // >
  ND_LTE,      // >=
  ND_ASSIGN,   // =
  ND_LVAR,     // ローカル変数
  ND_RETURN,   // return
  ND_IF,       // if
  ND_WHILE,    // while
  ND_FOR,      // for
  ND_BLOCK,    // {}
  ND_FUNC,     // 関数
  ND_FUNCCALL, //関数呼び出し
  ND_ARGS,     // 引数
  ND_NUM,      // 整数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  Node *next;    // 次のノード
  // if-else/for/while statement
  Node *init; // init-expression
  Node *cond; // cond-expression
  Node *loop; // loop-expression
  Node *els;  // else(option)

  Node *stat;     // statement
  char *funcname; // funcname
  Node *args;     // 引数
  Node *block;    // { }
  int val;        // kindがND_NUMの場合のみ使う
  int offset;     // kindがND_LVARの場合のみ使う
};

typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
  LVar *next; // 次の変数かNULL
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
};

typedef struct Function Function;
// 関数
struct Function {
  Function *next; //　次の関数かNULL
  char *name;     // 関数名
  Node *node;     // 関数内のノード
  LVar *locals;   // 関数内のローカル変数
};

typedef struct Program Program;
// プログラム
struct Program {
  Function *func;
};

// 入力プログラム
extern char *user_input;

//　現在着目しているトークン
extern Token *token;
// プログラムを保存
extern Program *prog;

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

void program();

Token *tokenize();
// void gen(Node *node);
void codegen(Program *program);
//#define DEBUG 11111
#if DEBUG
#define DEBUG_PRINT()                                                          \
  do {                                                                         \
    printf("error at %s:%d\n", __FILE__, __LINE__);                            \
  } while (0);
#else
#define DEBUG_PRINT() ;
#endif