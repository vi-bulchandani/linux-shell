#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<map>
#include<regex>
#include<vector>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <fcntl.h>  

#define BUF_SIZE 1024
using namespace std;

//colors
// string color_red=
// string color_green=
// string color_yellow=
// string color_blue=
// string color_magenta=
// string color_cyan=


//Read, Evaluate, Print, Loop

char hostname[1024];
char user[1024];
char cwd[1024];
char path[102400];
char home[1024];
const char *term = "xterm-256color";
map<string, string> var;

string inputfile;
string outputfile;
string appendfile;
char *input;

int inputredir=0;
int outputredir=0;
int appendredir=0;

//char *tokens[1000];

char *command[50];
char *commands[100][50];
FILE* history;
FILE* input_buf;

vector<string>tokens;
int numOfCommands;

void initializeShell(){
    // strcpy(hostname,getenv("HOSTNAME"));
    strcpy(user, getenv("USER"));
    strcpy(path, getenv("PATH"));
    strcpy(home, getenv("HOME"));
    gethostname(hostname, sizeof(hostname));
    setenv("TERM","xterm-256color",1);
    
    // for (int i = 0; i < 1000; i++)
    // {
    //     for (int j = 0; j < 50; j++)
    //     {
    //         commands[i][j]=new char[1024];
    //     }
        
    // }
    
    //strcpy()

    // fopen("")
    
}

bool printPrompt(){
    getcwd(cwd,sizeof(cwd));
    gethostname(hostname, 1024);
    strcpy(user,getenv("USER"));
    cout<<"\033[1;31m["<<user<<" @ "<< hostname << "]\033[;32m"<<cwd<<"$\033[0m";
    //cout<<getenv("TERM");
    return true;
}

void addHistory(const char *i){
    fputs(i, history);
    fputs("\n",history);
}



char*  getInput(){
    char* input;
    int bufsize=BUF_SIZE;
    int position = 0;
    char c;
    input = (char*)calloc(sizeof(char), bufsize);
    c=getchar();
    while((c)!='\n' && (c!=EOF) ){
        input[position++]=c;
        if(position==bufsize){
            bufsize+=BUF_SIZE;
        input=(char*) realloc(input,sizeof(char)*bufsize);
        
        }
        c=getchar();
    }
    input[position]=0;

    //cout << input<<endl;
    return input;
}

string replaceEnvs(string s){

    for (auto i: var)
    {
        regex re("[\\$]"+i.first+"[^A-Za-z0-9]|[\\$]"+i.first+"$");

        s = regex_replace(s,re,"$0");
    }

    return s;
}


void tokenizeInput(char *arr){
    inputfile="";
    outputfile="";
    appendfile="";
    inputredir=0;
    outputredir=0;
    appendredir=0;


    numOfCommands=0;

    string s=arr;
    tokens.clear();

    regex re("[^\\s\"'\\|<>]+|\"([^\"]*)\"|'([^']*)'|>>|>|<|[\\|]");

    for(sregex_iterator it= sregex_iterator(s.begin(),s.end(),re); it!= sregex_iterator(); it++){
        smatch match;
        match = *it;
        if(!match.str(1).empty()){tokens.push_back(match.str(1));}
        else if(!match.str(2).empty()){
            tokens.push_back(match.str(2));
        }
        else{
            tokens.push_back(match.str(0));
        }
    }

    if(tokens.size()>0){

    // for (string i: tokens)
    // {
    //     cout<<i<<endl;
    // }
    
    int count=1;

    for (string i: tokens)
    {
        if(i=="|"){count++;}
    }
    //cout<<count<<endl;
    numOfCommands=count;
    //cout<<"hi"<<endl;
    for(int i=0; i<tokens.size(); i++){
        if(tokens[i]=="<"){
            inputfile=tokens[i+1];
            inputredir=1;
        }
        if(tokens[i]==">"){
            outputfile=tokens[i+1];
            outputredir=1;
        }
        if(tokens[i]==">>"){
            appendfile=tokens[i+1];
            appendredir=1;
        }
    }

    // if(tokens.size()>0){
        history = fopen("/tmp/history", "a");
         fputs(input, history);
         fclose(history);
    // }
    //cout<<"hi"<<endl;
    count=0;
    int arg=0;
    for (string cmd: tokens)
    {
        //cout<<cmd<<"hello";
        if(cmd=="|"){
            commands[count][arg]=(char *) NULL;
            count++;
            arg=0;
        }
        else if(cmd=="<"|| cmd==">"|| cmd==">>"){
            commands[count][arg]=(char *) NULL;
            break;
        }
        else{
            commands[count][arg++]= (char*) malloc(1024);
        strcpy(commands[count][arg-1],cmd.c_str());
        //cout<<count<<arg<<cmd.c_str()<<"a"<<endl;
        }
        //cout<<cmd.c_str();
    }

    commands[count][arg]=(char *) NULL;
    //cout<<endl<<endl<<commands[0][0];

    // for (int i  = 0; i < numOfCommands; i++)
    // {
    //     for (int j = 0; j < 50; j++)
    //     {

    //         if(commands[i][j]==NULL){
    //             cout<<" NULL"<<endl;
    //             break;
    //         }
    //         else cout<<commands[i][j]<<" ";
    //     }
        
        
    // }

    //cout<<outputfile<<" "<<inputfile<<" "<<appendfile<<endl;
    }

    
}


