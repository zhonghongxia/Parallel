/*
 * =====================================================================================
 *
 *       Filename:  mynative.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/27/14 00:45:30
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  liangyao(ly), liangyao.ndsl@gmail.com
 *        Company:  NDSL
 *
 * =====================================================================================
 */

#include "interface.h"
#include "mynative.h"

Matcher ts;

jstring string2jstring(JNIEnv* env,const char* pat)            
{
     jclass strClass = (env)->FindClass("java/lang/String");   
     jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
     jbyteArray bytes = (env)->NewByteArray(strlen(pat));      
     (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*)pat);
     jstring encoding = (env)->NewStringUTF("GB2312");
     return (jstring)(env)->NewObject(strClass, ctorID, bytes, encoding);                                                         
}

char* jstringTostring(JNIEnv* env, jstring jstr)               
{
    char* rtn = NULL;                                          
    jclass clsstring = env->FindClass("java/lang/String");     
    jstring strencode = env->NewStringUTF("utf-8");            
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr= (jbyteArray)env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);                    
    jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);    
    if (alen > 0)
    {
        rtn = (char*)malloc(alen + 1);                       
        memcpy(rtn, ba, alen);                               
        rtn[alen] = 0;                                        
    }                                                         
    env->ReleaseByteArrayElements(barr, ba, 0);               
    return rtn;

} 

JNIEXPORT jint JNICALL Java_org_chudian_mj_bean_Match_train//Java_org_chudian_mj_bean_Match_train
    (JNIEnv * env, jclass arg, jstring trainDir, jstring feaDir, jstring indexPath)
{
    char* strain = jstringTostring(env, trainDir);    
    char* sfea = jstringTostring(env, feaDir);    
    char* sindex = jstringTostring(env, indexPath);
    ts.train(strain, sfea, sindex);
    return 1;
}

JNIEXPORT jint JNICALL Java_org_chudian_mj_bean_Match_reTrain//Java_org_chudian_mj_bean_Match_reTrain
    (JNIEnv * env, jclass arg, jstring trainDir, jstring feaDir, jstring indexPath)
{
    char* strain = jstringTostring(env, trainDir);    
    char* sfea = jstringTostring(env, feaDir);    
    char* sindex = jstringTostring(env, indexPath);
    ts.reTrain(strain, sfea, sindex);
    return 1;
}

    JNIEXPORT jint JNICALL Java_org_chudian_mj_bean_Match_load//Java_org_chudian_mj_bean_Match_load
    (JNIEnv * env, jclass arg, jstring indexPath)
{
    char* sindex = jstringTostring(env, indexPath);
    ts.loadIndex(sindex);
    return 1;
}

JNIEXPORT jstring JNICALL Java_org_chudian_mj_bean_Match_match//Java_org_chudian_mj_bean_Match_match
    (JNIEnv * env, jclass arg, jstring pathname, jstring feaDir, jstring trainDir)
{
    char* spath = jstringTostring(env, pathname);
    char* sfea = jstringTostring(env, feaDir);
    char* strain = jstringTostring(env, trainDir);
    string temp = ts.match(spath, sfea, strain);
    jstring ret = string2jstring(env, temp.c_str()); 
    return ret;
}

//JNIEXPORT jstring JNICALL Java_Match_matchBuf
//    (JNIEnv * env, jclass arg, jstring buf, jint bufSize, jstring feaDir)
//{
//    char* sbuf = jstringTostring(env, buf);
//    char* sfea = jstringTostring(env, feaDir);
//    std::string matchName = ts.tvSignMatchBuf(sbuf, bufSize, sfea);
//    jstring ret = string2jstring(env, matchName.c_str()); 
//    return ret;
//}

JNIEXPORT jint JNICALL Java_org_chudian_mj_bean_Match_matchDir//Java_org_chudian_mj_bean_Match_matchDir
    (JNIEnv * env, jclass arg, jstring matchDir, jstring feaDir, jstring trainDir)
{
    char* sma = jstringTostring(env, matchDir);
    char* sfea = jstringTostring(env, feaDir);
    char* strain = jstringTostring(env, trainDir);
    ts.matchDir(sma, sfea, strain);
    return 1;
}
