#include <unistd.h> 
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifndef _WIN32
    #include <termios.h>
    #include <fcntl.h>
    int kbhit(){
        struct termios oldt, newt;
        int ch, oldf;
        tcgetattr(STDIN_FILENO,&oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
        ch = getchar();    
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
    if( ch != EOF){
        ungetc(ch,stdin);
        return 1;
    }
    return 0;
    }
    int getch(void) {
        int ch;
        struct termios oldt;
        struct termios newt;
        tcgetattr(STDIN_FILENO,&oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }
#else
    #include <conio.h>
#endif

#ifdef _WIN32
    #define CLEAR "cls"
#else
    #define CLEAR "clear"
#endif

#define RAND ((rand()%100 + 1))

char tabuleiro[10][135];
int altura=10, largura=135;
int probX=29, probF=14;
int velocidade=50000;
int combustivel=400, pontos=0;

int usleep();
void mainmenu();
void jogo();

/*mostra uma tela de gameover*/
void gameover(int x){
    system(CLEAR);
    int c=0; /*recebe o inteiro correspondente a tecla apertada*/
    printf("\n");
    printf("  ___ ____ ___ _  ___   ___ _  _____ ____"); printf("\n");
    printf(" / _ `/ _ `/  ' \\/ -_) / _ \\ |/ / -_) __/"); printf("\n");
    printf(" \\_, /\\_,_/_/_/_/\\__/  \\___/___/\\__/_/   "); printf("\n");
    printf("/___/                                    "); printf("\n");
    printf("\n");
    if(x==0)
        printf("Seu combustivel acabou!\n");
    else if(x==1)
        printf("Voce bateu!\n");
    
    printf("Voce fez %d pontos.\n", pontos);
    printf("Pressione ESPACO para voltar a tela inicial.\n");
    while(c!=32){
        c = getch();
        if(c==32)/*ESPAÇO pressionado*/
            jogo();
    }
}

/*mostra o atual tabuleiro na tela*/
void print_tabuleiro(){
    int linha, coluna;/*dimensoes da matriz*/
    for(linha=0; linha<altura; linha++){
        for(coluna=0; coluna<largura; coluna++){
            /*imprimindo os 'x' com cor vermelha*/
            if(tabuleiro[linha][coluna]=='X')
                printf("\e[1;31m" "%c" "\e[1;0m", tabuleiro[linha][coluna]);
            else if(tabuleiro[linha][coluna]=='F')
                printf("\e[1;34m" "%c" "\e[1;0m", tabuleiro[linha][coluna]);
            else
                printf("%c", tabuleiro[linha][coluna]);
        }
        printf("\n");
    }
}

/*configura o tabuleiro para sua forma inicial*/
void set_tabuleiro(int posicaonave){
    int linha, coluna; /*dimensoes da matriz*/
    for(linha=0; linha<altura; linha++){
        for(coluna=0; coluna<largura; coluna++){
            if(linha==0)
                tabuleiro[linha][coluna]='#';
            if(linha==9)
                tabuleiro[linha][coluna]='#';
            if(linha!=0 && linha!=9)
                tabuleiro[linha][coluna]=' ';
        }
    }
    tabuleiro[posicaonave][0]='+';
}

/*faz com que os tiros ou 'f' ou 'x' que chegaram ao final do mapa, desapareçam*/
void fimdemapa(){
    int linha, coluna; /*dimensoes do tabuleiro*/
    for(linha=1; linha<(altura-1); linha++){
        for(coluna=0; coluna<largura; coluna=coluna+(largura-1)){
            if(tabuleiro[linha][coluna]=='>')/*remove os '>' do fim do mapa*/
                tabuleiro[linha][coluna]=' ';
            if(tabuleiro[linha][coluna]=='X' || tabuleiro[linha][coluna]=='F')/*remove os 'F' e os 'X' do fim do mapa*/
                tabuleiro[linha][coluna]=' ';
        }
    }
}

/*move os elementos do tabuleiro*/
/*e é responsavel pelas colisoes*/
void movertabuleiro(){
    int linha, coluna; /*dimensoes da matriz*/
    for(linha=1; linha<(altura-1); linha++){ /*exclui-se a primeira e a ultima linhas pois elas delimitam o mapa*/
        for(coluna=(largura-2); coluna>0; coluna--){
            /*colisao entre '>' e 'X'*/
            if(tabuleiro[linha][coluna]=='>' && tabuleiro[linha][coluna+1]=='X'){
                tabuleiro[linha][coluna]=' ';
                tabuleiro[linha][coluna+1]=' ';
                pontos+=50;
            }
            else if(tabuleiro[linha][coluna]=='>' && tabuleiro[linha][coluna+2]=='X'){
                tabuleiro[linha][coluna]=' ';
                tabuleiro[linha][coluna+2]=' ';
                pontos+=50;
            }
            /*colisao entre '>' e 'F'*/
            else if(tabuleiro[linha][coluna]=='>' && tabuleiro[linha][coluna+1]=='F'){
                tabuleiro[linha][coluna]=' ';
                tabuleiro[linha][coluna+1]=' ';
            }
            else if(tabuleiro[linha][coluna]=='>' && tabuleiro[linha][coluna+2]=='F'){
                tabuleiro[linha][coluna]=' ';
                tabuleiro[linha][coluna+2]=' ';
            }
            /*movimento normal dos '>'*/
            else if(tabuleiro[linha][coluna]=='>'){
                tabuleiro[linha][coluna]=' ';
                tabuleiro[linha][coluna+1]='>';
            }
        }
    }
    for(linha=1; linha<(altura-1); linha++){
        for(coluna=1; coluna<largura; coluna++){
            /*colisao entre '+' e 'X'*/
            if(tabuleiro[linha][coluna]=='X' && tabuleiro[linha][coluna-1]=='+')
                gameover(1);
            /*colisao entre '+' e 'F'*/
            else if(tabuleiro[linha][coluna]=='F' && tabuleiro[linha][coluna-1]=='+'){
                tabuleiro[linha][coluna]=' ';
                combustivel+=40;
            }
            /*movimento normal dos 'X'*/
            else if(tabuleiro[linha][coluna]=='X'){
                tabuleiro[linha][coluna]=' ';
                tabuleiro[linha][coluna-1]='X';
            }
            /*movimento normal dos 'F'*/
            else if(tabuleiro[linha][coluna]=='F'){
                tabuleiro[linha][coluna]=' ';
                tabuleiro[linha][coluna-1]='F';
            }
        }
    }
}

/*responsável por gerar 'F' ou 'X' no tabuleiro*/
void spawn() {
    int coluna=(largura-1); /*os mobs sempre aparecem na ultima coluna*/
    int r = (rand()%(altura-2))+1; /*decisao pseudorandomica da linha que o x ou f aparecerá*/
    if(RAND<=probF){
        if(tabuleiro[r][coluna-1]==' ') /*para que nao aconteçam x e f juntos (xf ou fx)*/
            tabuleiro[r][coluna]='F';
    }
    else if(RAND>probF && RAND<=(probF+probX)){
        if(tabuleiro[r][coluna-1]==' ')/*para que nao aconteçam x e f juntos (xf ou fx)*/
            tabuleiro[r][coluna]='X';
    }
}

/*responsável pelos tiros '>'*/
void atirar(int posicaonave){
    if(tabuleiro[posicaonave][1]=='F')/*caso exista um f na frente da nave*/
        tabuleiro[posicaonave][1]=' ';
    else if(tabuleiro[posicaonave][1]=='X'){/*caso exista um x na frente da nave*/
        tabuleiro[posicaonave][1]=' ';
        pontos+=50;
    }
    else
        tabuleiro[posicaonave][1]='>';
}

/*responsável por mover o '+' verticalmente*/
/* e responsavel pelas colisoes verticais entre o '+' o os mobs 'F' e 'X'*/
void movernave(int c, int *posicaonave){
    if((*posicaonave)>1){
        if(c==119 || c==87){/*w pressionado*/ /*movimento para cima*/
            if(tabuleiro[(*posicaonave)-1][0]=='F'){/*encontro vertical entre '+' e 'F'*/
                tabuleiro[(*posicaonave)-1][0]='+';
                tabuleiro[(*posicaonave)][0]=' ';
                combustivel+=40;
            }
            else if(tabuleiro[(*posicaonave)-1][0]=='X')/*encontro vertical entre '+' e 'X'*/
                gameover(1);
            else{/*movimento normal do '+'*/
                tabuleiro[(*posicaonave)-1][0]='+';
                tabuleiro[(*posicaonave)][0]=' ';
            }
            (*posicaonave)--;
        }
    }
    if((*posicaonave)<8){
        if (c==115 || c==83){/*s pressionado*/ /*movimento para baixo*/
            if(tabuleiro[(*posicaonave)+1][0]=='F'){/*encontro vertical entre '+' e 'F'*/
                tabuleiro[(*posicaonave)+1][0]='+';
                tabuleiro[(*posicaonave)][0]=' ';
                combustivel+=40;
            }
            else if(tabuleiro[(*posicaonave)+1][0]=='X')/*encontro vertical entre '+' e X'*/
                gameover(1);
            else{/*movimento normal do '+'*/
                tabuleiro[(*posicaonave)+1][0]='+';
                tabuleiro[(*posicaonave)][0]=' ';
            }
            (*posicaonave)++;
        }
    }
}

/*inicia a execução do jogo*/
void start(){
    int posicaonave=4;/*linha em que o '+' se encontra*/
    set_tabuleiro(posicaonave);
    int moveu, atirou; /*valem 1 caso a nave se mexa ou atire, respectivamente, e 0 caso contrario*/
    int c;/*recebe o inteiro correspondente a tecla pressionada*/
    while(combustivel>=0){
        moveu=0; atirou=0;
        usleep(velocidade);
        system(CLEAR);
        printf("Combustivel: %d\tPontos: %d\n", combustivel, pontos);
        print_tabuleiro();
        if(kbhit()==1){
            c = getch();
            if(c==119 || c==115 || c==87 || c==83){/*w ou s pressionado*/
                movernave(c, &posicaonave);
                combustivel-=2;
                moveu=1;
            }
            else if(c==107 || c==75){/*k pressionado*/
                atirar(posicaonave);
                combustivel-=4;
                atirou=1;
            }
        }
        if(!moveu && !atirou)
            combustivel--;
        pontos++;
        movertabuleiro();
        fimdemapa();
        spawn();
        if(combustivel<=0)
            gameover(0);
    }
}

/*mostra as instrucoes*/
void instrucoes() {
    system(CLEAR);
    int c;/*recebe o inteiro correspondente a tecla pressionada*/
    printf("'+' eh o avatar do jogador;\n");
    printf("Utilize 'w' para mover-se para cima, e 's' para mover-se para baixo;\n");
    printf("'X' representa os inimigos, 'F' representa o combustivel;\n");
    printf("Utilize 'k', para atirar;\n");
    printf("Ficar parado gasta 1 em combustivel, mover-se gasta 2, e atirar, 4;\n");
    printf("Atirar nos 'X' soma 50 ao total de pontos;\n");
    printf("Por padrao o jogador recebe 1 enquanto estiver vivo;\n");
    printf("O jogo acaba quando o avatar atinge algum 'X' ou quando o combustivel acaba.\n");
    printf("\n");
    printf("Pressione ESPACO para retornar ao menu.\n");
    while(c!=32){
        c = getch();
        if(c==32)/*ESPAÇO pressionado*/
            mainmenu();
    }
}

/*mostra o menu do jogo*/
void mainmenu() {
    int c=0; /*recebe o inteiro correspondente a tecla pressionada*/
    system(CLEAR);
    printf("(J)ogar\n(C)onfiguracoes\n(R)anking\n(I)nstrucoes\n(S)air\n");
    while(c!=106 && c!=105 && c!=74 && c!=73 && c!=115 && c!=83){
        c = getch();
        switch(c){
            /*j pressionado*/
            case 106:
                start();
                break;
            case 74:
                start();
                break;
            /*i pressionado*/            
            case 105:
                instrucoes();
                break;
            case 73:
                instrucoes();
                break;
            /*s pressionado*/
            case 115:
                jogo();
                break;
            case 83:
                jogo();
                break;
            default:
                break;
        }
    }
}

/*mostra uma tela de boas-vindas*/
void jogo () {
    combustivel=400; pontos=0;
    int c=0; /*recebe o inteiro correspondente a tecla pressionada*/
    system(CLEAR);
    printf("\n");
    printf("       __                                            __    /\\ \\    "); printf("\n");
    printf(" _ __ /\\_\\  __  __     __   _ __       _ __    __   /\\_\\   \\_\\ \\   "); printf("\n");
    printf("/\\`'__\\/\\ \\/\\ \\/\\ \\  /'__`\\/\\`'__\\    /\\`'__\\/'__`\\ \\/\\ \\  /'_` \\  "); printf("\n");
    printf("\\ \\ \\/ \\ \\ \\ \\ \\_/ |/\\  __/\\ \\ \\/     \\ \\ \\//\\ \\L\\.\\_\\ \\ \\/\\ \\L\\ \\ "); printf("\n");
    printf(" \\ \\_\\  \\ \\_\\ \\___/ \\ \\____\\\\ \\_\\      \\ \\_\\\\ \\__/.\\_\\\\ \\_\\ \\___,_\\"); printf("\n");
    printf("  \\/_/   \\/_/\\/__/   \\/____/ \\/_/       \\/_/ \\/__/\\/_/ \\/_/\\/__,_ /"); printf("\n");
    printf("\n");
    printf("Pressione ESPACO para continuar ou 'S' para sair.\n");
    while(c!=32 && c!=115 && c!=83){
        c = getch();
        if(c == 32)/*ESPAÇO pressionado*/
            mainmenu();
        if(c==115 || c==83)
            system(CLEAR);
    }
}

int main (){
    srand(time(0));
    jogo();

return 0;
}