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

#include <unordered_map>

#include "LabelGProvider.hpp"
#include "Config.hpp"
#include "graph64.hpp"
#include "Subgraph.hpp"
//#include "nautinv.h"
//#include "nauty.h"


typedef uint64_t graph64; //Nauty label

class NautyLink
{
public:
	NautyLink(const std::string& kr_str_LABELG_PATH_, int subgraphsize, std::unordered_map<edge, edgetype> edgeset, bool dir = false) 
     : G_N(subgraphsize), directed(dir), edges(edgeset)
	{
		//nautyinit();
        m_lgp_cannonical_labeler.start_up(kr_str_LABELG_PATH_);
	}

    virtual ~NautyLink() = default;
    void nautyinit();

    void getAdjacency(Subgraph&, char**);

    std::string nautylabel_helper(Subgraph&);

	int get_G_N()
	{
		return G_N;
	}

private:
    LabelGProvider m_lgp_cannonical_labeler;
    bool directed;
    int G_N; // subgraphsize
    int G_M;
    int lab[MAXN];
    int ptn[MAXN];
    int orbits[MAXN];

	std::unordered_map<edge, edgetype> edges; // get the edges when initialize it
};

#endif /* NAUTYLINK_H */
