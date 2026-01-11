//43.98s 78.7%
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define STR_LEN 15
#define Q 3
#define NUM_GRAMS 1000
#define THRESHOLD 6
#define MAX_STRINGS 1000000

// A-J を 0-9 にマッピング
static inline int char_to_int(char c) {
    return c - 'A';
}

// 3-gramのハッシュ値を計算
static inline int gram_hash(int a, int b, int c) {
    return a * 100 + b * 10 + c;
}

// Myersのビットパラレル編集距離アルゴリズム
//CPUのビット演算を並列処理の道具として使う
//peq[c]:クエリの文字列中で文字cが出現するビットが1になっている配列
//26個の要素 一つの要素は15ビット
//pv (Positive Vertical): 縦方向の差分が +1 であることを示すビット列。
//mv (Minus Vertical): 縦方向の差分が -1 であることを示すビット列 一致すると発生
//ph (Positive Horizontal): 横方向の差分が +1 であることを示すビット列。
//mh (Minus Horizontal): 横方向の差分が -1 であることを示すビット列。
//eq (Equality): 現在処理している文字が、クエリのどの位置と一致するかを示すマスク
static inline int myers_distance(const uint8_t *str, const uint8_t *query, const uint64_t *peq) {
    //strはdbの文字列 queryはクエリの文字列
    
    //完全一致チェック
    //__builtin_expect で「ほとんど起こらない」ことをコンパイラに伝える
    if (__builtin_expect(memcmp(str, query, STR_LEN) == 0, 0)) return 0;
    
    uint64_t mv = 0; //全ビット0で初期化
    uint64_t pv = ~0ULL; //全ビット1で初期化 0のunsigned long long型に~をかけて全ビット1にする
    // 縦に見ると1,2,3,...と増えているから初期値が全ビット1
    int score = STR_LEN; //編集距離の初期値 15 虚無とqueryを比較
    
    for (int j = 0; j < STR_LEN; j++) {
        uint64_t eq = peq[str[j]]; //strのj番目の文字がqueryのどこにあるか
        uint64_t xv = eq | mv; //上との差がマイナスになりそうな場所
        uint64_t xh = (((eq & pv) + pv) ^ pv) | eq; //左との差が増えなさそうな場所
        //ここがポイント 
        //eqで一致する場所があると下の文字にも影響を与える 全部1ずつ減る
        //それを+pvで表現している
        //そして前後で変化した部分をxorで取り出す
        
        uint64_t ph = mv | ~(xh | pv); //右に行くと1増える場所
        // ~(xh | pv)= (~xh) & (~pv) は 左か上で増えない場所
        uint64_t mh = pv & xh; //右に行くと1減る場所
        //上から見ると1増える場所と
        score += ((ph >> (STR_LEN - 1)) & 1) - ((mh >> (STR_LEN - 1)) & 1);
        
        // 早期終了強化
        if (__builtin_expect(score > 3 + (STR_LEN - 1 - j), 0)) return 4;
        
        ph <<= 1;
        mh <<= 1;
        
        pv = mh | ~(xv | ph);
        mv = ph & xv;
    }
    
    return score;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <query_file> <index_file>\n", argv[0]);
        return 1;
    }

    // インデックスファイルをmmap
    int fd = open(argv[2], O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Cannot open index file\n");
        return 1;
    }

    struct stat sb;
    if (fstat(fd, &sb) < 0) {
        fprintf(stderr, "Cannot stat index file\n");
        return 1;
    }

    void *mapped = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        fprintf(stderr, "Cannot mmap index file\n");
        return 1;
    }
    close(fd);

    // プリフェッチ
    madvise(mapped, sb.st_size, MADV_WILLNEED);

    // ヘッダ読み込み
    uint8_t *ptr = (uint8_t*)mapped;
    uint32_t n = *(uint32_t*)ptr;
    ptr += sizeof(uint32_t);

    // String dataへのポインタ
    uint8_t (*string_data)[STR_LEN] = (uint8_t (*)[STR_LEN])ptr;
    ptr += STR_LEN * n;

    // Bitsetsへのポインタ
    size_t bitset_size = (n + 7) / 8;
    uint8_t **bitsets = malloc(NUM_GRAMS * sizeof(uint8_t*));
    for (int i = 0; i < NUM_GRAMS; i++) {
        bitsets[i] = ptr;
        ptr += bitset_size;
    }

    // クエリファイルをmmap
    int qfd = open(argv[1], O_RDONLY);
    if (qfd < 0) {
        fprintf(stderr, "Cannot open query file\n");
        return 1;
    }
    struct stat qsb;
    fstat(qfd, &qsb);
    char *query_mapped = mmap(NULL, qsb.st_size, PROT_READ, MAP_PRIVATE, qfd, 0);
    close(qfd);
    madvise(query_mapped, qsb.st_size, MADV_SEQUENTIAL);

    char output_buffer[200000];
    int output_pos = 0;
    
    // 64bitブロック数
    uint32_t n64 = (n + 63) / 64;

    char *qptr = query_mapped;
    char *qend = query_mapped + qsb.st_size;

    int query_count = 0;

    while (qptr < qend) {
        // 行の終端を探す
        char *line_end = qptr;
        while (line_end < qend && *line_end != '\n') line_end++;

        if(query_count % 5000 == 0) {
            fprintf(stderr, "%d\r", query_count);
        }
        query_count++;
        
        int line_len = line_end - qptr;
        if (line_len < STR_LEN) {
            output_buffer[output_pos++] = '0';
            if (output_pos >= 180000) {
                fwrite(output_buffer, 1, output_pos, stdout);
                output_pos = 0;
            }
            qptr = line_end + 1;
            continue;
        }
        
        char *line = qptr;
        qptr = line_end + 1;

        // クエリの文字を数値化
        uint8_t query[STR_LEN];
        for (int i = 0; i < STR_LEN; i++) {
            query[i] = char_to_int(line[i]);
        }

        // peq計算
        uint64_t peq[10] = {0};
        for (int i = 0; i < STR_LEN; i++) {
            peq[query[i]] |= (1ULL << i);
        }

        // q-gramを抽出
        int num_grams = STR_LEN - Q + 1;
        uint64_t *gram_bitsets[13];
        for (int i = 0; i < num_grams; i++) {
            int gram = gram_hash(query[i], query[i+1], query[i+2]);
            gram_bitsets[i] = (uint64_t*)bitsets[gram];
        }

        int found = 0;

        // ビットスライス法による高速フィルタリング
        // 64文書まとめてカウントし、閾値(4)以上の候補を抽出
        for (uint32_t k = 0; k < n64; k++) {
            // 早期スキップ: 始点と中点のAND
            uint64_t quick = gram_bitsets[0][k] & gram_bitsets[6][k];
            if (__builtin_expect(quick == 0, 1)) continue;

            uint64_t c0 = 0, c1 = 0, c2 = 0, c3 = 0;
            uint64_t v, t;

            // 13個のgramについてビット並列加算
            // ループ展開で高速化
            #define ADD_GRAM(idx) \
                v = gram_bitsets[idx][k]; \
                t = c0 & v; c0 ^= v; v = t; \
                t = c1 & v; c1 ^= v; v = t; \
                t = c2 & v; c2 ^= v; v = t; \
                c3 ^= v;

            ADD_GRAM(0); ADD_GRAM(1); ADD_GRAM(2); ADD_GRAM(3);
            ADD_GRAM(4); ADD_GRAM(5); ADD_GRAM(6); ADD_GRAM(7);
            ADD_GRAM(8); ADD_GRAM(9); ADD_GRAM(10); ADD_GRAM(11);
            ADD_GRAM(12);

            // 閾値4以上判定: count >= 4（精度優先）
            uint64_t mask = c3 | (c2 & c0) | (c2 & c1);

            if (__builtin_expect(mask == 0, 1)) continue;

            while (mask) {
                int b = __builtin_ctzll(mask);
                uint32_t j = k * 64 + b;
                
                if (j < n) {
                    // 即時検証
                    if (myers_distance(string_data[j], query, peq) <= 3) {
                        found = 1;
                        goto FOUND;
                    }
                }
                mask &= mask - 1;
            }
        }

        FOUND:
        output_buffer[output_pos++] = found ? '1' : '0';
        
        if (output_pos >= 180000) {
            fwrite(output_buffer, 1, output_pos, stdout);
            output_pos = 0;
        }
    }
    
    if (output_pos > 0) {
        fwrite(output_buffer, 1, output_pos, stdout);
    }
    printf("\n");

    munmap(mapped, sb.st_size);
    munmap(query_mapped, qsb.st_size);
    free(bitsets);

    return 0;
}