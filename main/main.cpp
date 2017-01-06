#include <iostream>
#include <cstdlib>
#include <unistd.h>

#include "network.h"

#define DBG
#ifdef DBG
#define DEBUG(statement) statement
#else
#define DEBUG(statement)
#endif

using namespace std;

Network* readFileName(char * filename);

int main(int argc, char **argv)
{
  //DdManager *manager; 
  //manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);

  cout <<endl;
  if (argc != 2) {
    std::cerr << "Usage diffnet <file1> \n";
    exit(1);
  }

  int maxbdd_power = 25;
  Bdd::alloc(20, maxbdd_power);
  Network* circ1 = readFileName(argv[1]);

  bool is_th = std::string(argv[1]).find(".log") != std::string::npos;
  bool in_format = ( is_th ) ? false : true;

  if ( in_format == false ) {
    circ1->calAllSpecTh('0');
    circ1->setCSPF_AllTh();
  }
  else{
    exit(1);
  }

  //std::cerr << "Check is finished. -- The two files are the same functionally. \n";
}

// ファイルname (openする)を渡して、そのファイルから読み込んで network をつくる。
// file が openできなければ exit(1)。
// Bdd が初期化されてないといけない。

Network * readFileName(char * filename)
{
    Network* net = new Network();
    net->readFileName(filename);
    return net;
}
