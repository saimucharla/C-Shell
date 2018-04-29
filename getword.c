#include <stdio.h>
#include "getword.h"

int getword(char *w){

        int ch = 0;                             //holds current character from stdin
        int count = 0;                          //counter for current word
        int flag = 0;                           //flag for whether a backslash lies before current character

        for(;;){

                ch = getchar();
                if (count == 254) {
                        ungetc(ch, stdin);
                        *w = '\0';
                        return count;
                }
                if (ch == '\\'){
                        flag = 1;
                        ch = getchar();;
                }

                if (ch ==  ' ') {
                        if (count != 0 && flag == 0){
                                *w = '\0';
                                return count;
                        }

                        if (flag == 1) {
                                *w++ = ch;
                                count++;
                                flag = 0;
                        }

                        

                } else if (ch != ' ') {
                        if (ch == '\n'){
                                if (count == 0){
                                        *w = '\0';
                                        return -10;
                                }
                        ungetc(ch, stdin);
                        *w = '\0';
                        return count;
                        } else if (ch == EOF) {
                                if (count == 0){
                                        *w = '\0';
                                        return 0;
                                }
                                ungetc(ch, stdin);
                                *w = '\0';
                                return count;
                        } else if (ch == '<' || ch == '>' || ch == '|' || ch == '&' || ch == '#') {
                                if (flag == 1) {
                                        *w++ = ch;
                                        count++;
                                        flag = 0;
                                } else if (ch == '#') {
                                        if (count == 0){
                                                *w++ = ch;
                                                *w = '\0';
                                                return -1;
                                        } else {
                                                *w++ = ch;
                                                count++;
                                        }
                                } else if (count != 0){
                                        ungetc(ch, stdin);
                                        *w = '\0';
                                        return count;
                                } else  {                                               
                                        do {
                                                if (count == 1 && ch == '&') {
                                                        if (*(w-1) == '|') {
                                                                *w++ = ch;
                                                                count++;
                                                                *w = '\0';
                                                                return -count;
                                                        }
                                                }
                                                *w++ = ch;
                                                count++;
                                                ch = getchar();
                                        } while (ch == '<' || ch == '>' || ch == '|' || ch == '&' || ch == '#');        
                                                ungetc(ch, stdin);
                                                *w = '\0';
                                                return -count;
                                }
                        } else {
                                *w++ = ch;
                                count++;
                        }
                }
        }
}
