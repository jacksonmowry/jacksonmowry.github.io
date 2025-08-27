import Data.Char (toLower, toUpper, isLower, isUpper)

translate :: Char -> Char
translate c
  | isUpper c = toLower c
  | isLower c = toUpper c
  | otherwise = c

mapper :: String -> String
mapper s = map translate s

main :: IO ()
main = do
    fname <- getLine
    fileContents <- readFile fname
    putStrLn $ unlines $ map mapper $ filter (\s -> not (null s)) $ lines fileContents
