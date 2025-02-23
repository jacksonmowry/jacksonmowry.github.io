// Jackson Mowry
public class DataReader
{
    public DataReader(string filename)
    {
        List<double> doubles = new List<double>();
        try
        {
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
                                this.doubles = doubles.ToArray();
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
        catch (FileNotFoundException fnfe)
        {
            throw new FileNotFoundException(fnfe.Message);
        }
        catch (EndOfStreamException eose)
        {
            throw new EndOfStreamException(eose.Message);
        }
    }

    public DataReaderEnum GetEnumerator()
    {
        return new DataReaderEnum(ref doubles);
    }

    public double this[int i]
    {
        get
        {
            if (i >= doubles.Length)
            {
                throw new IndexOutOfRangeException();
            }
            return doubles[i];
        }
    }

    public int Count
    {
        get => doubles.Length;
    }

    private double[] doubles;
}
