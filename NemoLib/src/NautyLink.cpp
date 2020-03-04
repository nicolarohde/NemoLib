/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

 /*
  * File:   NautyLink.cpp
  * Author: Wooyoung
  *
  * Created on October 27, 2017, 11:36 AM
  */

#include "Global.hpp"
#include "NautyLink.h"

graph64 NautyLink::getLabel(graph* canon, set* gv, const int G_N, const int G_M)
{
	// Fanmod code to retrieve canon data
	graph64 res_gr = 0ULL;
	for (int a = 0; a < G_N; a++)
	{
		gv = GRAPHROW(canon, a, G_M);
		for (int b = 0; b < G_N; b++)
		{
			res_gr <<= 1;
			if (ISELEMENT(gv, b))
			{
				res_gr |= 1;
			}
		}
	}
	return res_gr;
}


void NautyLink::nautyinit()
{
	G_M = (G_N + WORDSIZE - 1) / WORDSIZE;

	options.writeautoms = FALSE;
	options.getcanon = TRUE;

	if (directed)
	{
		options.digraph = TRUE;
		options.invarproc = adjacencies;
		options.mininvarlevel = 1;
		options.maxinvarlevel = 10;
	}

	// Initialize nautyGraph
	for (int i = 0; i < G_N; i++)
	{
		EMPTYSET(GRAPHROW(nautyGraph, i, G_M), G_M);
	}
	nauty_check(WORDSIZE, G_M, G_N, NAUTYVERSIONID);
}


/* For each subgraph, return its cannonical label */
graph64 NautyLink::nautylabel(Subgraph& subgraph)
{
	// get the node(index) list from subgraph
	std::size_t subsize = subgraph.getSize();

	// get the adjacenylists of the node(index) list from subgraph
	// key = node id. value = the adjacencylist of the node (key)

	//go through each pair from nodes and connect if there is an edge
	for (std::size_t i = 0; i < subsize; i++)
	{
		for (std::size_t j = 0; j < subsize; j++)
		{
			if (i != j)
			{
				vertex uc = subgraph.get(i);
				vertex vc = subgraph.get(j);
				edge e_check = edge_code(uc, vc);
				//if there is an edge between nodes[i] and nodes[j], connect i and j 
				if (edges.count(e_check) > 0)
				{
					edgetype ec = edges[e_check];
					if (((uc < vc) && (ec == DIR_U_T_V)) || ((uc > vc) && (ec == DIR_V_T_U)))
					{
						// cout<<"1. connect "<<uc<<" and "<<vc<<endl;
						DELELEMENT(GRAPHROW(nautyGraph, i, G_M), j);
						gv = GRAPHROW(nautyGraph, i, G_M);
						ADDELEMENT(gv, j);
					}
					else if (ec == UNDIR_U_V)
					{
						DELELEMENT(GRAPHROW(nautyGraph, i, G_M), j);
						gv = GRAPHROW(nautyGraph, i, G_M);
						ADDELEMENT(gv, j);
					}
				}
			}
		}
	}
#if _USE_THREAD_POOL
	ESU_Parallel::nauty_mtx.lock();
#endif
	nauty(nautyGraph, lab, ptn, NILSET, orbits, &options, &stats, workspace, 160 * MAXM, G_M, G_N, canon);
#if _USE_THREAD_POOL
	ESU_Parallel::nauty_mtx.unlock();
#endif
	// get the cannonical label 
	graph64 res_gr = getLabel(canon, gv, G_N, G_M);

	// then initialize the graph
	for (int i = 0; i < G_N; i++)
	{
		EMPTYSET(GRAPHROW(nautyGraph, i, G_M), G_M);
	}

	return res_gr;
}

