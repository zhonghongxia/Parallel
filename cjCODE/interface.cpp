/*
 * =====================================================================================
 *
 *       Filename:  interface.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/21/2013 03:37:57 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  liangyao(ly), liangyao.ndsl@gmail.com 
 *        Company:  NDSL UESTC
 *
 * =====================================================================================
 */

#include "interface.h"
#include <unistd.h>

void Matcher::train(const char* trainDataDir,
        const char* featureDataDir, const char* indexPath)
{
    cout << "Train Begin !!!!!!!!" << endl;
    TestTool tool;
    tool.setBeginMoment();
    if (access(indexPath, F_OK) == 0)
    {
        cout << "index already exsit" << endl;
        return;
    }
    
    m_vcTree.training(trainDataDir, featureDataDir);
    ofstream outFile(indexPath);
    m_vcTree.writeVocabularyTree(outFile);
    tool.setEndMoment();
    cout << "training running time = " << tool.getTime() / 1000 << " s" << endl;
}

void Matcher::reTrain(const char* trainDataDir,
        const char* featureDataDir, const char* indexPath)
{
    cout << "reTrain Begin !!!!!!!!" << endl;
    TestTool tool;
    tool.setBeginMoment();
    
    if (access(indexPath, F_OK) == 0)
        if (unlink(indexPath) < 0)
        {
            cout << "unlink error" << endl;
            return;
        }
    m_vcTree.deleteVocabularyTree();
    m_vcTree.training(trainDataDir, featureDataDir);
    ofstream outFile(indexPath);
    m_vcTree.writeVocabularyTree(outFile);
    
    tool.setEndMoment();
    cout << "reTraining running time = " << tool.getTime() / 1000 << " s" << endl;
}

void Matcher::loadIndex(const char* indexPath)
{
    if (access(indexPath, F_OK) == -1)
    {
        cerr << "index not exist" << endl;
        exit(-1);
    }

    ifstream inFile(indexPath);
    m_vcTree.readVocabularyTree(inFile);
}

string Matcher::match(const char* matchSignPath, const char* featureDataDir, 
        const char* trainDataDir)
{
    TestTool tool;
    tool.setBeginMoment();
    vector<pair<string, double> > t_result;
    string ret = m_pMatch.matchPic(matchSignPath, featureDataDir, trainDataDir, m_vcTree);
    tool.setEndMoment();
    cout << "matching running time = " << tool.getTime() << " ms" << endl;

    return ret;
}

//void Matcher::matchDir(const char* matchDir,
//        const char* featureDataDir, const char* trainDataDir)
//{
//    string fileName;
//    list<string> fileNameList;
//    list<string>::iterator iter;
//    //vector<pair<string, double> > result;
//    TestTool tool;
//    tool.getFilenameList(fileNameList, matchDir);
//    tool.setBeginMoment();
//    for (iter = fileNameList.begin(); iter != fileNameList.end();
//            iter++)
//    {
//        fileName = *iter;
//        string ret = match(fileName.c_str(), featureDataDir, trainDataDir);
//        
//        cout << "************************ " << fileName << " ****************************" << endl;
//        cout << "match result : " << endl;
//        //for (int i = 0; i < result.size(); i++)
//        //    cout << result[i].first << "\t" << result[i].second << endl;
//        cout << "ret = " << ret << endl;
//        cout << "********************************************************************************" << endl << endl;
//    }
//    tool.setEndMoment();
//    cout << "total running time = " << tool.getTime() << " ms " << endl;
//    cout << "average running time per image = " << (float)tool.getTime() / fileNameList.size() << " ms" << endl;
//
//}


