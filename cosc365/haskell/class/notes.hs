import Text.Printf (printf)

myDrop :: Int -> String -> String
myDrop i s@(_:sx)
  | i <= 0 = s
  | otherwise = myDrop (i-1) sx

main :: IO()
main = do
  line <- getLine
  print $ myDrop 3 line
