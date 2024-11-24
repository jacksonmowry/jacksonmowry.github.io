def indices_of_nonzero (xs: []i32): []i32 =
  let xs_and_is = zip xs (indices xs)
  let xs_and_is' = filter (\(x,_) -> x != 0) xs_and_is
  let (_, is') = unzip xs_and_is'
  in is'
