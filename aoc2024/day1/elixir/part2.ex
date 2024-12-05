input =
  IO.read(:stdio, :eof)
  |> String.trim()
  |> String.split(~r{\n}, trim: true)
  |> Enum.map(fn x ->
    String.split(x, "   ")
    |> Enum.map(&String.to_integer/1)
  end)
  |> List.zip()
  |> Enum.map(fn x -> Tuple.to_list(x) end)

freqs = Enum.frequencies(Enum.at(input, 1))

IO.inspect(
  Enum.at(input, 0)
  |> Enum.reduce(0, fn x, acc ->
    acc + x * Map.get(freqs, x, 0)
  end)
)
