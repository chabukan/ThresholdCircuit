#include "utility.h"

#include <cstring> // memcpy

// Arrayint Arrayint::inverse()
// {
//     int i, n = size();
//     Arrayint result(n);
//     for ( i=n; --i>=0; ) result[ (*this)[i] ] = i;
//     return result;
// }

vector<int> inverse(const vector<int>& ai) {
    int n = ai.size();
    vector<int> res(n);

    for (int i=n; --i >= 0; ) res[ai[i]] = i;
    return res;
}

// void Arrayint::jumpup(int i, int j)
// {
//     assert( i <= j );
//     if ( i < j ) {
//         int tmp = ent[i];
//         for ( int k=i; k<j; k++ ) ent[k] = ent[k+1];
//         ent[j] = tmp;
//     }
// }
// template <typename T>
// void jumpUD(T& ai, const int i, const int j) {
//     if (i < j) {
//         ai.insert(ai.begin() + j + 1, ai[i]);
//         ai.erase(ai.begin() + i);

//     } else if (i > j) {
//         ai.insert(ai.begin() + j, ai[i]);
//         ai.erase(ai.begin() + i + 1);
//     }
// }

// index以上の部分を degree だけずらす
// degreeが正の場合，元の場所にはdefvが入れられる
// void Arrayint::shift(int index, int degree, int defv)
// {
//     int n = size();
//     if ( degree > 0 ) {
//         reserve( n + degree );
//         for ( int i=n-1; i >= index; i-- ) {
//             ent[i+degree] = ent[i];
//             ent[i] = defv;
//         }
//     }
//     if ( degree < 0 ) {
//         assert( index+degree >= 0 );
//         // index未満の部分は上書きされるかもしれない
//         for ( int i=index; i<n; i++ ) ent[i+degree] = ent[i];
//         reserve( n + degree );
//     }
// }

// idx 以上の部分を degree だけずらす
// degree が正の場合，元の場所にはdefvが入れられる
void shift(vector<int>& ai, int idx, int degree, int defv)
{
    if ( degree > 0 ) {
        for (int i = 0; i < degree; i++)
            ai.insert(ai.begin() + idx, defv);

    } else if ( degree < 0 ) {
        assert( idx+degree >= 0 );
        for (int i=0; i < (-1*degree); i++)
            ai.erase(ai.begin() + idx + degree);
    }
}

// void Arrayint::changeOrder(const Arrayint& permutation)
// {
//     int n = permutation.size();
//     Arrayint localpermu(permutation);

//     // 大きい方から決めていく
//     for ( int dest=n; --dest>0; ) for ( int idx=dest; --idx>0; ) {
//             if ( localpermu[idx] == dest ) {
//                 if ( idx != dest ) {
//                     //              printf("jumpup( %d, %d )\n", idx, dest);
//                     jumpup(idx, dest);
//                     localpermu.jumpup(idx, dest);
//                 }
//                 break;
//             }
//         }
//     for ( int idx=n; --idx>0; ) assert( localpermu[idx] == idx );
//     // 
//     //     return *this;
// }


// template <typename T>
// void changeOrder(T& ai, const vector<int>& perm)
// {
//     int n = perm.size();
//     vector<int> localpermu(perm);

//     // 大きい方から決めていく
//     for (int dest = n; --dest > 0;) for (int idx = dest; --idx > 0;) {
//             if (localpermu[idx] == dest) {
//                 if (idx != dest) {
//                     jumpUD<T>(ai, idx, dest);
//                     jumpUD<T>(localpermu, idx, dest);
//                 }
//                 break;
//             }
//         }
//     for (int idx = n; --idx>0;) assert(localpermu[idx] == idx);
// }


void printArrayint(const vector<int>& ai)
{
    unsigned int i;
    for ( i=0; i < ai.size(); i++ ) {
        fprintf(stderr, "%2d ", ai[i]);
        if ( i % 20 == 19 ) fprintf(stderr, "\n");
    }
    if ( --i % 20 != 19 ) fprintf(stderr, "\n");
}




/* util.h */

void my_abort(char* c)
{
    fprintf(stderr, "Error: %s\n", c);
    abort();
}


static const string sep = std::string(" ") + std::string("\0");
string getAword(const char* c)
{
    assert( *c != ' ' );	// 1文字目は空白でないこと

    std::string str = std::string(c);

    int start = str.find_first_not_of(sep); // 現在の文字列のインデックス番目の文字から検索を開始し、文字列の中に含まれない文字が最初に見つかった位置を返す。見つからない場合にはstring::nposを返す
    int stop = str.find_first_of(sep); // 現在の文字列のインデックス番目の文字から検索を開始し、文字列の中に含まれる文字が最初に見つかった位置を返す。見つからない場合にはstring::nposを返す

    return str.substr(start, stop-start); // 現在の文字列のインデックス から始まり長さ の部分文字列部分文字列を返す
}

int word_cnt(const char* c) {
    int count=0;
    while ( *c != '\0' ) {
        while ( *c == ' ' ) c++;
        if ( *c != '\0' ) {
            count++;
            while ( *c != ' ' && *c != '\0' ) c++;
        }
    }
    return count;
}

void ifile_error(int row, char *s) {
    fprintf(stderr, "In line %d, %s.\n", row, s);
    abort();
}

//// PLA, BLIF 用，1行読みルーチン
//// aline は `\0` で終わること
//// 行頭がEOFのときのみaline[0]に '\0'が入る
void readline(FILE* filep, char*& aline, int& aline_size, int& row)
{
    int i=0;
    char* newaline;
    while( 1 ) {
        aline[i] = getc(filep);
        if ( aline[i] == EOF ) {
            aline[i] = '\0';
            break;
        }
        if ( aline[i] == '#' || aline[i] == '\n' ) { // コメント or 行末
            // コメントだったら行末まで読み飛ばす
            if ( aline[i] == '#' ) while( getc(filep) != '\n' );
            row++;
            if ( i > 0 ) { // 少なくとも1文字あれば戻る
                aline[i] = '\0';
                break;
            }
            else continue;
        }
        else if ( aline[i] == '\\' ) { // 次の行をつなぐ
            aline[i] = getc(filep);
            if ( aline[i] == '\n' ) {
                aline[i] = getc(filep);
                row++;
            }
            else ifile_error(row,
                             (char*)", '\\' should be the last character of a line.\n");
        }
        if ( i++ >= aline_size-1 ) { // 次の文字へ
            newaline = new char[aline_size<<1];
            if ( newaline == 0 ) assert(!static_cast<bool>("no more memory"));
            memcpy(newaline, aline, sizeof(char)*aline_size);
            delete [] aline;
            aline = newaline;
            newaline = 0;
            aline_size <<= 1;
        }
    }
}


/* -- util.h -- */


