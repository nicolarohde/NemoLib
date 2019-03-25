#ifndef GRAPH64_HPP
#define GRAPH64_HPP

#include "Config.hpp"
#include <cstdint>

extern "C" {
#include <cstdio>
}

typedef uint64_t graph64;
typedef uint64_t edge;
typedef uint32_t vertex;
typedef uint16_t edgetype;

const edgetype NOEDGE_UV = 0;
const edgetype DIR_U_T_V = 1;
const edgetype DIR_V_T_U = 2;
const edgetype UNDIR_U_V = 3;
const int16_t INDEG = 0;
const int16_t OUDEG = 1;
const vertex NILLVERTEX = 0xFFFFFFFFUL;


inline edge new_edge(vertex u, vertex v)
{
    return uint64_t(u) << 32 | uint64_t(v);
}

inline edge edge_code(vertex u, vertex v)
{
	return u < v ? (uint64_t(u) << 32 | uint64_t(v)) : uint64_t(v) << 32 | uint64_t(u);
}

inline vertex edge_get_u(edge e)
{
    return vertex(e >> 32);
}

inline vertex edge_get_v(edge e)
{
    return vertex(e & 0xFFFFFFFFULL);
}

inline edgetype reverse(edgetype et)
{
    return (et >> 1) | ((et << 1) & 2);
}

void adj(graph64 g);

inline void DEL(graph64 &g, long row, long col) 
{
	(g &= ~(1ULL << (63-(row*8+col))));
}

inline void SET(graph64 &g, long row, long col) 
{
	(g |=  (1ULL << (63-(row*8+col))));
}

#endif
