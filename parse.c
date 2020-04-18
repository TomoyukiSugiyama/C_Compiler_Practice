#include "9cc.h"

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

//　次のトークンが期待している記号のときには、トークンを１つ読み進める。
//　それ以外の場合にはエラーを報告する。
static void expect(char *op) {
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
            error_at(token->str,"'%c'ではありません",op);
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

//　新しいトークンを作成してcurを繋げる
static Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
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

        if (!strncmp(p, "==", 2) ||
            !strncmp(p, "!=", 2)){
            cur = new_token(TK_RESERVED, cur, p, 2);
            p+=2;
            continue;
        } 

        if (!strncmp(p, "<=", 2) ||
            !strncmp(p, ">=", 2)){ 
            cur = new_token(TK_RESERVED, cur, p, 2);
            p+=2;
            continue;
        }

        if (!strncmp(p, "<", 1) ||
            !strncmp(p, ">", 1)){
            cur = new_token(TK_RESERVED, cur, p++, 1);
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

// expr = equality
Node *expr(){
    return equality();
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality(){
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

// relational = add ("<" add | "<=" add | ">" add | "=>" add)*
static Node *relational(){
    Node *node = add();

    for (;;) {
        if (consume("<"))
            node = new_node(ND_GLT, node, add());
        else if (consume("<="))
            node = new_node(ND_GTE, node, add());
        else if (consume(">"))
            node = new_node(ND_LET, node, add());
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


// primary = num | "(" expr ")"
static Node *primary(){
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }
    return new_node_num(expect_number());
}