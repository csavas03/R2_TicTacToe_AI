#include <stdio.h>
#include <string.h>
#include <inttypes.h>

//0=none 1=X 2=O 3=draw
int get_win(char *m){
	for(int i=0;i<3;i++){//Horizontal
		if(m[i*3 + 0] == m[i*3 + 1] && m[i*3 + 1] == m[i*3 + 2]){
			return m[i*3 + 0];
		}
	}
	for(int i=0;i<3;i++){//Vertical
		if(m[i + 0*3] == m[i + 1*3] && m[i + 1*3] == m[i + 2*3]){
			return m[i + 0*3];
		}
	}
	if(m[0] == m[4] && m[4] == m[8]) return m[0];//Diag 0-8
	if(m[2] == m[4] && m[4] == m[6]) return m[2];//Diag 2-6

	int s = 0;
	for(int i=0;i<9;i++) if(m[i]) s++;

	return (s == 9)?3:0;
}
int minmax(char *m, int player, int depth){//Player = 1/2
	int w = get_win(m);
	if(w == 1 || w == 2) return (w == player)?1:-1;
	if(w == 3) return 0;

	int move = -1;
	int score = -2;
	for(int i=0;i<9;i++){
		if(m[i]) continue;
		char q[3*3];
		memcpy(q, m, 3*3);
		q[i] = player;
		int scoreForMove = -minmax(q, (player == 1)?2:1, depth+1);
		if(scoreForMove > score){
			score = scoreForMove;
			move = i;
		}
	}
	if(move == -1) return 0;
	if(depth) return score;
	return move;
}
int get_move(char *m){
	int mv = minmax(m, 2, 0);

	for(int i=0;i<9;i++){
		if(m[i]) continue;
		char q[3*3];
		memcpy(q, m, 3*3);
		q[i] = 2;
		if(get_win(q) == 2){
			//if(mv != i) fprintf(stderr, "Earlier win correction\n");
			return i;
		}
	}

	return mv;
}

//0b AAAA AAAA AAAA BBBB	A = addr of next table	B = [0-8 = move]
//0b 1111 1111 111A BBBB	A = [0 = AI win][1 = DRAW]	B = [0-8 = move]
uint16_t data[4096];
int addr_offset = 0x40;

int play(char *m, int addr){
	int lmod = addr + 9;

	for(int i=0;i<9;i++){
		char q[3*3];
		memcpy(q, m, 3*3);
		if(q[i]) continue;
		q[i] = 1;

		int getw = get_win(q);
		if(getw == 3){
			data[addr+i] = 0xFFF0;//Draw
			continue;
		}
		if(getw == 1){
			printf("NON PERFECT AI!");
		}

		int mv = get_move(q);
		q[mv] = 2;
		if(get_win(q) == 2){
			data[addr+i] = 0xFFE0 | mv;//AI win
			continue;
		}
		data[addr+i] = ((lmod + addr_offset) << 4) | mv;//Next AI move
		lmod = play(q, lmod);
	}
	return lmod;
}

int main(){
	char m[3*3];
	for(int i=0;i<9;i++) m[i] = 0;
	for(int i=0;i<4096;i++) data[i] = 0;

	int last_mod = play(m, 0);

	printf(";AI DATA --> length: %d\t|\tORG=0x%04X\n", last_mod, addr_offset);
	for(int i=0;i<4096 && i<last_mod;i+=16){
		printf("dw\t\t");
		for(int j=0;j<16 && i+j<last_mod;j++){
			printf("0x%04X", data[i+j]);
			if(j!=15 && i+j+1<last_mod) printf(", ");
		}
		printf("\t;%04X\n", addr_offset+i);
	}

	/*for(int i=0;i<4096;i++){
		printf("%04X: %04X\n", i, data[i]);
	}*/

	return 0;
}
