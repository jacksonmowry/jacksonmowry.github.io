defmodule Day4 do
  defp rotate_cw(matrix) do
    transposed = Enum.zip_with(matrix, &Function.identity/1)
    Enum.map(transposed, &Enum.reverse/1)
  end

  def rotations(matrix) do
    [
      matrix,
      rotate_cw(matrix),
      rotate_cw(rotate_cw(matrix)),
      rotate_cw(rotate_cw(rotate_cw(matrix)))
    ]
  end
end

input =
  IO.read(:stdio, :eof)
  |> String.trim()
  |> String.split("\n")
  |> Enum.map(fn x -> String.graphemes(x) end)

# |> Day4.rotations()
# |> Enum.map(fn full -> Enum.map(full, fn x -> Enum.chunk_every(x, 4, 1, :discard) end) end)
# |> Enum.map(fn full ->
#   Enum.map(full, fn x -> Enum.filter(x, fn y -> y === ["X", "M", "A", "S"] end) end)
# end)

IO.inspect(input)
