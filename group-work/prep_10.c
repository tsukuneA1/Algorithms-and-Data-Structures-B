
// フォルダのハイライト
// C言語コード群は、3-gram法とMyersのビットパラレルアルゴリズムを用いた高速な編集距離検索の実装と最適化（prep_10.c, search_10.cなど）に焦点を当てている。

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define STR_LEN 15
#define Q 3
#define NUM_GRAMS 1000
#define MAX_STRINGS 1000000

// A-J を 0-9 にマッピング
static inline int char_to_int(char c) {
    return c - 'A';
}

// 文字列を数値配列に変換 (A-J -> 0-9)
static inline void convert_string(const char *str, uint8_t *out) {
    for (int i = 0; i < STR_LEN; i++) {
        out[i] = char_to_int(str[i]);
    }
}

// 3-gramのハッシュ値を計算 (A-J を 0-9 として 10 進数化)
static inline int gram_hash(uint8_t a, uint8_t b, uint8_t c) {
    return a * 100 + b * 10 + c; // 0-999 の範囲に収まる
}

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

    // メモリ確保
    uint8_t (*string_data)[STR_LEN] = malloc(MAX_STRINGS * STR_LEN);
    if (!string_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // ビットセット用メモリ確保 (1000 grams)
    size_t bitset_size = (MAX_STRINGS + 7) / 8;
    uint8_t **bitsets = calloc(NUM_GRAMS, sizeof(uint8_t*));
    if (!bitsets) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    for (int i = 0; i < NUM_GRAMS; i++) {
        bitsets[i] = calloc(bitset_size, 1);
        if (!bitsets[i]) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }
    }

    // DBを読み込み、数値化 & q-gram構築
    char line[256];
    uint32_t n = 0;
    while (fgets(line, sizeof(line), fp) && n < MAX_STRINGS) {
        // 改行削除
        line[strcspn(line, "\n")] = 0;
        
        if (strlen(line) < STR_LEN) continue;
        
        // 文字列を数値化
        convert_string(line, string_data[n]);
        
        // q-gramを抽出してビットセットに追加
        for (int i = 0; i <= STR_LEN - Q; i++) {
            int gram = gram_hash(string_data[n][i], string_data[n][i+1], string_data[n][i+2]);
            // ビットを立てる
            bitsets[gram][n / 8] |= (1 << (n % 8));
        }
        
        n++;
    }
    fclose(fp);

    // バイナリ出力
    // ヘッダ: n (uint32)
    fwrite(&n, sizeof(uint32_t), 1, stdout);
    
    // String data
    fwrite(string_data, STR_LEN, n, stdout);
    
    // Bitsets
    size_t actual_bitset_size = (n + 7) / 8;
    for (int i = 0; i < NUM_GRAMS; i++) {
        fwrite(bitsets[i], 1, actual_bitset_size, stdout);
    }

    // クリーンアップ
    free(string_data);
    for (int i = 0; i < NUM_GRAMS; i++) {
        free(bitsets[i]);
    }
    free(bitsets);

    return 0;
}