-- Jackson Mowry

import Data.Binary
import Data.Bits
import Data.ByteString qualified as B
import Data.Char
import Data.List
import Data.List.Split
import System.Environment
import Text.Printf (printf)

main :: IO ()
main = do
  args <- getArgs
  progName <- getProgName
  let argLen = length args

  if argLen /= 1
    then printf "Usage: %s <binary file>" progName
    else do
      f <- B.readFile (head args)
      putStrLn $ tail (foldl (\acc x -> acc ++ "\n" ++ show x) [] (readBCD f))

readBCD :: B.ByteString -> [Word32]
readBCD bs =
  map decodeBCD (chunksOf 4 (B.unpack bs))

decodeBCD :: [Word8] -> Word32
decodeBCD bytes =
  read (concatMap (\byte -> show ((.&.) byte 0xf0 `shiftR` 4) ++ show ((.&.) byte 0xf)) bytes) :: Word32
