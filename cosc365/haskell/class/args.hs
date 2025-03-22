import System.Environment (getArgs, getProgName)
import System.IO (hPutChar, stdout)
import Data.List

main :: IO ()
main = do
  args <- getArgs
  case args of
    [x] -> readFile x >>= print . sumFile
    _ -> getProgName >>= printUsage

sumFile :: String -> Int
sumFile d = sum $ map read $ concatMap words (lines d)

myPutStrLn :: String -> IO()
myPutStrLn [] = pure ()
myPutStrLn (c:s) = hPutChar stdout c >> myPutStrLn s

printUsage :: String -> IO ()
printUsage pname = myPutStrLn $ "Usage: " ++ pname ++ " <file>"
