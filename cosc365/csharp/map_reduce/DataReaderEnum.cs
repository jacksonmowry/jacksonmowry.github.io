public class DataReaderEnum
{
    private readonly double[] values;
    private int itr_pos = -1;

    public DataReaderEnum(double[] vals)
    {
        values = vals;
    }

    public bool MoveNext()
    {
        itr_pos++;
        return itr_pos < values.Length;
    }
    public void Reset()
    {
        itr_pos = -1;
    }

    public double Current {
        get {
            try {
                return values[itr_pos];
            }
            catch (IndexOutOfRangeException) {
                throw new InvalidOperationException();
            }
        }
    }
}
