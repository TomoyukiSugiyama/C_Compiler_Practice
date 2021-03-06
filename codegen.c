#include "9cc.h"
static void gen(Node *node);
static void gen_lval(Node *node);

static void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}
// プログラム内の関数毎に、コードを生成
void codegen(Program *program) {
  for (Function *func = program->func->next; func; func = func->next) {
    gen(func->node);
  }
}
// 関数内のノード毎に、コードを生成
static void gen(Node *node) {
  static int label_cnt = 0;
  int _label_cnt = 0;
  int arg_num = 0;
  char *arg_reg[10] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

  switch (node->kind) {
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  case ND_RETURN:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  case ND_IF:
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    if (node->els) {
      printf("  je .Lelse%03d\n", label_cnt);
      _label_cnt = label_cnt;
      label_cnt++;
      gen(node->stat);
      printf("  jmp .Lend%03d\n", _label_cnt);
      printf(".Lelse%03d:\n", _label_cnt);
      gen(node->els);
    } else {
      printf("  je .Lend%03d\n", label_cnt);
      _label_cnt = label_cnt;
      label_cnt++;
      gen(node->stat);
    }
    printf(".Lend%03d:\n", _label_cnt);
    return;
  case ND_WHILE:
    printf(".Lbegin%03d:\n", label_cnt);
    _label_cnt = label_cnt;
    label_cnt++;
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%03d\n", _label_cnt);
    gen(node->stat);
    printf("  jmp .Lbegin%03d\n", _label_cnt);
    printf(".Lend%03d:\n", _label_cnt);
    return;
  case ND_FOR:
    if (node->init) {
      gen(node->init);
    }
    printf(".Lbegin%03d:\n", label_cnt);
    _label_cnt = label_cnt;
    label_cnt++;
    if (node->cond) {
      gen(node->cond);
    }
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%03d\n", _label_cnt);
    gen(node->stat);
    if (node->loop) {
      gen(node->loop);
    }
    printf("  jmp .Lbegin%03d\n", _label_cnt);
    printf(".Lend%03d:\n", _label_cnt);
    return;
  case ND_BLOCK:
    for (Node *block = node->next; block; block = block->next) {
      gen(block);
      //　式の評価結果としてスタックに一つの値が残っている
      //　はずなので、スタックが溢れないようにポップしておく
      printf("  pop rax\n");
    }
    return;
  case ND_FUNCCALL:
    arg_num = 0;
    for (Node *arg = node->args; arg; arg = arg->next) {
      gen(arg);
      arg_num++;
    }
    if (arg_num > 6)
      error("6個以上の引数はサポートされていません\n");
    for (int i = 1; i <= arg_num; i++) {
      printf("  pop %s\n", arg_reg[arg_num - i]);
    }
    printf("  mov rax, rsp\n");
    printf("  and rax, 15\n");
    printf("  cmp rax, 0\n");
    printf("  je .Laligned%03d\n", label_cnt);
    _label_cnt = label_cnt;
    label_cnt++;
    printf("  sub rsp, 8\n");
    printf(".Laligned%03d:\n", _label_cnt);
    printf("  call %s\n", node->funcname);
    // 関数の返り値をスタックに積む。
    printf("  push rax\n");
    return;
  case ND_FUNC:
    for (Node *func = node; func; func = func->next) {
      printf("%s:\n", func->funcname);
      //　プロローグ
      printf("  push rbp\n");
      printf("  mov rbp, rsp\n");
      arg_num = 0;
      //　引数の領域を確保する。
      for (Node *arg = func->args; arg; arg = arg->next) {
        // printf("  sub rsp, %d\n", arg->offset);
        printf("  push %s\n", arg_reg[arg_num]);
        arg_num++;
      }
      //　ローカル変数８個分の領域を確保する。
      printf("  sub rsp, 64\n");

      // ブロックの中身を生成する。
      gen(func->block);

      //　エピローグ
      //　最後の式の結果がRAXに残っているのでそれが返り値になる
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
    }
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
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NEQ:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_GT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_GTE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rdi, rax\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LTE:
    printf("  cmp rdi, rax\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}