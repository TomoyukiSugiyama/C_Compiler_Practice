#include "9cc.h"

int main(int argc, char **argv){
    if (argc != 2) {
        error("%s:引数の個数が正しくありません\n",argv[0]);
        return 1;
    }

    user_input = argv[1];
    // トークナイズする
    token = tokenize();
    program();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    //　プロローグ
    //　26個分の領域を確保する。
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    for (int i = 0; code[i]; i++) {
        gen(code[i]);

        //　式の評価結果としてスタックに一つの値が残っている
        //　はずなので、スタックが溢れないようにポップしておく
        printf("  pop rax\n");
    }

    //　エピローグ
    //　最後の式の結果がRAXに残っているのでそれが返り値になる
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;
}
