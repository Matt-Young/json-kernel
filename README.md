json-kernel
===========

JSON join kernel

This is a follow on to the semantic machine which was a collection of modules hung together for testing.  Now I work on the basicm
JSON join kernel, and hang the other models onto the join interface. My development is simple.  All json expression graphs are accessible via a graph database interface. I can debug the join kernel using a memory graph interface, then graft on my sqlite module for json graph storage.
This project is a lot like the HpergraphDB, built on top of BerkeleyDB, a very good idea. I likely will be looking at HypergraphDB for some clues and idea. I am not hurrying this project up.

My main purpose here is to promote Lazy Json, a json format syntax with redundancies removed, match, pass and collect operators added. The kernel has name space capability, and this project does not use json-ld, it simply introduces keywords just like javascript does. That is the plan, when json adds something, I find the same thing in javascript and add it to the kernel.
