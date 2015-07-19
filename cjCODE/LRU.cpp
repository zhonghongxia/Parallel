/*
 * =====================================================================================
 *
 *       Filename:  LRU.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/07/2015 06:06:57 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  liangyao(ly), liangyao.ndsl@gmail.com 
 *        Company:  NDSL UESTC  
 *
 * =====================================================================================
 */
#include "LRU.h"

LRU::LRU(int capacity)
{
    assert(capacity > 0);
    m_capacity = capacity;
    m_head = new node();
    m_tail = new node();
    m_head->next = m_tail;
    m_tail->pre = m_head;
}

void LRU::put2Head(node* p)
{
    assert(p != NULL);
    if (p->next && p->pre)
    {
        if (p->pre == m_head)
            return;
        p->pre->next = p->next;
        p->next->pre = p->pre;
    }

    m_head->next->pre = p;
    p->next = m_head->next;
    m_head->next = p;
    p->pre = m_head;
}

IpVec* LRU::get(string key)
{
    m_iter = m_cache.find(key);
    if (m_iter == m_cache.end())
        return NULL;
    node* t_node = m_iter->second;
    put2Head(t_node);
    
    return t_node->value;
}

IpVec* LRU::getWithoutChange(string key)
{
    return m_cache[key]->value;
}

void LRU::set(string key, IpVec* v)
{
    m_iter = m_cache.find(key);
    if (m_iter != m_cache.end())
    {
        node* t_node = m_iter->second;
        put2Head(t_node);
        t_node->value = v; 
    }
    else
    {
        node *new_node = new node(key, v);
        m_cache[key] = new_node;
        put2Head(new_node); 
        
        if (m_cache.size() > m_capacity)
        {
            node* t_node = m_tail->pre;
            t_node->pre->next = m_tail;
            m_tail->pre = t_node->pre;
            m_iter = m_cache.find(t_node->key);
            m_cache.erase(m_iter);
            if (t_node->value != NULL)
            {
                delete t_node->value;
                t_node->value = NULL;
            }
            delete(t_node);
            t_node = NULL;
        }    
    }
}

void LRU::show()
{
    node *t_node = m_head->next;
    for(int i = 0; i < m_cache.size(); ++i)
    {
        cout << t_node->key << "\t";
        t_node = t_node->next;
    }    
    cout << endl;
}

void LRU::getCache(vector<string> &cacheVec)
{
    cacheVec.clear();
    node *t_node = m_head->next;
    for (int i = 0; i < m_cache.size(); ++i)
    {
        cacheVec.push_back(t_node->key);
        t_node = t_node->next;
    }
}

LRU::~LRU()
{
    node *pnode = m_head;
    while(pnode != NULL)
    {
        node *t_node = pnode->next;
        if (pnode->value != NULL)
        {
            delete(pnode->value);
            pnode->value = NULL;
        }
        delete(pnode);
        pnode = NULL;
        pnode = t_node;
    }
}
