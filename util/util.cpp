#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cstdint>

#include "util.h"


// todo: 特に型変換する必要はなさそう

// ValList.cppで使われている for debug?
void error(char* msg)
{
    fprintf(stderr, "%s\n", msg);
    abort();
}

void report_time()
{
    struct rusage   rusage;
    getrusage(RUSAGE_SELF, &rusage);
    long utime_s = rusage.ru_utime.tv_sec;
    long utime_u = rusage.ru_utime.tv_usec;
    double utime = (double) utime_s + ((double) utime_u / 1e6);
    long stime_s = rusage.ru_stime.tv_sec;
    long stime_u = rusage.ru_stime.tv_usec;
    double stime = (double) stime_s + ((double) stime_u / 1e6);
    fprintf(stderr, "Time %.2fu + %.2fs = %.2f sec.\n",
            utime, stime, utime+stime);
}

//////////////////////////////////////////////////////////////////

int time_expire;
int timer_value;

static struct itimerval value = {{0, 0}, {0, 0}};
//     value.it_interval.tv_sec = 0;
//     value.it_interval.tv_usec = 0;
//     value.it_value.tv_sec = 100;
//     value.it_value.tv_usec = 0;

void timeout(int sig)
{
    fprintf(stderr, "timeout by SIGPROF[%d], %d sec\n", sig, timer_value);
    time_expire = 1;
    return;
}

void setTimer(int sec)
{
    ::value.it_value.tv_sec = sec;
    ::value.it_value.tv_usec = 0;
    setitimer(ITIMER_PROF, &::value, 0);
    signal(SIGPROF, timeout);
    timer_value = sec;
    time_expire = 0;
}

int isTimerExpire()
{
    return time_expire;
}

//void my_abort(const char* c)
void my_abort(char* c)
{
    fprintf(stderr, "Error: %s\n", c);
    abort();
}

//////////////////////////////////////////////////////////////////

void ifile_error(int row, char *s) {
    fprintf(stderr, "In line %d, %s.\n", row, s);
    abort();
}

////	PLA, BLIF 用，1行読みルーチン
////	aline は `\0` で終わること
////	行頭がEOFのときのみaline[0]に '\0'が入る
void readline(FILE* filep, char*& aline, int& alineSize, int& row)
{
    int i=0;
    char* newaline;
    while( 1 ) {
        aline[i] = getc(filep);
        if ( aline[i] == EOF ) {
            aline[i] = '\0';
            break;
        }
        if ( aline[i] == '#' || aline[i] == '\n' ) {	// コメント or 行末
            // コメントだったら行末まで読み飛ばす
            if ( aline[i] == '#' ) while( getc(filep) != '\n' );
            row++;
            if ( i > 0 ) {			// 少なくとも1文字あれば戻る
                aline[i] = '\0';
                break;
            }
            else continue;		
        }
        else if ( aline[i] == '\\' ) {		// 次の行をつなぐ
            aline[i] = getc(filep);
            if ( aline[i] == '\n' ) {
                aline[i] = getc(filep);
                row++;
            }
            else ifile_error(row,
                             (char*)", '\\' should be the last character of a line.\n");
        }
        if ( i++ >= alineSize-1 ) {	// 次の文字へ
            newaline = new char[alineSize<<1];
            if ( newaline == 0 ) assert(!(int64_t)"no more memory");
            memcpy(newaline, aline, sizeof(char)*alineSize);
            delete [] aline;
            aline = newaline;
            newaline = 0;
            alineSize <<= 1;
        }
    }
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

// marimo:
// a bb ccならreturn a?
// stringstreamでも可
static const std::string sep = std::string(" ") + std::string("\0");

std::string getAword(const char* c)
{
    assert( *c != ' ' );	// 1文字目は空白でないこと

    std::string str = std::string(c);

    int start = str.find_first_not_of(sep); // 現在の文字列のインデックス番目の文字から検索を開始し、文字列の中に含まれない文字が最初に見つかった位置を返す。見つからない場合にはstring::nposを返す
    int stop = str.find_first_of(sep); // 現在の文字列のインデックス番目の文字から検索を開始し、文字列の中に含まれる文字が最初に見つかった位置を返す。見つからない場合にはstring::nposを返す

    return str.substr(start, stop-start); // 現在の文字列のインデックス から始まり長さ の部分文字列部分文字列を返す
}

int hash_string(const std::string& key) // From Dragon book, p436
{
    const char* x = key.c_str();

    unsigned int h = 0;
    unsigned int g;

    while (*x != 0)
    {
        h = (h << 4) + *x++;
        if ((g = h & 0xf0000000) != 0)
            h = (h ^ (g >> 24)) ^ g;
    }
    return h;
}
