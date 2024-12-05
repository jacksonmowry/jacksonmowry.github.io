IO.inspect(
  IO.read(:stdio, :eof)
  |> String.trim()
  |> String.split("\n")
  |> Enum.map(fn x -> String.split(x, " ") |> Enum.map(fn y -> String.to_integer(y) end) end)
  |> Enum.map(fn list ->
    Enum.reduce(list, {[], nil}, fn element, {deltas, prev} ->
      case prev do
        nil -> {deltas, element}
        _ -> {deltas ++ [element - prev], element}
      end
    end)
  end)
  |> Enum.map(fn x -> elem(x, 0) end)
  |> Enum.filter(fn deltas -> Enum.all?(deltas, fn x -> x != 0 and abs(x) <= 3 end) end)
  |> Enum.filter(fn deltas ->
    Enum.all?(deltas, fn x -> x < 0 end) or Enum.all?(deltas, fn x -> x > 0 end)
  end)
  |> length()
)
