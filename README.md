# spang
An implementation of gSpan in C++20

## Input format
```
t # <id>
v <id> <label>
...
e <from_id> <to_id> <label>
...
t # <index>
...
```
Takes a list of graphs, with each graph starting with `t #` followed by its ID (an integer). The graph is then followed by its list of vertices (each starting with `v`) and list of edges (each starting with `e`). All IDs and labels must be integers, and may be negative. IDs may be in any order, though it is generally simpler to order them sequentially from 0. Comments may be added by starting a line with "#".

## Output format
```
t # <id> * <support for this subgraph>
v <id> <label>
...
e <from_id> <to_id> <label>
...
[x: <list of input graph IDs that contain this subgraph>]

t # <id> * <support for this subgraph>
...
```
Outputs a list of subgraphs that are frequent, along with their support and, optionally, the list of input graphs that this subgraph occurs in.
