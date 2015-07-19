/*
 * =====================================================================================
 *
 *       Filename:  matchpic.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  9/31/13 01:14:15
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  liangyao(ly), liangyao.ndsl@gmail.com
 *        Company:  NDSL UESTC
 *
 * =====================================================================================
 */

#include"matchpic.h"
#include <unistd.h>

int PicMatch::getMatchNum(IpVec &source_ipts,IpVec &match_ipts)
{
    int matchedNum = 0;

    for(int i = 0; i < match_ipts.size(); ++i)
    {
        float d1, d2;
        int index1, index2;

        d1 = d2 = 1024;
        index1 = index2 = 0;

        for(int j = 0; j < source_ipts.size(); ++j)
        {
            float dis = 0;

            for(int k = 0; k < 64; ++k)
            {
                dis += pow(source_ipts.at(j).descriptor[k] - 
                        match_ipts.at(i).descriptor[k], 2);
            }

            dis = sqrt(dis);

            if(dis < d1)
            {
                d2 = d1;
                index2 = index1;
                index1 = j;
                d1 = dis;
            }
            else if(dis < d2)
            {
                d2 = dis;
                index2 = j;
            }
        }

        if(d1 / d2  < 0.75)
            ++ matchedNum;
    }

    return matchedNum;
}


string PicMatch::preMatch(Mat &mat, IpVec &source_ipts, const char* featureDir,
        const char* trainDataDir)
{
    vector<string> cacheVec;
    IpVec match_ipts, *pIpts = NULL; 
    vector<DMatch> good_matches;
    vector<KeyPoint> source_keypoints, match_keypoints;
    string ret = "";

    m_lru.getCache(cacheVec);

    for (int i = 0; i < source_ipts.size(); ++i)
    {
        KeyPoint kp;
        kp.pt.x = source_ipts[i].x;
        kp.pt.y = source_ipts[i].y;
        source_keypoints.push_back(kp);
    }   

    for (int i = 0; i < cacheVec.size(); ++i)
    {
        match_ipts.clear();
        match_keypoints.clear();
        string picName = cacheVec[i];
        pIpts =  m_lru.getWithoutChange(picName);
        assert(pIpts != NULL);
        match_ipts = *pIpts;
        int num = matchFunc(source_ipts, match_ipts, good_matches);

        if (num >= FILTER_NUM)
        {
            for (int j = 0; j < match_ipts.size(); ++j)
            {
                KeyPoint kp;
                kp.pt.x = match_ipts[j].x;
                kp.pt.y = match_ipts[j].y;
                match_keypoints.push_back(kp);
            }
            
            string trainPicPath; 
            trainPicPath += trainDataDir;
            if (trainPicPath[trainPicPath.size() - 1] != '/')
                trainPicPath += '/'; 
            trainPicPath += picName;
            
            vector<Point2f> scene_corners;

            if (drawRectAngle(mat, trainPicPath, source_keypoints, match_keypoints,
                        good_matches, scene_corners) == 1)
            {
                m_lru.set(picName, pIpts);
                ostringstream os;
                for (int k = 0; k < scene_corners.size(); ++k)
                    os << scene_corners[k].x << "&&" << scene_corners[k].y << "&&";
                ret = picName + "&&" + os.str();
                break;
            }
        } 
    }

    return ret;
}


