#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

// 定数定義
#define STR_LEN 15          // 文字列の長さ
#define GRAM_LEN 3          // Q-gramの長さ (3文字)
#define NUM_GRAMS 1000      // 3-gramの全パターン数 (10x10x10)
#define FILTER_THRESHOLD 4  // フィルタリングを通過する閾値 (共通gram数)
#define DISTANCE_THRESHOLD 3 // 許容する編集距離

// データベースの情報をまとめる構造体
typedef struct {
    uint32_t total_count;           // 全文字列数
    uint8_t (*strings)[STR_LEN];    // 文字列データへのポインタ
    uint8_t **gram_bitsets;         // 各gramを持つ文字列IDのビットマップ配列
} Database;

// ---------------------------------------------------------
// ヘルパー関数: 文字変換など
// ---------------------------------------------------------

// 文字 A-J を 0-9 に変換
int char_to_int(char c) {
    return c - 'A';
}

// 3文字からハッシュ値(0-999)を計算
int calculate_gram_hash(int c1, int c2, int c3) {
    return c1 * 100 + c2 * 10 + c3;
}

// 指定された文字列IDが、あるgramを持っているかビットマップを確認する
// (元のコードではビット演算で64個まとめてやっていましたが、ここでは1つずつ確認します)
bool has_gram(const Database *db, int gram_hash, int string_id) {
    uint8_t *bitmap = db->gram_bitsets[gram_hash];
    int byte_index = string_id / 8;
    int bit_index = string_id % 8;
    
    // 指定ビットが立っているかチェック
    return (bitmap[byte_index] >> bit_index) & 1;
}

// ---------------------------------------------------------
// アルゴリズム部分: Myersのビットパラレル法
// ---------------------------------------------------------

// 編集距離が3以内かどうかを判定する関数
// ロジックは複雑ですが、入力(DBの文字列)と出力(距離)の関係は明確です
int calculate_edit_distance(const uint8_t *db_str, const uint8_t *query_str) {
    // 完全に一致する場合は距離0
    if (memcmp(db_str, query_str, STR_LEN) == 0) return 0;

    // Myersアルゴリズム用の変数（論文準拠の名前ですが、意味をコメント記述）
    // Pattern Match Vector: クエリ内の各文字の出現位置ビットマップ
    uint64_t pattern_match[10] = {0};
    for (int i = 0; i < STR_LEN; i++) {
        pattern_match[query_str[i]] |= (1ULL << i);
    }

    uint64_t vp = ~0ULL; // Vertical Positive (縦方向の差分 +1)
    uint64_t vn = 0;     // Vertical Negative (縦方向の差分 -1)
    int score = STR_LEN; // 現在の編集距離（最大値からスタート）

    for (int j = 0; j < STR_LEN; j++) {
        uint64_t eq = pattern_match[db_str[j]]; // DB文字とクエリの一致箇所
        
        // 以下、ビット演算によるDPテーブルの更新（ここだけは数式のまま残します）
        uint64_t x = eq | vn;
        uint64_t d0 = ((eq & vp) + vp) ^ vp;
        uint64_t hp = vn | ~(d0 | vp);
        uint64_t hn = vp & d0;

        // スコアの更新: 右に進むときのスコア変動を計算
        if ((hp >> (STR_LEN - 1)) & 1) score++;
        if ((hn >> (STR_LEN - 1)) & 1) score--;

        // シフトして次の列へ
        hp <<= 1;
        hn <<= 1;
        vp = hn | ~(x | hp);
        vn = hp & x;
    }

    return score;
}

// ---------------------------------------------------------
// メイン処理フロー
// ---------------------------------------------------------

// インデックスファイルを読み込んで構造体にセットする
Database load_database(const char *filename) {
    Database db;
    int fd = open(filename, O_RDONLY);
    if (fd < 0) { perror("File open error"); exit(1); }

    struct stat sb;
    fstat(fd, &sb);
    
    // メモリマップ (ファイルをメモリ配列として扱う)
    uint8_t *map_ptr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    // バイナリデータのパース（読み取り位置をずらしながらポインタをセット）
    // 1. 文字列の総数を読む
    db.total_count = *(uint32_t*)map_ptr;
    map_ptr += sizeof(uint32_t);

    // 2. 文字列データ本体へのポインタ
    db.strings = (uint8_t (*)[STR_LEN])map_ptr;
    map_ptr += STR_LEN * db.total_count;

    // 3. ビットマップへのポインタ配列を確保してセット
    size_t bitset_size_bytes = (db.total_count + 7) / 8;
    db.gram_bitsets = malloc(NUM_GRAMS * sizeof(uint8_t*));
    
    for (int i = 0; i < NUM_GRAMS; i++) {
        db.gram_bitsets[i] = map_ptr;
        map_ptr += bitset_size_bytes;
    }

    return db;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <query_file> <index_file>\n", argv[0]);
        return 1;
    }

    // 1. データベース（インデックス）の読み込み
    Database db = load_database(argv[2]);

    // 2. クエリファイルのオープン
    FILE *query_fp = fopen(argv[1], "r");
    if (!query_fp) return 1;

    char line_buffer[256];
    
    // 3. クエリを1行ずつ処理するループ
    while (fgets(line_buffer, sizeof(line_buffer), query_fp)) {
        // 改行削除
        line_buffer[strcspn(line_buffer, "\n")] = 0;
        if (strlen(line_buffer) < STR_LEN) {
            printf("0"); // エラー処理簡略化
            continue;
        }

        // クエリ文字列を数値配列に変換
        uint8_t query_ints[STR_LEN];
        for (int i = 0; i < STR_LEN; i++) {
            query_ints[i] = char_to_int(line_buffer[i]);
        }

        // クエリに含まれる 3-gram のハッシュ値を計算しておく
        int query_grams[13]; // 15文字なら13個のgramができる
        for (int i = 0; i < STR_LEN - GRAM_LEN + 1; i++) {
            query_grams[i] = calculate_gram_hash(query_ints[i], query_ints[i+1], query_ints[i+2]);
        }

        bool found_match = false;

        // =========================================================
        // 検索ロジック (元のコードで並列化・圧縮されていた部分)
        // =========================================================
        
        // 全データベース文字列に対してループ（非効率だがわかりやすい）
        for (uint32_t i = 0; i < db.total_count; i++) {
            
            // --- Step 1: フィルタリング ---
            // 共通するgramの数を数える
            int match_count = 0;
            for (int g = 0; g < 13; g++) {
                if (has_gram(&db, query_grams[g], i)) {
                    match_count++;
                }
            }

            // 共通gramが少なければ、似ていないのでスキップ
            if (match_count < FILTER_THRESHOLD) {
                continue; 
            }

            // --- Step 2: 詳細検証 ---
            // 候補に残ったものだけ、正確な編集距離を計算
            int distance = calculate_edit_distance(db.strings[i], query_ints);
            
            if (distance <= DISTANCE_THRESHOLD) {
                found_match = true;
                break; // 1つでも見つかればOK
            }
        }

        // 結果出力
        printf("%c", found_match ? '1' : '0');
    }
    
    printf("\n");
    fclose(query_fp);
    // free処理は省略
    return 0;
}