void executeFinally(int i){
    if(strcmp(commands[i][0],"history")==0){
        //fclose(history);
        FILE *a= fopen("/tmp/history", "r");
        while(!feof(a)){
            putchar(fgetc(a));
        }
        fclose(a);
        //fopen("/tmp/history","a");
    }
    else if(strcmp(commands[i][0],"getenv")==0){
        if(getenv(commands[i][1])){
            puts(commands[i][1]);
        }
        
    }
    else{
    execvp(commands[i][0],commands[i]);
    puts("command not found");}
}

void executeCommand(){
    
    if(strcmp(commands[0][0], "cd")==0){
        if(commands[0][1]==NULL){
            chdir(home);
        }
        else{
            if(chdir(commands[0][1])){
                puts("directory not found");
            }
        }
    }
    else if(strcmp(commands[0][0], "setenv")==0){
        setenv(commands[0][1],commands[0][3],1);
    }
    else if(strcmp(commands[0][0],"exit")==0){
        puts("goodbye");
        //fclose(history);
        system("rm /tmp/history");
        exit(0);
    }
    else if(numOfCommands==1 && !outputredir && !appendredir && !outputredir){
            
            if(fork()==0){
            execvp(commands[0][0], commands[0]);
            puts("command not found");
        }
        else{
            wait(NULL);
        }
    }
    else{
        int tmpin=dup(0);
        int tmpout=dup(1);

        int fdin;
        if(inputredir){
            fdin = open(inputfile.c_str(),O_RDONLY);
            
        }
        else{
            fdin=dup(tmpin);
        }

        int ret;
        int fdout;
        for (int i = 0; i < numOfCommands; i++)
        {
            dup2(fdin,0);
            close(fdin);
            //close(fdin);
            if(i==numOfCommands-1){
                if(outputredir){
                    fdout=creat(outputfile.c_str(), 0644);
                }
                else if(appendredir){
                    fdout=open(appendfile.c_str(), O_WRONLY|O_APPEND|O_CREAT, 0644);
                }else {
                    fdout=dup(tmpout);
                }
            }
            else{
                int fdpipe[2];
                pipe(fdpipe);
                fdout=fdpipe[1];
                fdin=fdpipe[0];
            }

            dup2(fdout,1);
            close(fdout);
            ret=fork();
            if(ret==0){
            executeFinally(i);
            }

        }

        dup2(tmpin,0);
        dup2(tmpout,1);

        while(wait(NULL)>0);
        
    }
}
int main(){


     initializeShell();
     while(1){
     printPrompt();  

     input =getInput(); 
     tokenizeInput(input);
    executeCommand();
     
     }

    


}