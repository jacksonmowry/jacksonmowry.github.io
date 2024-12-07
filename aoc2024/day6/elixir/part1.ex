defmodule Day6 do
  @up {-1, 0}
  @right {0, 1}
  @down {1, 0}
  @left {0, -1}

  defp component_addition(a, b) do
    {al, ar} = a
    {bl, br} = b
    {al + bl, ar + br}
  end

  defp oob(board, pos) do
    {row, col} = pos

    cond do
      row < 0 or row >= length(board) or col < 0 or col >= length(Enum.at(board, 0)) -> true
      true -> false
    end
  end

  def pos(board) do
    row =
      Enum.find_index(board, fn x ->
        Enum.any?(x, fn y -> y === ?^ || y === ?> || y === ?v || y === ?< end)
      end)

    col =
      Enum.find_index(Enum.at(board, row), fn y ->
        y === ?^ || y === ?> || y === ?v || y === ?<
      end)

    {{row, col}, Enum.at(Enum.at(board, row), col)}
  end

  def move(board) do
    {cur_pos, dir} =
      pos(board)

    new_pos =
      case dir do
        ?^ -> component_addition(cur_pos, @up)
        ?> -> component_addition(cur_pos, @right)
        ?v -> component_addition(cur_pos, @down)
        ?< -> component_addition(cur_pos, @left)
      end

    case oob(board, new_pos) do
      true ->
        {:done}

      false ->
        {old_row, old_col} = cur_pos
        {new_row, new_col} = new_pos

        {{new_row, new_col}, new_dir} =
          case Enum.at(Enum.at(board, new_row), new_col) do
            ?\# ->
              case dir do
                ?^ -> {cur_pos, ?>}
                ?> -> {cur_pos, ?v}
                ?v -> {cur_pos, ?<}
                ?< -> {cur_pos, ?^}
              end

            _ ->
              {new_pos, dir}
          end

        board =
          List.update_at(
            board,
            old_row,
            fn _ -> List.update_at(Enum.at(board, old_row), old_col, fn _ -> ?X end) end
          )

        board =
          List.update_at(
            board,
            new_row,
            fn _ -> List.update_at(Enum.at(board, new_row), new_col, fn _ -> new_dir end) end
          )

        {:not_done, board}
    end
  end

  def full_traverse(board) do
    case move(board) do
      {:done} ->
        # must account for the fact that we don't walk off the screen in this implementation
        1 +
          Enum.sum(
            Enum.map(board, fn x ->
              Enum.count(x, fn x -> x === ?X end)
            end)
          )

      {:not_done, new_board} ->
        full_traverse(new_board)
    end
  end
end

IO.read(:stdio, :eof)
|> String.trim()
|> String.split("\n")
|> Enum.map(fn x -> String.to_charlist(x) end)
|> Day6.full_traverse()
|> IO.inspect()
