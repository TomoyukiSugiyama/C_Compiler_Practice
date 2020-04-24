#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    error("%s:引数の個数が正しくありません\n", argv[0]);
    return 1;
  }

  DEBUG_PRINT();
  user_input = argv[1];
  // トークナイズする
  token = tokenize();
  DEBUG_PRINT();
  // 構文木を生成する
  program();
  DEBUG_PRINT();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");

  // アセンブリコードを生成する
  codegen(prog);

  return 0;
}
