public class DataReader
{
    public DataReader(string filename)
    {
        doubles = new List<double>();
        using (FileStream fs = File.Open(filename, FileMode.Open))
        {
            using (BinaryReader reader = new BinaryReader(fs, System.Text.Encoding.ASCII, false))
            {
                while (true)
                {
                    try
                    {
                        double value = reader.ReadDouble();
                        doubles.Add(value);
                    }
                    catch (EndOfStreamException)
                    {
                        if (doubles.Count() > 0)
                        {
                            Count = doubles.Count();
                            return;
                        }
                        else
                        {
                            throw new EndOfStreamException();
                        }
                    }
                }
            }
        }

    }

    public DataReaderEnum GetEnumerator()
    {
        return new DataReaderEnum(doubles.ToArray());
    }

    public readonly int Count;
    private List<double> doubles;
}