string PicMatch::matchPic(const char* picName, const char* featureDir, 
        const char* trainDataDir, VocabularyTree &vc_tree)
{
    //TestTool tool;
    //tool.setBeginMoment();
    
    int rcount = 0, max_num = 0;
    string ret, feafileName;
    IpVec source_ipts, match_ipts;
    Mat t_mat = imread(picName);
    if (t_mat.rows == 0)
    {
        cerr << picName << " open error" << endl;
        return "";
    }

    Size sz;
    while(t_mat.rows > PIC_SIZE || t_mat.cols > PIC_SIZE)
        pyrDown(t_mat, t_mat, sz, BORDER_DEFAULT);
    IplImage img = t_mat;
    surfDetDes(&img, source_ipts, false, 4, 4, 2, SURF_PARA);
    if (source_ipts.empty())
        return "";

    //tool.setEndMoment();
    //cout << "init running time = " << tool.getTime() << " ms" << endl;
    
    //tool.setBeginMoment();
    ret = preMatch(t_mat, source_ipts, featureDir, trainDataDir);
    if (ret != "")
        return ret;
    
    //tool.setEndMoment();
    //cout << "preMatch running time = " << tool.getTime() << " ms" << endl;
    
    //tool.setBeginMoment(); 
    string hitName = "";
    IpVec good_match_ipts;
    vector<DMatch> good_matches, ret_matches;
    vector<pair<string, double> > m_result; 
    vector<pair<string, double> >::iterator m_iter;
    vector<KeyPoint> source_keypoints, match_keypoints;

    for (int i = 0; i < source_ipts.size(); ++i)
    {
        KeyPoint kp;
        kp.pt.x = source_ipts[i].x;
        kp.pt.y = source_ipts[i].y;
        source_keypoints.push_back(kp);
    }   

    if (vc_tree.matchPic(source_ipts, m_result) == -1)
        return "";

    for (m_iter = m_result.begin(); m_iter != m_result.end() && rcount < RE_SEARCH_NUM; m_iter++, rcount++)
    {
        match_ipts.clear();
        feafileName.clear();

        feafileName += featureDir;
        if (feafileName[feafileName.size() - 1] != '/')
            feafileName += '/'; 
        feafileName += m_iter->first;
        feafileName += ".feature";
        if (access(feafileName.c_str(), F_OK) == -1)
        {
            cout << "can't access file " << feafileName << endl;
            exit(-1);
        }
         
        loadSurf(feafileName.c_str(), match_ipts);
        int num = matchFunc(source_ipts, match_ipts, good_matches);
        if (num >= FILTER_NUM)
        {
            if (num > max_num)
            {
                max_num = num;
                hitName = m_iter->first;
                good_match_ipts = match_ipts;
                ret_matches = good_matches;
            }
        } 
    }

    for (int i = 0; i < good_match_ipts.size(); ++i)
    {
        KeyPoint kp;
        kp.pt.x = good_match_ipts[i].x;
        kp.pt.y = good_match_ipts[i].y;
        match_keypoints.push_back(kp);
    }
   
    IpVec *p_matchIpts = new IpVec;
    p_matchIpts->insert(p_matchIpts->end(), good_match_ipts.begin(),
            good_match_ipts.end());
   
    if (hitName.empty())
        return "";
   
    string trainPicPath; 
    trainPicPath += trainDataDir;
    if (trainPicPath[trainPicPath.size() - 1] != '/')
        trainPicPath += '/'; 
    trainPicPath += hitName;
    vector<Point2f> scene_corners;

    if (drawRectAngle(t_mat, trainPicPath, source_keypoints, match_keypoints,
                ret_matches, scene_corners) == -1)
        return "";

    m_lru.set(hitName, p_matchIpts);
    ostringstream os;
    for (int i = 0; i< scene_corners.size(); ++i)
        os << scene_corners[i].x << "&&" << scene_corners[i].y << "&&";
    ret = hitName + "&&" + os.str();
    //tool.setEndMoment(); 
    //cout << "second stage running time = " << tool.getTime() << " ms" << endl;
    return ret; 
}


