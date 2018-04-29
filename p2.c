#include "p2.h"


int FD[20];
int argc;
char argv[1000];                //holds the words that are parsed by getword.c to be determined whether they are stored in the newargv
char *newargv[MAXITEM];         //holds the arguments to be interpreted by p2
char *in;                       //name of the input file
char *out;                      //name of the file for output
int numch;                      //holds number of characters in word that has been parsed by getword.c
int tmpFile;                    //file descpriptor of file with commands to be executed
int backgroundFlag = 0;         //flag to note whether execution is to be done by a background process
int inFile;                     //file descriptor for the input file
int outFile;                    //file descriptor for output file
int pipeFlag = 0;               
int eofFlag = 0;                //flag to determine when end of executable instructions arrived so p2 can terminate
int tmpFlag = 0;                //flag to determine whether user should be re-prompted during execution of a script
int i;
int j;                          // i, j, r, are variables used to for loops for miscellanous purposes
int r;                          
int cmdFlag = 0;                
const char *name = "HOME";      //To determine home directory in CD block 
char *value;                    //
int pipeCtr;                    //Holds number of Pipes parsed by parse()
int dev_null;                   //file descripter for /dev/null
void pipeFunc();                //function name for piping call
int parse();            
int inFlag = 0;                 // flag determining direction of I/O
int outFlag = 0;                // ^^^^
int pipearg;                    
int poundFlag;                  // flag determining whether "#" is a comment or not
int and_pipe;                   // ****** unimplemented |& functions
int pipeLocation[100];          // array holding the location of the pipes in executed line
pid_t pid;                      //process ID's of parent and child
pid_t child_pid;
int wc;                         // holds number of arguments in parsed line





