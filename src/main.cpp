#include <iostream>
#include <fstream>
#include <stdint.h>
#include <cstdlib>
#include <string>
#include <sstream>
using namespace std;

class vote
{
  public:
    uint32_t uid;
    uint32_t sid;
};

void GenerateVote(int numberOffile, int numberOfVoteInFile)
{

    string name = "vote";
    string format = ".bin";
    unsigned int max = 4294967294; //max32bitint
    vote *y = new vote[10];
    vote x;
    for (int j = 0; j < numberOffile; j++)
    {
        stringstream filename;
        filename << "../inputs/" << name << j << format;
        stringstream o;
        fstream myFile(filename.str().c_str(), ios::out | ios::binary);
        myFile.seekp(0, ios::beg);
        //  cout << max <<endl << rand() % (max);
        for (int i = 0; i < numberOfVoteInFile; i++)
        {
            x.uid = (rand() % (max));
            x.sid = (rand() % (max));
            myFile.write((char *)&x, sizeof(vote));
        }
        myFile.close();
    }
}

int main()
{
    //   ofstream votefile;
    //   votefile.open("vote.txt");
    //   votefile << "here to go" << endl << 4455554145 << ", " <<5554478545;
    //   votefile.close();

    //  streampos begin,end;
    // ifstream myfile ("vote.txt", ios::binary);
    // begin = myfile.tellg();
    // myfile.seekg (0, ios::end);
    // end = myfile.tellg();
    // myfile.close();
    // cout << "size is: " << (end-begin) << " bytes.\n";
    // return 0;
    //     unsigned int max = 4294967294; //max32bitint
    //      vote *y = new vote[10];
    //     vote x;

    //     fstream myFile ("vote.bin",  ios::out | ios::binary);
    //        myFile.seekp (0, ios::beg);
    //   //  cout << max <<endl << rand() % (max);
    //     for(int i = 0 ; i < 1000; i++){
    //     x.uid  =  (rand() % (max));
    //     x.sid =  (rand() % (max));
    //    // myFile.write ((char*)&x, sizeof (vote));
    //     }

    //   //  myFile.seekg (0);
    //     myFile.seekp (0);
    //     myFile.read ((char*)y, sizeof (vote) * 10);

    //     cout << y[5].uid;
    GenerateVote(100, 10000);
}