Definitions.

INT = [0-9]+
WHITESPACE = [\s\t\n\r]
MUL = mul
DONT = don't\(\)
DO = do\(\)

Rules.

{MUL} : {token, {'mul', TokenLine}}.
{INT} : {token, {int, TokenLine, TokenChars}}.
\( : {token, {'(', TokenLine}}.
\) : {token, {')', TokenLine}}.
, : {token, {',', TokenLine}}.
{DONT} : {token, {"don't", TokenLine}}.
{DO} : {token, {"do", TokenLine}}.
{WHITESPACE}+ : {token, {' ', TokenLine}}.
. : {token, {'junk', TokenLine}}.

Erlang code.
% Given a ":name", chop off : and return name as an atom.
to_atom([$:|Chars]) ->
    list_to_atom(Chars).
