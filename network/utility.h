#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <vector>
#include <algorithm>
#include <cassert>

using namespace std;

/* arrayint.h */
vector<int> inverse(const vector<int>& ai); // ai = arrayint
// Node型のポインタを扱うがためにテンプレートに
// jumpUpDown
template <typename T>
void jumpUD(T& ai, const int i, const int j) {
    if (i < j) {
        ai.insert(ai.begin() + j + 1, ai[i]);
        ai.erase(ai.begin() + i);

    } else if (i > j) {
        ai.insert(ai.begin() + j, ai[i]);
        ai.erase(ai.begin() + i + 1);
    }
}

void shift(vector<int>& ai, int idx, int degree, int defv);

// todo: http://d.hatena.ne.jp/pknight/20090826/1251303641
/* 分割コンパイルする場合，templateは実装もヘッダへ. */
/* 与えられたpermutationに従って変数順を変更する．
   permutationをlev2idxと見て，これに合わせるように変数順を変更するのではない．
   変数順をlev2idxにするためには，permutationにlev2idxの逆写像を与える．
   逆に，変数順がlev2idxであるものをnormalizeするには，
   permutationにlev2idxそのものを与えればよい．*/
template <typename T>
void changeOrder(T& ai, const vector<int>& perm)
{
    int n = perm.size();
    vector<int> localpermu(perm);

    // 大きい方から決めていく
    for (int dest = n; --dest > 0;) for (int idx = dest; --idx > 0;) {
            if (localpermu[idx] == dest) {
                if (idx != dest) {
                    jumpUD<T>(ai, idx, dest);
                    jumpUD<vector<int>>(localpermu, idx, dest);
                }
                break;
            }
        }
    for (int idx = n; --idx>0;) assert(localpermu[idx] == idx);
}

void printArrayint(const vector<int>& ai); // ファイルにArrayの中身を出力（エラーの時に使用）


/* -- arrayint.h -- */

/* util.h */
void   my_abort(char* c);

int    word_cnt(const char* c);// blifから回路情報を読み込むときに使用 空白で区切られている文字列の数をカウントする関数 例: a ab abc -> 3
void   ifile_error(int row, char *s);
void   readline(FILE* filep, char*& aline, int& aline_size, int& row);// ファイルの一行をchar型の配列に置き換える関数

string getAword(const char* c);

// 文字列を分割する関数:例えばスペースで区切るとか
template <typename List>
void split(const std::string& s, const std::string& delim, List& result)
{
    result.clear();
    
    using string = std::string;
    string::size_type pos = 0;
    
    while(pos != string::npos)
    {
        string::size_type p = s.find(delim, pos);

        if(p == string::npos)
        {
            result.push_back(s.substr(pos));
            break;
        }
        else {
            result.push_back(s.substr(pos, p - pos));
        }
        pos = p + delim.size();
    }
}

/* -- util.h -- */

#endif // #ifndef __UTILITY_H__
