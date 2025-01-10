# zlua
Simple partial lua interpreter implementation to learn zig, lua, and writing lexers/parsers/interpreters/and maybe a full compiler some day.

# Build/Test
 * `zig build`
 * `zig build run`
 * `zig build test`
 * `zig build test --summary all`

## Notes
 * lua ref manual: https://www.lua.org/manual/5.4/manual.html
 * lua grammar: https://www.lua.org/manual/5.4/manual.html#9
 * Expression Parsing:
   * Strategy used in this code base is "Pratt Parsing".
     * Good Pratt Parsing article: https://chidiwilliams.com/posts/on-recursive-descent-and-pratt-parsing
 * Much of the artchitecture of this project is influenced by the book "Writing an Interpreter in Go" and the "aro" c compiler in the zig code base.
 * Language design tips from odin and zig creators as part of a podcast: https://youtu.be/3K8znjWN_Ig?t=7020
   * Components:
     * Recursive descent parser
     * Typed AST
   * Consider making a toy compiler for lisp or pascal
   * Avoid LLVM
   * Bill: Build the exe right away as fast as possible, then adding middleware pieces.
   * Andrew: Consider targeting WASM first, get the front end working first, then tackle the backend/replace.
