#ifndef _varsymm_h
#define _varsymm_h

class VarSymm {
public:
    VarSymm() : psym(1), nsym(1), skip0(1), skip1(1) {}
    VarSymm(int v) : psym(v), nsym(v), skip0(v), skip1(v) {}

    int		is_symm() { return psym || nsym; }
    int		is_xor() { return psym && nsym; }
    int		is_and() { return (psym || nsym) && (skip0 || skip1); }

    int		is_psym() { return psym; }
    int		is_nsym() { return nsym; }
    int		is_skip0() { return skip0; }
    int		is_skip1() { return skip1; }

    void	neg_sym() { psym = nsym = 0; }
    void	neg_psym() { psym = 0; }
    void	neg_nsym() { nsym = 0; }

    void	neg_skip0() { skip0 = 0; }
    void	neg_skip1() { skip1 = 0; }

    VarSymm	compose(VarSymm obj) {
	VarSymm result;
	result.psym = psym & obj.psym;
	result.nsym = nsym & obj.nsym;
	result.skip0 = skip0 & obj.skip0;
	result.skip1 = skip1 & obj.skip1;
	return result;
    }
    void print_sym() {
	printf("%d%d", psym, nsym);
    }

private:
    int	psym : 1;
    int	nsym : 1;
    int	skip0 : 1;
    int	skip1 : 1;
};

#endif

