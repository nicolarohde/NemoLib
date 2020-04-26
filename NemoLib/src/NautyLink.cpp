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

#include <cassert>
#include <vector>
#include <cmath>
#include <cstring>
#include <string>

#include "loguru.hpp"

#include "Global.hpp"
#include "NautyLink.hpp"


std::string NautyLink::nautylabel_helper(Subgraph& subgraph)
{
    DLOG_F(INFO, "In nautylabel helper ...");

	// get the node(index) list from subgraph
	std::size_t subsize = subgraph.getSize();
	std::size_t n_chars{0};
	std::vector<char> vect_label;

    std::vector<std::vector<bool>> vect_adj_matrix{subsize, std::vector<bool>(subsize, false)};

	// undirected only needs half the matrix
	if (directed == false)
	{
		n_chars = subsize * (subsize - 1) / 2;
	} // end if
	else
	{
		n_chars = subsize * subsize;
	} // end else

	// pad with 0s to length 6k + 0 
	n_chars += n_chars % 6 == 0 ? 0 : 6 - (n_chars % 6);

	// initialize all characters to 63
	vect_label.resize((n_chars / 6) + 1, static_cast<char>(63));

	// set N(n)
	vect_label[0] = static_cast<char>(63 + subsize);

	// get adjacency for R(x)
	getAdjacency(subgraph, vect_adj_matrix);

	// index currently being processed (0 is N(n) so start at 1)
	std::size_t current_index{1};
	// each 6 bits belong to a character
	int counter = 0;

	for (std::size_t i{0}; i < subsize; i++)
	{
		for (auto j = directed ? std::size_t {0} : std::size_t {i+1}; j < subsize; j++)
		{
			if (true == vect_adj_matrix[i][j])
			{
				vect_label[current_index] += static_cast<char>(static_cast<int>(std::pow(2, 5 - counter)));
			}
			counter += 1;

			if (counter == 6)
			{
				current_index += 1;
				counter = 0;
			} // end if
		} // end for j
	} // end for i
	
	std::string my_label(vect_label.begin(), vect_label.end());

	auto callback = std::packaged_task<std::string(std::string)>(
		[](std::string s)
		{
			return s;
		} // end lambda
	);// end callback

    DLOG_F(DEBUG_LEVEL, "Getting ready to get the label for %s ...", my_label);

	auto my_future = callback.get_future();

	// we have to wrap the callback as it can't be copied
	m_lgp_cannonical_labeler.add_job(my_label, [&](std::string s){callback(s);});

    DLOG_F(DEBUG_LEVEL, "Waiting for label ...");

	my_future.wait();

    DLOG_F(DEBUG_LEVEL, "Labeling done, retrieving future ...");

	std::string str_cannonical_lbl = my_future.get();

    DLOG_F(DEBUG_LEVEL, "G6 label %s has cannonical label %s", my_label, str_cannonical_lbl);

	return str_cannonical_lbl;
}

void NautyLink::getAdjacency(Subgraph& subgraph, std::vector<std::vector<bool>>& matrix)
{
	std::size_t subsize = subgraph.getSize();

	// go through each pair from nodes and connect if there is an edge
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
					if (((uc < vc) && (ec == DIR_U_T_V)) || ((uc > vc) && (ec == DIR_V_T_U)) || ec == UNDIR_U_V)
					{
						matrix[i][j] = true;
					} // end if
				} // end if (edges.count(e_check) > 0)
			} // end if (i != j)
		} // end for j
	} // end for i
} // end method getAdjacency

