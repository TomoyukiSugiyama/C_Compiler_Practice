#include "9cc.h"

static Node *stmt();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();

//　次のトークンが期待している記号のときには、トークンを１つ読み進めて
//　真を返す。それ以外の場合には偽を返す。
static bool consume(char *op) {
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

//　次のトークンが期待している識別子のときには、トークンを１つ読み進めて
//　真を返す。それ以外の場合には偽を返す。
static Token *consume_ident() {
    if (token->kind != TK_IDENT )
        return NULL;
    Token *tmp = token;
    token = token->next;
    return tmp;
}

//　次のトークンが期待している記号のときには、トークンを１つ読み進める。
//　それ以外の場合にはエラーを報告する。
static void expect(char *op) {
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
            error_at(token->str,"'%s'ではありません",op);
    token = token->next;
}

//　次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す。
//　それ以外の場合にはエラーを報告する。
static int expect_number() {
    if (token->kind != TK_NUM)
        error_at(token->str,"数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

static bool at_eof() {
    return token->kind == TK_EOF;
}

//　新しいノードを作成してlhs,rhsを繋げる
static Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

//　新しいノードを作成してvalを繋げる
static Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// program = stmt*
void program(){
    int i = 0;
    while (!at_eof())
        code[i++] = stmt();
    code[i] = NULL;
}

// stmt = expr ";"
static Node *stmt(){
    Node *node = expr();
    expect(";");
    return node;
}

// expr = assign
static Node *expr(){
    return assign();
}

// assign = equality ("=" assign)?
static Node *assign(){
    Node *node = equality();
    if (consume("="))
        node = new_node(ND_ASSIGN, node, assign());
    return node;
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality(){
    Node *node = relational();

    for (;;) {
        if (consume("=="))
            node = new_node(ND_EQ, node, mul());
        else if (consume("!="))
            node = new_node(ND_NEQ, node, mul());
        else
            return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | "=>" add)*
static Node *relational(){
    Node *node = add();

    for (;;) {
        if (consume("<"))
            node = new_node(ND_GT, node, add());
        else if (consume("<="))
            node = new_node(ND_GTE, node, add());
        else if (consume(">"))
            node = new_node(ND_LT, node, add());
        else if (consume(">="))
            node = new_node(ND_LTE, node, add());
        else
            return node;
    }
}


// add = mul ("+" mul | "-" mul)*
static Node *add(){
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
static Node *mul(){
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
static Node *unary(){
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();        
}


// primary = num | ident | "(" expr ")"
static Node *primary(){

    Token *tok = consume_ident();
    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        node->offset = (tok->str[0] - 'a' + 1) * 8;
        return node;
    }

    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }
    return new_node_num(expect_number());
}