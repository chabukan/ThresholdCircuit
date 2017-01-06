#ifndef _util_h
#define _util_h

#include <stdio.h>
#include <string>
#include <cstring>
#include "Array.H"

typedef unsigned char	u_char;
typedef unsigned short	u_short;
typedef unsigned int	u_int;

extern void	error(char* msg);//center: ValList.cppで使われている
extern void	report_time();//center: 現在使われていない 計算時間を計測するための関数

extern void	setTimer(int sec);// center: 現在使われていない タイマのセット
extern int	isTimerExpire(); // center: 現在使われていない タイムアウトした数を返す関数

//extern void	my_abort(const char* c);
extern void	my_abort(char* c);// center: network.cppで使われている

extern int	word_cnt(const char* c);// center: network.cppで使われている blifから回路情報を読み込むときに使用 空白で区切られている文字列の数をカウントする関数 例: a ab abc -> 3
extern void	ifile_error(int row, char *s); // center: network.cppで使われている
extern void	readline(FILE* filep, char*& aline, int& alineSize, int& row);// center: network.cppで使われている ファイルの一行をchar型の配列に置き換える関数

extern std::string   getAword(const char* c);// center: network.cppで使われている plaから回路情報を読み込むときに使用?

extern int	hash_string(const std::string& key);// center: ???

#endif
