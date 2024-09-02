## Performance and Data Oriented Design
http://ithare.com/infographics-operation-costs-in-cpu-clock-cycles/


## Andrew Kelley Practical Data Oriented Design
https://vimeo.com/649009599

Modern programs are mostly bottle-necked by memory throughput. If you can find ways to make your data designs have a smaller footprint, you will increase your performance.

Memory footprint reduction strategies
 * Use indexes instead of pointers. Pointers can be 64 bits, but if you know you'll have less than 32 million elements, a 4 byte integer is going to save a lot of space.
   * Watch out for type safety with this kind of approach.
 * Segregate data to reduce data stored. Rather than having a large array of structs with a boolean in each struct. Store 2 arrays, one for each boolean state. The "state" of the boolean is stored once in the array name. Store typed information (bools/enums) "out-of-band" like this when possible. "Struct of arrays".
 * Store sparse data in hashmaps.
 * Encode data when it makes sense to

Summary
 * Add CPU cache to your mental model of computers.
 * The CPU is fast but main memory is slow.
 * Identify where you have many things in memory, and make the size of each thing smaller.
 * Tricks:
   * Indexs instead of pointers
   * Store booleans/enums out of band to eliminate padding (struct of arrays)
   * Store sparse data in hash maps
   * Use "encodings" instead of polymorphism

Recommended Readings/Watchings:
 * Infographics: Operation Costs in CPU Clock Cycles
   * http://ithare.com/infographics-operation-costs-in-cpu-clock-cycles/
 * Mike Acton, Insomniac Games Engine Director, "Data-Oriented Design and C++"
   * https://www.youtube.com/watch?v=rX0ItVEVjHc
 * "Handles are the better pointers" - floooh - FlohOfWoe - Andrew Weissflog
   * https://floooh.github.io/2018/06/17/handles-vs-pointers.html
 * Data-Oriented Design - Richard Fabian
   * https://www.dataorienteddesign.com/dodbook/
 * Enter The Arena: Simplifying Memory Management - Ryan Fleury
   * https://www.youtube.com/watch?v=TZ5a3gCCZYo
