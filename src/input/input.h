/**************************************
****   INPUT.H  -  Input devices   ****
****         Header File           ****
**************************************/

#ifndef	INPUT_H
#define INPUT_H

/*-- input.c functions -----------------------------------------------------*/
void input_init(void);
void input_shutdown(void);

void processEvents(void);

unsigned char read_player1(void);
unsigned char read_player2(void);
unsigned char read_pl12_startsel(void);
extern int select_lock;


#endif /* INPUT_H */
