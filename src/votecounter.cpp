
#include <iostream>
#include "vote.h"
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <fstream>
#include <dirent.h>
#include <stack>
#include <string>
#include <pthread.h>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>
#include <vector>

#define NUM_OF_READER 10
#define BLOCK_SIZE 64
#define BLOCK_PER_THREAD 100
#define VOTE_PER_THREAD 100

using namespace std;

stack<string> listFile();

struct struct_votes_number
{
        Vote *votes;
        int number;
        struct_votes_number(Vote *votes, int number) : votes(votes), number(number){};
};
bool canUserVoteForMoreThenOneTime = true;

typedef map<unsigned int, unsigned int> map_condidate_votenumber;
typedef map<unsigned int, vector<uint32_t> *> user_vote_map;
map_condidate_votenumber global_condidatesMap;
user_vote_map global_user_votesMAP;
pthread_mutex_t global_condidatesMutex, global_user_votesMutex;

int getFileAndStartLocation(fstream *file)
{

        static fstream *cur_File = 0;

        static streampos cur_position;
        static streampos EndOfcorrentFile;

        if (cur_File == 0 || cur_position >= EndOfcorrentFile)
        {
                // cur_File = input_stack.top();
                // cout << cur_File << endl;
                //  (*cur_File).seekg(0);
                //  cur_position =cur_File->tellg();
                //  cur_File->seekp (0, ios::end);
                //  EndOfcorrentFile = cur_File->tellp();
        }

        file = cur_File;
        int temp = cur_position;
        cur_position += BLOCK_SIZE;
        return temp;
}

void updateGlobalScore(map_condidate_votenumber &localmap)
{
        pthread_mutex_lock(&global_condidatesMutex);
        for (map_condidate_votenumber::iterator it = localmap.begin(); it != localmap.end(); ++it)
        {

                map_condidate_votenumber::iterator globalmapiterator = global_condidatesMap.find(it->first);

                if (globalmapiterator != global_condidatesMap.end())
                {
                        globalmapiterator->second += it->second;
                        //cout << globalmapiterator->second << it->second<<'\t';
                }
                else
                {
                        //global_condidatesMap.insert(*it);  .
                        global_condidatesMap.insert(std::make_pair(it->first, it->second));
                }
        }
        pthread_mutex_unlock(&global_condidatesMutex);
}

void updateGlovalmoreoneTimeVoters(user_vote_map localuvm)
{
        pthread_mutex_lock(&global_user_votesMutex);
        for (user_vote_map::iterator it = localuvm.begin(); it != localuvm.end(); ++it)
        {

                user_vote_map::iterator glovalvotermapiterator = global_user_votesMAP.find(it->first);

                if (glovalvotermapiterator != global_user_votesMAP.end())
                {

                        glovalvotermapiterator->second->insert(glovalvotermapiterator->second->end(), it->second->begin(), it->second->end());
                }
                else
                {
                        //global_condidatesMap.insert(*it);  .
                        global_user_votesMAP.insert(std::make_pair(it->first, it->second));
                        //  b.insert(b.end(), a.begin(), a.end());
                }
        }
        pthread_mutex_unlock(&global_user_votesMutex);
}