int PicMatch::matchFunc(IpVec &source_ipts, IpVec &match_ipts, vector<DMatch> &good_matches)
{    
    good_matches.clear();
    gpu::GpuMat gpu_source, gpu_match;
    int source_size, match_size, num = 0;
    source_size = source_ipts.size();
    match_size = match_ipts.size();
    Mat source_descriptors(source_size, 64, CV_32FC1);
    Mat match_descriptors(match_size, 64, CV_32FC1);
    
    for (int i = 0; i < source_size; ++i)
    {
        for (int j = 0; j < 64; j++)
            source_descriptors.at<float>(i,j) = source_ipts[i].descriptor[j]; 
    }   
    
    for (int i = 0; i < match_size; ++i)
    {
        for (int j = 0; j < 64; j++)
            match_descriptors.at<float>(i,j) = match_ipts[i].descriptor[j];
    }

    vector<vector<DMatch> > matches;
#if USE_GPU
    gpu_source.upload(source_descriptors);
    gpu_match.upload(match_descriptors);
    gpu:: BruteForceMatcher_GPU_base matcher;
    gpu::GpuMat mask;
    matcher.knnMatch(gpu_source, gpu_match, matches, 2, mask);
#else
    BFMatcher matcher(NORM_L2, false);
    matcher.knnMatch(source_descriptors, match_descriptors, matches, 2);
#endif
    for (int i = 0; i < matches.size(); i++)
    {
        const cv::DMatch& bestMatch = matches[i][0];
        const cv::DMatch& betterMatch = matches[i][1];
        float distanceRatio = bestMatch.distance / betterMatch.distance;
        if (distanceRatio < RATIO_THRESHOLD)
        {
            ++num;
            good_matches.push_back(bestMatch);
        }
    }

    return num;
}


int PicMatch::drawRectAngle(Mat &img_scene, const string& trainPicPath, vector<KeyPoint> &keypoints_scene, 
        vector<KeyPoint> &keypoints_object, vector<DMatch>& good_matches, vector<Point2f> &scene_corners)
{
     scene_corners.clear();
     Mat img_object = imread(trainPicPath.c_str(), IMREAD_GRAYSCALE);
     if (!img_object.data)
     {
         cout << "open error " << trainPicPath << endl;
         exit(-1);
     }

     Size sz;
     while(img_object.rows > PIC_SIZE || img_object.cols > PIC_SIZE)
         pyrDown(img_object, img_object, sz, BORDER_DEFAULT);

     std::vector<Point2f> obj,scene;
     for( int i = 0; i < good_matches.size(); i++ )
     {
         obj.push_back( keypoints_object[ good_matches[i].trainIdx].pt);
         scene.push_back( keypoints_scene[ good_matches[i].queryIdx].pt);
     }

     Mat H = findHomography( obj, scene, RANSAC );
   
     std::vector<Point2f> obj_corners(4);
     obj_corners[0] = cvPoint(0,0); 
     obj_corners[1] = cvPoint( img_object.cols, 0 );
     obj_corners[2] = cvPoint( img_object.cols, img_object.rows ); 
     obj_corners[3] = cvPoint( 0, img_object.rows );
     
     scene_corners.resize(4);
     perspectiveTransform( obj_corners, scene_corners, H);
     
     if (scene_corners[2].x - scene_corners[0].x < MIN_LENGTH || scene_corners[3].y - scene_corners[1].y < MIN_LENGTH)
        return -1;
     return 1; 

     ////-- Draw lines between the corners (the mapped object in the scene - image_2 )
     //line( img_matches, scene_corners[0] + Point2f( img_object.cols, 0), scene_corners[1] + Point2f( img_object.cols, 0), Scalar(0, 255, 0), 4 );
     //line( img_matches, scene_corners[1] + Point2f( img_object.cols, 0), scene_corners[2] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
     //line( img_matches, scene_corners[2] + Point2f( img_object.cols, 0), scene_corners[3] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
     //line( img_matches, scene_corners[3] + Point2f( img_object.cols, 0), scene_corners[0] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
     
     //IplImage outImg = img_matches;
     //cvSaveImage("result.jpg", &outImg);
     
     //line(img_scene, scene_corners[0], scene_corners[1],  Scalar(0, 255, 0), 4);
     //line(img_scene, scene_corners[1], scene_corners[2],  Scalar(0, 255, 0), 4);
     //line(img_scene, scene_corners[2], scene_corners[3],  Scalar(0, 255, 0), 4);
     //line(img_scene, scene_corners[3], scene_corners[0],  Scalar(0, 255, 0), 4);
     //
     //IplImage outImg = img_scene;
     //cvSaveImage("result.jpg", &outImg);
     //return 1;
}

