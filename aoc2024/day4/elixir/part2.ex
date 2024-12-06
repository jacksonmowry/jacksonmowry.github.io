defmodule Day4 do
  def xmas(tbt) do
    {
      [Enum.at(elem(tbt, 0), 0), Enum.at(elem(tbt, 1), 1), Enum.at(elem(tbt, 2), 2)],
      [Enum.at(elem(tbt, 0), 2), Enum.at(elem(tbt, 1), 1), Enum.at(elem(tbt, 2), 0)]
    }
  end
end

input =
  IO.read(:stdio, :eof)
  |> String.trim()
  |> String.split("\n")
  |> Enum.map(fn x -> String.graphemes(x) end)
  |> Enum.chunk_every(3, 1, :discard)
  |> Enum.map(fn three_chunk ->
    Enum.map(three_chunk, fn line -> Enum.chunk_every(line, 3, 1, :discard) end)
  end)
  |> Enum.map(fn x -> Enum.zip(x) end)
  |> Enum.flat_map(fn x -> x end)
  |> Enum.map(fn x -> Day4.xmas(x) end)
  |> Enum.filter(fn
    {["M", "A", "S"], ["M", "A", "S"]} -> true
    {["M", "A", "S"], ["S", "A", "M"]} -> true
    {["S", "A", "M"], ["S", "A", "M"]} -> true
    {["S", "A", "M"], ["M", "A", "S"]} -> true
    _ -> false
  end)
  |> length()

IO.inspect(input)
