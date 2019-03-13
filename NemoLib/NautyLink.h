/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   NautyLink.h
 * Author: Wooyoung
 *
 * Created on October 27, 2017, 11:36 AM
 */
#ifndef NAUTYLINK_H
#define NAUTYLINK_H


#ifndef MAXN
#define MAXN 64
#endif


#include "Config.hpp"
#include "graph64.hpp"
#include "Subgraph.h"
#include "nauty/nautinv.h"
#include "nauty/nauty.h"
#include <unordered_map> 


typedef uint64_t graph64; //Nauty label

class NautyLink 
{
public:
	NautyLink(int subgraphsize, std::unordered_map<edge, edgetype> edgeset, bool dir = false) : G_N(subgraphsize), directed(dir), edges(edgeset)
	{
		nautyinit();
	}

    virtual ~NautyLink() = default;
    void nautyinit();
    graph64 nautylabel(Subgraph&);

	int get_G_N()
	{
		return G_N;
	}
    
private:
    bool directed;
    graph64 getLabel(graph*, set*, const int, const int);
    int G_N; // subgraphsize
    int G_M;
    graph canon[MAXN * MAXM]; 
    graph nautyGraph[MAXN * MAXM];
    int lab[MAXN];
    int ptn[MAXN];
    int orbits[MAXN];
    setword workspace[160 * MAXM]; 
    set *gv = NULL;
    

    DEFAULTOPTIONS(options);
    statsblk(stats); 
	std::unordered_map<edge, edgetype> edges; // get the edges when initialize it     
};

#endif /* NAUTYLINK_H */

