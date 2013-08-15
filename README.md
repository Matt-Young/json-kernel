json-kernel
===========

JSON join kernel

This is a follow on to the semantic machine which was a collection of modules hung together for testing.  Now I work on the basicm
JSON join kernel, and hang the other models onto the join interface. My development is simple.  All json expression graphs are accessible via a graph database interface. I can debug the join kernel using a memory graph interface, then graft on my sqlite module for json graph storage.
This project is a lot like the HpergraphDB, built on top of BerkeleyDB, a very good idea. I likely wuill be looking at HypergraphDB for some clues and idea. I am not hurrying this project up.
