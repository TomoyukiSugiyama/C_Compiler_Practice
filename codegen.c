#include "9cc.h"

static void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(Node *node) {
  static int label_cnt = 0;

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
      gen(node->stat);
      printf("  jmp .Lend%03d\n", label_cnt);
      printf(".Lelse%03d:\n", label_cnt);
      gen(node->els);
    } else {
      printf("  je .Lend%03d\n", label_cnt);
      gen(node->stat);
    }
    printf(".Lend%03d:\n", label_cnt);
    label_cnt++;
    return;
  case ND_WHILE:
    printf(".Lbegin%03d:\n", label_cnt);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%03d\n", label_cnt);
    gen(node->stat);
    printf("  jmp .Lbegin%03d\n", label_cnt);
    printf(".Lend%03d:\n", label_cnt);
    label_cnt++;
    return;
  case ND_FOR:
    if (node->init) {
      gen(node->init);
    }
    printf(".Lbegin%03d:\n", label_cnt);
    if (node->cond) {
      gen(node->cond);
    }
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%03d\n", label_cnt);
    gen(node->stat);
    if (node->loop) {
      gen(node->loop);
    }
    printf("  jmp .Lbegin%03d\n", label_cnt);
    printf(".Lend%03d:\n", label_cnt);
    return;
  case ND_BLOCK:
    for (int i = 0; block[i]; i++) {
      gen(block[i]);

      //　式の評価結果としてスタックに一つの値が残っている
      //　はずなので、スタックが溢れないようにポップしておく
      printf("  pop rax\n");
    }
    return;
  case ND_FUNC:
    printf("  call %s\n", node->funcname);
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