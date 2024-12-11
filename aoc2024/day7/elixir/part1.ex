defmodule Day7 do
  defp matching_result?(total, numbers, ops) do
    [head | tail] = numbers

    result =
      Enum.zip(tail, ops)
      |> Enum.reduce(head, fn pair, acc ->
        case pair do
          {a, ?+} -> acc + a
          {b, ?*} -> acc * b
          {c, ?|} -> String.to_integer(Integer.to_string(acc) <> Integer.to_string(c))
        end
      end)

    total === result
  end

  def explore(total, numbers, ops) do
    cond do
      length(ops) === length(numbers) - 1 ->
        case matching_result?(total, numbers, ops) do
          true -> total
          _ -> 0
        end

      true ->
        Enum.map([?+, ?*, ?|], fn op -> explore(total, numbers, ops ++ [op]) end)
        |> Enum.filter(fn x -> x != 0 end)
        |> Enum.uniq()
        |> Enum.filter(fn x -> length(List.wrap(x)) != 0 end)
    end
  end

  def drill_down(arr) do
    case arr do
      [head | _] -> drill_down(head)
      elem -> elem
    end
  end
end

IO.read(:stdio, :eof)
|> String.trim()
|> String.split("\n")
|> Enum.map(fn line ->
  [total | rest] = String.split(line, ": ")

  numbers =
    String.split(hd(rest), " ")
    |> Enum.map(fn x -> String.to_integer(x) end)

  Day7.explore(String.to_integer(total), numbers, [])
end)
|> Enum.filter(fn x -> length(x) != 0 end)
|> Enum.map(fn x -> Day7.drill_down(x) end)
|> Enum.sum()
|> IO.inspect()