void *do_countingVote(void *votesToCount)
{
        struct_votes_number vn = *(struct_votes_number *)votesToCount;
        user_vote_map localuvm;
        map_condidate_votenumber localmap;
        //vector<uint32_t> local_voter_ides;

        // cout << vn.number << vn.votes[1].sid << ',' << vn.votes[1].uid <<endl ;

        // if(!canUserVoteForMoreThenOneTime){
        //         if(local_voter_ides.)
        //         local_voter_ides.push_back(vn.votes[0].sid);
        // }
        //cout << vn.votes << '\t';
        for (int i = 0; i < vn.number; i++)
        {

                map_condidate_votenumber::iterator it = localmap.find(vn.votes[i].sid);

                if (it != localmap.end())
                {
                        it->second++;
                }
                else
                {
                        localmap.insert(std::make_pair(vn.votes[i].sid, 1));
                }

                if (!canUserVoteForMoreThenOneTime)
                {
                        user_vote_map::iterator it = localuvm.find(vn.votes[i].uid);
                        if (it != localuvm.end())
                        {
                                it->second->push_back(vn.votes[i].sid);
                        }
                        else
                        {

                                vector<uint32_t> *vec = new vector<uint32_t>(); //deleted TODO
                                vec->push_back(vn.votes[i].sid);
                                localuvm.insert(std::make_pair(vn.votes[i].uid, vec));
                        }
                }
        }
        delete[] vn.votes;
        updateGlobalScore(localmap);
        if (!canUserVoteForMoreThenOneTime)
        {
                updateGlovalmoreoneTimeVoters(localuvm);
        }
        //after each thread do conting shuld update score in global variable
}

int calculateVoteNumberInAfile(fstream *votefile)
{
        streampos begin, end;
        votefile->seekg(0, ios::beg);
        begin = votefile->tellg();
        votefile->seekg(0, ios::end);
        end = votefile->tellg();
        return (end - begin) / sizeof(Vote);
}

int load_vote_from_file(string filename)
{

        //  pthread_t threads[NUM_OF_READER];

        //  for(int i = 0 ; i < NUM_OF_READER;i++){
        //          pthread_create(&threads[i],NULL,do_countingVote,(void *)&i);

        //  }

        fstream myFile(filename.c_str(), ios::in | ios::binary);

        unsigned int numberOfVoteInFile = calculateVoteNumberInAfile(&myFile);
        unsigned int numberOfThreadNead = numberOfVoteInFile / VOTE_PER_THREAD;
        unsigned int numberOfremind = numberOfVoteInFile % VOTE_PER_THREAD;

        pthread_t *threads = new pthread_t[numberOfThreadNead + 1];
        myFile.seekg(0, ios::beg);
        Vote *votes[numberOfThreadNead + 1];
        for (int i = 0; i < numberOfThreadNead; i++)
        {
                votes[i] = new Vote[VOTE_PER_THREAD];
                struct_votes_number *votewithnumberstruct = new struct_votes_number(votes[i], VOTE_PER_THREAD);
                myFile.read((char *)votes[i], sizeof(Vote) * VOTE_PER_THREAD);
                pthread_create(&threads[i], NULL, do_countingVote, (void *)votewithnumberstruct);
        }

        if (numberOfremind > 0)
        {
                votes[numberOfThreadNead] = (new Vote[numberOfremind]);
                struct_votes_number *votewithnumberstruct = new struct_votes_number(votes[numberOfThreadNead], numberOfremind);
                myFile.read((char *)votes[numberOfThreadNead], sizeof(Vote) * numberOfremind);
                pthread_create(&threads[numberOfThreadNead], NULL, do_countingVote, (void *)votewithnumberstruct);
        }
        for (int i = 0; i < numberOfThreadNead; i++)
        {
                pthread_join(threads[i], NULL);
        }
        if (numberOfremind > 0)
                threads[numberOfThreadNead];

        // dont forgot
        delete[] threads;
}

//this function collect the vote file in directory
stack<string> listFile(string inputdir)
{
        stack<string> input_stack;
        DIR *pDIR;
        struct dirent *entry;
        if (pDIR = opendir(inputdir.c_str()))
        {
                while (entry = readdir(pDIR))
                {
                        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, "") != 0)
                        {
                                stringstream filename;
                                filename << "../inputs/" << entry->d_name;
                                //cout << filename.str() << endl;
                                input_stack.push(filename.str());
                        }
                }

                closedir(pDIR);
        }
        return input_stack;
}

