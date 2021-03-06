#include "9cc.h"

static Function *function();
static Node *function_args();
static Node *create_lvar();
static Node *stmt();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();
static Node *args();

// プログラムを保存
Program *prog;
// ローカル変数を保存
static LVar *locals;

//　次のトークンが期待している記号のときには、トークンを１つ読み進めて
//　真を返す。それ以外の場合には偽を返す。
static bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

//　次のトークンが期待している識別子のときには、トークンを１つ読み進めて
//　真を返す。それ以外の場合には偽を返す。
static Token *consume_ident() {
  if (token->kind != TK_IDENT)
    return NULL;
  Token *tmp = token;
  token = token->next;
  return tmp;
}

//　次のトークンが期待している記号のときには、トークンを１つ読み進める。
//　それ以外の場合にはエラーを報告する。
static void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "'%s'ではありません", op);
  token = token->next;
}

//　次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す。
//　それ以外の場合にはエラーを報告する。
static int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

// 最後のトークンに到達したかどうかを判定する。
static bool at_eof() { return token->kind == TK_EOF; }

// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
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

// program = function*
void program() {
  int i = 0;
  prog = calloc(1, sizeof(Program));

  prog->func = calloc(1, sizeof(Function));
  Function *ftmp = prog->func;
  while (!at_eof()) {
    ftmp->next = function();
    ftmp = ftmp->next;
  }
}
// function = ident "(" function_args ("{" stmt* "}")?
static Function *function() {
  Function *func = calloc(1, sizeof(Function));
  locals = func->locals;
  DEBUG_PRINT();
  Token *tok = consume_ident();
  if (tok) {
    func->node = calloc(1, sizeof(Node));
    DEBUG_PRINT();

    if (consume("(")) {
      func->node->kind = ND_FUNC;
      func->node->funcname = strndup(tok->str, tok->len);
      func->node->args = function_args();
    }
    DEBUG_PRINT();
    if (consume("{")) {
      Node *block = calloc(1, sizeof(Node));
      block->kind = ND_BLOCK;
      Node *tmp = block;
      DEBUG_PRINT();
      while (!consume("}")) {
        tmp->next = stmt();
        tmp = tmp->next;
      }
      func->node->block = block;
    }
    DEBUG_PRINT();
    return func;
  } else {
    error("関数が宣言されていません。");
  }
}
// function_args = (create_lvar ("," create_lvar)* )? ")"
static Node *function_args() {
  LVar *lvar;
  Node *node;
  if (consume(")"))
    return NULL;
  DEBUG_PRINT();
  node = create_lvar();
  DEBUG_PRINT();
  Node *tmp = node;
  while (consume(",")) {
    tmp->next = create_lvar();
    tmp = tmp->next;
  }
  expect(")");
  DEBUG_PRINT();

  return node;
}
// create_lvar = ident
static Node *create_lvar() {
  Node *node = calloc(1, sizeof(Node));
  LVar *lvar;
  Token *tok = consume_ident();
  if (tok) {
    lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    if (locals == NULL) {
      lvar->offset = 8;
    } else {
      lvar->offset = locals->offset + 8;
    }
    node->offset = lvar->offset;
    locals = lvar;
    return node;
  } else {
    error("引数が正しくありません。");
  }
}
// stmt = expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "return" expr ";"
//      | "{" stmt* "}"
static Node *stmt() {
  Node *node;

  if (consume("return")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(";");
  } else if (consume("if")) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    node->cond = expr();
    expect(")");
    node->stat = stmt();
    if (consume("else")) {
      node->els = stmt();
    }
  } else if (consume("while")) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    node->cond = expr();
    expect(")");
    node->stat = stmt();
  } else if (consume("for")) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    if (!consume(";")) {
      node->init = expr();
      expect(";");
    }
    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->loop = expr();
      expect(")");
    }
    node->stat = stmt();
  } else if (consume("{")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    Node *tmp = node;
    while (!consume("}")) {
      tmp->next = stmt();
      tmp = tmp->next;
    }
  } else {
    node = expr();
    expect(";");
  }
  return node;
}

// expr = assign
static Node *expr() { return assign(); }

// assign = equality ("=" assign)?
static Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality() {
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
static Node *relational() {
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
static Node *add() {
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
static Node *mul() {
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
static Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}

// primary = num | ident ("(" args)? | "(" expr ")"
static Node *primary() {

  Token *tok = consume_ident();
  if (tok) {
    Node *node = calloc(1, sizeof(Node));
    if (consume("(")) {
      node->kind = ND_FUNCCALL;
      node->funcname = strndup(tok->str, tok->len);
      node->args = args();
    } else {
      node->kind = ND_LVAR;
      LVar *lvar = find_lvar(tok);
      if (lvar) {
        // 定義済みのローカル変数を使用
        node->offset = lvar->offset;
      } else {
        // ローカル変数を新規に追加
        lvar = calloc(1, sizeof(LVar));
        lvar->next = locals;
        lvar->name = tok->str;
        lvar->len = tok->len;
        if (locals == NULL) {
          lvar->offset = 8;
        } else {
          lvar->offset = locals->offset + 8;
        }
        node->offset = lvar->offset;
        locals = lvar;
      }
    }
    return node;
  }

  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
  return new_node_num(expect_number());
}

// args = (assign ("," assign)* )? ")"
static Node *args() {
  if (consume(")"))
    return NULL;
  Node *node = assign();
  Node *tmp = node;
  while (consume(",")) {
    tmp->next = assign();
    tmp = tmp->next;
  }
  expect(")");
  return node;
}