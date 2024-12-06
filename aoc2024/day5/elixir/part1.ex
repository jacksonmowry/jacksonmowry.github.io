defmodule Day5 do
  def to_map(x) do
    x
    |> Enum.map(fn rule -> List.to_tuple(String.split(rule, "|")) end)
    |> Enum.group_by(fn y -> elem(y, 0) end, fn y -> elem(y, 1) end)
  end

  def contains(x, y) do
    x
    |> Enum.any?(fn z -> z == y end)
  end

  def valid(print_order, map) do
    print_order
    |> String.split(",")
    |> Enum.reverse()
    |> Enum.scan([], fn x, acc -> [x | acc] end)
    |> Enum.reverse()
    |> Enum.filter(fn x -> length(x) != 1 end)
    |> Enum.map(fn x ->
      [leading | trailing] = x

      case map[leading] do
        nil ->
          false

        must_trail ->
          Enum.map(trailing, fn y -> contains(must_trail, y) end)
      end
    end)
    |> Enum.map(fn x -> List.wrap(x) end)
    |> Enum.map(fn x -> Enum.all?(x, fn y -> y == true end) end)
    |> Enum.all?(fn x -> x == true end)
  end
end

input =
  IO.read(:stdio, :eof)
  |> String.trim()
  |> String.split("\n")
  |> Enum.split_while(fn x -> x != "" end)

{map, prints} = input

prints =
  prints
  |> Enum.filter(fn x -> x != "" end)

ex_map = Day5.to_map(map)

IO.inspect(
  prints
  |> Enum.filter(fn x -> Day5.valid(x, ex_map) end)
  |> Enum.map(fn x -> String.split(x, ",") end)
  |> Enum.map(fn list -> Enum.map(list, fn x -> String.to_integer(x) end) end)
  |> Enum.reduce(0, fn x, acc -> acc + Enum.at(x, div(length(x), 2)) end)
)