void vote_conter(Vote *f, int number)
{
        for (int i = 0; i < number; i++)
        {
                cout << i;
        }
}
int findMax(map_condidate_votenumber map)
{
        uint32_t max_key = 0;
        uint32_t max_value = 0;
        for (map_condidate_votenumber::iterator it = map.begin(); it != map.end(); it++)
        {
                //  cout << it->second << ' ';
                if (max_value < it->second)
                {
                        //      cout << it->second;
                        max_key = it->first;
                        max_value = it->second;
                }
        }
        return max_key;
}
void deleteCheeterUser()
{
        // (map_condidate_votenumber::iterator it = map.begin() ; it != map.end() ; it++);
        for (user_vote_map::iterator it = global_user_votesMAP.begin(); it != global_user_votesMAP.end(); ++it)
        {
                if (it->second->size() > 1)
                {
                        // cout <<it->second->size() << '\t';
                        for (std::vector<uint32_t>::iterator ite = it->second->begin(); ite != it->second->end(); ++ite)
                        {
                                map_condidate_votenumber::iterator globalmapiterator = global_condidatesMap.find(*ite);
                                globalmapiterator->second--;
                        }
                        //TODO delete [] it->second;
                }
        }
}

static void show_usage(std::string name)
{
        std::cerr << "Usage: " << name << " <option(s)> SOURCES"
                  << "Options:\n"
                  << "\t-h,--help\t\tShow this help message\n"
                  << "\t-d,--dupvote ignore duplicated vote"
                  << "\t-r,--directory pass directory of vote files"
                  << std::endl;
}
//usage @amin-secondChanse:~/Desktop/finalos/src$ ./a.out  -r "../inputs"

int main(int argc, char *argv[])
{
        bool isdir = false;
        string dir;
        stack<string> filenames; //= listFile("../inputs");

        if (argc < 2)
        {
                show_usage(argv[0]);
                return 1;
        }
        std::vector<std::string> sources;
        std::string destination;
        for (int i = 1; i < argc; ++i)
        {
                std::string arg = argv[i];
                if ((arg == "-h") || (arg == "--help"))
                {
                        show_usage(argv[0]);
                        return 0;
                }
                else if ((arg == "-d") || (arg == "--dupvote"))
                {
                        canUserVoteForMoreThenOneTime = false;
                }
                else if ((arg == "-r") || (arg == "--directory"))
                {
                        isdir = true;
                        if (i + 1 < argc)
                        {
                                dir = argv[++i];
                        }
                        else
                        {
                                std::cerr << "--directory option requires one argument." << std::endl;
                                return 1;
                        }
                }
                else
                {
                        filenames.push(argv[i]);
                }
        }
        cout << dir;
        filenames = listFile(dir);

        while (!filenames.empty())
        {
                load_vote_from_file(filenames.top());
                filenames.pop();
        }

        if (!canUserVoteForMoreThenOneTime)
        {
                deleteCheeterUser();
        }
        map_condidate_votenumber::iterator winer1 = global_condidatesMap.find(findMax(global_condidatesMap));
        cout << "1st is " << winer1->first << "\t"
             << "with " << winer1->second << " votes" << endl;
        global_condidatesMap.erase(winer1);

        map_condidate_votenumber::iterator winer2 = global_condidatesMap.find(findMax(global_condidatesMap));
        cout << "2nd is " << winer2->first << "\t"
             << "with " << winer2->second << " votes" << endl;
        global_condidatesMap.erase(winer2);

        map_condidate_votenumber::iterator winer3 = global_condidatesMap.find(findMax(global_condidatesMap));
        cout << "3ed is " << winer3->first << "\t"
             << "with " << winer3->second << " votes" << endl;
        global_condidatesMap.erase(winer3);
        // auto pr = std::max_element
        // (
        //         global_condidatesMap.begin, global_condidatesMap.end,
        //          [] (const map_condidate_votenumber::value_type & p1, const  map_condidate_votenumber::value_type & p2) {
        //         return p1.second < p2.second;
        //  }
        //  );
        //std::cout << "A mode of the vector: " << pr->first << '\n';
        return 0;
}
