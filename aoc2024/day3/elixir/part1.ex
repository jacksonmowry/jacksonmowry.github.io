input =
  IO.read(:stdio, :eof)
  |> String.trim()
  |> to_charlist()

IO.inspect(input)

{:ok, tokens, _} = :grammar.string(input)

IO.inspect(
  tokens
  |> Enum.chunk_while(
    [],
    fn element, acc ->
      case element do
        {~c"don't", _} -> {:cont, Enum.reverse(acc), []}
        # {~c"do", _} -> {:cont, [], []}
        _ -> {:cont, [element | acc]}
      end
    end,
    fn
      [] -> {:cont, []}
      acc -> {:cont, Enum.reverse(acc), []}
    end
  )
  |> Enum.map(y, fn chunk ->
    Enum.drop_while(chunk, fn elem -> chunk != Enum.get(y, 0) and elem != {~c"do", _} end)
  end)
  # |> Enum.map(fn x -> Enum.chunk_every(x, 6, 1, :discard) end)
  # |> Enum.flat_map(fn
  #   [] -> []
  #   [head | tail] -> [head] ++ tail
  #   element -> [element]
  # end)
  # |> Enum.map(fn x -> List.to_tuple(x) end)
  # |> Enum.filter(fn x ->
  #   case x do
  #     {{:mul, _}, {:"(", _}, {:int, _, _}, {:",", _}, {:int, _, _}, {:")", _}} ->
  #       true

  #     _ ->
  #       false
  #   end
  # end)
  # |> Enum.reduce(0, fn x, acc ->
  #   {_, _, {:int, _, a}, _, {:int, _, b}, _} = x
  #   acc + String.to_integer(List.to_string(a)) * String.to_integer(List.to_string(b))
  # end)
)
