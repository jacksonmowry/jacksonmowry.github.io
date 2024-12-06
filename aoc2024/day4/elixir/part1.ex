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

  def diags(matrix) do
    matrix
    |> Enum.chunk_every(4, 1, :discard)
    |> Enum.map(fn four_chunk ->
      Enum.map(four_chunk, fn line -> Enum.chunk_every(line, 4, 1, :discard) end)
    end)
    |> Enum.map(fn four_chunk -> Enum.zip(four_chunk) end)
    |> Enum.map(fn four_chunk ->
      Enum.map(four_chunk, fn tile ->
        [
          Enum.at(elem(tile, 0), 0),
          Enum.at(elem(tile, 1), 1),
          Enum.at(elem(tile, 2), 2),
          Enum.at(elem(tile, 3), 3)
        ]
      end)
    end)
    |> Enum.flat_map(fn group -> group end)
  end
end

input =
  IO.read(:stdio, :eof)
  |> String.trim()
  |> String.split("\n")
  |> Enum.map(fn x -> String.graphemes(x) end)
  |> Day4.rotations()
  |> Enum.map(fn x ->
    Enum.concat(
      Enum.flat_map(Enum.map(x, fn y -> Enum.chunk_every(y, 4, 1, :discard) end), fn group ->
        group
      end),
      Day4.diags(x)
    )
  end)
  |> Enum.flat_map(fn group -> group end)
  |> Enum.filter(fn y -> y === ["X", "M", "A", "S"] end)
  |> length()

IO.inspect(input)
