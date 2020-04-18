#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



//　トークンの種類
typedef enum {
    TK_RESERVED,    //　文字
    TK_NUM,         //　整数トークン
    TK_EOF,         //　入力の終わりを表すトークン
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
    ND_ADD,         // +
    ND_SUB,         // -
    ND_MUL,         // *
    ND_DIV,         // /
    ND_EQL,         // ==
    ND_NEQ,         // !=
    ND_GLT,         // <
    ND_GTE,         // <=
    ND_LET,         // >
    ND_LTE,         // >=
    ND_NUM,         // 整数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
    NodeKind kind;  // ノードの型
    Node *lhs;      // 左辺
    Node *rhs;      // 右辺
    int val;        // kindがND_NUMの場合のみ使う
};

// 入力プログラム
char *user_input;

//　現在着目しているトークン
Token *token;

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *primary();
Node *unary();
void gen(Node *node);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);


//　エラーを報告するための関数
//　printfと同じ引数を取る
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

//　次のトークンが期待している記号のときには、トークンを１つ読み進めて
//　真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

//　次のトークンが期待している記号のときには、トークンを１つ読み進める。
//　それ以外の場合にはエラーを報告する。
void expect(char *op) {
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
            error_at(token->str,"'%c'ではありません",op);
    token = token->next;
}

//　次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す。
//　それ以外の場合にはエラーを報告する。
int expect_number() {
    if (token->kind != TK_NUM)
        error_at(token->str,"数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

//　新しいトークンを作成してcurを繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

//　新しいノードを作成してlhs,rhsを繋げる
Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

//　新しいノードを作成してvalを繋げる
Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// expr = equality
Node *expr(){
    return equality();
}

// expr = relational ("==" relational | "!=" relational)*
Node *equality(){
    Node *node = relational();

    for (;;) {
        if (consume("=="))
            node = new_node(ND_EQL, node, mul());
        else if (consume("!="))
            node = new_node(ND_NEQ, node, mul());
        else
            return node;
    }
}

// expr = add ("<" add | "<=" add | ">" add | "=>" add)*
Node *relational(){
    Node *node = add();

    for (;;) {
        if (consume("<"))
            node = new_node(ND_GLT, node, add());
        else if (consume("<="))
            node = new_node(ND_GTE, node, add());
        else if (consume(">"))
            node = new_node(ND_LET, node, add());
        else if (consume("=>"))
            node = new_node(ND_LTE, node, add());
        else
            return node;
    }
}


// expr = mul ("+" mul | "-" mul)*
Node *add(){
    Node *node = mul();

    for (;;) {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul(){
    Node *node = unary();

    for (;;) {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

// unary = ("+" | "-")? primary
Node *unary(){
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();        
}


// primary = num | "(" expr ")"
Node *primary(){
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }
    
    return new_node_num(expect_number());
}

//　入力文字列pをトークナイズしてそれを返す
Token *tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        //　空白文字をスキップ
        if (isspace(*p)){
            p++;
            continue;
        }
        if (*p == '+' || *p == '-' || 
            *p == '*' || *p == '/' ||
            *p == '(' || *p == ')' ){
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p,"トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

void gen(Node *node) {
    if (node->kind == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
    case ND_ADD:
        printf("  add rax, rdi\n");
        break;
    case ND_SUB:
        printf("  sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("  imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    }

    printf("  push rax\n");
}

int main(int argc, char **argv){
    if (argc != 2) {
        error("%s:引数の個数が正しくありません\n",argv[0]);
        return 1;
    }

    user_input = argv[1];
    // トークナイズする
    token = tokenize();
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    gen(node);

    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}
