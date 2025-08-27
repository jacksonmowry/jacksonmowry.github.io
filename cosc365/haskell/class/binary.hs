import Data.Binary
import Data.Bits
import Data.ByteString.Lazy qualified as BSL

func x y = x + y

main :: IO ()
main =
  map (\c -> fromIntegral :: Word32) $ BSL.unpack $ BSL.pack [10,20,30,40,50, 1]
  map (fromIntegral :: Word8 -> Word32) $ BSL.unpack $ BSL.pack [10,20,30,40,50, 1]