int main(int argc, char **argv) {


    setpgid(0, getpid());                       
    signal(SIGTERM, myhandler);                 

    if(argc == 2) {

        tmpFlag++;
        tmpFile = open(argv[1], O_RDONLY);                      
        dup2(tmpFile, STDIN_FILENO);                            
        close(tmpFile);
    }

    while (1) {
        
        if(tmpFlag == 0) {
                printf("p2: ");
        }
        argc = parse();                                         



///////////////////////////////////////////////////////////////Commenting Functionality

        if (poundFlag != 0) {
                int poundTmp;
                for (j = 0; j < argc;) {
                        if (strcmp(newargv[j],"#") == 0) {
                                poundTmp = j;
                                break;
                        }
                        j++;
                }



                if(strcmp(newargv[0],"echo") != 0) {
                        for (j = poundTmp; j < argc; j++) {
                                newargv[j] = NULL;
                        }
                }

        }

///////////////////////////////////////////////////////////// Continue Main

// the eofFlag and argc are being determined based on value of numch to either interpret the next line or end p2 process 
        if (eofFlag != 0) break; 
        if (argc == 0) continue; 

///////////////////////////////////////////////////////////// CD functionality


        if ((strcmp(newargv[0], "cd")) == 0) { 
            if (newargv[2] != NULL) {
                fprintf(stderr, "Too many args\n");           
                continue;
            } else if (newargv[1] == NULL) {
                chdir(getenv("HOME"));                    
            } else if (chdir(newargv[1]) != 0) {
                perror("cd error: ");                           
                continue;
            }
        }

///////////////////////////////////////////////////////////// MV Function

          if (strcmp(newargv[0], "MV") == 0) {
            int tmpcount = 1;                   
            char *flag = '\0';                  
            int argSrcPlace = '\0';             
            int argTargetPlace = '\0';          

            if (argc == 3) {                    
                if(access(newargv[2], F_OK) == 0) {
                    fprintf(stderr, "Overwrite Attempted on destination file %s during during MV command.\n", newargv[2]);
                    continue;
                }
                if(link(newargv[1], newargv[2]) == 0) {                 
                    unlink(newargv[1]);
                    continue;
                }
            }

            if (argc < 3) {                                             
                fprintf(stderr, "Not enough args to complete MV command.\n");
                continue;
            }


            while(tmpcount < argc) {                                            
                if (strcmp(newargv[tmpcount], "-n") == 0){
                        flag = newargv[tmpcount];
                } else if (strcmp(newargv[tmpcount], "-f") == 0) {              
                        flag = newargv[tmpcount];
                } else {
                    if (argSrcPlace == '\0') {
                        argSrcPlace = tmpcount;                                 
                    } else if (argTargetPlace == '\0') {
                        argTargetPlace = tmpcount;
                    } else if (argSrcPlace != '\0' && argTargetPlace != '\0') {         
                        fprintf(stderr, "Too many files for MV command to process\n");
                    }
                }
                tmpcount++;
            }

            if (strcmp(flag, "-f") == 0) {                                                             
                link(newargv[argSrcPlace], newargv[argTargetPlace]);                                  
                unlink(newargv[argSrcPlace]);
            } else if (access(newargv[argTargetPlace], F_OK) == 0) {                                   
                fprintf(stderr, "Overwrite attempted on destination file %s during MV command.\n", newargv[argTargetPlace]);    
            } else if (link(newargv[argSrcPlace], newargv[argTargetPlace]) == 0) {                      
                unlink(newargv[argSrcPlace]);
            }

        continue;                                      
        }

///////////////////////////////////////////////////////////// I/O Functionality
        if (outFlag != 0) {
            if ((outFile = open(out, O_EXCL | O_RDWR | O_CREAT, S_IRWXU)) < 0) {
                perror("out I/O error ");                                               
                continue;
            }
            if (out == NULL) {
                fprintf(stderr, "Out file is missing\n");                      
                continue;
            }
        }
        if (inFlag != 0) {
            if ((inFile = open(in, O_RDONLY)) < 0) {
                perror("in I/O error: ");                                              
                continue;
            }
            if (in == NULL) {
                fprintf(stderr, "In file is missing\n");                
                continue;
            }
        }

///////////////////////////////////////////////////////////////// Piping Function Call
        if (pipeCtr != 0) { //PIPING
            pipeFunc(pipeCtr);
            continue;
        }
///////////////////////////////////////////////////////////////// Forking 
        fflush(stdout);
        fflush(stderr);                                         
        child_pid = fork();
        if (child_pid == 0) {                                   
            if (backgroundFlag != 0 && inFlag == 0) {                  
                if ((dev_null = open("/dev/null", O_RDONLY)) < 0) {     
                    perror("/dev/null error: ");
                    exit(9);
                }
                dup2(dev_null, STDIN_FILENO);
                close(dev_null);
            }
            if (inFlag != 0) {                                  
                dup2(inFile, STDIN_FILENO);
                close(inFile);
            }
            if (outFlag != 0) {
                dup2(outFile, STDOUT_FILENO);                  
                close(outFile);
            }
            if ((execvp(newargv[0], newargv)) < 0) {
                perror("execvp error");                         
                exit(9);
            }
        }
        if (backgroundFlag != 0) {                              
            printf("%s [%d]\n", newargv[0], child_pid);
            backgroundFlag = 0;
            continue;
        } else {
            while(1) {                                          
                pid = wait(NULL);
                if (pid == child_pid) {
                        break;
                }
            }
        }


    }
    killpg(getpgrp(), SIGTERM);                                
    printf("p2 terminated.\n");
    exit(0);                                                    

}

