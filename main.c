//#1 complete the list of include files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE 1024

struct itimerval start, stop;
static int timedOut = 0; 

//#2 implement PrintS using write
int PrintS(const char* str)
{
  size_t the_len = strlen(str);
  //and outputs it to STDOUT using write
  int result = write(STDOUT_FILENO, str, the_len);

  if (result == -1 || (size_t)result != the_len)
    {
        //returns 0 on failure
        return 0; 
    }
//returns 1 on success
  return 1; 
}

//#3 implement printint
//takes in a const int
int PrintInt(const int val)
{
  char buf[BUF_SIZE]; 
  //and outputs it to STDOUT using write
  int result = write(STDOUT_FILENO, buf, snprintf(buf, BUF_SIZE, "%d", val));
  if (result < 0)
  {
    return 0; 
  }
  return 1; 
}

//4 implement the signalhandler
void signalHandler(int sig)
{
    //sigint handler
  if (sig == SIGINT)
{
    write(STDOUT_FILENO, "Do you want to exit??? (Y/N) ", 26);
    char buf;
    
    if (read(STDIN_FILENO, &buf, 1) != 1)
    {
      return;
    }
    if (buf == 'Y' || buf == 'y')
    {
      exit(0);
    } 
    else if (buf == 'N' || buf == 'n')
    {
      return;
    } 
    else
    {
      signal(SIGINT, signalHandler); 
    }
} 
//sig alarm handler
else if (sig == SIGALRM) 
{
    timedOut = 1;
}
}

//#5 implement readline 
//This function reads a line of text from a file
int readLine(int fd, char *line)
{
  int i = 0; 
  char r;

  while (read(fd, &r, 1) == 1)
{
    if (r == '\n' || r == '\r')
    {

    line[i] = '\0'; 
      if (i > 0)
    {
        return 1;
    } 
      else
    {
        return 0;
    }
    }
    line[i] = r;
    i++;
}
  if (i > 0) 
{
    return 1;
} 
  else
{
    return 0;
}

}
//#5...
//This function reads a question answer pairing from the provided pair of file descriptors
//It returns 0 if the files are empty and 1 if if successfully reads the pairing
int readQA(int questFd, int ansFd, char *quest, char *ans)
{
  if (readLine(questFd,quest) == 0) return 0; 
  if (readLine(ansFd, ans) == 0) return 0;
  return 1;
}
//#6a
//if timer is on
void ifON(int time)
{
  start.it_value.tv_sec = time;
  setitimer(ITIMER_REAL, &start, NULL);
}
//if timer is off
void ifOFF()
{
  stop.it_value.tv_sec = 0;
  setitimer(ITIMER_REAL, &stop, NULL);
}

int main(int argc, char *argv[]) 
{
  int numRead = 0; 
  int numWrite = 0; 
  int question = 1; 
  int correct = 0; 
  char buf[BUF_SIZE];
  char quest[BUF_SIZE]; 
  char ans[BUF_SIZE]; 
  int questFd, ansFd; 

//6b 
//6c
  struct sigaction sig;
  sig.sa_handler = signalHandler;
  sigemptyset(&sig.sa_mask);
  sigaddset(&sig.sa_mask, SIGALRM);
  sigaddset(&sig.sa_mask, SIGINT);
  sig.sa_flags = 0;

//#7
  if (sigaction(SIGALRM, &sig, NULL)==-1)
  {
    perror("ERROR setting SIGALRM signal");
    exit(1);
  }
  if (sigaction(SIGINT, &sig, NULL)==-1) 
  {
    perror("ERROR setting SIGINT signal");
    exit(1);
  }

//#8
  questFd = open("quest.txt", O_RDONLY);
  if (questFd == -1) 
  {
    perror("Failed to open file");
    exit(1);
  }

  ansFd = open("ans.txt", O_RDONLY);
  if (ansFd == -1) 
  {
    perror("Failed to open file");
    exit(1);
  }




  //#8
  readQA(questFd, ansFd, quest, ans);
  while (1) 
  {

    PrintS("#");
    PrintInt(question);
    PrintS(" ");
    PrintS(quest);
    PrintS ("? "); 
        
    timedOut = 0;

//#9
    ifON(20);

//#10
//read in the user's response
    char buf[BUF_SIZE];
    int numRead = read(STDIN_FILENO,buf,BUF_SIZE-1);

    if (numRead == 0) break;
    if (numRead == -1) 
    { 
      if (errno == EINTR) 
      {
        if (timedOut) 
        {
          PrintS("\nTime's up, next question!\n");
          if (readQA(questFd, ansFd, quest, ans) == 0) break;
          question++;
        } 
        continue;
      }
      perror("Read");
      exit(EXIT_FAILURE);
    }
    buf[numRead+1] = '\0';

//#11
//disable the time by setting the timer’s values to 0 seconds
    ifOFF();
    buf[numRead-1]= 0;
    if (strcmp(buf,ans) == 0) 
    {
      correct++;
      PrintS("\nCorrect\n");
    } 
    else 
    {
      PrintS(ans);
      PrintS("\nWrong\n");
    }
        
    if (readQA(questFd, ansFd, quest, ans) == 0) break;
    question++;
  }

  PrintS("Final Score is: ");
  PrintInt(correct);
  PrintS(" out of ");
  PrintInt(question);

//#12 
//closes both files
  close(questFd);
  close(ansFd);
}
