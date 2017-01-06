//////////////////////////////////////////////////////////////////
////////        BDD manipulator for logical operations    ////////
//////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <sys/wait.h>

#include "bddop.h"

//////////////////////////////////////////////////////////////////

int Bdd::objnum = 0;

void Bdd::printX() const
{
    // char* bdlfilename = tempnam("/tmp", "bdl_"); // todo 危険
    char bdlfilename[] = "/tmp/bdl_XXXXXX";
    mkstemp(bdlfilename);

    FILE* bdlfile = fopen(bdlfilename, "w");
    bddprintbdl(edge, bdlfile);
    fclose(bdlfile);
    if ( fork() == 0 ) {
        execlp("xbddfig", "xbddfig", bdlfilename, 0);
    }
    else {
        sleep(1);
        // 	int status;
        // 	wait(&status);
    }
    unlink(bdlfilename);
}

vector<int> Bdd::getPermuForCompress() const
{
    int n = top();
    vector<int> result(n+1);
    for ( int i=n; i>=0; i-- ) result[i] = i; // 0 1 .. n

    vector<int> variables(n+1);
    for ( int i=n; i>0; i-- ) variables[i] = 0;

    Bdd vars = varSet();
    while ( vars != Bdd::empty ) {
        int topvar = vars.top();

        variables[topvar] = 1;
        vars = vars.subset0(topvar);
    }

    int dest = n;
    for ( int i=n; i>0; i-- ) {
        if ( variables[i] == 0 ) jumpUD<vector<int>>(result, i, dest--);
    }
    return inverse(result);
}

/* 与えられたpermutationに従って変数順を変更する．
 permutationをlev2idxと見て，これに合わせるように変数順を変更するのではない．
 変数順をlev2idxにするためには，permutationにlev2idxの逆写像を与える．
 lev2idx 1342,    1234 ->[1423(逆写像)] -> 1342
 逆に，変数順がlev2idxであるものをnormalizeするには，
 permutationにlev2idxそのものを与えればよい．
 lev2idx 1342,    1342 ->[1342] -> 1234 */
Bdd Bdd::changeOrder(const vector<int>& permutation) const
{
    int n = permutation.size();
    vector<int> localpermu(permutation);

    Bdd result = *this;
    Bdd sup = this->varSet();
    // 大きい方から決めていく
    for ( int dest=n; --dest>0; ) {
        if ( localpermu[dest] == dest ) continue;
        for ( int idx=dest; --idx>0; ) {
            if ( localpermu[idx] == dest ) {
                // 		fprintf(stderr, "jumpup( %d, %d ) sup:%d\n", idx, dest,
                // 			sup.subset1(idx) == Bdd::base );
                result = result.jumpup(idx, dest);
                jumpUD<vector<int>>(localpermu, idx, dest);
                break;
            }
        }
    }
    /* ここでいろいろと工夫してみたが，速くならない．
     Bddパッケージの中で工夫しないとダメみたい．*/

    //     fprintf(stderr, "\n");
    //     for ( int idx=n; --idx>0; ) assert( localpermu[idx] == idx );
    return result;
}

Bdd Bdd::getAelement() const // 集合から１つだけ要素を取って返す
{
    int v = top();
    if ( v == 0 ) return *this;   // Bdd::base, Bdd:emptyはそのまま返す
    return subset1(v).getAelement().change(v);
    // subset1の方に行けば，必ずBdd::baseがある．
}

Bdd Bdd::dupLit() const
{
    // multiLitを使って行なっている．
    // 一個だけで良いので，もう少し良いやり方がある気がする．
    int n = top();
    char* literals = (char*)calloc(n+1, sizeof(char));
    multiLit(literals);

    Bdd dup_lit = Bdd::null;
    for ( int lev = n; lev >=1; lev-- ) {
        if ( literals[lev] >= 2 ) {
            dup_lit = Bdd::var(lev);
            break;
        }
    }
    free(literals);
    return dup_lit;
}	
