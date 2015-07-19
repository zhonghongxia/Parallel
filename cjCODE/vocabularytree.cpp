/*
 * =====================================================================================
 *
 *       Filename:  vocabularytree.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/31/12 03:36:06
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  liangyao(ly), liangyao.ndsl@gmail.com 
 *        Company:  NDSL UESTC
 *
 * =====================================================================================
 */

#include"vocabularytree.h"

IpVec *VocabularyTree::getIpts(const char* dataDir, 
        const char* featureDir)     
{
    DIR *dp;
    struct dirent *dirp;
    IpVec ipts;
    IpVec* ipts_list = new IpVec;
    memset(ipts_list, 0, sizeof(IpVec));    
    string filename, feafilename;
    Mat mat;
    int count = 0; 

    if ((dp = opendir(dataDir)) == NULL)
    {
        cout << "can't open dir :" << dataDir << endl;
        exit(-1);
    }  

    while ((dirp = readdir(dp)) != NULL)
    {
        filename = dirp->d_name;
        if(strcmp(filename.c_str(),".") == 0 || 
                strcmp(filename.c_str(), "..") == 0)
            continue;

        m_filesIndex.insert(pair<unsigned int, string>
                (count, filename));
        count++;
    }       
    assert(m_filesIndex.size());

    for (int i = 0; i < m_filesIndex.size(); i++)
    {
        filename.clear();
        feafilename.clear();
        ipts.clear();

        filename += dataDir;
        if (filename[filename.size() - 1] != '/')
            filename += '/';
        filename +=	m_filesIndex[i];

        mat = imread(filename.c_str());
        if (mat.rows == 0)
        {
            cerr << filename << " open error" << endl;   
            continue;
        }
         
        if (mat.rows > PIC_SIZE || mat.cols > PIC_SIZE)
        {
            Size sz;
            while(mat.rows > PIC_SIZE || mat.cols > PIC_SIZE)
                pyrDown(mat, mat, sz, BORDER_DEFAULT);
            IplImage outImg = mat;
            unlink(filename.c_str());
            cvSaveImage(filename.c_str(), &outImg);
        }
        
        feafilename += featureDir;
        if (feafilename[feafilename.size() - 1] != '/')
            feafilename += '/';
        feafilename += m_filesIndex[i];
        feafilename += ".feature";

        if (access(feafilename.c_str(), F_OK) == -1)
        {
            IplImage img = mat;
            surfDetDes(&img, ipts, false, 4, 4, 2, SURF_PARA);  
            if (ipts.empty())
                continue;
            saveSurf(feafilename.c_str(),ipts);
        }   
        else
            loadSurf(feafilename.c_str(), ipts); 

        if (!ipts.empty())  
        {
            for (int j = 0; j < ipts.size(); j++)
                ipts[j].frompic = i;
            ipts_list->insert(ipts_list->end(),
                    ipts.begin(), ipts.end()); 
            this->m_picSum ++; 
        }
    }

    closedir(dp);    
    return ipts_list;
}

Re_index VocabularyTree::getIndex(IpVec *ipts_list)   
{
    Hi_gram temp_gram;
    Re_index temp_index;

    Hi_gram::iterator  gram_iter;  
    Re_index::iterator re_iter; 

    for (int i = 0; i < ipts_list->size(); i++)     
    {
       string filename = m_filesIndex[ipts_list->at(i).frompic];
       gram_iter = temp_gram.find(filename);
       
       if(gram_iter == temp_gram.end())   
          temp_gram.insert(pair<string,unsigned int>(filename, 1));
       else       
          gram_iter->second++;
    } 
    
    for (gram_iter = temp_gram.begin(); gram_iter != temp_gram.end(); 
            gram_iter++)  
    {
        int num = gram_iter->second;
        double rel = num * (log((float)m_picSum / temp_gram.size()));
        temp_index.insert(pair<string, double>(gram_iter->first, rel));           
    }
    temp_gram.clear();

    return temp_index;    
}

uint32_t VocabularyTree::generateID()
{
    ++m_nodeID;
    return m_nodeID;
}

