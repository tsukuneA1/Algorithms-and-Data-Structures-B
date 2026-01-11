#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 64

int main(int argc, char *argv[]) {
    // 引数チェック
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file1_vertical> <file2_vertical>\n", argv[0]);
        fprintf(stderr, "Example: %s answer.txt result_vertical.txt\n", argv[0]);
        return 1;
    }

    // ファイルオープン
    FILE *fp1 = fopen(argv[1], "r");
    FILE *fp2 = fopen(argv[2], "r");

    if (!fp1 || !fp2) {
        fprintf(stderr, "Error: Cannot open file(s).\n");
        return 1;
    }

    char line1[MAX_LINE];
    char line2[MAX_LINE];
    int total_lines = 0;
    int mismatch_count = 0; // ハミング距離 (評価指標)

    printf("--- Comparing %s and %s ---\n", argv[1], argv[2]);

    // 両方のファイルを1行ずつ読み込む
    while (1) {
        char *p1 = fgets(line1, sizeof(line1), fp1);
        char *p2 = fgets(line2, sizeof(line2), fp2);

        // 両方とも終了したらループを抜ける
        if (p1 == NULL && p2 == NULL) {
            break;
        }

        // 片方だけ先に終わってしまった場合 (行数不一致)
        if (p1 == NULL || p2 == NULL) {
            fprintf(stderr, "\nWarning: File lengths differ at line %d!\n", total_lines + 1);
            mismatch_count++; // 長さが違う部分もエラーとしてカウントする場合
            total_lines++;
            continue;
        }

        // 改行文字を除去 (念のため)
        line1[strcspn(line1, "\n")] = 0;
        line2[strcspn(line2, "\n")] = 0;

        // 空行はスキップ
        if (strlen(line1) == 0 || strlen(line2) == 0) continue;

        // 比較 (0か1か)
        if (line1[0] != line2[0]) {
            mismatch_count++;
            // デバッグ用: 食い違っている行を表示したい場合はコメントアウトを外す
            // printf("Diff at line %d: File1='%c' vs File2='%c'\n", total_lines + 1, line1[0], line2[0]);
        }

        total_lines++;
    }

    fclose(fp1);
    fclose(fp2);

    // 結果出力
    if (total_lines == 0) {
        printf("No data found.\n");
        return 0;
    }

    double accuracy = (double)(total_lines - mismatch_count) / total_lines * 100.0;

    printf("\nTotal Queries   : %d\n", total_lines);
    printf("Hamming Distance: %d (Lower is Better)\n", mismatch_count);
    printf("Accuracy Rate   : %.2f%%\n", accuracy);

    // 課題の評価指標に基づき、ハミング距離を返す
    return 0;
}