void pipeFunc(){
                                                
    int FD[pipeCtr * 2];                                       

    int ctr = 0;                                               

    int pipe_loc;                       

    pid_t pid, child, middlePID;                                

    fflush(stdout);
    fflush(stderr);                                             

    if (backgroundFlag != 0 && inFlag == 0) {                           
        if ((dev_null = open("/dev/null", O_RDONLY)) < 0) {
            perror("dev/null error");
            exit(9);
        }
        dup2(dev_null, STDIN_FILENO);                             
        close(dev_null);                                           
    }
    if ((child = fork()) < 0) {
        perror("Child Fork Failed");
    }
    if (child == 0) {


//----------------------------------------------------------

        while (ctr < pipeCtr) {
            pipe(FD + 2 * ctr);                                         
            fflush(stdout);
            fflush(stderr);                             
            if ((middlePID = fork()) < 0) {
                perror("Middle fork error");                            
                exit(9);
            } else if (middlePID == 0) {

//===============================================================


                ctr++;
                pipe_loc = 2*ctr;
                if (ctr <= pipeCtr) {
                    dup2(FD[pipe_loc- 1], STDOUT_FILENO);          
                    close(FD[pipe_loc- 1]);
                } else if (ctr == pipeCtr) {
                    if (inFlag != 0) {                                  
                        dup2(inFile, STDIN_FILENO);
                        close(inFile);
                    }
                }
                continue;
           
//=================================================================
             }
            if (ctr != pipeCtr) {
                pipe_loc = 2*ctr;
                dup2(FD[pipe_loc], STDIN_FILENO);                 
                close(FD[(pipe_loc) + 1]);                 
            }

            if (ctr == 0) {
                if (outFlag != 0) {
                    dup2(outFile, STDOUT_FILENO);                  
                    close(outFile);                                
                }
            }

                break;
        }

        if (ctr != pipeCtr) {
            pipe_loc = pipeCtr - 1;
            if (execvp(newargv[pipeLocation[(pipe_loc) - ctr]], &newargv[pipeLocation[(pipe_loc) - ctr]]) < 0) {        
                perror("execvp error");
                exit(9);
            }
        } else {
            if ((execvp(newargv[0], newargv)) < 0) {
                perror("execvp error");
                exit(9);
            }
        }
        while (1) {
            pid = wait(NULL);
            if (pid == middlePID) break;                                        
        }
    }

//--------------------------------------------------------------------

    if (backgroundFlag == 1) {
        printf("%s [%d]\n", *newargv, child);
        backgroundFlag = 0;
    } else {
        while (1) {
            pid = wait(NULL);
            if (pid == child) break;
        }
    }
}



int parse() {
    char *word_ptr;             
    int p = 0;                  
    int ptr = 0;                
    in = out = 0;
    numch = wc = 0;
    pipeCtr = inFlag = outFlag = poundFlag = backgroundFlag = and_pipe = eofFlag = 0;


    while (1) {
        word_ptr = (argv + ptr);                
        numch = getword(word_ptr);                      
        if (numch == -10) {                             
            break;
        } else if (numch == 0) {                        
            eofFlag = 1;
            break;

        } 

        if (*word_ptr == '&') {                         
            backgroundFlag++;
            break;
        }

        if (numch == -2) {
            and_pipe++;                                 
            numch = 2;
        }

        if (*word_ptr == '#' && wc == 0) {
            poundFlag++;                                
        }
        if (*word_ptr == '#' && numch == -1){
            if (wc == 1){
                poundFlag++;                            
            } else {
                numch = 1;
            }
        }
        if (*word_ptr == '|') {
            if (pipeCtr > 10) {                                 
                fprintf(stderr, "SYNTAX ERROR: Too many pipes\n");
                break;
            }
            newargv[p++] = NULL;
            pipeLocation[pipeCtr++] = p;                        


        } else if (*word_ptr == '<' || inFlag == 1) {
            inFlag++;
            in = word_ptr;                                      
            if (inFlag > 2) {                                   
                fprintf(stderr, "Too many in flags\n");                         
                break;
            }

        } else if (*word_ptr == '>' || outFlag == 1) {
            outFlag++;
            out = word_ptr;                                     
            if (outFlag > 2) {
                fprintf(stderr, "Too many out flags\n");
                break;
            }
        } else {
            newargv[p++] = word_ptr;                            
        }
        argv[ptr + numch] = '\0';
        ptr += numch + 1;                                      
        wc++;                                                  
    }

    newargv[p] = NULL;                                         
    return wc;
}

void myhandler(){};