vctree VocabularyTree::buildTree(vctree &m_vctree,
        IpVec *&ipts_list, Ipoint &center, int level)  
{
    level--;
    if (ipts_list->size() == 0)
    {
        m_vctree = NULL;
        return m_vctree;
    }   
    m_vctree = new vctree_node();
    m_vctree->id = generateID();
    m_vctree->center = center;

    if (!level || ipts_list->size() <= m_branch) 
    {
        m_vctree->leaf = true;
        m_vctree->reverse_index = getIndex(ipts_list);
        return m_vctree;            
    }
    else
    {
        Kmeans m_kmeans(m_branch);
        m_vctree->child = new vctree[m_branch];
        memset(m_vctree->child, 0, m_branch * sizeof(vctree));
        m_kmeans.run(ipts_list);

        delete ipts_list;
        ipts_list = NULL;

        for (int i = 0; i < m_branch; i++)
            buildTree(m_vctree->child[i], 
                    m_kmeans.m_clusters[i], m_kmeans.m_centers[i], 
                    level);      
    }
}

void VocabularyTree::buildVocabularyTree(IpVec* iptslist)
{
     if(m_vctree != NULL)
     {
         cout << "Build tree error:tree existed already" << endl;
         return;
     }
    
     Ipoint point;
     buildTree(m_vctree, iptslist, point, m_level);
     m_filesIndex.clear();
}

void VocabularyTree::training(const char* dataDir, const char* featureDir) 
{
     if (m_vctree != NULL)
     {
         cout << "vocabularytree existed already" << endl;
         return;
     }

     m_nodeID = 0;
     m_picSum = 0;
     IpVec* iptslist = getIpts(dataDir, featureDir);
     buildVocabularyTree(iptslist);

}

void VocabularyTree::nodeWrite(vctree &m_vctree, ofstream &outfile)
{
   if(!m_vctree)
   {
       cout << "error: tree node is null" << endl;
       return;
   } 
   outfile << m_vctree->id << "\n";
   outfile << m_vctree->leaf<< "\n";
   
   outfile << m_vctree->center.scale << "  ";
   outfile << m_vctree->center.x << "  ";
   outfile << m_vctree->center.y << "  ";
   outfile << m_vctree->center.orientation << "  ";
   outfile << m_vctree->center.laplacian << "  ";

   for (int i = 0; i < 64; i++)
       outfile << m_vctree->center.descriptor[i] << "  ";

   outfile << "\n";
}

void VocabularyTree::nodeRead(vctree &m_vctree,ifstream &infile)
{
   if(!m_vctree)
   {
       cout<<"error: tree node is null"<<endl;
       return;
   }
   infile >> m_vctree->id;
   infile >> m_vctree->leaf;

   infile >> m_vctree->center.scale;
   infile >> m_vctree->center.x;
   infile >> m_vctree->center.y;
   infile >> m_vctree->center.orientation;
   infile >> m_vctree->center.laplacian;

   for(int i = 0;i < 64; i++)
     infile >> m_vctree->center.descriptor[i];  
}

void VocabularyTree::writeTree(vctree &m_vctree,ofstream &outfile)
{
    if(m_vctree == NULL)
    {
        outfile << "##" <<"\n";
        return;
    }
    else if(!m_vctree->leaf)
    {
       outfile << "$$" <<"\n";
       nodeWrite(m_vctree,outfile); 
       
       for(int i = 0;i < m_branch; i++)
          writeTree(m_vctree->child[i], outfile);   
    }
    else
    {
        outfile << "$$" <<"\n";
        nodeWrite(m_vctree,outfile);
        
        outfile << m_vctree->reverse_index.size() <<"\n";
        Re_index::iterator iter;
        for(iter = m_vctree->reverse_index.begin(); 
                iter != m_vctree->reverse_index.end(); 
                iter++)
        {
            outfile << iter->first <<"  ";
            outfile << iter->second << "\n";
        }
    }
}

void VocabularyTree::writeVocabularyTree(ofstream &outfile)
{
    if(m_vctree == NULL)
    {
        cerr << "write tree error::tree null " << endl;
        exit(-1);
    }

    writeTree(m_vctree, outfile);
    outfile.close();
}

void VocabularyTree::readTree(vctree &m_vctree,
        ifstream &infile)
{
    string flag , file_name;
    int len;
    double value; 
    Re_index temp_index;

    infile >> flag;
    if (flag == "##")
    {
        m_vctree = NULL;
        return;
    }
    
    m_vctree = new vctree_node();
    memset(m_vctree, 0, sizeof(vctree_node)); 
    nodeRead(m_vctree, infile);
    
    if (!m_vctree->leaf)
    {
       m_vctree->child = new vctree[m_branch];
       memset(m_vctree->child , 0 , m_branch * sizeof(vctree));
       
       for (int i = 0; i < m_branch; i++)
           readTree(m_vctree->child[i], infile);
    }
    else
    {
        infile >> len; 
        for(int i = 0; i < len; i++)
        {
            infile >> file_name;
            infile >> value;
            temp_index.insert(pair<string, double>(file_name,value));  
        }
        m_vctree->reverse_index = temp_index;
    }
}

