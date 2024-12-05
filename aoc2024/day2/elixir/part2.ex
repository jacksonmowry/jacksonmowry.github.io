defmodule MyHelper do
  def test(input) do
    input
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
  end
end

input =
  IO.read(:stdio, :eof)
  |> String.trim()
  |> String.split("\n")
  |> Enum.map(fn x -> String.split(x, " ") |> Enum.map(fn y -> String.to_integer(y) end) end)
  |> Enum.map(fn obs -> Enum.with_index(obs) end)
  |> Enum.map(fn obs ->
    Enum.map(0..length(obs), fn x -> Enum.filter(obs, fn element -> elem(element, 1) != x end) end)
  end)
  |> Enum.map(fn collection ->
    Enum.map(collection, fn line -> Enum.map(line, fn observation -> elem(observation, 0) end) end)
  end)
  |> Enum.map(fn collection -> MyHelper.test(collection) end)
  |> Enum.filter(fn count -> count != 0 end)
  |> length()

IO.inspect(input)
