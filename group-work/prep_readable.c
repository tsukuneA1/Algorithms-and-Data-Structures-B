#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// ==========================================
// 定数定義
// ==========================================
#define STRING_LEN 15        // 文字列の長さ (A-Jが15文字)
#define GRAM_LEN 3           // n-gramのn (3文字)
#define GRAM_PATTERN_MAX 1000 // 3-gramの全パターン数 (000-999)
#define MAX_STRINGS 1000000  // 読み込む最大行数

// ==========================================
// ヘルパー関数
// ==========================================

// 文字 'A'-'J' を数値 0-9 に変換する
int convert_char_to_index(char c) {
    return c - 'A';
}

// 3つの数値からハッシュ値(0-999)を計算する
// 例: (2, 7, 7) -> 277 ("CHH")
int calculate_gram_hash(int n1, int n2, int n3) {
    return n1 * 100 + n2 * 10 + n3;
}

// ==========================================
// メイン処理
// ==========================================
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <db_file>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "Cannot open file: %s\n", argv[1]);
        return 1;
    }

    uint8_t (*all_strings)[STRING_LEN] = malloc(MAX_STRINGS * STRING_LEN);
    if (!all_strings) {
        fprintf(stderr, "Memory allocation failed (all_strings)\n");
        return 1;
    }

    size_t bitset_size_bytes = (MAX_STRINGS + 7) / 8;

    uint8_t **gram_bitsets = calloc(GRAM_PATTERN_MAX, sizeof(uint8_t*));
    if (!gram_bitsets) {
        fprintf(stderr, "Memory allocation failed (bitset pointers)\n");
        return 1;
    }

    for (int i = 0; i < GRAM_PATTERN_MAX; i++) {
        gram_bitsets[i] = calloc(bitset_size_bytes, 1);
        if (!gram_bitsets[i]) {
            fprintf(stderr, "Memory allocation failed (bitset body)\n");
            return 1;
        }
    }

    char line_buffer[256];
    uint32_t current_string_id = 0;

    while (fgets(line_buffer, sizeof(line_buffer), fp) && current_string_id < MAX_STRINGS) {
        // 改行文字を削除
        line_buffer[strcspn(line_buffer, "\n")] = 0;

        // 15文字未満の行はスキップ
        if (strlen(line_buffer) < STRING_LEN) continue;

        // 文字列を数値配列に変換して保存
        for (int i = 0; i < STRING_LEN; i++) {
            all_strings[current_string_id][i] = (uint8_t)convert_char_to_index(line_buffer[i]);
        }

        // 3-gramを抽出してビットセットに反映
        int num_grams = STRING_LEN - GRAM_LEN + 1; // 13個のgramができる

        for (int i = 0; i < num_grams; i++) {
            int gram_hash = calculate_gram_hash(
                all_strings[current_string_id][i],
                all_strings[current_string_id][i + 1],
                all_strings[current_string_id][i + 2]
            );

            int byte_pos = current_string_id / 8;
            int bit_pos = current_string_id % 8;
            
            gram_bitsets[gram_hash][byte_pos] |= (1 << bit_pos);
        }
        current_string_id++;
    }

    fclose(fp);

    // バイナリ出力
    fwrite(&current_string_id, sizeof(uint32_t), 1, stdout);
    fwrite(all_strings, STRING_LEN, current_string_id, stdout);

    size_t actual_written_size = (current_string_id + 7) / 8;
    for (int i = 0; i < GRAM_PATTERN_MAX; i++) {
        fwrite(gram_bitsets[i], 1, actual_written_size, stdout);
    }

    // メモリ解放
    free(all_strings);
    for (int i = 0; i < GRAM_PATTERN_MAX; i++) {
        free(gram_bitsets[i]);
    }
    free(gram_bitsets);

    return 0;
}