void VocabularyTree::readVocabularyTree(ifstream &infile)
{
    if(m_vctree != NULL)
    {
        cerr << "tree exsited already" << endl;
        exit(-1);
    } 

    readTree(m_vctree,infile);
    infile.close();
}

int VocabularyTree::searchTree(Ipoint& point, vctree m_vctree, 
        Re_index &temp_result)    
{
    //list<pair<int, float> > disList;
    //list<pair<int, float> >::iterator iter;
    int index = 0;
    float dis = 0, min_dis = 1000000;
    if (m_vctree == NULL)
        return 1;     
    
    if (!m_vctree->leaf) 
    {
        for(int i = 0; i < m_branch; i++)
        {
            if(m_vctree->child[i] != NULL)
            {
                dis = point - m_vctree->child[i]->center;
                if (dis < min_dis)
                {
                    min_dis = dis;
                    index = i;
                }
                //disList.push_back(pair<int, float>(i, dis)); 
            }
        }
        //disList.sort(PointDisCompare());
        //iter = disList.begin();
        //int index = iter->first; 
        searchTree(point, m_vctree->child[index], temp_result);
    }
    else
    {
        temp_result = m_vctree->reverse_index;  
        return 1;
    }        
}

int VocabularyTree::sortValue(Re_index &t_map , 
        vector<pair<string,double> > &t_vc)  
{
    t_vc.clear();
 
    Re_index::iterator iter;
    for(iter = t_map.begin();iter != t_map.end(); iter++)
    {
        t_vc.push_back(make_pair(iter->first, iter->second));
    }
    sort(t_vc.begin(), t_vc.end(), PairCompare());
    
    return 1;
}

int VocabularyTree::matchPic(IpVec &ipts, vector<pair<string, double> > &result)   
{
    if (ipts.size() == 0 || m_vctree == NULL)
    {
        cout << "matchpic error" << endl;
        return -1;
    }
    Re_index temp_result;
    Re_index temp_sum;
    Re_index::iterator index_iter1, index_iter2;
    Ipoint tm_point;

    for (int i = 0; i < ipts.size(); i++)
    {
        tm_point = ipts[i];
        searchTree(tm_point, m_vctree, temp_result);

        for (index_iter1 = temp_result.begin();
            index_iter1 != temp_result.end(); index_iter1 ++)   
        {
            index_iter2 = temp_sum.find(index_iter1->first);
            if (index_iter2 == temp_sum.end())
                temp_sum.insert(pair<string,double>
                     (index_iter1->first, index_iter1->second));
            else
                index_iter2->second += index_iter1->second; 
        }

        temp_result.clear();
    }
    
    if (temp_sum.empty())
       return -1;

    sortValue(temp_sum, result);

    return 1;
}

void VocabularyTree::preoder(vctree m_vctree) const          
{
    if(!m_vctree)
        return ; 
    
    if(!m_vctree->leaf)
    { 
       for(int i = 0; i < m_branch ; i++)
           preoder(m_vctree->child[i]);
    }
    
    if(m_vctree->leaf)
    {
       cout<<"node id = "<<m_vctree->id<<"\t";
       Re_index::iterator iter;
       for(iter = m_vctree->reverse_index.begin();
               iter != m_vctree->reverse_index.end(); iter++)
       {
           cout<<iter->first<<endl;
           cout<<iter->second<<endl;
       }  
    }
}

void VocabularyTree::deleteTree(vctree& m_vctree)
{
    if (!m_vctree)
        return;

    if (!m_vctree->leaf)
    {
        for(int i = 0; i < m_branch; i++)
            deleteTree(m_vctree->child[i]);
    }
    else
    {
        if (m_vctree->child != NULL)
            delete[] m_vctree->child;
        delete m_vctree;
        m_vctree = NULL;
    }

    if (m_vctree != NULL)
    {
        if (m_vctree->child != NULL)
            delete[] m_vctree->child;
        delete m_vctree;
        m_vctree = NULL;
    }
}

void VocabularyTree::deleteVocabularyTree()
{
    if(m_vctree == NULL)
    {
        //cerr << "delete error: tree NULL" << endl;
        return ;
    }

    deleteTree(m_vctree);
}

VocabularyTree::~VocabularyTree()
{
    deleteVocabularyTree();
}

