input =
  IO.read(:stdio, :eof)
  |> String.trim()
  |> String.split(~r{\n}, trim: true)
  |> Enum.map(fn x ->
    String.split(x, "   ")
    |> Enum.map(&String.to_integer/1)
  end)
  |> List.zip()
  |> Enum.map(fn x ->
    Tuple.to_list(x)
    |> Enum.sort()
  end)
  |> Enum.zip_reduce(0, fn [a, b], acc -> acc + abs(b - a) end)

IO.inspect(input)
