defmodule MyApp.MixProject do
  use Mix.Project

  def project do
    [
      app: :my_app,
      version: "0.1.0",
      elixir: "~> 1.10",
      erlc_paths: ["."],
      start_permanent: Mix.env() == :prod,
      deps: deps(),
      compilers: [:leex, :yecc] ++ Mix.compilers()
    ]
  end

  defp deps do
    []
  end
end
