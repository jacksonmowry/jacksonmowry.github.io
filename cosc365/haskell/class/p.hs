import Text.Printf (printf)
import Data.Char (digitToInt)

powersOfTwo :: [Int]
powersOfTwo = [2^n | n <- [0..]]

accMult :: Int -> (Int, Int) -> Int
accMult acc (x, y) = acc + x * y

convStr :: String -> Int
convStr s = foldl accMult 0 $ zip (map digitToInt $ reverse s) powersOfTwo

main :: IO ()
main = putStrLn "Enter a base 2 value: " >> getLine >>= printf "0x%X\n" . convStr
